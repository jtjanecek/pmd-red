#include "global.h"
#include "globaldata.h"
#include "dungeon_cutscene.h"
#include "effect_main.h"
#include "dungeon_vram.h"
#include "code_8041AD0.h"
#include "code_804267C.h"
#include "code_806CD90.h"
#include "constants/bg_music.h"
#include "constants/direction.h"
#include "constants/dungeon_exit.h"
#include "constants/friend_area.h"
#include "constants/weather.h"
#include "dungeon_items.h"
#include "dungeon_range.h"
#include "dungeon_map_access.h"
#include "dungeon_message.h"
#include "dungeon_music.h"
#include "dungeon_misc.h"
#include "dungeon_logic.h"
#include "dungeon_random.h"
#include "dungeon_util.h"
#include "exclusive_pokemon.h"
#include "friend_area.h"
#include "items.h"
#include "pokemon_3.h"
#include "pokemon.h"
#include "position_util.h"
#include "trap.h"
#include "math.h"
#include "dungeon_config.h"
#include "dungeon_boss_dialogue.h"
#include "dungeon_pos_data.h"
#include "dungeon_mon_spawn.h"
#include "dungeon_floor_spawns.h"

extern void sub_8042B0C(Entity *);
extern u8 sub_806FD18(Entity *);
extern void sub_806FDF4(Entity *, Entity *, Entity **);
extern void sub_8049884(void);
extern void sub_8049B8C(void);
extern void sub_8041888(u32);
extern void sub_8052D44(s16 *, Entity *, Entity *);

static void SkarmoryEntry(Entity * skarmoryEntity);
static Entity *EnsureBehaviorEntity(u8 behavior, s16 species, Entity *leader);

void sub_8086B14(void)
{
  Entity * leaderEntity;
  Entity * diglettEntity;
  Entity * skarmoryEntity;

  leaderEntity = CutsceneGetLeader();
  diglettEntity = GetEntityFromMonsterBehavior(BEHAVIOR_DIGLETT);
  skarmoryEntity = GetEntityFromMonsterBehavior(BEHAVIOR_SKARMORY);
  if (!EntityIsValid(diglettEntity))
    diglettEntity = EnsureBehaviorEntity(BEHAVIOR_DIGLETT, MONSTER_DIGLETT, leaderEntity);
  if (!EntityIsValid(skarmoryEntity))
    skarmoryEntity = EnsureBehaviorEntity(BEHAVIOR_SKARMORY, MONSTER_SKARMORY, leaderEntity);
  DungeonStartNewBGM(MUS_IN_THE_DEPTHS_OF_THE_PIT);
  sub_8085374();
  sub_80854D4();
  sub_8085930(DIRECTION_NORTH);
  sub_80855E4(sub_8086A3C);
  if (EntityIsValid(skarmoryEntity))
    sub_8086A3C(skarmoryEntity);
  if (EntityIsValid(diglettEntity))
    GetEntInfo(diglettEntity)->unk15C = 1;
  sub_8085860(leaderEntity->pos.x,leaderEntity->pos.y - 2);
  CopyMonsterNameToBuffer(gFormatBuffer_Monsters[2],MONSTER_DIGLETT);
  CopyMonsterNameToBuffer(gFormatBuffer_Monsters[3], MONSTER_SKARMORY);
}

void sub_8086B94(void)
{
  Entity * leaderEntity;
  Entity * diglettEntity;
  Entity * skarmoryEntity;

  leaderEntity = CutsceneGetLeader();
  diglettEntity = GetEntityFromMonsterBehavior(BEHAVIOR_DIGLETT);
  skarmoryEntity = GetEntityFromMonsterBehavior(BEHAVIOR_SKARMORY);
  if (!EntityIsValid(diglettEntity))
    diglettEntity = EnsureBehaviorEntity(BEHAVIOR_DIGLETT, MONSTER_DIGLETT, leaderEntity);
  if (!EntityIsValid(skarmoryEntity))
    skarmoryEntity = EnsureBehaviorEntity(BEHAVIOR_SKARMORY, MONSTER_SKARMORY, leaderEntity);

  HandleFaint(skarmoryEntity,DUNGEON_EXIT_DELETED_FOR_EVENT,0);
  HandleFaint(diglettEntity,DUNGEON_EXIT_DELETED_FOR_EVENT,0);
  sub_80854D4();
  sub_8085930(DIRECTION_NORTH);
  sub_8085860(leaderEntity->pos.x,leaderEntity->pos.y);
}

void sub_8086BDC(u8 param_1, u8 param_2)
{
    if ((param_2 == 1 || param_2 == 2) && param_1 == 3) {
        sub_8097FA8(1);
        gDungeon->unk2 = 1;
    }
}

