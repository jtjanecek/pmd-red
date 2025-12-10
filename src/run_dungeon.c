#include "global.h"
#include "globaldata.h"
#include "run_dungeon.h"
#include "constants/dungeon.h"
#include "constants/dungeon_exit.h"
#include "constants/fixed_rooms.h"
#include "constants/monster.h"
#include "constants/status.h"
#include "constants/trap.h"
#include "structs/rgb.h"
#include "structs/sprite_oam.h"
#include "bg_control.h"
#include "bg_palette_buffer.h"
#include "code_800558C.h"
#include "graphics_memory.h"
#include "effect_main.h"
#include "effect_data.h"
#include "code_803D110.h"
#include "dungeon_vram.h"
#include "dungeon_tilemap.h"
#include "code_805D8C8.h"
#include "code_8094F88.h"
#include "code_8099360.h"
#include "cpu.h"
#include "dungeon_exit.h"
#include "dungeon_info.h"
#include "dungeon_entity_movement.h"
#include "dungeon_config.h"
#include "dungeon_engine.h"
#include "dungeon_generation.h"
#include "dungeon_floor_spawns.h"
#include "dungeon_main.h"
#include "dungeon_items.h"
#include "dungeon_leveling.h"
#include "dungeon_range.h"
#include "dungeon_map.h"
#include "dungeon_map_access.h"
#include "dungeon_message.h"
#include "dungeon_message_log.h"
#include "dungeon_misc.h"
#include "dungeon_music.h"
#include "dungeon_name_banner.h"
#include "dungeon_floor_spawns.h"
#include "dungeon_pokemon_sprites.h"
#include "dungeon_random.h"
#include "dungeon_strings.h"
#include "dungeon_serializer.h"
#include "dungeon_util.h"
#include "exclusive_pokemon.h"
#include "file_system.h"
#include "moves.h"
#include "pokemon.h"
#include "random.h"
#include "sprite.h"
#include "text_1.h"
#include "text_3.h"
#include "weather.h"
#include "dungeon_data.h"
#include "dungeon_jobs.h"
#include "dungeon_tilemap.h"
#include "dungeon_cleared_window.h"
#include "dungeon_cutscene.h"
#include "dungeon_mon_spawn.h"
#include "mgba_log.h"
#include "dungeon_action_execution.h"
#include "dungeon_8041AD0.h"
#include "ground_main.h"
#include "dungeon_mon_sprite_render.h"
#include "adventure_info.h"
#include "dungeon_seed_overrides.h"
#include "save.h"
#include "rescue_scenario.h"
#include "code_80A26CC.h"
#include "constants/rescue_dungeon_id.h"
#include "type_selection.h"

EWRAM_INIT struct UnkStruct_203B414 *gUnknown_203B414 = NULL;
EWRAM_INIT Dungeon *gDungeon = NULL;
static EWRAM_INIT u8 *gSerializedData_203B41C = NULL;

extern void sub_8068BDC(u8 r0);
extern void sub_803D4AC(void);
extern void sub_8068F28(void);
extern void sub_8043D60(void);
extern void sub_80840A4(void);
extern void sub_806AB2C(void);
extern void sub_807E5AC(void);
extern void nullsub_16(void);
extern struct unkStruct_203B0CC *gUnknown_203B0CC;

static u32 GetDungeonSeededRng(void)
{
    s32 seed = sub_8011C34();

    if (seed == -1 || gDungeon == NULL || gDungeon->unk644.dungeonLocation.id > DUNGEON_PURITY_FOREST)
        return YetAnotherRandom24();
    return DungeonSeedOverrides_GetDungeonRngSeed(seed, gDungeon->unk644.dungeonLocation.id, gDungeon->unk644.dungeonLocation.floor);
}
extern void sub_80521D0(void);
extern void sub_8068A84(Pokemon *pokemon);
extern void sub_806AA70(void);
extern void ReevaluateSnatchMonster(void);
extern void sub_8051E3C(void);
extern void sub_807FA18(void);
extern void sub_806A974(void);
extern void sub_8068F80(void);
extern void sub_806A914(bool8 a0, bool8 a1, bool8 showRunAwayEffect);
extern void EnforceMaxItemsAndMoney(void);

extern OpenedFile *gDungeonNameBannerPalette;

// These functions are not part of dungeon's overlay5 and connect, in a way, overworld with dungeon.

static const s16 sDeoxysForms[4] = {MONSTER_DEOXYS_NORMAL, MONSTER_DEOXYS_ATTACK, MONSTER_DEOXYS_DEFENSE, MONSTER_DEOXYS_SPEED};

static void sub_8043CD8(void);

// Actual function in Sky. Macro instead of static inline for matching
#define GetForcedLossReason()(gDungeon->unk10)

