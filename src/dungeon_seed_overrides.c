#include "global.h"
#include "dungeon_seed_overrides.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "constants/rescue_dungeon_id.h"
#include "constants/bg_music.h"
#include "constants/item.h"
#include "constants/move_id.h"
#include "constants/weather.h"
#include "pokemon_3.h"
#include "save.h"
#include "code_800D090.h"
#include "code_80A26CC.h"
#include "strings.h"
#include "rescue_scenario.h"
#include "dungeon_items.h"
#include "dungeon_map.h"
#include "dungeon_map_access.h"
#include "dungeon_floor_spawns.h"
#include "main_loops.h"
#include "mgba_log.h"
#include "pokemon.h"
#include "items.h"
#include "structs/map.h"
#include "type_selection.h"

#define SEEDED_TILESET_COUNT 75  // Max valid tileset ID (gNaturePowerCalledMoves uses max 74)
#define SEEDED_MIN_FLOORS 3
#define SEEDED_MAX_FLOORS 60
#define SEEDED_MIN_SPAWNS 6
#define SEEDED_MAX_SPAWNS 16
#define SEEDED_DUNGEON_NAME_MAX_LEN 32
#define SEEDED_PREFIX_BUFFER_LEN 16

// List of rescue dungeon IDs that appear in the dungeon list, for sequential unlocking
// Exactly 20 dungeons - ONLY single-part dungeons (no peaks, summits, grottos, pits, or 2nd floors)
static const s16 sSequentialDungeonList[] = {
    // Main story dungeons (1-7) - only single-part
    RESCUE_DUNGEON_TINY_WOODS,           // 1
    RESCUE_DUNGEON_THUNDERWAVE_CAVE,     // 2
    RESCUE_DUNGEON_MT_STEEL,             // 3
    RESCUE_DUNGEON_SINISTER_WOODS,       // 4
    RESCUE_DUNGEON_SILENT_CHASM,         // 5
    RESCUE_DUNGEON_GREAT_CANYON,         // 6 (excludes _2)
    RESCUE_DUNGEON_LAPIS_CAVE,           // 7
    // Post-game dungeons (8-30) - only single-part
    RESCUE_DUNGEON_UPROAR_FOREST,        // 8
    RESCUE_DUNGEON_HOWLING_FOREST,       // 9
    RESCUE_DUNGEON_STORMY_SEA,           // 10
    RESCUE_DUNGEON_SILVER_TRENCH,        // 11
    RESCUE_DUNGEON_JOYOUS_TOWER,         // 12
    RESCUE_DUNGEON_FIERY_FIELD,          // 13
    RESCUE_DUNGEON_LIGHTNING_FIELD,      // 14
    RESCUE_DUNGEON_NORTHWIND_FIELD,      // 15
    RESCUE_DUNGEON_MT_FARAWAY,           // 16
    RESCUE_DUNGEON_WESTERN_CAVE,         // 17
    RESCUE_DUNGEON_NORTHERN_RANGE,       // 18
    RESCUE_DUNGEON_PITFALL_VALLEY,       // 19
    RESCUE_DUNGEON_BURIED_RELIC,         // 20 - FINAL DUNGEON (credits after)
};

#define SEQUENTIAL_DUNGEON_COUNT ARRAY_COUNT(sSequentialDungeonList)

typedef struct DungeonSeedRng {
    u32 state;
} DungeonSeedRng;

typedef struct SeedSpeciesPool {
    const s16 *species;
    u8 count;
} SeedSpeciesPool;

static void ClearFloorOverrides(DungeonSeedFloorOverrides *result);
static DungeonSeedRng DungeonSeedRng_Init(s32 seed, u8 dungeonId, s32 floorId, u32 salt);
static u32 DungeonSeedRng_Next(DungeonSeedRng *rng);
static s32 DungeonSeedRng_NextRange(DungeonSeedRng *rng, s32 min, s32 max);
static u8 SelectMinionFormation(s32 seed, u8 dungeonId, s32 floorId);
static u8 SelectTileset(s32 floorId);
static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result);
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed);
static const SeedSpeciesPool* GetBossPool(s32 floorId);
static u16 SelectRandomLoot(DungeonSeedRng *rng, s32 floorId);
static bool8 TryGetTypeSelectionBoss(s16 *bossSpecies);
static bool8 GetTypeBossMinions(s16 bossSpecies, s16 *minionsOut, u8 *minionCountOut);
static bool8 GetTypeBossMoves(s16 bossSpecies, u16 *movesOut);
static bool8 GetTypeBossMinionMoves(s16 bossSpecies, u16 minionMovesOut[][MAX_MON_MOVES], bool8 *minionHasCustomMovesOut);
static const BossWeatherConfig *GetBossWeatherConfigForSpecies(s16 species);
static void MaybeApplyBossWeather(BossFightConfig *bossFight, DungeonSeedRng *rng);
static bool8 IsBossSpecies(s16 species);
static bool8 SpeciesMatchesTypeMask(s16 species, u32 typeMask);
static s32 BuildSpawnCandidates(u32 typeMask, s16 *out, s32 outCapacity);

static const s16 sPoolElectric[] = {MONSTER_PICHU, MONSTER_PIKACHU, MONSTER_PLUSLE, MONSTER_MINUN, MONSTER_MANECTRIC};
static const s16 sPoolFire[] = {MONSTER_VULPIX, MONSTER_GROWLITHE, MONSTER_CHARMELEON, MONSTER_FLAREON, MONSTER_BLAZIKEN};
static const s16 sPoolWater[] = {MONSTER_SQUIRTLE, MONSTER_WARTORTLE, MONSTER_POLIWHIRL, MONSTER_SEADRA, MONSTER_LANTURN};
static const s16 sPoolGrass[] = {MONSTER_BULBASAUR, MONSTER_IVYSAUR, MONSTER_GLOOM, MONSTER_SUNKERN, MONSTER_BRELOOM};
static const s16 sPoolPsychic[] = {MONSTER_RALTS, MONSTER_KADABRA, MONSTER_ESPEON, MONSTER_XATU, MONSTER_GARDEVOIR};
static const s16 sPoolDark[] = {MONSTER_POOCHYENA, MONSTER_MIGHTYENA, MONSTER_SNEASEL, MONSTER_HOUNDOOM, MONSTER_UMBREON};
static const s16 sPoolIce[] = {MONSTER_SNORUNT, MONSTER_SEALEO, MONSTER_GLALIE, MONSTER_CLOYSTER, MONSTER_LAPRAS};
static const s16 sPoolDragon[] = {MONSTER_BAGON, MONSTER_SHELGON, MONSTER_ALTARIA, MONSTER_DRAGONITE, MONSTER_FLYGON};