void SkarmoryPreFightDialogue(void)
{
  Entity *leaderEntity;
  Entity *partnerEntity;
  Entity * diglettEntity;
  Entity * skarmoryEntity;

  PixelPos pos1;
  PixelPos pos2;

  leaderEntity = CutsceneGetLeader(); // Player
  partnerEntity = CutsceneGetPartner(); // Partner
  diglettEntity = GetEntityFromMonsterBehavior(BEHAVIOR_DIGLETT); // Diglett
  skarmoryEntity = GetEntityFromMonsterBehavior(BEHAVIOR_SKARMORY); // Skarmory
  if (!EntityIsValid(diglettEntity))
    diglettEntity = EnsureBehaviorEntity(BEHAVIOR_DIGLETT, MONSTER_DIGLETT, leaderEntity);
  if (!EntityIsValid(skarmoryEntity))
    skarmoryEntity = EnsureBehaviorEntity(BEHAVIOR_SKARMORY, MONSTER_SKARMORY, leaderEntity);

  if (EntityIsValid(diglettEntity)) {
    pos1.x = diglettEntity->pixelPos.x;
    pos1.y = diglettEntity->pixelPos.y + 0x3000;
  } else {
    pos1.x = leaderEntity->pixelPos.x;
    pos1.y = leaderEntity->pixelPos.y;
  }

  if (EntityIsValid(skarmoryEntity)) {
    pos2.x = skarmoryEntity->pixelPos.x;
    pos2.y = skarmoryEntity->pixelPos.y + 0x2000;
  } else {
    pos2 = leaderEntity->pixelPos;
  }

  sub_8086448();
  sub_803E708(10,0x46);
  if (EntityIsValid(partnerEntity))
    SpriteShockEffect(partnerEntity);
  sub_803E708(0x20,0x46);
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(&gSkarmoryPreFightDialogue_1);
  ShiftCameraToPosition(&pos1,0x40);
  sub_803E708(0x40,0x46);
  ShiftCameraToPosition(&pos2,0x30);
  DisplayDungeonDialogue(gSkarmoryPreFightDialogue_2);
  sub_803E708(10,0x46);
  if (EntityIsValid(diglettEntity))
    GetEntInfo(diglettEntity)->unk15D = 1;
  ShiftCameraToPosition(&pos1,0x30);
  DisplayDungeonDialogue(&gSkarmoryPreFightDialogue_3); // Diglett: ...I...\nI'm scared.
  sub_803E708(10,0x46);
  ShiftCameraToPosition(&pos2,0x20);
  sub_803E708(0x20,0x46);
  SkarmoryEntry(skarmoryEntity);
  DisplayDungeonDialogue(&gSkarmoryPreFightDialogue_4); // Skarmory: You!\nWhat do you think you're doing here?!
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(gSkarmoryPreFightDialogue_5);
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(&gSkarmoryPreFightDialogue_6);
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(gSkarmoryPreFightDialogue_7);
  sub_803E708(10,0x46);
  if (EntityIsValid(skarmoryEntity))
    sub_806CDD4(skarmoryEntity,0xd,DIRECTION_SOUTH);
  DisplayDungeonDialogue(&gSkarmoryPreFightDialogue_8);
  sub_803E708(10,0x46);
  if (EntityIsValid(partnerEntity))
    sub_80869E4(partnerEntity,4,1,DIRECTION_EAST);
  sub_80869E4(leaderEntity,4,2,DIRECTION_WEST);
  DisplayDungeonDialogue(gSkarmoryPreFightDialogue_9);
  if (EntityIsValid(partnerEntity))
    sub_80869E4(partnerEntity,4,2,DIRECTION_NORTH);
  sub_80869E4(leaderEntity,4,1,DIRECTION_NORTH);
  sub_803E708(10,0x46);
  DungeonStartNewBGM(MUS_BOSS_BATTLE);
  ShiftCameraToPosition(&leaderEntity->pixelPos,0x10);
}

// Ensure a behavior-specific entity exists by spawning it near the leader if missing.
static Entity *EnsureBehaviorEntity(u8 behavior, s16 species, Entity *leader)
{
  Entity *entity = GetEntityFromMonsterBehavior(behavior);
  if (EntityIsValid(entity))
    return entity;

  if (!EntityIsValid(leader))
    leader = GetLeader();
  if (!EntityIsValid(leader))
    return NULL;

  // Try adjacent offsets around the leader position
  {
    s32 j = 0;
    DungeonPos pos;
    const Tile *tile;
    struct MonSpawnInfo info;
    while ((pos = gUnknown_80F4598[j]).x != 99) {
      pos.x += leader->pos.x;
      pos.y += leader->pos.y;
      tile = GetTile(pos.x, pos.y);
      if (!sub_807034C(species, tile)) {
        info.species = species;
        info.level = GetSpawnedMonsterLevel(species);
        info.unk2 = behavior;
        info.pos = pos;
        info.unk4 = 0;
        info.unk10 = 0;
        return SpawnWildMon(&info, TRUE);
      }
      j++;
    }
  }
  return NULL;
}

void SkarmoryReFightDialogue(void)
{
  Entity * leaderEntity;
  Entity * skarmoryEntity;
  PixelPos pos;

  leaderEntity = CutsceneGetLeader();
  skarmoryEntity = GetEntityFromMonsterBehavior(BEHAVIOR_SKARMORY);
  pos.x = skarmoryEntity->pixelPos.x;
  pos.y = skarmoryEntity->pixelPos.y + 0x2000;
  sub_8086448();
  sub_803E708(10,0x46);
  SkarmoryEntry(skarmoryEntity);
  ShiftCameraToPosition(&pos,0x10);
  DisplayDungeonDialogue(&gSkarmoryReFightDialogue_1);
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(&gSkarmoryReFightDialogue_2);
  sub_803E708(10,0x46);
  sub_806CDD4(skarmoryEntity,0xd,DIRECTION_SOUTH);
  DisplayDungeonDialogue(&gSkarmoryReFightDialogue_3);
  sub_803E708(10,0x46);
  ShiftCameraToPosition(&leaderEntity->pixelPos,0x10);
  DungeonStartNewBGM(MUS_BOSS_BATTLE);
}

void sub_8086E40(void)
{
  SpriteLookAroundEffect(CutsceneGetLeader());
  sub_803E708(10,0x46);
  DisplayDungeonDialogue(&gUnknown_8100D3C);
  sub_803E708(10,0x46);
  gDungeon->unk2 = 1;
}

static void SkarmoryEntry(Entity * skarmoryEntity)
{
  sub_806CDD4(skarmoryEntity,0xf,DIRECTION_SOUTH);
  sub_8086A54(skarmoryEntity);
  PlaySoundEffect(0x1f8);
  sub_803E708(0x44,0x46);
}
