#include "global.h"
#include "globaldata.h"
#include "dungeon_floor_spawns.h"
#include "constants/dungeon.h"
#include "constants/trap.h"
#include "bg_palette_buffer.h"
#include "graphics_memory.h"
#include "code_800D090.h"
#include "code_803D110.h"
#include "dungeon_vram.h"
#include "cpu.h"
#include "def_filearchives.h"
#include "dungeon_info.h"
#include "dungeon_random.h"
#include "file_system.h"
#include "game_options.h"
#include "pokemon.h"
#include "pokemon_3.h"
#include "text_1.h"
#include "text_3.h"
#include "dungeon_config.h"
#include "structs/dungeon_mapparam.h"
#include "dungeon_seed_overrides.h"
#include "mgba_log.h"
#include "save.h"
#include "constants/difficulty.h"

static bool8 TryGetSeedOverrideValue(s32 *seedOut);
static void ApplySeedOverridesToCurrentFloor(void);
static u8 GetSeededKecleonFaintChance(u32 difficulty);
static u8 GetSeededEnemyDensityFallback(u32 difficulty, s32 floorNumber);
static u8 RollSeededKecleonFaintChance(s32 seed, s32 dungeonId, u8 faintChance);

static EWRAM_DATA s16 sSeededTrapPercentOverride = -1;
static EWRAM_DATA u8 sSeededKecleonFaintChance = 0;
static EWRAM_DATA u8 sSeededKecleonFaintRoll = 0xFF;
static EWRAM_DATA bool8 sSeededKecleonSpawnShopkeeper = TRUE;
#ifdef DEV
static EWRAM_DATA s16 sSeedDumpDungeonId = -1;
static EWRAM_DATA s16 sSeedDumpFloorId = -1;
#endif

// Getter for trap percent override (percent of placeable tiles)
s16 DungeonFloorSpawns_GetTrapPercentOverride(void)
{
    return sSeededTrapPercentOverride;
}

u8 DungeonFloorSpawns_GetSeededKecleonFaintChance(void)
{
    return sSeededKecleonFaintChance;
}

u8 DungeonFloorSpawns_GetSeededKecleonFaintRoll(void)
{
    return sSeededKecleonFaintRoll;
}

bool8 DungeonFloorSpawns_ShouldSpawnKecleonShopkeeper(void)
{
    return sSeededKecleonSpawnShopkeeper;
}

static u8 GetSeededKecleonFaintChance(u32 difficulty)
{
    switch (difficulty) {
        case DIFFICULTY_HARD:
            return 20;
        case DIFFICULTY_NIGHTMARE:
            return 30;
        case DIFFICULTY_NORMAL:
        default:
            return 10;
    }
}

static u8 GetSeededEnemyDensityFallback(u32 difficulty, s32 floorNumber)
{
    u8 base;

    switch (difficulty) {
        case DIFFICULTY_HARD:
            base = 5;
            break;
        case DIFFICULTY_NIGHTMARE:
            base = 6;
            break;
        case DIFFICULTY_NORMAL:
        default:
            base = 4;
            break;
    }

    // Scale slightly deeper in the dungeon so late floors don't feel empty.
    if (floorNumber >= 16)
        base++;
    if (floorNumber >= 28)
        base++;

    return base;
}

// Deterministically roll 0-99 using the personality seed and dungeon id.
static u8 RollSeededKecleonFaintChance(s32 seed, s32 dungeonId, u8 faintChance)
{
    u32 rngSeed;
    u32 hash;

    if (faintChance == 0)
        return 0xFF;

    // Use the seeded RNG helper but salt it to a fixed per-dungeon roll (no floor dependency).
    rngSeed = DungeonSeedOverrides_GetDungeonRngSeed(seed, (u8)dungeonId, 0);
    rngSeed ^= 0x4B45434C; // "KECL"
    hash = (rngSeed ^ (rngSeed >> 16)) * 1664525 + 1013904223;
    return (u8)(hash % 100);
}