static const SeedSpeciesPool sSpeciesPools[] = {
    {sPoolElectric, ARRAY_COUNT(sPoolElectric)},
    {sPoolFire, ARRAY_COUNT(sPoolFire)},
    {sPoolWater, ARRAY_COUNT(sPoolWater)},
    {sPoolGrass, ARRAY_COUNT(sPoolGrass)},
    {sPoolPsychic, ARRAY_COUNT(sPoolPsychic)},
    {sPoolDark, ARRAY_COUNT(sPoolDark)},
    {sPoolIce, ARRAY_COUNT(sPoolIce)},
    {sPoolDragon, ARRAY_COUNT(sPoolDragon)},
};

static const s32 sFloorBandTable[] = {5, 7, 8, 10, 12, 14, 16, 18, 20, 25, 30, 35, 40, 50};

// Boss species pools (tier-appropriate bosses)
static const s16 sPoolEarlyBosses[] = {
    MONSTER_MANKEY, MONSTER_GEODUDE, MONSTER_MACHOP,
    MONSTER_DROWZEE, MONSTER_NIDORINO, MONSTER_HAUNTER,
    MONSTER_MAGMAR, MONSTER_ELECTABUZZ
};

static const s16 sPoolMidBosses[] = {
    MONSTER_PRIMEAPE, MONSTER_GOLEM, MONSTER_MACHOKE,
    MONSTER_HYPNO, MONSTER_NIDOKING, MONSTER_ARCANINE,
    MONSTER_RHYDON, MONSTER_MAGNETON
};

static const s16 sPoolLateBosses[] = {
    MONSTER_MACHAMP, MONSTER_ALAKAZAM, MONSTER_GENGAR,
    MONSTER_TYRANITAR, MONSTER_SALAMENCE, MONSTER_METAGROSS,
    MONSTER_AGGRON, MONSTER_SLAKING
};

// Minion pools - general purpose minions for any boss
static const s16 sPoolMinions[] = {
    MONSTER_RATTATA, MONSTER_SENTRET, MONSTER_POOCHYENA,
    MONSTER_ZIGZAGOON, MONSTER_ZUBAT, MONSTER_GEODUDE
};

static const char *const sSeededPrefixTable[] = {
    "Tiny",
    "Ancient",
    "Misty",
    "Shocking",
    "Blazing",
    "Shadow",
    "Azure",
    "Verdant",
    "Howling",
    "Radiant",
    "Dusky",
    "Rustic",
    "Obsidian",
    "Gleaming",
    "Stormy",
    "Frosted",
};

static const char *const sSeededSuffixTable[] = {
    "Woods",
    "Forest",
    "Cavern",
    "Depths",
    "Blaze",
    "Sea",
    "Grove",
    "Range",
    "Hollows",
    "Marsh",
    "Abyss",
    "Spire",
    "Pass",
    "Valley",
    "Peak",
    "Gale",
    "Gorge",
    "Shore",
    "Fens",
    "Wilds",
};

// Bitmask of allowed spawn types per tileset (from docs/tileset_types.csv).
// Bit position matches the type constant value (TYPE_*). Index 0 is unused.
static const u32 sTilesetSpawnTypeMask[SEEDED_TILESET_COUNT] = {
    [0] = 0x00000000,
    [1] = 0x00001012,
    [2] = 0x00002202,
    [3] = 0x00000802,
    [4] = 0x00011310,
    [5] = 0x00022080,
    [6] = 0x00000600,
    [7] = 0x00005102,
    [8] = 0x00000086,
    [9] = 0x00000042,
    [10] = 0x00008402,
    [11] = 0x00001410,
    [12] = 0x00014900,
    [13] = 0x00002202,
    [14] = 0x00001212,
    [15] = 0x00020048,
    [16] = 0x00020048,
    [17] = 0x00002206,
    [18] = 0x00020048,
    [19] = 0x00020048,
    [20] = 0x00001012,
    [21] = 0x00002202,
    [22] = 0x00022204,
    [23] = 0x00001092,
    [24] = 0x00020048,
    [25] = 0x00000212,
    [26] = 0x00001012,
    [27] = 0x00000422,
    [28] = 0x00000800,
    [29] = 0x00000422,
    [30] = 0x00020800,
    [31] = 0x00003200,
    [32] = 0x00001018,
    [33] = 0x00010108,
    [34] = 0x00008400,
    [35] = 0x0000C400,
    [36] = 0x00008440,
    [37] = 0x00002A00,
    [38] = 0x00002202,
    [39] = 0x00002202,
    [40] = 0x0000004A,
    [41] = 0x0000001A,
    [42] = 0x00000022,
    [43] = 0x00000420,
    [44] = 0x00001606,
    [45] = 0x00008042,
    [46] = 0x00000006,
    [47] = 0x00000042,
    [48] = 0x00022004,
    [49] = 0x00000008,
    [50] = 0x00002210,
    [51] = 0x00001012,
    [52] = 0x00001012,
    [53] = 0x00001012,
    [54] = 0x00000048,
    [55] = 0x00008000,
    [56] = 0x00000612,
    [57] = 0x00006900,
    [58] = 0x00008A00,
    [59] = 0x00002204,
    [60] = 0x00000492,
    [61] = 0x00010300,
    [62] = 0x00001012,
    [63] = 0x0000001A,
};

static u8 sSeededDungeonName1[NUM_DUNGEONS][SEEDED_DUNGEON_NAME_MAX_LEN];
static u8 sSeededDungeonName2[NUM_DUNGEONS][SEEDED_DUNGEON_NAME_MAX_LEN];
static bool8 sSeededDungeonNameValid[NUM_DUNGEONS];
static s32 sSeededDungeonNameSeed = -2;
static s32 sSeededDungeonNameType = -2;