// This functions is the main 'loop' when the player is in a Dungeon. It runs from the moment the player enters a dungeon, until they leave(by completing or by fainting).
// arm9.bin::0206A848
void RunDungeon_Async(DungeonSetupStruct *setupPtr)
{
    bool8 check;
    Entity *leader;
    u8 *dungeonPtr;
    s32 i;
    bool8 r6;
    bool8 r9;
    bool8 r10;
    u8 sp;
    RGB color;

    gUnknown_203B40C = 0;
    MGBA_Warnf("[demo] Enter dungeon id=%d seed=%d", setupPtr->info.sub0.unk0.id, setupPtr->info.dungeonSeed);
    MGBA_Warnf("[Dungeon] Setup: quicksave=%d unkD=%d unkC=%d canRecruit=%d hasInventory=%d unkB=%d unk5=%d unk8=%d unk6=%d unk7=%d unk9=%d unkA=%d",
               setupPtr->info.sub0.unk4,
               setupPtr->info.sub0.unkD,
               setupPtr->info.sub0.unkC,
               setupPtr->info.sub0.unk6,
               setupPtr->info.sub0.unk9,
               setupPtr->info.sub0.unkB,
               setupPtr->info.sub0.unk5,
               setupPtr->info.sub0.unk8,
               setupPtr->info.sub0.unk6,
               setupPtr->info.sub0.unk7,
               setupPtr->info.sub0.unk9,
               setupPtr->info.sub0.unkA);
    r6 = setupPtr->info.sub0.unk4;
    r9 = setupPtr->info.sub0.unkD;
    r10 = setupPtr->info.sub0.unkC;
    MGBA_Warnf("[Dungeon] Setup parameters loaded");
    gSerializedData_203B41C = setupPtr->info.unk74;
    gDungeon = setupPtr->info.dungeon;
    if (!r6) {
        *gSerializedData_203B41C = 0;
    }

    // Why not use memset?
    dungeonPtr = (u8 *)(gDungeon);
    for (i = 0; i < sizeof(Dungeon); i++) {
        dungeonPtr[i] = 0;
    }
    MGBA_Warnf("[Dungeon] Memory cleared");

    gPlayerDotMapPosition.x = 0; // Needed to match
    gPlayerDotMapPosition.x = 100;

    if (!r6) {
        gDungeon->unk644.unk34 = setupPtr->info.sub0.unkB;
        gDungeon->unk644.dungeonSeed = setupPtr->info.dungeonSeed;
        gDungeon->unk644.windTurns = GetTurnLimit(setupPtr->info.sub0.unk0.id);
        gDungeon->unk644.windPhase = 0;
        gDungeon->unk644.unk37 = GetRescuesAllowed(setupPtr->info.sub0.unk0.id);
        TypeSelection_HandleDungeonStart();
    }
    gDungeon->unk644.unk54 = 0;
    gDungeon->unk644.unk55 = 0;
    gDungeon->unk644.unk18 = setupPtr->info.sub0.unk5;
    gDungeon->unk644.unk16 = setupPtr->info.sub0.unk8;
    gDungeon->unk644.canRecruit = setupPtr->info.sub0.unk6;
    gDungeon->unk644.unk15 = setupPtr->info.sub0.unk7;
    gDungeon->unk644.hasInventory = setupPtr->info.sub0.unk9;
    // Enable Toolbox at start of Tiny Woods for testing so we can pick up many items
    if (setupPtr->info.sub0.unk0.id == DUNGEON_TINY_WOODS) {
        gDungeon->unk644.hasInventory = TRUE;
    }
    gDungeon->unk644.unk19 = setupPtr->info.sub0.unkA;
    MGBA_Warnf("[Dungeon] Dungeon struct initialized");
    StopDungeonBGM();
    MGBA_Warnf("[Dungeon] BGM stopped");
    sub_803D4AC();
    MGBA_Warnf("[Dungeon] sub_803D4AC complete");
    MGBA_Warnf("[Dungeon] Calling sub_804513C");
    sub_804513C();
    MGBA_Warnf("[Dungeon] Calling sub_8043CD8");
    sub_8043CD8();
    MGBA_Warnf("[Dungeon] Calling sub_80495E4");
    sub_80495E4();
    MGBA_Warnf("[Dungeon] Calling sub_803E250");
    sub_803E250();
    MGBA_Warnf("[Dungeon] Opening dungeon map file");
    OpenDungeonMapFile();
    MGBA_Warnf("[Dungeon] Setting map to not shown");
    SetDungeonMapToNotShown();
    MGBA_Warnf("[Dungeon] Calling sub_803F27C");
    sub_803F27C(1);
    MGBA_Warnf("[Dungeon] sub_803F27C returned, setting globals");
    gUnknown_2026E4E = 2056;
    MGBA_Warnf("[Dungeon] Calling sub_80095CC");
    sub_80095CC(1, 0x14);
    MGBA_Warnf("[Dungeon] Freeing previous effect context before dungeon");
    MGBA_Warnf("[Dungeon] gUnknown_203B0CC = %p", gUnknown_203B0CC);
    if (gUnknown_203B0CC != NULL) {
        MGBA_Warnf("[Dungeon] Calling sub_800DB7C to free effect context");
        sub_800DB7C();
        MGBA_Warnf("[Dungeon] sub_800DB7C completed, gUnknown_203B0CC = %p", gUnknown_203B0CC);
    } else {
        MGBA_Warnf("[Dungeon] gUnknown_203B0CC is NULL, nothing to free");
    }
    MGBA_Warnf("[Dungeon] Calling sub_800DAC0");
    sub_800DAC0(0);
    MGBA_Warnf("[Dungeon] Calling UpdateFadeInTile");
    UpdateFadeInTile(1);
    MGBA_Warnf("[Dungeon] Calling sub_803DF60");
    sub_803DF60();
    MGBA_Warnf("[Dungeon] Calling sub_803E02C");
    sub_803E02C();
    MGBA_Warnf("[Dungeon] Calling sub_8042E98");
    sub_8042E98();
    gUnknown_202F32C = 0;
    if (r6) {
        ReadDungeonState(gSerializedData_203B41C, 0x4800);
        sub_8049840();
    }
    if (r9) {
        sub_8043D60();
    }

    if (!r6) {
        gDungeon->unk181e8.allTilesRevealed = 1;
        gDungeon->unk181e8.unk1820C = 1;
        if (gDungeon->unk644.unk34 == 1) {
            gDungeon->unk644.dungeonLocation.id = setupPtr->info.dungeonSeed.location.id;
            gDungeon->unk644.dungeonLocation.floor = 1;
        }
        else {
            gDungeon->unk644.dungeonLocation = setupPtr->info.sub0.unk0;
        }

        gDungeon->unk644.unk30 = 0;
        EnforceMaxItemsAndMoney();
    }
    if (!r6) {
        if (gDungeon->unk644.unk34 == 1) {
            gDungeon->unk644.unk38 = setupPtr->info.dungeonSeed.seed;
        }
        else {
            gDungeon->unk644.unk38 = Rand32Bit() & 0xFFFFFF;
        }
        sub_808408C(gDungeon->unk644.unk38);
    }
    if (!r6) {
        if (!sub_80980A4() && gDungeon->unk644.dungeonLocation.id == DUNGEON_TINY_WOODS) {
            sub_8043FD0();
        }
        SetDungeonMonsFromTeam();
    }

    if (r9) {
        gFormatArgs[0] = gDungeon->unk644.unk37;
        if (gFormatArgs[0] != 0) {
            DisplayDungeonMessage(0, gUnknown_80FEC48, 1);
        }
        else {
            DisplayDungeonMessage(0, gUnknown_80FEC7C, 1);
        }
    }

    if (r10) {
        setupPtr->info.mon.heldItem.id = 0;
        // Do not reset leader/guest to Level 1 on entry
        if (FALSE && IsLevelResetDungeon(gDungeon->unk644.dungeonLocation.id)) {
            sub_808D0D8(&setupPtr->info.mon);
        }
        sub_8068A84(&setupPtr->info.mon);
        if (r6) {
            sub_806B404();
        }
    }

    OpenDungeonPaletteFile();
    if (!r6 && gDungeon->unk644.unk34 == 1) {
        if (sub_8099394(&sp)) {
            unkStruct_203B480 *mailStr = GetMailatIndex(sp);
            if (mailStr->rescuesAllowed) {
                gFormatArgs[0] = mailStr->rescuesAllowed;
                DisplayDungeonMessage(0, gUnknown_81002B8, 1);
            }
            else {
                DisplayDungeonMessage(0, gPtrFinalChanceMessage, 1);
            }
        }
    }

    while (TRUE) {
        sub_8098080();
        nullsub_16();
        sub_80521D0();
        ResetMessageLog();
        InitDungeonPokemonSprites();
        if (!r6) {
            sub_804513C();
        }
        gLeaderPointer = NULL;
        gDungeon->unk0 = 0;
        if (!r6) {
            gDungeon->unk644.unk3C = GetDungeonSeededRng();
            gDungeon->unk644.unk24 = 10;
            InitDungeonRNG(gDungeon->unk644.unk3C);
        }
        gDungeon->monsterSpawnsPopulated = FALSE;
        if (!r6) {
            s32 rnd;

            gDungeon->decoyIsActive = FALSE;
            rnd = DungeonRandInt(ARRAY_COUNT(sDeoxysForms));
            gDungeon->unk37FD = 0;
            gDungeon->deoxysDefeat = FALSE;
            gDungeon->deoxysForm = sDeoxysForms[rnd];
            gDungeon->unk37FF = 0;
            gDungeon->unk644.unk31 = 0;
        }
        SetFloorItemMonsterSpawns();
        gDungeon->unk1 = 0;
        gDungeon->unk10 = 0;
        gDungeon->unk2 = 0;
        gDungeon->unk4 = 0;
        gDungeon->unk11 = 0;
        gDungeon->unk8 = 0;
        gDungeon->unk3 = 0;
        gDungeon->unk6 = 0;
        gDungeon->noActionInProgress = FALSE;
        gDungeon->unk5C0 = -1;
        gDungeon->unk7 = 0;
        gDungeon->unk9 = 0;
        gDungeon->unkA = 0;
        gDungeon->unkB = 1;
        gDungeon->unkD = 1;
        gDungeon->unkE = 0;
        gDungeon->unk1BDD4.unk1C05E = 0;
        if (!r6) {
            gDungeon->unk644.emptyBellyAlert = 0;
            gDungeon->unk644.unk48 = 0;
            gDungeon->unk644.unk4C = 0;
            gDungeon->unk644.unk50 = 0;
            gDungeon->unk644.fractionalTurn = 0;
            gDungeon->unk644.wildMonSpawnFrames = 0;
            gDungeon->unk644.stoleFromKecleon = 0;
            gDungeon->unk644.unk2B = 0;
            gDungeon->unk644.unk2C = 0;
            gDungeon->unk644.itemHoldersIdentified = 0;
            gDungeon->unk644.monsterHouseTriggered = 0;
            gDungeon->unk644.monsterHouseTriggeredEvent = 0;
            gDungeon->unk644.bossSongIndex = 999;
            gDungeon->unk644.unk44 = 0;
            gDungeon->unk644.unk46 = 0;
            gDungeon->unk644.unk40 = 99;
            gDungeon->unk644.unk42 = 99;
            gDungeon->weather.weather = 0;
            gDungeon->tileset = gDungeon->floorProperties.tileset;
            gDungeon->unk3A10 = gDungeon->floorProperties.bgMusic;
            gDungeon->fixedRoomNumber = gDungeon->floorProperties.fixedRoomNumber;
            sub_807E5E4(0);
            sub_80842F0();
        }
        SetCurrentMonsterSpawns();
        LoadDungeonPokemonSprites();
        if (!r6) {
            sub_80687AC();
        }
        else {
            LoadDungeonActivePokemonSprites();
            sub_8082B40();
        }
        sub_806C42C();
        sub_806AD3C();

        if (!r6) {
            DungeonStartNewBGM(gDungeonMusic[gDungeon->unk3A10]);
            sub_80847D4();
        }
        sub_8049840();
        sub_803E178();
        gDungeonBrightness = 0;
        SetDungeonMapToNotShown();
        sub_803EAF0(4, NULL);
        sub_8052210(0);
        sub_803F27C(r6);
        ShowDungeonNameBanner_Async();

        if (!r6) {
            MGBA_Warnf("[Dungeon] Before GenerateFloor");
            GenerateFloor();
            MGBA_Warnf("[Dungeon] After GenerateFloor shop=%d mh=%d tileset=%d", gDungeon->unk3A0A, gDungeon->forceMonsterHouse, gDungeon->tileset);
            // Junction T1 initialization removed - using A* pathfinding instead
            gDungeon->unk644.windTurns = GetTurnLimit(gDungeon->unk644.dungeonLocation.id);
            gDungeon->unk644.windPhase = 0;
        }
        MGBA_Warnf("[Dungeon] Before sub_804AAD4");
        sub_804AAD4();
        MGBA_Warnf("[Dungeon] After sub_804AAD4");
        sub_8049B8C();
        MGBA_Warnf("[Dungeon] After sub_8049B8C");
        MGBA_Warnf("[Dungeon] Calling LoadDungeonTilesetAssets (tileset=%d)", gDungeon->tileset);
        LoadDungeonTilesetAssets();
        MGBA_Warnf("[Dungeon] LoadDungeonTilesetAssets complete");
        if (!r6) {
            MGBA_Warnf("[Dungeon] Calling sub_806B168 to spawn player team");
            sub_806B168();
            MGBA_Warnf("[Dungeon] After sub_806B168");
            {
                const BossFightConfig *bossFight = DungeonFloorSpawns_GetBossFightConfig();

                // Reset any lingering boss pointer from previous floors
                DungeonSeedOverrides_RegisterBossEntity(NULL);

                // Check if this floor has a boss fight
                if (bossFight != NULL && bossFight->enabled) {
                    // STEP 2: Populate gDungeon->unk57C array with boss
                    // sub_806C3C0() will spawn it using the working mechanism
                    MGBA_Warnf("[Dungeon] About to call SpawnBossFightEntities");
                    SpawnBossFightEntities((BossFightConfig*)bossFight);
                    MGBA_Warnf("[Dungeon] SpawnBossFightEntities returned successfully");
                }

                // This will spawn any entities in gDungeon->unk57C (including our boss!)
                MGBA_Warnf("[Dungeon] About to call sub_806C3C0 to spawn entities");
                MGBA_Warnf("[Dungeon] unk57C.unk40 = %d", gDungeon->unk57C.unk40);
                sub_806C3C0();
                MGBA_Warnf("[Dungeon] sub_806C3C0 returned successfully");

                // STEP 3: Apply boss HP/music override after spawning
                if (bossFight != NULL && bossFight->enabled) {
                    MGBA_Warnf("[Dungeon] About to call ApplyBossFightOverrides");
                    ApplyBossFightOverrides((BossFightConfig*)bossFight);
                    MGBA_Warnf("[Dungeon] ApplyBossFightOverrides returned successfully");
                }

                // Spawn normal enemies only on non-boss floors
                if (bossFight == NULL || !bossFight->enabled) {
                    MGBA_Warnf("[Dungeon] Before SpawnWildMonsOnFloor");
                    // Normal floor - spawn regular enemies
                    SpawnWildMonsOnFloor();
                    MGBA_Warnf("[Dungeon] After SpawnWildMonsOnFloor");
                } else {
                    MGBA_Warnf("[Dungeon] Skipping SpawnWildMonsOnFloor (boss floor)");
                }

                MGBA_Warnf("[Dungeon] Boss spawn section complete");
            }
        }
        else {
            MGBA_Warnf("[Dungeon] Calling sub_806B678");
            sub_806B678();
            MGBA_Warnf("[Dungeon] sub_806B678 returned");
        }

        MGBA_Warnf("[Dungeon] Setting floor state variables");
        gDungeon->lightningRodPokemon = NULL;
        gDungeon->unk17B38 = 0;
        gDungeon->snatchPokemon = NULL;
        gDungeon->unk17B3C = 0;
        gDungeon->illuminatePokemon = NULL;
        gDungeon->illuminateMonSpawnGenID = 0;
        MGBA_Warnf("[Dungeon] Floor state variables set");

        if (!r6) {
            MGBA_Warnf("[Dungeon] Calling sub_807FA18");
            sub_807FA18();
            MGBA_Warnf("[Dungeon] Calling CreateFloorItems");
            CreateFloorItems();
            MGBA_Warnf("[Dungeon] Setting unk644 values");
            gDungeon->unk644.unk50 = gDungeon->unk644.unk48;
            gDungeon->unk644.unk4C = 0;
            MGBA_Warnf("[Dungeon] Calling sub_8051E3C");
            sub_8051E3C();
            MGBA_Warnf("[Dungeon] Calling sub_804AAAC");
            sub_804AAAC();
            MGBA_Warnf("[Dungeon] Floor initialization complete");
        }
        else {
            ReevaluateSnatchMonster();
        }
        sub_8068F80();
        sub_8049884();
        UpdateTrapsVisibility();

        if (!r6) {
            sub_806A914(TRUE, FALSE, FALSE);
        }
        else {
            DetermineAllMonsterShadow();
            sub_806A974();
        }
        sub_8041888(1);

#ifdef DEV
        // DEV: Apply permanent Protect status to the leader
        {
            Entity *leader = GetLeader();
            if (EntityIsValid(leader)) {
                EntityInfo *leaderInfo = GetEntInfo(leader);
                leaderInfo->reflectClassStatus.status = STATUS_PROTECT;
                leaderInfo->reflectClassStatus.turns = 255; // Max u8 value (won't decrement anyway due to mod above)
                // Note: Status sprite will update naturally on first turn
            }
        }
#endif

        MGBA_Warnf("[Dungeon] Post-init: About to call sub_80848F0");
        if (!r6) {
            sub_80848F0();
            IncrementAdventureFloorsExplored();
        }

        MGBA_Warnf("[Dungeon] Post-init: Setting gUnknown_203B40C");
        gUnknown_203B40C = 1;
        if (r6) {
            sub_807E88C();
            sub_806AB2C();
        }

        // CRITICAL FIX: Move leader entity to new spawn position before rendering
        // Fixed rooms set playerSpawn, but the leader entity hasn't been moved yet
        // This causes crashes in rendering code that assumes valid entity positions
        MGBA_Warnf("[Dungeon] Post-init: Repositioning leader to spawn point");
        {
            Entity *leader = GetLeader();
            MGBA_Warnf("[Dungeon] Leader entity: ptr=%p valid=%d", leader, EntityIsValid(leader));
            if (EntityIsValid(leader)) {
                MGBA_Warnf("[Dungeon] Moving leader from (%d,%d) to (%d,%d)",
                           leader->pos.x, leader->pos.y,
                           gDungeon->playerSpawn.x, gDungeon->playerSpawn.y);
                leader->pos.x = gDungeon->playerSpawn.x;
                leader->pos.y = gDungeon->playerSpawn.y;
                leader->pixelPos.x = X_POS_TO_PIXELPOS(leader->pos.x);
                leader->pixelPos.y = Y_POS_TO_PIXELPOS(leader->pos.y);
            } else {
                MGBA_Warnf("[Dungeon] Leader not valid, cannot reposition!");
            }
        }

        MGBA_Warnf("[Dungeon] Post-init: About to call sub_803E748/sub_803E7C8");

        // Normal fade-in animation for all floors
        if (gDungeon->unk7 == 0) {
            sub_803E748();
        }
        else {
            sub_803E7C8();
        }
        MGBA_Warnf("[Dungeon] Post-init: About to call sub_8040094");
        sub_8040094(0);
        MGBA_Warnf("[Dungeon] Post-init: About to call sub_803EAF0");
        sub_803EAF0(0, NULL);
        MGBA_Warnf("[Dungeon] Post-init: About to call InitDungeonMap");
        InitDungeonMap(r6);
        MGBA_Warnf("[Dungeon] Post-init: About to call UpdateMinimap");
        UpdateMinimap();
        MGBA_Warnf("[Dungeon] Post-init: Setting dungeon variables");
        gDungeon->unkB8 = NULL;
        gDungeon->unk644.unk28 = 0;
        gDungeon->unk644.unk29 = 0;
        gDungeon->unk12 = 99;
        gDungeon->unk0 = 1;

        MGBA_Warnf("[Dungeon] Post-init: Tutorial/dest floor messages");
        if (!r6) {
            TryDisplayGeneralTutorialMessage();
            if (gDungeon->unk9 != 0) {
                gDungeon->unk9 = 0;
                sub_8083D68();
                DisplayYouReachedDestFloorStr();
            }
        }
        MGBA_Warnf("[Dungeon] Post-init: Setting leader pointer");
        gLeaderPointer = NULL;
        gDungeon->unk5 = 0;
        if (!r6) {
            MGBA_Warnf("[Dungeon] Post-init: About to call DisplayPreFightDialogue");
            DisplayPreFightDialogue();
            MGBA_Warnf("[Dungeon] Post-init: DisplayPreFightDialogue complete, unk4=%d unk2=%d", gDungeon->unk4, gDungeon->unk2);
            if (gDungeon->unk4 != 0 || gDungeon->unk2 != 0) {
                MGBA_Warnf("[Dungeon] Post-init: Setting unk5=1");
                gDungeon->unk5 = 1;
            }
            else {
                MGBA_Warnf("[Dungeon] Post-init: Calling sub_803F4A0");
                sub_803F4A0(GetLeader());
                MGBA_Warnf("[Dungeon] Post-init: sub_803F4A0 complete");
                MGBA_Warnf("[Dungeon] Post-init: Calling UpdateMinimap");
                UpdateMinimap();
                MGBA_Warnf("[Dungeon] Post-init: UpdateMinimap complete");
            }

            {
                Entity *leader = GetLeader();
                MGBA_Warnf("[Dungeon] Leader check: valid=%d pos=(%d,%d)",
                           EntityIsValid(leader),
                           leader ? leader->pos.x : -1,
                           leader ? leader->pos.y : -1);
            }
        }

        MGBA_Warnf("[Dungeon] Post-leader-check: Weather setup");
        if (!r6) {
            if (gDungeon->unk5 == 0) {
                MGBA_Warnf("[Dungeon] Calling sub_807E5AC");
                sub_807E5AC();

                if (GetApparentWeather(NULL) != 0) {
                    MGBA_Warnf("[Dungeon] Calling sub_807E7FC");
                    sub_807E7FC(1);
                }
            }
        }
        else {
            MGBA_Warnf("[Dungeon] Calling TryActivateArtificialWeatherAbilities");
            TryActivateArtificialWeatherAbilities();
        }

        MGBA_Warnf("[Dungeon] Post-weather: Pre-game-loop setup");
        if (r6) {
            r6 = FALSE;
        }
        else {
            MGBA_Warnf("[Dungeon] Calling sub_80427AC");
            sub_80427AC();
            MGBA_Warnf("[Dungeon] Calling TryTriggerMonsterHouseWithMsg");
            TryTriggerMonsterHouseWithMsg(GetLeader(), gDungeon->forceMonsterHouse);
            MGBA_Warnf("[Dungeon] Calling sub_807EAA0");
            sub_807EAA0(1, 0);
        }

        MGBA_Warnf("[Dungeon] Calling nullsub_16");
        nullsub_16();
        MGBA_Warnf("[Dungeon] After nullsub_16, unk5=%d", gDungeon->unk5);
        if (gDungeon->unk5 == 0) {
            bool8 param = TRUE;
            s32 turnCount = 0;
            gDungeon->unk644.unk10 = 0;
            gDungeon->unk181e8.unk18218 = 0;
            gDungeon->unk181e8.unk18219 = 1;

            MGBA_Warnf("[Dungeon] Entering main game loop");
            do {
                MGBA_Warnf("[Dungeon] ===== Turn %d: Calling RunFractionalTurn (param=%d) =====", turnCount, param);
                RunFractionalTurn(param);
                turnCount++;
                MGBA_Warnf("[Dungeon] Turn %d complete, checking IsFloorOver", turnCount);
                param = FALSE;
            } while (!IsFloorOver());
            MGBA_Warnf("[Dungeon] Main game loop exited after %d turns", turnCount);
        }

        MGBA_Warnf("[Dungeon] Post-loop: Getting leader");
        leader = GetLeader();
        MGBA_Warnf("[Dungeon] Post-loop: Leader valid=%d", EntityIsValid(leader));
        if (EntityIsValid(leader)) {
            MGBA_Warnf("[Dungeon] Post-loop: Calling EnemyEvolution");
            EnemyEvolution(leader);
        }

        MGBA_Warnf("[Dungeon] Post-loop: Checking forced loss (unk10=%d)", gDungeon->unk644.unk10);
        if (gDungeon->unk644.unk10 != 1) {
            MGBA_Warnf("[Dungeon] Post-loop: Calling TryForcedLoss");
            if (TryForcedLoss(TRUE)) {
                gDungeon->unk644.unk10 = 1;
            }
        }
        MGBA_Warnf("[Dungeon] Post-loop: Checking cleanup (unk10=%d unk11=%d)", gDungeon->unk644.unk10, gDungeon->unk11);
        if (gDungeon->unk644.unk10 == 1 || gDungeon->unk11 != 0) {
            const BossFightConfig *bossFight;

            if (gDungeon->unk6 == 0) {
                // CRITICAL FIX: Skip sub_806AA70 for boss floors - it crashes with fixed rooms
                bossFight = DungeonFloorSpawns_GetBossFightConfig();
                MGBA_Warnf("[Dungeon] Post-loop: bossFight check - ptr=%p", bossFight);
                if (bossFight != NULL) {
                    MGBA_Warnf("[Dungeon] Post-loop: bossFight - enabled=%d useFixedRoomLayout=%d", bossFight->enabled, bossFight->useFixedRoomLayout);
                }
                if (bossFight != NULL && bossFight->enabled && bossFight->useFixedRoomLayout) {
                    MGBA_Warnf("[Dungeon] Post-loop: Skipping sub_806AA70 for boss floor");
                } else {
                    MGBA_Warnf("[Dungeon] Post-loop: Calling sub_806AA70");
                    sub_806AA70();
                }
            }
        }

        MGBA_Warnf("[Dungeon] Post-loop: Final leader checks");
        if (EntityIsValid(GetLeader())) {
            MGBA_Warnf("[Dungeon] Post-loop: Calling sub_80526D0");
            sub_80526D0(0x4F);
            MGBA_Warnf("[Dungeon] Post-loop: Calling sub_8052740");
            sub_8052740(0x4F);
        }

        MGBA_Warnf("[Dungeon] Post-loop: Calling SetDungeonMapToNotShown");
        SetDungeonMapToNotShown();
        MGBA_Warnf("[Dungeon] Post-loop: SetDungeonMapToNotShown complete");
        MGBA_Warnf("[Dungeon] Post-loop: Calling sub_803EAF0(1, NULL)");
        sub_803EAF0(1, NULL);
        MGBA_Warnf("[Dungeon] Post-loop: sub_803EAF0 complete");
        gDungeon->unk181e8.unk18219 = 0;
        gDungeon->unk181e8.unk18218 = 1;
        MGBA_Warnf("[Dungeon] Post-loop: Checking BGM fade (unk3=%d unk6=%d)", gDungeon->unk3, gDungeon->unk6);
        if (gDungeon->unk3 == 0
            && gDungeon->unk6 == 0
            && gDungeon->musPlayer.queuedSongIndex == 0x72
            && gDungeon->unk644.dungeonLocation.id == DUNGEON_BURIED_RELIC)
        {
            MGBA_Warnf("[Dungeon] Post-loop: Fading out BGM");
            DungeonFadeOutBGM(60);
        }

        MGBA_Warnf("[Dungeon] Post-loop: Calling sub_803E708(4, 0x4F)");
        // CRITICAL FIX: Skip sub_803E708 for boss floors with fixed rooms - it crashes
        {
            const BossFightConfig *bossFight;
            bossFight = DungeonFloorSpawns_GetBossFightConfig();
            if (bossFight != NULL && bossFight->enabled && bossFight->useFixedRoomLayout) {
                MGBA_Warnf("[Dungeon] Post-loop: Skipping sub_803E708 for boss floor");
            } else {
                sub_803E708(4, 0x4F);
            }
        }
        MGBA_Warnf("[Dungeon] Post-loop: sub_803E708 complete, unk7=%d", gDungeon->unk7);
        if (gDungeon->unk7 == 0) {
            MGBA_Warnf("[Dungeon] Post-loop: Calling sub_803E830");
            // CRITICAL FIX: Skip sub_803E830 for boss floors with fixed rooms - it may also crash
            {
                const BossFightConfig *bossFight;
                bossFight = DungeonFloorSpawns_GetBossFightConfig();
                if (bossFight != NULL && bossFight->enabled && bossFight->useFixedRoomLayout) {
                    MGBA_Warnf("[Dungeon] Post-loop: Skipping sub_803E830 for boss floor");
                } else {
                    sub_803E830();
                }
            }
            MGBA_Warnf("[Dungeon] Post-loop: sub_803E830 complete");
        }

        MGBA_Warnf("[Dungeon] Post-loop: Setting BG/OBJ flags");
        SetBGOBJEnableFlags(0);
        MGBA_Warnf("[Dungeon] Post-loop: Setting palette color");
        color.r = 0x60;
        color.g = 0x80;
        color.b = 0xF8;
        SetBGPaletteBufferColorRGB(253, &color, gDungeonBrightness, NULL);
        MGBA_Warnf("[Dungeon] Post-loop: Calling sub_8040094(1)");
        sub_8040094(1);
        MGBA_Warnf("[Dungeon] Post-loop: Setting unk18218");
        gDungeon->unk181e8.unk18218 = 1;
        if ((GetForcedLossReason() == 2 || GetForcedLossReason() == 3) && gDungeon->unk6 != 0) {
            leader = GetLeader();
            DisplayDungeonMessage(0, gPtrClientFaintedMessage, 1);
            gDungeon->unk6 = 0;
            SetUpDungeonExitData(DUNGEON_EXIT_FAILED_TO_PROTECT_CLIENT, leader, leader);
        }
        CloseAllSpriteFiles();
        sub_8049820();
        CloseFile(gDungeonNameBannerPalette);
        FreeDungeonPokemonSprites();
        gUnknown_203B40C = 0;

        if (gDungeon->unk3 != 0) {
            SaveDungeonState(gSerializedData_203B41C, 0x4800);
            setupPtr->info.unk7C = 3;
            setupPtr->info.unk80 = gDungeon->unk644.dungeonLocation;
            check = FALSE;
        }
        else
        {
            s16 var;

            if (gDungeon->unk6 != 0) {
                SaveDungeonState(gSerializedData_203B41C, 0x4800);
            }
            else {
                ClearAllItemsWithFlag(ITEM_FLAG_IN_SHOP);
            }
            sub_806C1D8();

            if (gDungeon->unk644.unk10 == 1) {
                if (gDungeon->unk644.stoleFromKecleon != 0) {
                    AllItemsToPlainSeed();
                }
                check = TRUE;
            }
            else if (gDungeon->unk11 == 1) {
                SetUpDungeonExitData(DUNGEON_EXIT_ESCAPED_MIDDLE_OF_EXPLORATION, NULL, GetLeader());
                check = TRUE;
            }
            else if (gDungeon->unk11 == 2) {
                SetUpDungeonExitData(DUNGEON_EXIT_IMPRESSIVELY_COMPLETED_MISSION, NULL, GetLeader());
                if (gDungeon->unk644.stoleFromKecleon != 0) {
                    IncrementThievingSuccesses();
                }
                check = TRUE;
            }
            else if (gDungeon->unk11 == 3) {
                SetUpDungeonExitData(DUNGEON_EXIT_BEFRIENDED_MEW, NULL, GetLeader());
                if (gDungeon->unk644.stoleFromKecleon != 0) {
                    IncrementThievingSuccesses();
                }
                check = TRUE;
            }
            else if (gDungeon->unk11 == 4) {
                var = DUNGEON_EXIT_CLEARED_DUNGEON;
                SetUpDungeonExitData(var, NULL, GetLeader());
                check = TRUE;
            }
            else if (gDungeon->unk644.unk34 == 1 && GetFloorType() == FLOOR_TYPE_RESCUE && gDungeon->unk644.unk10 == 2) {
                SetUpDungeonExitData(DUNGEON_EXIT_SUCCEEDED_IN_RESCUE_MISSION, NULL, GetLeader());
                if (gDungeon->unk644.stoleFromKecleon != 0) {
                    IncrementThievingSuccesses();
                }
                check = TRUE;
            }
            else {
                if (gDungeon->unk644.stoleFromKecleon != 0) {
                    IncrementThievingSuccesses();
                }
                if (gDungeon->unk644.dungeonLocation.floor + 1 < gDungeon->unk1CEC8) {
                    gDungeon->unk644.dungeonLocation.floor++;
                    if (gDungeon->unk644.dungeonLocation.id == DUNGEON_FROSTY_FOREST
                        && gDungeon->unk644.dungeonLocation.floor == 6
                        && !sub_8098100(0x1F))
                    {
                        sub_8097FA8(0x1F);
                        sub_8086130();
                        sub_8097FF8();
                    }
                    // We go back to the loop's start.
                    continue;
                }
                else {
                    var = DUNGEON_EXIT_CLEARED_DUNGEON;
                    SetUpDungeonExitData(var, NULL, GetLeader());
                    check = TRUE;
                    // This goto is a fakematch I had to create in order to generate matching code.
                    // It has no real effect, because the control flow is the same without it(since check is TRUE). Unfortunately agbcc is blind and goto is needed.
                    // Feel free to remove it.
                    goto FAKEMATCH;
                }
            }
        }
        break;
    }

    if (check) {
    // See comment above
    FAKEMATCH:
        gUnknown_203B40C = 0;
        setupPtr->info.unk7E = 0;
        sub_8097FF8();
        sub_80095CC(1, 0x14);
        sub_803E13C();
        sub_800CDA8(4);
        if (gDungeon->unk6 == 0 && sub_8083C88(gDungeon->unk644.unk34)) {
            ShowDungeonClearedWindow();
        }

        if (IsUnsuccessfulDungeonExit()) {
            if (gDungeon->unk6 != 0) {
                setupPtr->info.unk7C = -2;
                memset(&setupPtr->info.unk84, 0, sizeof(setupPtr->info.unk84));
                setupPtr->info.unk80 = gDungeon->unk644.dungeonLocation;
                setupPtr->info.unk84.location = gDungeon->unk644.dungeonLocation;
                setupPtr->info.unk84.seed = gDungeon->unk644.unk38;

            }
            else {
                setupPtr->info.unk7C = -1;
            }
        }
        else if (sub_8083C50()) {
            // Mark dungeon as conquered for seed override progression
            s32 seed;
            if (DungeonSeedOverrides_IsEnabled(&seed)) {
                // Find which rescue dungeon corresponds to the current dungeon ID
                s32 i;
                for (i = 0; i < RESCUE_DUNGEON_COUNT; i++) {
                    if (RescueDungeonToDungeonId(i) == gDungeon->unk644.dungeonLocation.id) {
                        sub_8097418(i, TRUE);
                        break;
                    }
                }
            }

            if (gDungeon->unk644.unk34 == 1) {
                setupPtr->info.unk7C = 4;
            }
            else if (gDungeon->unk644.unk34 == 0) {
                setupPtr->info.unk7C = 1;
                sub_8084424();
            }
            else {
                setupPtr->info.unk7C = 1;
                sub_8084424();
            }
            setupPtr->info.unk7E = gDungeon->unk644.unk30;
        }
        else {
            setupPtr->info.unk7C = 2;
            sub_8084424();
        }
    }

    CloseDungeonPaletteFile();
    sub_803E214();
    nullsub_56();
    CloseDungeonMapFile();
    if (setupPtr->info.unk7C == 1 || setupPtr->info.unk7C == 4 || setupPtr->info.unk7C == 2) {
        CleanUpInventoryItems();
    }
    if (setupPtr->info.unk7C == 1 || setupPtr->info.unk7C == -2 || setupPtr->info.unk7C == 4 || setupPtr->info.unk7C == -1 || setupPtr->info.unk7C == 2) {
        if (setupPtr->info.unk7C == 1 || setupPtr->info.unk7C == 4 || setupPtr->info.unk7C == 2) {
            sub_8068BDC(1);
        }
        else if (setupPtr->info.unk7C == -2) {
            sub_8068BDC(0);
        }
        else {
            sub_8068F28();
            sub_8068BDC(0);
        }
    }
    sub_800DB7C();
    gDungeon = NULL;
    gSerializedData_203B41C = 0;
    nullsub_16();
}