static bool8 TryGetSeedOverrideValue(s32 *seedOut)
{
    s32 seed = 0;
    bool8 enabled = DungeonSeedOverrides_IsEnabled(&seed);

    // If the custom seed is unset (-1), keep overrides active using a zero seed
    // so boss floors still generate instead of silently falling back to vanilla.
    if (!enabled) {
        if (seed == -1) {
            seed = 0;
        } else {
            MGBA_Warnf("[BossGen] Overrides disabled for seed=%d", seed);
            return FALSE;
        }
    }

    if (seedOut != NULL)
        *seedOut = seed;
    if (gDungeon == NULL)
        return FALSE;
    if (gDungeon->unk644.dungeonLocation.id > DUNGEON_PURITY_FOREST)
        return FALSE;
    return TRUE;
}

// Storage for current floor's boss fight config
static EWRAM_DATA BossFightConfig sCurrentBossFight = {0};

// Getter for boss fight config (used by other modules)
const BossFightConfig* DungeonFloorSpawns_GetBossFightConfig(void)
{
    return &sCurrentBossFight;
}

static void ApplySeedOverridesToCurrentFloor(void)
{
    DungeonSeedFloorOverrides overrides;
    s32 seed;
    s32 i;

    sSeededTrapPercentOverride = -1;
    sSeededKecleonFaintChance = 0;
    sSeededKecleonFaintRoll = 0xFF;
    sSeededKecleonSpawnShopkeeper = TRUE;
    DungeonSeedOverrides_ResetItemPools();
    if (!TryGetSeedOverrideValue(&seed))
        return;

    DungeonSeedOverrides_GenerateFloorConfig(seed, gDungeon->unk644.dungeonLocation.id, gDungeon->unk644.dungeonLocation.floor, &overrides);

    // Copy boss fight config to static storage
    sCurrentBossFight = overrides.bossFight;
    MGBA_Warnf("[BossGen] ApplyOverrides: dungeonId=%d floor=%d seed=%d bossEnabled=%d boss=%d",
               gDungeon->unk644.dungeonLocation.id,
               gDungeon->unk644.dungeonLocation.floor,
               seed,
               overrides.bossFight.enabled,
               overrides.bossFight.bossSpecies);

    gDungeon->floorProperties.tileset = overrides.tileset;
    gDungeon->floorProperties.fixedRoomNumber = 0;  // Disable boss rooms/cutscenes for randomized dungeons
    if (overrides.bossFight.enabled && overrides.bossFight.applyWeather) {
        gDungeon->floorProperties.weather = overrides.bossFight.weather;
        MGBA_Warnf("[BossGen] Weather override applied: weather=%d", overrides.bossFight.weather);
    } else if (!overrides.bossFight.enabled && overrides.applyWeather) {
        gDungeon->floorProperties.weather = overrides.weather;
        MGBA_Warnf("[Weather] Floor override applied: weather=%d", overrides.weather);
    } else {
        // No weather override - force clear weather
        gDungeon->floorProperties.weather = WEATHER_CLEAR;
        MGBA_Warnf("[Weather] No override - forcing WEATHER_CLEAR");
    }
    // Trap overrides: force max density and a uniform trap table
    if (overrides.trapDensityPercent >= 0) {
        sSeededTrapPercentOverride = overrides.trapDensityPercent;
        MGBA_Warnf("[Traps] Percent override applied: percent=%d", overrides.trapDensityPercent);
    }
    if (overrides.trapDensityOverride >= 0) {
        s32 density = overrides.trapDensityOverride;
        if (density > 56 && overrides.trapDensityPercent < 0) {
            density = 56;
        }
        gDungeon->floorProperties.trapDensity = (u8)density;
        MGBA_Warnf("[Traps] Density override applied: density=%d", density);
    }
    if (overrides.hasTrapTable) {
        s32 i;
        for (i = 0; i < NUM_TRAPS; i++) {
            gDungeon->trapSpawnChances[i] = overrides.trapSpawnChances[i];
        }
        MGBA_Warnf("[Traps] Trap table override applied (uniform all traps)");
    }

    DungeonSeedOverrides_ApplyFloorProperties(&gDungeon->floorProperties,
                                              seed,
                                              gDungeon->unk644.dungeonLocation.id,
                                              gDungeon->unk644.dungeonLocation.floor);

    // Some mapparam entries carry wrapped/invalid enemy density bytes (e.g. 255).
    // Clamp those to a normal seeded baseline so floors don't over- or under-spawn.
    if (!overrides.bossFight.enabled && gDungeon->floorProperties.enemyDensity > 32) {
        u8 oldDensity = gDungeon->floorProperties.enemyDensity;
        u8 oldItemDensity = gDungeon->floorProperties.itemDensity;
        u32 difficulty = GetGameDifficultySetting();
        u8 fallbackDensity;

        if (difficulty >= NUM_DIFFICULTY_SETTINGS)
            difficulty = DIFFICULTY_NORMAL;

        fallbackDensity = GetSeededEnemyDensityFallback(difficulty, gDungeon->unk644.dungeonLocation.floor);
        gDungeon->floorProperties.enemyDensity = fallbackDensity;
        MGBA_Warnf("[FloorProps] Enemy density overflow clamped: dungeon=%d floor=%d old=%d new=%d diff=%d",
                   gDungeon->unk644.dungeonLocation.id,
                   gDungeon->unk644.dungeonLocation.floor,
                   oldDensity,
                   fallbackDensity,
                   difficulty);

        // Keep item counts from collapsing on the same overflow floors where the
        // normal +/-2 variation can otherwise bottom out at a single item.
        if (gDungeon->floorProperties.itemDensity < 4) {
            gDungeon->floorProperties.itemDensity = 4;
            MGBA_Warnf("[FloorProps] Item density raised on overflow floor: dungeon=%d floor=%d old=%d new=%d",
                       gDungeon->unk644.dungeonLocation.id,
                       gDungeon->unk644.dungeonLocation.floor,
                       oldItemDensity,
                       gDungeon->floorProperties.itemDensity);
        }
    }

    // Seeded shop floor: bias the chosen floor to roll a shop using natural generation
    {
        s32 kecleonFloors[SEEDED_KECLEON_SHOP_COUNT] = {0};
        s32 targetFloors[SEEDED_KECLEON_SHOP_COUNT] = {0}; // dungeon floors appear 1-indexed
        s32 matchedIndex = -1;

        DungeonSeedOverrides_GetKecleonFloors(gDungeon->unk644.dungeonLocation.id, seed, &kecleonFloors[0], &kecleonFloors[1]);
        for (i = 0; i < SEEDED_KECLEON_SHOP_COUNT; i++) {
            targetFloors[i] = gDungeon->startFloorId + kecleonFloors[i] + 1;
            if (gDungeon->unk644.dungeonLocation.floor == targetFloors[i]) {
                matchedIndex = i;
            }
        }

        if (matchedIndex >= 0) {
            u32 difficulty = GetGameDifficultySetting();
            u8 faintChance = GetSeededKecleonFaintChance(difficulty);
            u8 roll = RollSeededKecleonFaintChance(seed, gDungeon->unk644.dungeonLocation.id, faintChance);

            gDungeon->floorProperties.kecleonShopChance = 100; // Always generate the shop; NPC may faint
            sSeededKecleonFaintChance = faintChance;
            sSeededKecleonFaintRoll = roll;
            sSeededKecleonSpawnShopkeeper = !(faintChance > 0 && roll < faintChance);
            MGBA_Warnf("[Kecleon] Seeded shop floor: dungeonId=%d floor=%d seed=%d shopChance=%d difficulty=%d faintChance=%d slot=%d",
                       gDungeon->unk644.dungeonLocation.id,
                       gDungeon->unk644.dungeonLocation.floor,
                       seed,
                       gDungeon->floorProperties.kecleonShopChance,
                       difficulty,
                       faintChance,
                       matchedIndex);
            MGBA_Warnf("[Kecleon] Shopkeeper roll: faintChance=%d roll=%d spawn=%d",
                       faintChance, roll, sSeededKecleonSpawnShopkeeper);

            // Ensure Kecleon sprite data is loaded by adding it to the spawn table if missing
            {
                bool8 found = FALSE;
                s32 emptyIndex = -1;

                for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
                    s16 species = ExtractSpeciesIndex(&gDungeon->fileMonsterSpawns[i]);
                    if (species == 0 && emptyIndex == -1) {
                        emptyIndex = i;
                    }
                    if (species == MONSTER_KECLEON) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found && emptyIndex >= 0) {
                    SetSpeciesLevelToExtract(&gDungeon->fileMonsterSpawns[emptyIndex], 90, MONSTER_KECLEON);
                    gDungeon->fileMonsterSpawns[emptyIndex].randNum[0] = 1;
                    gDungeon->fileMonsterSpawns[emptyIndex].randNum[1] = 1;
                    MGBA_Warnf("[Kecleon] Injected Kecleon into spawn table at index %d for sprite load", emptyIndex);
                }
            }
        }
    }

    // Seeded Monster House floor: force 100% chance on its assigned floor
    {
        s32 monsterHouseFloor = DungeonSeedOverrides_GetGuaranteedMonsterHouseFloor(gDungeon->unk644.dungeonLocation.id, seed);
        s32 targetFloor = gDungeon->startFloorId + monsterHouseFloor + 1; // dungeon floors appear 1-indexed

        if (gDungeon->unk644.dungeonLocation.floor == targetFloor) {
            gDungeon->floorProperties.monsterHouseChance = 100;
            MGBA_Warnf("[MonsterHouse] Guaranteed floor: dungeonId=%d floor=%d seed=%d targetFloor=%d",
                       gDungeon->unk644.dungeonLocation.id,
                       gDungeon->unk644.dungeonLocation.floor,
                       seed,
                       targetFloor);
        }
    }

    // If boss fight is enabled, set up boss spawn table (not normal enemies)
    if (overrides.bossFight.enabled) {
        s32 spawnIndex = 0;

        // CRITICAL FIX: Add boss to spawn table so sprite gets loaded!
        // This prevents LoadDungeonPokemonSprites() crash
        SetSpeciesToExtract(&gDungeon->fileMonsterSpawns[spawnIndex], overrides.bossFight.bossSpecies);
        gDungeon->fileMonsterSpawns[spawnIndex].randNum[0] = 0;
        gDungeon->fileMonsterSpawns[spawnIndex].randNum[1] = 0;
        spawnIndex++;

        // Also enqueue each unique minion species so their sprites load before entering the floor
        for (i = 0; i < overrides.bossFight.minionCount && spawnIndex < MONSTER_SPAWNS_ARR_COUNT; i++, spawnIndex++) {
            SetSpeciesToExtract(&gDungeon->fileMonsterSpawns[spawnIndex], overrides.bossFight.minionSpecies[i]);
            gDungeon->fileMonsterSpawns[spawnIndex].randNum[0] = 0;
            gDungeon->fileMonsterSpawns[spawnIndex].randNum[1] = 0;
        }

        // Clear remaining entries
        while (spawnIndex < MONSTER_SPAWNS_ARR_COUNT) {
            SetSpeciesToExtract(&gDungeon->fileMonsterSpawns[spawnIndex], 0);
            gDungeon->fileMonsterSpawns[spawnIndex].randNum[0] = 0;
            gDungeon->fileMonsterSpawns[spawnIndex].randNum[1] = 0;
            spawnIndex++;
        }

        // Disable auto-spawn but allow spawn initialization (for sprite loading)
        gDungeon->floorProperties.enemyDensity = 0;
        // DON'T set currFloorMonsterSpawnsCount = 0 yet!
        // Let SetCurrentMonsterSpawns() populate it so sprites load
        gDungeon->monsterSpawnsPopulated = FALSE;   // Will be populated with boss
    } else {
        // Normal floor - apply monster spawns
        for (i = 0; i < overrides.spawnCount && i < MONSTER_SPAWNS_ARR_COUNT; i++) {
            gDungeon->fileMonsterSpawns[i] = overrides.spawns[i];
        }

        while (i < MONSTER_SPAWNS_ARR_COUNT) {
            SetSpeciesToExtract(&gDungeon->fileMonsterSpawns[i], 0);
            gDungeon->fileMonsterSpawns[i].randNum[0] = 0;
            gDungeon->fileMonsterSpawns[i].randNum[1] = 0;
            i++;
        }
    }