static void ResetSeededDungeonNameCache(void);
static void GenerateSeededDungeonNames(u8 dungeonId, s32 seed);
static const char *SelectPrefixForDungeon(u8 dungeonId, DungeonSeedRng *rng, char *scratch, s32 scratchSize);
static bool8 CopyFirstTokenFromBaseName(u8 dungeonId, char *buffer, s32 bufferSize);
static s32 GetSelectedTypeForDisplay(void);

void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId, DungeonSeedFloorOverrides *result)
{
    DungeonSeedRng rng;

    if (result == NULL)
        return;

    MGBA_Warnf("[SeedOverrides] GenFloor start: seed=%d dungeon=%d floor=%d", seed, dungeonId, floorId);
    ClearFloorOverrides(result);
    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0xC0FFEE);
    result->tileset = SelectTileset(floorId);

    // NEW: Procedurally generate boss fight configuration
    PopulateBossFightConfig(result, &rng, dungeonId, floorId, seed);

    // If boss fight enabled, use boss tileset; otherwise normal generation
    if (result->bossFight.enabled) {
        // Use the boss room's tileset (set in PopulateBossFightConfig) for the entire floor
        result->tileset = result->bossFight.roomTileset;
        result->spawnCount = 0;  // No normal spawns in boss rooms
    } else {
        PopulateSpawnTable(result, &rng, dungeonId, floorId);
        FinalizeSpawnWeights(result);
    }

    MGBA_Warnf("[SeedOverrides] GenFloor done: tileset=%d spawns=%d bossEnabled=%d boss=%d",
               result->tileset,
               result->spawnCount,
               result->bossFight.enabled,
               result->bossFight.bossSpecies);
}

s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId)
{
    // For testing: all dungeons have 5 floors
    (void)seed;
    (void)dungeonId;
    return 5;
}

u32 DungeonSeedOverrides_GetDungeonRngSeed(s32 seed, u8 dungeonId, s32 floorId)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0x5EED5EED);
    u32 rngSeed = DungeonSeedRng_Next(&rng) | 1;
    MGBA_Warnf("[SeedOverrides] RngSeed: seed=%d dungeon=%d floor=%d rngSeed=%u", seed, dungeonId, floorId, rngSeed);
    return rngSeed;
}

bool8 DungeonSeedOverrides_IsEnabled(s32 *seedOut)
{
    s32 seed = sub_8011C34();

    MGBA_Warnf("[SeedOverrides] IsEnabled? seed=%d", seed);
    // Fallback: if the global seed isn't set, try to recover it from the
    // persisted TeamBasicInfo and mirror it back into the global slot.
    if (seed == -1) {
        TeamBasicInfo basicInfo;
        ReadTeamBasicInfo(&basicInfo);
        if (basicInfo.customSeed != -1) {
            seed = basicInfo.customSeed;
            sub_8011C40(seed);
            MGBA_Infof("[DungeonSeedOverrides] Restored seed from TeamBasicInfo: %d", seed);
        }
    }

    if (seed == -1)
    {
        if (seedOut != NULL)
            *seedOut = seed;
        return FALSE;
    }
    if (seedOut != NULL)
        *seedOut = seed;
    MGBA_Warnf("[SeedOverrides] Enabled=TRUE seed=%d", seed);
    return TRUE;
}

const u8 *DungeonSeedOverrides_GetDungeonName(u8 dungeonId, bool8 secondLine)
{
    s32 seed;
    s32 displayType;

    if (dungeonId >= NUM_DUNGEONS)
        return NULL;
    if (dungeonId > DUNGEON_PURITY_FOREST)
        return NULL;
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return NULL;

    displayType = GetSelectedTypeForDisplay();
    if (displayType != sSeededDungeonNameType) {
        sSeededDungeonNameType = displayType;
        ResetSeededDungeonNameCache();
    }

    if (sSeededDungeonNameSeed != seed) {
        sSeededDungeonNameSeed = seed;
        ResetSeededDungeonNameCache();
    }

    if (!sSeededDungeonNameValid[dungeonId])
        GenerateSeededDungeonNames(dungeonId, seed);

    if (secondLine)
        return sSeededDungeonName2[dungeonId];
    else
        return sSeededDungeonName1[dungeonId];
}

static void ClearFloorOverrides(DungeonSeedFloorOverrides *result)
{
    s32 i, j;

    result->tileset = 0;
    result->spawnCount = 0;
    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
        result->spawns[i].bits = 0;
        result->spawns[i].randNum[0] = 0;
        result->spawns[i].randNum[1] = 0;
    }

    // Initialize boss fight config
    result->bossFight.enabled = FALSE;
    result->bossFight.bossSpecies = 0;
    result->bossFight.bossHP = 0;
    result->bossFight.bossMusic = 0;
    result->bossFight.dropItem = 0;
    result->bossFight.monsterBehavior = 0;
    result->bossFight.minionCount = 0;
    result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
    result->bossFight.roomTileset = 0;
    result->bossFight.weather = WEATHER_CLEAR;
    result->bossFight.applyWeather = FALSE;
    for (i = 0; i < 4; i++) {
        result->bossFight.minionSpecies[i] = 0;
        result->bossFight.minionUseCustomMoves[i] = FALSE;
        for (j = 0; j < MAX_MON_MOVES; j++) {
            result->bossFight.minionMoves[i][j] = MOVE_NOTHING;
        }
    }
    for (i = 0; i < MAX_MON_MOVES; i++) {
        result->bossFight.bossMoves[i] = MOVE_NOTHING;
    }
    result->bossFight.useCustomMoves = FALSE;
}

static u8 SelectMinionFormation(s32 seed, u8 dungeonId, s32 floorId)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0xB0551EED);
    return DungeonSeedRng_NextRange(&rng, 0, MINION_FORMATION_COUNT);
}