static void sub_8043CD8(void)
{
    ResetVramPalOAM();
}

bool8 sub_8043CE4(s32 dungeonId)
{
    if (gUnknown_202F1A8)
        return TRUE;

    return (gDungeonWaterType[dungeonId] == 2);
}

u8 GetFloorType(void)
{
    if (gDungeon->unk644.unk34 == 1 && gDungeon->unk644.dungeonSeed.location.floor == gDungeon->unk644.dungeonLocation.floor)
        return FLOOR_TYPE_RESCUE;
    else if (IsFloorwideFixedRoom())
        return FLOOR_TYPE_FIXED;
    else
        return FLOOR_TYPE_NORMAL;
}

void sub_8043D50(s32 *a0, s32 *a1)
{
    *a1 = sizeof(Dungeon);
    *a0 = 0x4800; // Hmmm
}

void sub_8043D60(void)
{
    s32 x, y, monId;

    for (monId = 0; monId < DUNGEON_MAX_WILD_POKEMON; monId++) {
        Entity *mon = gDungeon->wildPokemon[monId];
        if (EntityIsValid(mon)) {
            EntityInfo *monInfo = GetEntInfo(mon);
            bool32 unk = TRUE;

            if (monInfo->shopkeeper == TRUE)
                unk = FALSE;
            if (IsExperienceLocked(monInfo->joinedAt.id))
                unk = FALSE;
            if (monInfo->monsterBehavior == BEHAVIOR_RESCUE_TARGET)
                unk = FALSE;

            if (unk) {
                HandleFaint(mon, DUNGEON_EXIT_DEBUG_DAMAGE, mon);
            }
        }
    }

    for (monId = 0; monId < MAX_TEAM_MEMBERS; monId++) {
        Entity *mon = gDungeon->teamPokemon[monId];
        if (EntityIsValid(mon)) {
            s32 i;
            EntityInfo *monInfo;

            mon->unk22 = 0;
            mon->isVisible = TRUE;
            monInfo = GetEntInfo(mon);
            monInfo->HP = monInfo->maxHPStat;
            monInfo->belly = monInfo->maxBelly;
            gDungeon->unk644.itemHoldersIdentified = FALSE;
            ResetMonEntityData(monInfo, 0);
            monInfo->apparentID = monInfo->id;
            monInfo->perishSongTurns = 0;
            for (i = 0; i < MAX_MON_MOVES; i++) {
                Move *move = &monInfo->moves.moves[i];
                if (move->moveFlags & MOVE_FLAG_EXISTS) {
                    move->PP = GetMoveBasePP(move);
                }
            }
        }
    }

    for (y = 0; y < 32; y++) {
        for (x = 0; x < 56; x++) {
            Entity *object = GetTileMut(x, y)->object;
            if (EntityIsValid(object) && GetEntityType(object) == ENTITY_TRAP) {
                Trap *trapData = GetTrapInfo(object);
                if (trapData->id == 27) {
                    trapData->id = TRAP_PITFALL_TRAP;
                }
            }
        }
    }

    ClearAllItemsWithFlag(ITEM_FLAG_IN_SHOP);
}

