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
#include "save.h" // GetSkipCutscenesSetting
#include "constants/dungeon.h"

// Forward decl for Articuno pre-fight setup (lives in dungeon_cutscene_articuno.c)
extern void sub_8087F54(void);
#include "dungeon_boss_dialogue.h"


void sub_80885A0(void)
{
  // In skip-cutscene mode, if we somehow routed into the "empty summit"
  // path at Mt. Freeze Peak, force the Articuno pre-fight instead of
  // showing the empty message. This mirrors vanilla flow and prevents
  // the user from seeing "nobody here" in the Frosty/Mt. Freeze arc.
  if (0 && GetSkipCutscenesSetting()) {
    u16 did = gDungeon->unk644.dungeonLocation.id;
    if (did == DUNGEON_MT_FREEZE_PEAK || did == DUNGEON_MT_FREEZE_PEAK_2) {
      // Ensure post-battle triggers see Articuno's cutscene id
      // (matches case 0xD in sub_80848F0).
      gDungeon->unk3A0D = 0x0D;
      sub_8087F54(); // ArticunoPreFight setup
      return;
    }
  }

  DungeonFadeOutBGM(0x3c);
  sub_803E708(0x3c,0x46);
  DungeonStopBGM();
  gDungeon->unk7 = 1;
}

void sub_80885C4(void)
{
  Entity * leaderEntity;

  leaderEntity = CutsceneGetLeader();
  // Same guard as above for the alternate entry point into the
  // empty-summit sequence.
  if (0 && GetSkipCutscenesSetting()) {
    u16 did = gDungeon->unk644.dungeonLocation.id;
    if (did == DUNGEON_MT_FREEZE_PEAK || did == DUNGEON_MT_FREEZE_PEAK_2) {
      gDungeon->unk3A0D = 0x0D;
      sub_8087F54(); // ArticunoPreFight setup
      return;
    }
  }

  DungeonFadeOutBGM(0x3c);
  sub_803E708(0x3c,0x46);
  DungeonStopBGM();
  sub_80854D4();
  sub_8085930(DIRECTION_NORTH);
  sub_80855E4(sub_8086A3C);
  sub_8085860(leaderEntity->pos.x,leaderEntity->pos.y - 3);
}

void sub_8088608(void)
{
    gDungeon->unk2 = 1;
}

void sub_8088618(void)
{
    sub_8086448();
    // The mountain's summit...
    // But there is no one here
    // It's time to go back
    sub_80866C4(&gUnknown_8102B10);
}