static DungeonSeedRng DungeonSeedRng_Init(s32 seed, u8 dungeonId, s32 floorId, u32 salt)
{
    DungeonSeedRng rng;
    u32 state = 0xA511E9B5;

    state ^= (u32)seed;
    state += salt;
    state ^= (u32)dungeonId * 0x45D9F3B;
    state += (u32)floorId * 0x27D4EB2D;
    rng.state = state | 1;
    return rng;
}

static u32 DungeonSeedRng_Next(DungeonSeedRng *rng)
{
    rng->state = rng->state * 1664525 + 1013904223;
    return rng->state;
}

static s32 DungeonSeedRng_NextRange(DungeonSeedRng *rng, s32 min, s32 max)
{
    u32 span;

    if (max <= min)
        return min;

    span = (u32)(max - min);
    return min + (DungeonSeedRng_Next(rng) % span);
}

static u8 SelectTileset(s32 floorId)
{
    u8 tileset;

    if (TypeSelection_HasActiveTileset()) {
        tileset = TypeSelection_GetActiveTileset();
        if (tileset < SEEDED_TILESET_COUNT)
            MGBA_Infof("[SeedOverrides] Tileset (active) id=%d type=%d", tileset, TypeSelection_GetActiveType());
            return tileset;
    }

    if (TypeSelection_HasCommittedTileset()) {
        tileset = TypeSelection_GetCommittedTileset();
        if (tileset < SEEDED_TILESET_COUNT)
            MGBA_Infof("[SeedOverrides] Tileset (committed) id=%d type=%d", tileset, TypeSelection_GetCommittedType());
            return tileset;
    }

    if (floorId < 0)
        floorId = 0;
    tileset = (u8)(floorId % SEEDED_TILESET_COUNT);
    MGBA_Infof("[SeedOverrides] Tileset (fallback) id=%d floor=%d", tileset, floorId);
    return tileset;
}

static bool8 IsBossSpecies(s16 species)
{
    s32 i, j;

    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return FALSE;

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        s32 poolCount = pool->count;

        if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

        for (j = 0; j < poolCount; j++) {
            if (pool->species[j] == species)
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 SpeciesMatchesTypeMask(s16 species, u32 typeMask)
{
    s32 i;

    if (typeMask == 0)
        return FALSE;
    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return FALSE;
    if (IsBossSpecies(species))
        return FALSE;

    for (i = 0; i < 2; i++) {
        u8 type = GetPokemonType(species, (u32)i);
        if (type > TYPE_NONE && type < NUM_TYPES && (typeMask & (1u << type)))
            return TRUE;
    }
    return FALSE;
}

static s32 BuildSpawnCandidates(u32 typeMask, s16 *out, s32 outCapacity)
{
    s32 count = 0;
    s32 species;

    if (out == NULL || outCapacity <= 0 || typeMask == 0)
        return 0;

    for (species = 1; species < MONSTER_MAX && count < outCapacity; species++) {
        if (SpeciesMatchesTypeMask((s16)species, typeMask)) {
            out[count++] = (s16)species;
        }
    }
    return count;
}

static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId)
{
    s32 entryCount;
    s32 i;
    u32 typeMask = 0;
    s16 candidates[MONSTER_MAX];
    s32 candidateCount = 0;
    bool8 useBulbasaurOnly = FALSE;
    bool8 loggedFallback = FALSE;
    bool8 loggedInvalid = FALSE;

    entryCount = DungeonSeedRng_NextRange(rng, SEEDED_MIN_SPAWNS, SEEDED_MAX_SPAWNS + 1);
    if (entryCount > MONSTER_SPAWNS_ARR_COUNT)
        entryCount = MONSTER_SPAWNS_ARR_COUNT;

    if (result != NULL && result->tileset < SEEDED_TILESET_COUNT)
        typeMask = sTilesetSpawnTypeMask[result->tileset];

    if (typeMask != 0)
        candidateCount = BuildSpawnCandidates(typeMask, candidates, ARRAY_COUNT(candidates));
    if (typeMask != 0 && candidateCount == 0)
        useBulbasaurOnly = TRUE;

    MGBA_Infof("[SeedOverrides] Spawn selection tileset=%d mask=0x%08x candidates=%d bulbaOnly=%d",
               (result != NULL ? result->tileset : -1), typeMask, candidateCount, useBulbasaurOnly);

    for (i = 0; i < entryCount; i++) {
        const SeedSpeciesPool *pool;
        s16 species;
        s32 baseLevel;
        s32 levelVariance;
        s32 level;

        if (useBulbasaurOnly) {
            species = MONSTER_BULBASAUR;
        } else if (candidateCount > 0) {
            s32 idx = DungeonSeedRng_NextRange(rng, 0, candidateCount);
            species = candidates[idx];
        } else {
            s32 attempts = 0;
            pool = &sSpeciesPools[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSpeciesPools))];
            species = MONSTER_NONE;

            while (attempts < 4) {
                species = pool->species[DungeonSeedRng_NextRange(rng, 0, pool->count)];
                if (!IsBossSpecies(species))
                    break;
                attempts++;
            }

            if (species == MONSTER_NONE || IsBossSpecies(species)) {
                s32 j;
                for (j = 0; j < pool->count; j++) {
                    if (!IsBossSpecies(pool->species[j])) {
                        species = pool->species[j];
                        break;
                    }
                }
                if (!loggedFallback) {
                    MGBA_Warnf("[SeedOverrides] Spawn fallback to generic pool (mask=0x%08x)", typeMask);
                    loggedFallback = TRUE;
                }
            }
        }

        if (species <= MONSTER_NONE || species >= MONSTER_MAX) {
            species = MONSTER_BULBASAUR;
            if (!loggedInvalid) {
                MGBA_Warnf("[SeedOverrides] Spawn invalid species, forcing Bulbasaur (entry=%d)", i);
                loggedInvalid = TRUE;
            }
        }

        baseLevel = 3 + (dungeonId % 10) + floorId;
        levelVariance = DungeonSeedRng_NextRange(rng, 0, 6);
        level = baseLevel + levelVariance;

        if (level > 90)
            level = 90;

        SetSpeciesLevelToExtract(&result->spawns[i], level, species);
    }

    result->spawnCount = entryCount;
}