bool8 TryForcedLoss(bool8 a0)
{
    bool8 ret = FALSE;

    if (!a0 && IsFloorOver())
        return FALSE;

    if (GetForcedLossReason() == 1) {
        Entity *leader = GetLeader();
        if (EntityIsValid(leader)) {
            if (!a0) {
                strcpy(gFormatBuffer_Monsters[0], gDungeon->faintStringBuffer);
                DisplayDungeonMessage(0, gUnknown_80F89B4, 1);
            }
            sub_8042B0C(leader);
            HandleFaint(leader, DUNGEON_EXIT_RETURNED_WITH_FALLEN_PARTNER, leader);
            ret = TRUE;
        }
    }
    else if (GetForcedLossReason() == 2) {
        Entity *leader = GetLeader();
        if (EntityIsValid(leader)) {
            if (!a0) {
                strcpy(gFormatBuffer_Monsters[0], gDungeon->faintStringBuffer);
                DisplayDungeonMessage(0, gUnknown_80F89D4, 1);
            }
            sub_8042B0C(leader);
            HandleFaint(leader, DUNGEON_EXIT_FAILED_TO_PROTECT_CLIENT, leader);
            ret = TRUE;
        }
    }
    else if (GetForcedLossReason() == 3) {
        Entity *leader = GetLeader();
        if (EntityIsValid(leader)) {
            if (!a0) {
                strcpy(gFormatBuffer_Monsters[0], gDungeon->faintStringBuffer);
                DisplayDungeonMessage(0, gUnknown_80F89D8, 1);
            }
            sub_8042B0C(leader);
            HandleFaint(leader, DUNGEON_EXIT_FAILED_TO_PROTECT_CLIENT, leader);
            ret = TRUE;
        }
    }

    return ret;
}