#ifdef DEV
    {
        if (gDungeon->unk644.dungeonLocation.id != sSeedDumpDungeonId
            || gDungeon->unk644.dungeonLocation.floor != sSeedDumpFloorId)
        {
            DungeonSeedOverrides_LogSeedDump(seed,
                                             gDungeon->unk644.dungeonLocation.id,
                                             gDungeon->unk644.dungeonLocation.floor,
                                             gDungeon->startFloorId,
                                             &overrides,
                                             gDungeon->fileMonsterSpawns);
            sSeedDumpDungeonId = gDungeon->unk644.dungeonLocation.id;
            sSeedDumpFloorId = gDungeon->unk644.dungeonLocation.floor;
        }
    }
#endif
}

void sub_803D4AC(void)
{
    gDungeon->unk1C570.id = DUNGEON_INVALID;
    gDungeon->unk1C570.floor = 0xFF;
}

void SetFloorItemMonsterSpawns(void)
{
    s16 spArray[NUM_ITEM_CATEGORIES + NUMBER_OF_ITEM_IDS];
    s32 i, j, k;
    OpenedFile *file;
    struct DungeonMapParam1 *strPtr;

    GeneralizeMazeDungeonLoc(&gDungeon->unk644.dungeonLocation2, &gDungeon->unk644.dungeonLocation);
    if (gDungeon->unk1C570.id == gDungeon->unk644.dungeonLocation2.id && gDungeon->unk1C570.floor == gDungeon->unk644.dungeonLocation2.floor)
        return;

    gDungeon->unk1C570 = gDungeon->unk644.dungeonLocation2;
    MGBA_Warnf("[FloorInit] Begin: dungeon=%d floor=%d", gDungeon->unk1C570.id, gDungeon->unk1C570.floor);
    file = OpenFileAndGetFileDataPtr("mapparam", &gDungeonFileArchive);
    strPtr = &((struct DungeonMapParam2 *)(file->data))->unk0[gDungeon->unk1C570.id][gDungeon->unk1C570.floor];

    gDungeon->unk1CEC8 = GetDungeonFloorCount(gDungeon->unk644.dungeonLocation.id);
    gDungeon->startFloorId = GetDungeonStartingFloor(gDungeon->unk644.dungeonLocation.id);

    gDungeon->floorProperties = ((struct DungeonMapParam2 *)(file->data))->floorProperties[strPtr->unk0];
    MGBA_Warnf("[FloorInit] Props: tileset=%d fixedRoom=%d startFloor=%d floorCount=%d",
               gDungeon->floorProperties.tileset,
               gDungeon->floorProperties.fixedRoomNumber,
               gDungeon->startFloorId,
               gDungeon->unk1CEC8);

    for (i = 0; i < NUM_TRAPS; i++) {
        gDungeon->trapSpawnChances[i] = ((struct DungeonMapParam2 *)(file->data))->trapSpawnChances[strPtr->unk4][i];
    }
    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT - 1; i++) {
        gDungeon->fileMonsterSpawns[i] = ((struct DungeonMapParam2 *)(file->data))->monsterSpawns[strPtr->unk2][i];
        if (ExtractSpeciesIndex(&gDungeon->fileMonsterSpawns[i]) == 0)
            break;
    }
    while (i < MONSTER_SPAWNS_ARR_COUNT) {
        SetSpeciesToExtract(&gDungeon->fileMonsterSpawns[i], 0);
        i++;
    }

    for (i = 0; i < ITEM_SPAWN_TYPES_COUNT; i++)
    {
        u16 *src = ((struct DungeonMapParam2 *)(file->data))->itemSpawns[strPtr->unk6[i]];
        s32 arrId = 0;

        for (j = 0; j < NUM_ITEM_CATEGORIES + NUMBER_OF_ITEM_IDS; ) {
            if (src[arrId] >= ITEM_SETS_SKIP_NUMBER) {
                for (k = src[arrId] - ITEM_SETS_SKIP_NUMBER; k != 0; k--) {
                    spArray[j++] = 0;
                }
            }
            else {
                spArray[j++] = src[arrId];
            }
            arrId++;
        }

        arrId = 0;
        for (j = 0; j < NUM_ITEM_CATEGORIES; j++) {
            gDungeon->itemSpawns[i].categoryValues[arrId] = spArray[arrId];
            arrId++;
        }

        for (j = 0; j < NUMBER_OF_ITEM_IDS; j++) {
            gDungeon->itemSpawns[i].itemValues[j] = spArray[arrId];
            arrId++;
        }
    }

    CloseFile(file);
    // Force weather to clear before applying overrides
    // Only our override system should set weather
    gDungeon->floorProperties.weather = WEATHER_CLEAR;
    ApplySeedOverridesToCurrentFloor();
    MGBA_Warnf("[FloorInit] End: tileset=%d weather=%d bossEnabled=%d boss=%d enemyDensity=%d spawnsLoaded=%d",
               gDungeon->floorProperties.tileset,
               gDungeon->floorProperties.weather,
               sCurrentBossFight.enabled,
               sCurrentBossFight.bossSpecies,
               gDungeon->floorProperties.enemyDensity,
               gDungeon->monsterSpawnsPopulated);
}