static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result)
{
    s32 i;
    s32 cumulative = 0;
    s32 step;

    if (result->spawnCount <= 0)
        return;

    step = 10000 / result->spawnCount;
    if (step <= 0)
        step = 1;

    for (i = 0; i < result->spawnCount; i++) {
        cumulative += step;
        if (cumulative > 10000)
            cumulative = 10000;
        result->spawns[i].randNum[0] = (s16)cumulative;
        result->spawns[i].randNum[1] = (s16)cumulative;
    }
}

// Get boss pool based on floor tier
static const SeedSpeciesPool* GetBossPool(s32 floorId)
{
    // Return pointer to the array and count directly instead of using static locals
    // This avoids issues with static initialization on GBA
    static SeedSpeciesPool pool;

    if (floorId <= 5) {
        pool.species = sPoolEarlyBosses;
        pool.count = ARRAY_COUNT(sPoolEarlyBosses);
    } else if (floorId <= 15) {
        pool.species = sPoolMidBosses;
        pool.count = ARRAY_COUNT(sPoolMidBosses);
    } else {
        pool.species = sPoolLateBosses;
        pool.count = ARRAY_COUNT(sPoolLateBosses);
    }

    return &pool;
}

// Select random loot based on floor
static u16 SelectRandomLoot(DungeonSeedRng *rng, s32 floorId)
{
    // Simple item pool - can be expanded later
    static const u16 sLootPool[] = {
        0x05,  // Oran Berry
        0x06,  // Sitrus Berry
        0x1C,  // Reviver Seed
        0x14,  // Apple
        0x3C,  // Max Elixir
    };

    (void)floorId;  // Can use floor for tier-based loot later
    return sLootPool[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sLootPool))];
}

static bool8 TryGetTypeSelectionBoss(s16 *bossSpecies)
{
    if (bossSpecies == NULL)
        return FALSE;

    if (TypeSelection_HasActiveBoss()) {
        *bossSpecies = TypeSelection_GetActiveBoss();
    } else if (TypeSelection_HasCommittedBoss()) {
        *bossSpecies = TypeSelection_GetCommittedBoss();
    } else {
        return FALSE;
    }

    if (*bossSpecies <= MONSTER_NONE || *bossSpecies >= MONSTER_MAX)
        return FALSE;

    return TRUE;
}

static bool8 GetTypeBossMinions(s16 bossSpecies, s16 *minionsOut, u8 *minionCountOut)
{
    s32 i, j, k;
    s16 localMinions[TYPE_SELECTION_MINIONS_PER_BOSS];
    bool8 allValid;

    if (bossSpecies <= MONSTER_NONE || bossSpecies >= MONSTER_MAX)
        return FALSE;

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        s32 poolCount = pool->count;

        if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

        for (j = 0; j < poolCount; j++) {
            if (pool->species[j] != bossSpecies)
                continue;

            allValid = TRUE;
            for (k = 0; k < TYPE_SELECTION_MINIONS_PER_BOSS; k++) {
                localMinions[k] = pool->minions[j][k];
                if (localMinions[k] <= MONSTER_NONE || localMinions[k] >= MONSTER_MAX)
                    allValid = FALSE;
            }

            if (!allValid)
                return FALSE;

            if (minionsOut != NULL) {
                for (k = 0; k < TYPE_SELECTION_MINIONS_PER_BOSS; k++) {
                    minionsOut[k] = localMinions[k];
                }
            }

            if (minionCountOut != NULL)
                *minionCountOut = TYPE_SELECTION_MINIONS_PER_BOSS;

            return TRUE;
        }
    }

    return FALSE;
}

static bool8 GetTypeBossMoves(s16 bossSpecies, u16 *movesOut)
{
    s32 i, j, k;
    u16 localMoves[MAX_MON_MOVES];

    if (movesOut == NULL)
        return FALSE;
    if (bossSpecies <= MONSTER_NONE || bossSpecies >= MONSTER_MAX)
        return FALSE;

    for (k = 0; k < MAX_MON_MOVES; k++) {
        movesOut[k] = MOVE_NOTHING;
        localMoves[k] = MOVE_NOTHING;
    }

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        s32 poolCount = pool->count;

        if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

        for (j = 0; j < poolCount; j++) {
            if (pool->species[j] != bossSpecies)
                continue;
            if (!pool->hasCustomMoves[j])
                return FALSE;

            for (k = 0; k < MAX_MON_MOVES; k++) {
                localMoves[k] = pool->moves[j][k];
            }

            for (k = 0; k < MAX_MON_MOVES; k++) {
                movesOut[k] = localMoves[k];
            }
            return TRUE;
        }
    }

    return FALSE;
}

static bool8 GetTypeBossMinionMoves(s16 bossSpecies, u16 minionMovesOut[][MAX_MON_MOVES], bool8 *minionHasCustomMovesOut)
{
    s32 i, j, k;

    if (bossSpecies <= MONSTER_NONE || bossSpecies >= MONSTER_MAX)
        return FALSE;
    if (minionMovesOut == NULL || minionHasCustomMovesOut == NULL)
        return FALSE;

    for (i = 0; i < 4; i++) {
        minionHasCustomMovesOut[i] = FALSE;
        for (j = 0; j < MAX_MON_MOVES; j++) {
            minionMovesOut[i][j] = MOVE_NOTHING;
        }
    }

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        s32 poolCount = pool->count;

        if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

        for (j = 0; j < poolCount; j++) {
            if (pool->species[j] != bossSpecies)
                continue;

            for (k = 0; k < TYPE_SELECTION_MINIONS_PER_BOSS; k++) {
                s32 idx = k;
                s32 moveIdx;
                minionHasCustomMovesOut[idx] = pool->minionHasCustomMoves[j][k];
                for (moveIdx = 0; moveIdx < MAX_MON_MOVES; moveIdx++) {
                    minionMovesOut[idx][moveIdx] = pool->minionMoves[j][k][moveIdx];
                }
            }

            return TRUE;
        }
    }

    return FALSE;
}