void sub_8043FD0(void)
{
    s32 level;
#ifdef DEV
    for (level = 2; level <= 100; level++) { // DEV: Level up to 100
#else
    for (level = 2; level <= 2; level++) { // Normal: Level up to 2
#endif
        s32 i, monId, movesCount;
        for (monId = 0; monId < NUM_MONSTERS; monId++) {
            Pokemon *monStruct = &gRecruitedPokemonRef->pokemon[monId];
            if (PokemonExists(monStruct) && PokemonFlag2(monStruct)) {
                u16 learnedMoves[16];
                LevelData levelData;
                // I have to make the variables volatile to get matching code. I'm sure there's a solution for this, but keeping it like that for now.
                #ifdef NONMATCHING
                s32 atk, spAtk, def, spDef;
                #else
                vs32 atk, spAtk, def, spDef;
                #endif // NONMATCHING

                GetLvlUpEntry(&levelData, monStruct->speciesNum, level);
                monStruct->level = level;
                monStruct->currExp = levelData.expRequired;
                monStruct->pokeHP += levelData.gainHP;
                if (monStruct->pokeHP >= 999) // TODO: Make this a max hp define
                    monStruct->pokeHP = 999;

                atk = monStruct->offense.att[0];
                spAtk = monStruct->offense.att[1];
                def = monStruct->offense.def[0];
                spDef = monStruct->offense.def[1];

                atk += levelData.gainAtt[0];
                spAtk += levelData.gainAtt[1];
                def += levelData.gainDef[0];
                spDef += levelData.gainDef[1];

                // TODO: Make 255 max define for stats
                if (atk >= 255)     {atk = 255;}
                if (spAtk >= 255)   {spAtk = 255;}
                if (def >= 255)     {def = 255;}
                if (spDef >= 255)   {spDef = 255;}

                monStruct->offense.att[0] = atk;
                monStruct->offense.att[1] = spAtk;
                monStruct->offense.def[0] = def;
                monStruct->offense.def[1] = spDef;

                movesCount = GetMovesLearnedAtLevel(learnedMoves, monStruct->speciesNum, monStruct->level, 999);
                if (movesCount == 0)
                    continue;

                for (i = 0; i < movesCount; i++) {
                    s32 moveSlot;
                    for (moveSlot = 0; moveSlot < MAX_MON_MOVES; moveSlot++) {
                        if (!MoveFlagExists(&monStruct->moves[moveSlot])) {
                            InitZeroedPPPokemonMove(&monStruct->moves[moveSlot], learnedMoves[i]);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void EnforceMaxItemsAndMoney(void)
{
    s32 i;
    u8 dungeonId = gDungeon->unk644.dungeonLocation.id;

    if (GetMaxItemsAllowed(dungeonId) == 0)
    {
        for (i = 0; i < INVENTORY_SIZE; i++) {
            ZeroOutItem(&gTeamInventoryRef->teamItems[i]);
        }
        for (i = 0; i < NUM_MONSTERS; i++) {
            Pokemon *mon = (&gRecruitedPokemonRef->pokemon[i]);
            if (PokemonExists(mon) && PokemonFlag2(mon)) {
                mon->heldItem.id = 0;
            }
        }
    }
    else {
        DungeonSeedOverrides_ApplyItemLimit();
    }

    if (!IsMoneyAllowed(dungeonId)) {
        gTeamInventoryRef->teamMoney = 0;
    }
}

bool8 IsFloorwideFixedRoom(void)
{
    if (gDungeon->fixedRoomNumber != 0 && gDungeon->fixedRoomNumber <= LAST_FLOORWIDE_FIXED_ROOM)
    {
        return TRUE;
    }
    return FALSE;
}

bool8 IsCurrentFixedRoomBossFight(void)
{
    if (gDungeon->tileset > DUNGEON_OUT_ON_RESCUE)
    {
        return TRUE;
    }
    return FALSE;
}

bool8 IsMakuhitaTrainingMaze(void)
{
    if (DUNGEON_IS_MAZE(gDungeon->unk644.dungeonLocation.id))
        return TRUE;
    else
        return FALSE;
}