u8 GetRandomFloorTrap(void)
{
    s32 i;
    s32 rand = DungeonRandInt(10000);
    for (i = 0; i < NUM_TRAPS; i++) {
        if (gDungeon->trapSpawnChances[i] != 0 && gDungeon->trapSpawnChances[i] >= rand)
            return i;
    }

    return TRAP_CHESTNUT_TRAP;
}

u8 GetRandomFloorItem(s32 spawnType)
{
    s32 i;
    s32 rand;
    u8 seededItem;
    u8 category = NUM_ITEM_CATEGORIES;

    if (DungeonSeedOverrides_SelectFloorItem(spawnType, &seededItem))
        return seededItem;

    rand = DungeonRandInt(ITEM_SETS_RANDOM_CAP + 1);
    for (i = 0; i < NUM_ITEM_CATEGORIES; i++) {
        if (gDungeon->itemSpawns[spawnType].categoryValues[i] != 0 && gDungeon->itemSpawns[spawnType].categoryValues[i] >= rand) {
            category = i;
            break;
        }
    }
    if (category == NUM_ITEM_CATEGORIES)
        return ITEM_POKE;

    rand = DungeonRandInt(ITEM_SETS_RANDOM_CAP + 1);
    for (i = 0; i < NUMBER_OF_ITEM_IDS; i++) {
        if (gDungeon->itemSpawns[spawnType].itemValues[i] != 0 && GetItemCategory(i) == category && gDungeon->itemSpawns[spawnType].itemValues[i] >= rand) {
            return i;
        }
    }

    return ITEM_POKE;
}