static const BossWeatherConfig *GetBossWeatherConfigForSpecies(s16 species)
{
    s32 i, j;

    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return NULL;

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        const TypeBossWeatherPool *weatherPool = &gTypeBossWeatherTable[i];
        s32 poolCount = pool->count;

        if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

        for (j = 0; j < poolCount; j++) {
            if (pool->species[j] == species)
                return &weatherPool->bosses[j];
        }
    }

    return NULL;
}

static void MaybeApplyBossWeather(BossFightConfig *bossFight, DungeonSeedRng *rng)
{
    const BossWeatherConfig *config;
    u32 difficulty;
    u16 chance;

    if (bossFight == NULL || rng == NULL)
        return;

    bossFight->applyWeather = FALSE;
    bossFight->weather = WEATHER_CLEAR;

    config = GetBossWeatherConfigForSpecies(bossFight->bossSpecies);
    if (config == NULL || !config->enabled)
        return;

    difficulty = GetGameDifficultySetting();
    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    chance = config->chance[difficulty];
    if (chance == 0)
        return;

    if (chance >= BOSS_WEATHER_CHANCE_SCALE) {
        bossFight->applyWeather = TRUE;
        bossFight->weather = config->weather;
        return;
    }

    if (DungeonSeedRng_NextRange(rng, 0, BOSS_WEATHER_CHANCE_SCALE) < chance) {
        bossFight->applyWeather = TRUE;
        bossFight->weather = config->weather;
    }
}

// Procedurally generate boss fight configuration from seed
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed)
{
    const SeedSpeciesPool defaultMinionPool = {sPoolMinions, ARRAY_COUNT(sPoolMinions)};
    s32 i, j;
    s32 seedForLog = seed;
    s32 typeForLog = -1;
    const char *source = "unknown";
    s16 selectedBoss = MONSTER_NONE;
    bool8 bossValid = FALSE;
    bool8 bossWasFallback = FALSE;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 finalFloor = floorCount - 1;

    result->bossFight.applyWeather = FALSE;
    result->bossFight.weather = WEATHER_CLEAR;
    result->bossFight.minionCount = 0;
    result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
    for (i = 0; i < ARRAY_COUNT(result->bossFight.minionSpecies); i++) {
        result->bossFight.minionSpecies[i] = MONSTER_NONE;
    }
    for (i = 0; i < MAX_MON_MOVES; i++) {
        result->bossFight.bossMoves[i] = MOVE_NOTHING;
    }
    result->bossFight.useCustomMoves = FALSE;
    for (i = 0; i < ARRAY_COUNT(result->bossFight.minionSpecies); i++) {
        result->bossFight.minionUseCustomMoves[i] = FALSE;
        for (j = 0; j < MAX_MON_MOVES; j++) {
            result->bossFight.minionMoves[i][j] = MOVE_NOTHING;
        }
    }

    // Procedurally determine if this floor has a boss
    // Only spawn bosses on the final floor of the dungeon
    if (floorCount <= 1 || floorId != finalFloor) {
        result->bossFight.enabled = FALSE;
        return;
    }

    // If the global seed is missing, surface the issue by forcing a Bulbasaur boss.
    if (sub_8011C34() == -1) {
        result->bossFight.enabled = TRUE;
        result->bossFight.bossSpecies = MONSTER_BULBASAUR;
        result->bossFight.bossHP = 300 + (floorId * 25);
        result->bossFight.bossMusic = MUS_BOSS_BATTLE;
        result->bossFight.dropItem = SelectRandomLoot(rng, floorId);
        result->bossFight.minionCount = 2;
        result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
        for (i = 0; i < result->bossFight.minionCount; i++) {
            s32 minionIdx = DungeonSeedRng_NextRange(rng, 0, defaultMinionPool.count);
            result->bossFight.minionSpecies[i] = defaultMinionPool.species[minionIdx];
        }
        result->bossFight.roomTileset = 64;  // Skarmory boss fight tileset
        result->bossFight.monsterBehavior = 0;
        result->bossFight.useFixedRoomLayout = TRUE;
        result->bossFight.fixedRoomNumber = 1;  // Fixed Room 1
        MaybeApplyBossWeather(&result->bossFight, rng);
        source = "seed_missing";
        MGBA_Warnf("[BossGen] seed=-1 type=%d floor=%d boss=%d source=%s", typeForLog, floorId, result->bossFight.bossSpecies, source);
        return;
    }

    result->bossFight.enabled = TRUE;

    // Prefer the type-selected boss for this dungeon; fall back to tiered pool if unavailable
    bossValid = TryGetTypeSelectionBoss(&selectedBoss);
    if (!bossValid) {
        const SeedSpeciesPool *bossPool = GetBossPool(floorId);
        s32 bossIndex = DungeonSeedRng_NextRange(rng, 0, bossPool->count);
        selectedBoss = bossPool->species[bossIndex];
        source = "pool";
    }
    else {
        source = "type";
    }

    // If the selected boss exceeds the trimmed monster cap, fall back to Bulbasaur
    if (selectedBoss <= MONSTER_NONE || selectedBoss >= MONSTER_MAX) {
        bossWasFallback = TRUE;
        selectedBoss = MONSTER_BULBASAUR;
        source = "fallback_oob";
    }

    result->bossFight.bossSpecies = selectedBoss;
    result->bossFight.useCustomMoves = GetTypeBossMoves(selectedBoss, result->bossFight.bossMoves);

    // Procedurally set HP scaling with floor
    result->bossFight.bossHP = 300 + (floorId * 25);

    // Procedurally select music
    result->bossFight.bossMusic = MUS_BOSS_BATTLE;

    // Procedurally select loot drop
    result->bossFight.dropItem = SelectRandomLoot(rng, floorId);

    // Prefer configured minions for type-selected bosses; fall back to random pool otherwise
    if (!GetTypeBossMinions(selectedBoss, result->bossFight.minionSpecies, &result->bossFight.minionCount)) {
        result->bossFight.minionCount = 2;
        for (i = 0; i < result->bossFight.minionCount; i++) {
            s32 minionIdx = DungeonSeedRng_NextRange(rng, 0, defaultMinionPool.count);
            result->bossFight.minionSpecies[i] = defaultMinionPool.species[minionIdx];
        }
    }
    if (result->bossFight.minionCount > ARRAY_COUNT(result->bossFight.minionSpecies))
        result->bossFight.minionCount = ARRAY_COUNT(result->bossFight.minionSpecies);
    GetTypeBossMinionMoves(selectedBoss, result->bossFight.minionMoves, result->bossFight.minionUseCustomMoves);
    result->bossFight.minionFormation = SelectMinionFormation(seed, dungeonId, floorId);

    // Use Skarmory boss fight tileset for all boss arenas
    result->bossFight.roomTileset = 64;  // Skarmory boss fight tileset

    // Set behavior for boss identification
    result->bossFight.monsterBehavior = 0;  // Will define this constant later

    // Use Fixed Room 1 for boss arena layout
    result->bossFight.useFixedRoomLayout = TRUE;
    result->bossFight.fixedRoomNumber = 1;  // Fixed Room 1

    MaybeApplyBossWeather(&result->bossFight, rng);

    if (TypeSelection_HasActiveType())
        typeForLog = TypeSelection_GetActiveType();
    else if (TypeSelection_HasCommittedType())
        typeForLog = TypeSelection_GetCommittedType();

    MGBA_Warnf("[BossGen] seed=%d type=%d floor=%d boss=%d source=%s fallback=%d weather=%d applyWeather=%d",
               seedForLog, typeForLog, floorId, result->bossFight.bossSpecies, source, bossWasFallback,
               result->bossFight.weather, result->bossFight.applyWeather);
}

static void ResetSeededDungeonNameCache(void)
{
    s32 i;

    for (i = 0; i < NUM_DUNGEONS; i++) {
        sSeededDungeonNameValid[i] = FALSE;
        sSeededDungeonName1[i][0] = '\0';
        sSeededDungeonName2[i][0] = '\0';
    }
}

static void GenerateSeededDungeonNames(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0xB16B00B);
    const char *prefix;
    const char *suffix;
    char prefixBuffer[SEEDED_PREFIX_BUFFER_LEN];
    s32 dungeonIndex = -1;
    s32 i;
    const char *typeLabel = NULL;
    char typeBuffer[20];

    prefix = SelectPrefixForDungeon(dungeonId, &rng, prefixBuffer, ARRAY_COUNT(prefixBuffer));
    suffix = sSeededSuffixTable[DungeonSeedRng_NextRange(&rng, 0, ARRAY_COUNT(sSeededSuffixTable))];

    if (sSeededDungeonNameType > TYPE_NONE && sSeededDungeonNameType < NUM_TYPES) {
        sprintfStatic(typeBuffer, "[%s]", gUnformattedTypeStrings[sSeededDungeonNameType]);
        typeLabel = typeBuffer;
    }

    // Find this dungeon's index in the sequential list by checking each rescue dungeon
    for (i = 0; i < SEQUENTIAL_DUNGEON_COUNT; i++) {
        u8 listDungeonId = RescueDungeonToDungeonId(sSequentialDungeonList[i]);
        if (listDungeonId == dungeonId) {
            dungeonIndex = i;
            break;
        }
    }

    // If dungeon is in the sequential list, prepend "Dungeon X/YY" format
    if (dungeonIndex != -1) {
        if (typeLabel != NULL) {
            sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d) %s",
                          dungeonIndex + 1, SEQUENTIAL_DUNGEON_COUNT, typeLabel);
        } else {
            sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d)",
                          dungeonIndex + 1, SEQUENTIAL_DUNGEON_COUNT);
        }
        sprintfStatic((char *)sSeededDungeonName2[dungeonId], "%s %s", prefix, suffix);
    } else {
        // Not in sequential list, use normal format
        sprintfStatic((char *)sSeededDungeonName1[dungeonId], "%s %s", prefix, suffix);
        sprintfStatic((char *)sSeededDungeonName2[dungeonId], "%s %s", prefix, suffix);
    }

    sSeededDungeonNameValid[dungeonId] = TRUE;
}

static const char *SelectPrefixForDungeon(u8 dungeonId, DungeonSeedRng *rng, char *scratch, s32 scratchSize)
{
    if (scratch != NULL && scratchSize > 1 && CopyFirstTokenFromBaseName(dungeonId, scratch, scratchSize))
        return scratch;

    return sSeededPrefixTable[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSeededPrefixTable))];
}

static bool8 CopyFirstTokenFromBaseName(u8 dungeonId, char *buffer, s32 bufferSize)
{
    const u8 *baseName;
    s32 i;

    if (buffer == NULL || bufferSize <= 1)
        return FALSE;
    if (dungeonId >= NUM_DUNGEONS)
        return FALSE;

    baseName = gDungeonNames[dungeonId].name1;
    if (baseName == NULL)
        return FALSE;

    for (i = 0; i < bufferSize - 1; i++) {
        u8 ch = baseName[i];
        if (ch == '\0' || ch == ' ' || ch == '\n' || ch == '\t')
            break;
        buffer[i] = ch;
    }
    buffer[i] = '\0';
    return (i != 0);
}

static s32 GetSelectedTypeForDisplay(void)
{
    s32 type = -1;

    // Ensure the very first dungeon gets a committed type before displaying names.
    TypeSelection_EnsureInitialCommittedType();

    // Prefer the committed type for the upcoming dungeon
    if (TypeSelection_HasCommittedType())
        type = TypeSelection_GetCommittedType();
    // Fall back to the active type (already consumed for current dungeon)
    else if (TypeSelection_HasActiveType())
        type = TypeSelection_GetActiveType();
    else {
        // Last-ditch: read the committed/active slots even if their valids are false,
        // in case the flag was cleared but the value is still useful for debugging display.
        type = TypeSelection_GetCommittedType();
        if (type <= TYPE_NONE || type >= NUM_TYPES)
            type = TypeSelection_GetActiveType();
    }

    if (type <= TYPE_NONE || type >= NUM_TYPES)
        return -1;

    return type;
}

// Helper function to check if a rescue dungeon is in the sequential list
static bool8 IsInSequentialList(s16 rescueDungeonId, s32 *indexOut)
{
    s32 i;
    for (i = 0; i < SEQUENTIAL_DUNGEON_COUNT; i++) {
        if (sSequentialDungeonList[i] == rescueDungeonId) {
            if (indexOut != NULL)
                *indexOut = i;
            return TRUE;
        }
    }
    return FALSE;
}