s32 SetMonsterSpawnsArray(SpawnPokemonData *strPtr, s32 id)
{
    s32 i;

    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
        if (ExtractSpeciesIndex(&gDungeon->fileMonsterSpawns[i]) == 0)
            break;
        strPtr[id] = gDungeon->fileMonsterSpawns[i];
        id++;
    }

    return id;
}

s32 GetAvailableMonsToTransform(SpawnPokemonData *strPtr, s32 id)
{
    s32 i;

    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
        s16 species = ExtractSpeciesIndex(&gDungeon->fileMonsterSpawns[i]);
        if (species == 0)
            break;
        if (GetBodySize(species) < 2 && gDungeon->fileMonsterSpawns[i].randNum[0] != 0) {
            strPtr[id] = gDungeon->fileMonsterSpawns[i];
            id++;
        }
    }

    return id;
}

void SetCurrentMonsterSpawns(void)
{
    if (!gDungeon->monsterSpawnsPopulated) {
        gDungeon->monsterSpawnsPopulated = TRUE;
        gDungeon->currFloorMonsterSpawnsCount = SetMonsterSpawnsArray(gDungeon->monsterSpawns, 0);
    }
}

bool8 CanMonsterBeSpawnedHere(s16 speciesToFind)
{
    s32 i;
    s32 id = SpeciesId(speciesToFind);

    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
        s16 species = ExtractSpeciesIndex(&gDungeon->fileMonsterSpawns[i]);
        if (species == 0)
            break;
        if (species == id)
            return TRUE;
    }
    return FALSE;
}

s16 GetRandomFloorMonsterId(s32 arrId)
{
    s32 i;
    s32 rand = DungeonRandInt(10000);

    for (i = 0; i < gDungeon->currFloorMonsterSpawnsCount; i++) {
        if (gDungeon->monsterSpawns[i].randNum[arrId] != 0 && gDungeon->monsterSpawns[i].randNum[arrId] >= rand) {
            return ExtractSpeciesIndex(&gDungeon->monsterSpawns[i]);
        }
    }
    for (i = 0; i < gDungeon->currFloorMonsterSpawnsCount; i++) {
        if (gDungeon->monsterSpawns[i].randNum[arrId] != 0) {
            return ExtractSpeciesIndex(&gDungeon->monsterSpawns[i]);
        }
    }

    return MONSTER_KECLEON;
}

s32 GetSpawnedMonsterLevel(s32 species)
{
    s32 i;
    s32 speciesId = SpeciesId(species);

    for (i = 0; i < gDungeon->currFloorMonsterSpawnsCount; i++) {
        if (ExtractSpeciesIndex(&gDungeon->monsterSpawns[i]) == speciesId)
            return ExtractLevel(&gDungeon->monsterSpawns[i]);
    }
    return 1;
}