// Check if a rescue dungeon is in the sequential list (public version for menu)
bool8 DungeonSeedOverrides_IsInSequentialList(s16 rescueDungeonId)
{
    s32 seed;

    // If overrides aren't enabled, all dungeons are "in the list" (vanilla behavior)
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return TRUE;

    return IsInSequentialList(rescueDungeonId, NULL);
}

// Get the rescue dungeon ID that should currently be shown (the next unconquered one)
// Returns -1 if no dungeon should be shown
s16 DungeonSeedOverrides_GetCurrentDungeon(void)
{
    s32 seed;
    s32 i;

    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return -1;

    // Find the first unconquered dungeon in the sequential list
    for (i = 0; i < SEQUENTIAL_DUNGEON_COUNT; i++) {
        if (!RescueScenarioConquered(sSequentialDungeonList[i])) {
            return sSequentialDungeonList[i];
        }
    }

    // All dungeons conquered
    return -1;
}

// Custom sequential dungeon unlocking logic
// Returns TRUE if the dungeon can be entered (has GO icon), FALSE otherwise
// Only shows the NEXT unconquered dungeon, not all unlocked dungeons
bool8 DungeonSeedOverrides_CanEnterDungeon(s16 rescueDungeonId)
{
    s32 dungeonIndex;
    s32 seed;

    // Only filter when overrides are enabled
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return TRUE;  // Let vanilla logic handle it

    // Check if in sequential list
    if (!IsInSequentialList(rescueDungeonId, &dungeonIndex))
        return FALSE;  // Hide ALL non-sequential dungeons

    // Don't show if this dungeon has already been conquered
    if (RescueScenarioConquered(rescueDungeonId))
        return FALSE;

    // First dungeon is unlocked if not conquered
    if (dungeonIndex == 0)
        return TRUE;

    // Otherwise, check if the previous dungeon has been conquered
    return RescueScenarioConquered(sSequentialDungeonList[dungeonIndex - 1]);
}

// Check if all sequential dungeons have been cleared (run complete) to trigger credits
bool8 DungeonSeedOverrides_ShouldTriggerCredits(void)
{
    s32 seed;

    // Only check if overrides are enabled
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return FALSE;

    // When every sequential dungeon is conquered, the run is done.
    return (DungeonSeedOverrides_GetCurrentDungeon() == -1);
}

// Boss fight handling - global state
static Entity *sCustomBossEntity = NULL;
static s32 sStairsSpawnX = 0;
static s32 sStairsSpawnY = 0;

// Register the boss entity for tracking
void DungeonSeedOverrides_RegisterBossEntity(Entity *boss)
{
    sCustomBossEntity = boss;
}

// Set the position where stairs should spawn after boss defeat
void DungeonSeedOverrides_SetStairsPosition(s32 x, s32 y)
{
    MGBA_Warnf("[StairsPos] SetStairsPosition called: (%d, %d)", x, y);
    sStairsSpawnX = x;
    sStairsSpawnY = y;
    MGBA_Warnf("[StairsPos] Stored: sStairsSpawnX=%d, sStairsSpawnY=%d", sStairsSpawnX, sStairsSpawnY);
}

// Check if an entity is a custom boss
bool8 DungeonSeedOverrides_IsCustomBoss(Entity *pokemon)
{
    return (pokemon != NULL && pokemon == sCustomBossEntity);
}

// Handle boss defeat - spawn stairs and drop loot
void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon)
{
    const BossFightConfig *bossFight;
    Item item;
    Tile *tile;
    DungeonPos dropPos;

    MGBA_Warnf("[BossFaint] HandleBossFaint called for entity %p (boss=%p)", pokemon, sCustomBossEntity);

    if (pokemon != sCustomBossEntity) {
        MGBA_Warnf("[BossFaint] Entity mismatch - not our custom boss, returning");
        return;
    }

    MGBA_Warnf("[BossFaint] Boss defeated! Spawning stairs at (%d, %d)", sStairsSpawnX, sStairsSpawnY);
    DungeonSeedOverrides_RegisterBossEntity(NULL);

    bossFight = DungeonFloorSpawns_GetBossFightConfig();
    if (bossFight == NULL) {
        MGBA_Warnf("[BossFaint] ERROR: bossFight is NULL!");
        return;
    }

    // Spawn stairs at marked position
    tile = GetTileMut(sStairsSpawnX, sStairsSpawnY);
    if (tile != NULL) {
        MGBA_Warnf("[BossFaint] Setting stairs flags on tile at (%d, %d)", sStairsSpawnX, sStairsSpawnY);
        tile->terrainFlags |= TERRAIN_TYPE_STAIRS;
        tile->spawnOrVisibilityFlags.spawn |= SPAWN_FLAG_STAIRS;
        tile->spawnOrVisibilityFlags.spawn &= ~(SPAWN_FLAG_ITEM);
        gDungeon->stairsSpawn.x = sStairsSpawnX;
        gDungeon->stairsSpawn.y = sStairsSpawnY;

        // Render the stairs tile
        MGBA_Warnf("[BossFaint] Rendering stairs tile");
        sub_80498A8(sStairsSpawnX, sStairsSpawnY);
        sub_8049BB0(sStairsSpawnX, sStairsSpawnY);
        MGBA_Warnf("[BossFaint] Stairs spawned successfully!");
    } else {
        MGBA_Warnf("[BossFaint] ERROR: tile is NULL at (%d, %d)!", sStairsSpawnX, sStairsSpawnY);
    }

    // Drop loot if configured
    if (bossFight->dropItem != ITEM_NOTHING) {
        ItemIdToItem(&item, bossFight->dropItem, 0);
        dropPos.x = sStairsSpawnX;
        dropPos.y = sStairsSpawnY + 1;  // One tile in front of stairs
        SpawnItem(&dropPos, &item, TRUE);
    }

    // Update minimap and visibility
    UpdateTrapsVisibility();
    UpdateMinimap();
}
