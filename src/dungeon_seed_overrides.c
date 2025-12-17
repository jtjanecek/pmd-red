#include "global.h"
#include "dungeon_seed_overrides.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "constants/rescue_dungeon_id.h"
#include "constants/bg_music.h"
#include "constants/item.h"
#include "constants/difficulty.h"
#include "constants/trap.h"
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
#include "dungeon_info.h"
#include "run_dungeon.h"
#include "main_loops.h"
#include "mgba_log.h"
#include "dungeon_generation.h"
#include "pokemon.h"
#include "items.h"
#include "structs/map.h"
#include "type_selection.h"

#define SEEDED_TILESET_COUNT 75  // Max valid tileset ID (gNaturePowerCalledMoves uses max 74)
#define SEEDED_ITEM_DENSITY_MIN 3
#define SEEDED_ITEM_DENSITY_MAX 6
#define SEEDED_MIN_FLOORS 3
#define SEEDED_MAX_FLOORS 100
#define SEEDED_MIN_SPAWNS 6
#define SEEDED_MAX_SPAWNS 16
#define SEEDED_FIXED_SPAWN_COUNT 10
#define SEEDED_MAIN_TYPE_PERCENT 80
#define SEEDED_DUNGEON_NAME_MAX_LEN 32
#define SEEDED_PREFIX_BUFFER_LEN 16
#define SEEDED_TRAP_DENSITY_DEFAULT 15
#define SEEDED_TRAP_DENSITY_SUPER 56
#define SEEDED_TRAP_PERCENT_SUPER 56
#define SEEDED_FLOOR_WEATHER_CHANCE_PERCENT 20

#define BOSS_SECONDARY_LOOT_LEFT ITEM_ORAN_BERRY
#define BOSS_SECONDARY_LOOT_RIGHT ITEM_MAX_ELIXIR

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

typedef struct {
    u8 layout;
    s8 minRooms;
    s8 maxRooms;
    const char *label;
} SeededLayoutOption;

typedef struct {
    u8 weightPercent;
    SeededLayoutOption option;
} SeededWeightedLayoutOption;

typedef struct {
    u8 primaryLayoutPercent;
    u8 secondaryTerrainPercent;
    SeededLayoutOption primaryLayout;
    SeededWeightedLayoutOption alternateLayouts[5];
    u8 floorConnectivityMin;
    u8 floorConnectivityMax;
    u8 extraHallwaysMin;
    u8 extraHallwaysMax;
    u8 allowDeadEndsPercent;
} SeededFloorGenerationConfig;

typedef struct {
    u8 mainType;
    u32 spawnMask;
} TilesetTypeConfig;

static void ClearFloorOverrides(DungeonSeedFloorOverrides *result);
static DungeonSeedRng DungeonSeedRng_Init(s32 seed, u8 dungeonId, s32 floorId, u32 salt);
static u32 DungeonSeedRng_Next(DungeonSeedRng *rng);
static s32 DungeonSeedRng_NextRange(DungeonSeedRng *rng, s32 min, s32 max);
static void ApplySeededFloorProperties(FloorProperties *floorProps, s32 seed, u8 dungeonId, s32 floorId);
static s32 GetDungeonNumberForFloorScaling(u8 dungeonId);
static u8 SelectMinionFormation(s32 seed, u8 dungeonId, s32 floorId);
static u8 SelectTileset(s32 floorId);
static SeededLayoutOption SelectAlternateLayout(DungeonSeedRng *rng);
static s8 SelectRoomDensity(DungeonSeedRng *rng, const SeededLayoutOption *option);
static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result);
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed);
static void BuildUniformTrapTable(u16 *trapTable);
static bool8 SelectWeatherForType(u8 dungeonType, DungeonSeedRng *rng, u8 *weatherOut);
static void MaybeApplyFloorWeather(DungeonSeedFloorOverrides *result, s32 seed, u8 dungeonId, s32 floorId);
static const SeedSpeciesPool* GetBossPool(s32 floorId);
static u16 SelectRandomLoot(DungeonSeedRng *rng, s32 floorId);
static bool8 TryGetTypeSelectionBoss(s16 *bossSpecies);
static bool8 GetTypeBossMinions(s16 bossSpecies, s16 *minionsOut, u8 *minionCountOut);
static bool8 TrySpawnBossLoot(u16 itemId, s32 x, s32 y, const char *label);
static bool8 GetTypeBossMoves(s16 bossSpecies, u16 *movesOut);
static bool8 GetTypeBossMinionMoves(s16 bossSpecies, u16 minionMovesOut[][MAX_MON_MOVES], bool8 *minionHasCustomMovesOut);
static const BossWeatherConfig *GetBossWeatherConfigForSpecies(s16 species);
static void MaybeApplyBossWeather(BossFightConfig *bossFight, DungeonSeedRng *rng);
static bool8 IsBossSpecies(s16 species);
static bool8 SpeciesMatchesTypeMask(s16 species, u32 typeMask);
static s32 BuildSpawnCandidates(u32 typeMask, s16 *out, s32 outCapacity);
static u8 GetMainTypeForTileset(u8 tileset);
static u32 GetCombinedSpawnMask(u8 tileset, u32 mainTypeMask);
static s16 SelectSpeciesFromPool(DungeonSeedRng *rng, s16 *pool, s32 *poolCount, bool8 *selectedFlags);
static s16 SelectGenericSpecies(DungeonSeedRng *rng, bool8 *selectedFlags, bool8 *loggedFallback, u32 maskForLog);

static const u8 sItemLimitsByDifficulty[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = INVENTORY_SIZE,
    [DIFFICULTY_HARD] = 10,
    [DIFFICULTY_NIGHTMARE] = 5,
};

static const s16 sRecruitBaseChancePercent[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = 10,
    [DIFFICULTY_HARD] = 5,
    [DIFFICULTY_NIGHTMARE] = 1,
};

static const s16 sRecruitFriendBowBonusPercent[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = 5,
    [DIFFICULTY_HARD] = 3,
    [DIFFICULTY_NIGHTMARE] = 1,
};

static const SeededFloorGenerationConfig sSeededFloorGenConfig = {
    .primaryLayoutPercent = 85,
    .secondaryTerrainPercent = 20,
    .primaryLayout = {LAYOUT_LARGE, 8, 14, "Large"},
    .alternateLayouts = {
        {25, {LAYOUT_OUTER_RING, 8, 17, "OuterRing"}},
        {25, {LAYOUT_CROSSROADS, 8, 14, "Crossroads"}},
        {25, {LAYOUT_BEETLE, 5, 22, "Beetle"}},
        {20, {LAYOUT_CROSS, 5, 17, "Cross"}},
        {5,  {LAYOUT_OUTER_ROOMS, 5, 7, "OuterRooms"}},
    },
    .floorConnectivityMin = 10,
    .floorConnectivityMax = 50,
    .extraHallwaysMin = 0,
    .extraHallwaysMax = 50,
    .allowDeadEndsPercent = 50,
};

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

// Spawn type masks (Spawn Types column only) and main type per tileset.
// Source: rogue_files/tileset_types.csv. Bit position matches TYPE_*.
static const TilesetTypeConfig sTilesetTypeConfig[SEEDED_TILESET_COUNT] = {
    [0] = {TYPE_NONE, 0x00000000},
    [1] = {TYPE_GRASS, 0x00001002},
    [2] = {TYPE_NORMAL, 0x00002200},
    [3] = {TYPE_PSYCHIC, 0x00000002},
    [4] = {TYPE_DARK, 0x00001310},
    [5] = {TYPE_FIGHTING, 0x00022000},
    [6] = {TYPE_FLYING, 0x00000200},
    [7] = {TYPE_GHOST, 0x00001102},
    [8] = {TYPE_FIGHTING, 0x00000006},
    [9] = {TYPE_ICE, 0x00000002},
    [10] = {TYPE_DRAGON, 0x00000402},
    [11] = {TYPE_GRASS, 0x00001400},
    [12] = {TYPE_GHOST, 0x00010900},
    [13] = {TYPE_GROUND, 0x00002002},
    [14] = {TYPE_BUG, 0x00000212},
    [15] = {TYPE_ICE, 0x00020008},
    [16] = {TYPE_ICE, 0x00020008},
    [17] = {TYPE_ROCK, 0x00000206},
    [18] = {TYPE_ICE, 0x00020008},
    [19] = {TYPE_STEEL, 0x00000048},
    [20] = {TYPE_GRASS, 0x00001002},
    [21] = {TYPE_GROUND, 0x00002002},
    [22] = {TYPE_FIRE, 0x00022200},
    [23] = {TYPE_NORMAL, 0x00001090},
    [24] = {TYPE_STEEL, 0x00000048},
    [25] = {TYPE_NORMAL, 0x00000210},
    [26] = {TYPE_BUG, 0x00000410},
    [27] = {TYPE_FLYING, 0x00000022},
    [28] = {TYPE_PSYCHIC, 0x00004000},
    [29] = {TYPE_ELECTRIC, 0x00000402},
    [30] = {TYPE_STEEL, 0x00000800},
    [31] = {TYPE_GROUND, 0x00003000},
    [32] = {TYPE_WATER, 0x00001110},
    [33] = {TYPE_POISON, 0x00010008},
    [34] = {TYPE_DRAGON, 0x00000400},
    [35] = {TYPE_FLYING, 0x0000C000},
    [36] = {TYPE_ICE, 0x00008400},
    [37] = {TYPE_GROUND, 0x00002800},
    [38] = {TYPE_ROCK, 0x00000202},
    [39] = {TYPE_ROCK, 0x00000202},
    [40] = {TYPE_ICE, 0x0000000A},
    [41] = {TYPE_GRASS, 0x0000000A},
    [42] = {TYPE_ELECTRIC, 0x00000002},
    [43] = {TYPE_ELECTRIC, 0x00000400},
    [44] = {TYPE_GROUND, 0x00001406},
    [45] = {TYPE_DRAGON, 0x00000042},
    [46] = {TYPE_FIRE, 0x00000002},
    [47] = {TYPE_ICE, 0x00000002},
    [48] = {TYPE_FIRE, 0x00022000},
    [49] = {TYPE_WATER, 0x00000100},
    [50] = {TYPE_ROCK, 0x00000210},
    [51] = {TYPE_GRASS, 0x00001002},
    [52] = {TYPE_GRASS, 0x00001002},
    [53] = {TYPE_GRASS, 0x00001002},
    [54] = {TYPE_WATER, 0x00000140},
    [55] = {TYPE_DRAGON, 0x00008000},
    [56] = {TYPE_FLYING, 0x00000212},
    [57] = {TYPE_POISON, 0x00006800},
    [58] = {TYPE_PSYCHIC, 0x00008200},
    [59] = {TYPE_FIRE, 0x00002200},
    [60] = {TYPE_NORMAL, 0x00000490},
    [61] = {TYPE_DARK, 0x00000320},
    [62] = {TYPE_BUG, 0x00000012},
    [63] = {TYPE_GRASS, 0x0000000A},
    [64] = {TYPE_NONE, 0x00000000},
    [65] = {TYPE_NONE, 0x00000000},
    [66] = {TYPE_NONE, 0x00000000},
    [67] = {TYPE_NONE, 0x00000000},
    [68] = {TYPE_NONE, 0x00000000},
    [69] = {TYPE_NONE, 0x00000000},
    [70] = {TYPE_NONE, 0x00000000},
    [71] = {TYPE_NONE, 0x00000000},
    [72] = {TYPE_NONE, 0x00000000},
    [73] = {TYPE_NONE, 0x00000000},
    [74] = {TYPE_NONE, 0x00000000},
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
    // Use a seeded trap density and a uniform trap table; specific floors can override density below
    result->trapDensityOverride = SEEDED_TRAP_DENSITY_DEFAULT;
    result->hasTrapTable = TRUE;
    BuildUniformTrapTable(result->trapSpawnChances);
    {
        s32 superTrapFloor = DungeonSeedOverrides_GetSuperTrapFloor(dungeonId, seed);
        s32 superTrapFloorId = GetDungeonStartingFloor(dungeonId) + superTrapFloor + 1; // Floors are 1-indexed in mapparam

        if (floorId == superTrapFloorId) {
            result->trapDensityOverride = SEEDED_TRAP_DENSITY_SUPER;
            result->trapDensityPercent = SEEDED_TRAP_PERCENT_SUPER;
            MGBA_Warnf("[Traps] SuperTrap floor: dungeon=%d floor=%d density=%d percent=%d",
                       dungeonId, floorId, result->trapDensityOverride, result->trapDensityPercent);
        }
    }

    // NEW: Procedurally generate boss fight configuration
    PopulateBossFightConfig(result, &rng, dungeonId, floorId, seed);
    MaybeApplyFloorWeather(result, seed, dungeonId, floorId);

    // If boss fight enabled, use boss tileset; otherwise normal generation
    if (result->bossFight.enabled) {
        // Use the boss room's tileset (set in PopulateBossFightConfig) for the entire floor
        // UNLESS it's 0 (special value meaning "keep normal tileset" for custom arenas)
        if (result->bossFight.roomTileset != 0) {
            result->tileset = result->bossFight.roomTileset;
        }
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

void DungeonSeedOverrides_ApplyFloorProperties(FloorProperties *floorProps, s32 seed, u8 dungeonId, s32 floorId)
{
    ApplySeededFloorProperties(floorProps, seed, dungeonId, floorId);
}

static s32 GetDungeonNumberForFloorScaling(u8 dungeonId)
{
    s32 i;

    if (dungeonId >= NUM_DUNGEONS)
        return 1;

    for (i = 0; i < SEQUENTIAL_DUNGEON_COUNT; i++) {
        u8 listDungeonId = RescueDungeonToDungeonId(sSequentialDungeonList[i]);
        if (listDungeonId == dungeonId)
            return i + 1; // 1-indexed progression number
    }

    // Fallback: treat unknown dungeons as the first slot
    return 1;
}

s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId)
{
    s32 dungeonNumber = GetDungeonNumberForFloorScaling(dungeonId);
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, dungeonNumber, 0x464C4354); // "FLCT"
    s32 multiplier = DungeonSeedRng_NextRange(&rng, 2, 6); // 2-5 inclusive
    s32 desiredFloors = 6 + multiplier * (dungeonNumber - 1);
    s32 floorCount;

    if (desiredFloors > 99)
        desiredFloors = 99;

    // Engine uses (final floor number + 1); add 1 so visible floor total matches desiredFloors.
    floorCount = desiredFloors + 1;

    if (floorCount > SEEDED_MAX_FLOORS)
        floorCount = SEEDED_MAX_FLOORS;

    MGBA_Warnf("[FloorCount] seed=%d dungeon=%d number=%d mult=%d floors=%d (engineCount=%d)",
               seed, dungeonId, dungeonNumber, multiplier, desiredFloors, floorCount);
    return floorCount;
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

s32 DungeonSeedOverrides_GetItemLimit(void)
{
    u32 difficulty = GetGameDifficultySetting();

    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    return sItemLimitsByDifficulty[difficulty];
}

s32 DungeonSeedOverrides_ApplyItemLimit(void)
{
    s32 seed;
    s32 limit;
    s32 removed = 0;
    s32 kept = 0;
    s32 i;
    s32 startCount;
    u8 dungeonId;

    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return 0;

    if (gDungeon == NULL)
        return 0;

    dungeonId = gDungeon->unk644.dungeonLocation.id;
    limit = GetMaxItemsAllowed(dungeonId);
    if (limit >= INVENTORY_SIZE || limit <= 0)
        return 0;

    startCount = GetNumberOfFilledInventorySlots();

    MGBA_Warnf("[ApplyItemLimit] BEFORE: startCount=%d limit=%d", startCount, limit);
    // First pass: clean up any obviously broken items before counting
    for (i = 0; i < INVENTORY_SIZE; i++) {
        Item *item = &gTeamInventoryRef->teamItems[i];
        if (item->flags & ITEM_FLAG_EXISTS) {
            MGBA_Warnf("[ApplyItemLimit]   [%d] id=%d flags=0x%02x quantity=%d", i, item->id, item->flags, item->quantity);
            // If an item is flagged as existing but has no valid ID, it's broken - clean it up
            if (item->id == ITEM_NOTHING) {
                MGBA_Warnf("[ApplyItemLimit] Cleaning up broken item at slot %d (EXISTS but id=NOTHING)", i);
                ZeroOutItem(item);
            }
        }
    }

    // Recount after cleanup
    startCount = GetNumberOfFilledInventorySlots();

    for (i = 0; i < INVENTORY_SIZE; i++) {
        Item *item = &gTeamInventoryRef->teamItems[i];
        // Only process real items (EXISTS flag and valid ID)
        // Ghost items have EXISTS flag but id=ITEM_NOTHING
        if ((item->flags & ITEM_FLAG_EXISTS) && item->id != ITEM_NOTHING) {
            if (kept < limit) {
                kept++;
            }
            else {
                MGBA_Warnf("[ApplyItemLimit] Removing item at slot %d (id=%d)", i, item->id);
                ZeroOutItem(item);
                removed++;
            }
        }
        else if (item->flags & ITEM_FLAG_EXISTS) {
            // Clean up ghost items (EXISTS flag but invalid ID)
            MGBA_Warnf("[ApplyItemLimit] Cleaning up ghost item at slot %d (id=%d)", i, item->id);
            ZeroOutItem(item);
        }
    }

    if (removed > 0) {
        FillInventoryGaps();
    }

    MGBA_Warnf("[ApplyItemLimit] AFTER: kept=%d removed=%d finalCount=%d", kept, removed, GetNumberOfFilledInventorySlots());

    if (removed > 0) {
        MGBA_Infof("[ItemLimit] Trimmed inventory from %d to %d (removed %d)", startCount, kept, removed);
    }
    return removed;
}

void DungeonSeedOverrides_GetRecruitOverride(DungeonRecruitOverride *result)
{
    u32 difficulty;

    if (result == NULL)
        return;

    difficulty = GetGameDifficultySetting();
    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    result->baseRecruitPercent = sRecruitBaseChancePercent[difficulty];
    result->friendBowBonusPercent = sRecruitFriendBowBonusPercent[difficulty];
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
    result->trapDensityOverride = -1;
    result->trapDensityPercent = -1;
    result->hasTrapTable = FALSE;
    for (i = 0; i < NUM_TRAPS; i++) {
        result->trapSpawnChances[i] = 0;
    }
    result->weather = WEATHER_CLEAR;
    result->applyWeather = FALSE;
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

static void BuildUniformTrapTable(u16 *trapTable)
{
    s32 i;
    u32 accum = 0;
    u32 base = 10000 / NUM_TRAPS;
    u32 remainder = 10000 % NUM_TRAPS;

    if (trapTable == NULL)
        return;

    for (i = 0; i < NUM_TRAPS; i++) {
        u32 increment = base;
        if (remainder > 0) {
            increment++;
            remainder--;
        }
        accum += increment;
        trapTable[i] = (u16)accum;
    }

    // Safety: force the last entry to 10000 to match the expected cumulative cap
    trapTable[NUM_TRAPS - 1] = 10000;
}

static bool8 SelectWeatherForType(u8 dungeonType, DungeonSeedRng *rng, u8 *weatherOut)
{
    const TypeWeatherPool *pool;
    s32 choice;

    if (weatherOut == NULL || rng == NULL)
        return FALSE;
    if (dungeonType >= NUM_TYPES)
        return FALSE;

    pool = &gTypeWeatherTable[dungeonType];
    if (pool->count == 0)
        return FALSE;

    choice = DungeonSeedRng_NextRange(rng, 0, pool->count);
    if (choice < 0 || choice >= pool->count)
        return FALSE;

    *weatherOut = pool->weathers[choice];
    return TRUE;
}

static void MaybeApplyFloorWeather(DungeonSeedFloorOverrides *result, s32 seed, u8 dungeonId, s32 floorId)
{
    DungeonSeedRng rng;
    u8 dungeonType = TYPE_NONE;
    u8 weather = WEATHER_CLEAR;

    if (result == NULL)
        return;

    // Keep boss floors governed by their own weather rules.
    if (result->bossFight.enabled)
        return;

    if (TypeSelection_HasActiveType())
        dungeonType = TypeSelection_GetActiveType();
    else if (TypeSelection_HasCommittedType())
        dungeonType = TypeSelection_GetCommittedType();

    if (dungeonType == TYPE_NONE || dungeonType >= NUM_TYPES)
        return;
    if (gTypeWeatherTable[dungeonType].count == 0)
        return;

    // Deterministic 20% roll per floor; independent RNG salt to avoid
    // perturbing other seeded systems.
    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0x57454154); // "WEAT"
    if (DungeonSeedRng_NextRange(&rng, 0, 100) >= SEEDED_FLOOR_WEATHER_CHANCE_PERCENT)
        return;
    if (!SelectWeatherForType(dungeonType, &rng, &weather))
        return;
    if (weather >= WEATHER_RANDOM)
        return;

    result->applyWeather = TRUE;
    result->weather = weather;
    MGBA_Warnf("[Weather] Floor override: seed=%d dungeon=%d floor=%d type=%d weather=%d",
               seed, dungeonId, floorId, dungeonType, weather);
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

static s8 SelectRoomDensity(DungeonSeedRng *rng, const SeededLayoutOption *option)
{
    s32 minRooms;
    s32 maxRooms;
    s32 selected;

    if (rng == NULL || option == NULL)
        return -SEEDED_MIN_SPAWNS;

    minRooms = option->minRooms;
    maxRooms = option->maxRooms;
    if (maxRooms < minRooms) {
        s32 tmp = minRooms;
        minRooms = maxRooms;
        maxRooms = tmp;
    }
    if (minRooms < 2)
        minRooms = 2;
    if (maxRooms < minRooms)
        maxRooms = minRooms;

    selected = DungeonSeedRng_NextRange(rng, minRooms, maxRooms + 1);
    return (s8)(-selected);
}

static SeededLayoutOption SelectAlternateLayout(DungeonSeedRng *rng)
{
    s32 i;
    u32 roll;
    u32 accum = 0;
    SeededLayoutOption choice = sSeededFloorGenConfig.alternateLayouts[0].option;

    if (rng == NULL)
        return choice;

    roll = (u32)DungeonSeedRng_NextRange(rng, 0, 100);
    for (i = 0; i < (s32)ARRAY_COUNT(sSeededFloorGenConfig.alternateLayouts); i++) {
        const SeededWeightedLayoutOption *entry = &sSeededFloorGenConfig.alternateLayouts[i];
        accum += entry->weightPercent;
        if (roll < accum) {
            choice = entry->option;
            break;
        }
    }

    return choice;
}

static void ApplySeededFloorProperties(FloorProperties *floorProps, s32 seed, u8 dungeonId, s32 floorId)
{
    DungeonSeedRng rng;
    SeededLayoutOption layoutChoice;
    bool8 useAlternate;
    bool8 allowSecondaryTerrain = FALSE;
    u32 roll;
    u8 roomFlags;
    s32 roomCountForLog;
    const char *layoutLabel;
    u32 visRoll;
    u8 difficulty;
    u8 visibility = 0;

    if (floorProps == NULL)
        return;

    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0x464C4F52); // "FLOR"
    roll = (u32)DungeonSeedRng_NextRange(&rng, 0, 100);
    useAlternate = (roll >= sSeededFloorGenConfig.primaryLayoutPercent);

    if (!useAlternate) {
        layoutChoice = sSeededFloorGenConfig.primaryLayout;
        allowSecondaryTerrain = (DungeonSeedRng_NextRange(&rng, 0, 100) < sSeededFloorGenConfig.secondaryTerrainPercent);
    } else {
        layoutChoice = SelectAlternateLayout(&rng);
        allowSecondaryTerrain = FALSE;
    }

    floorProps->layout = layoutChoice.layout;
    floorProps->roomDensity = SelectRoomDensity(&rng, &layoutChoice);

    roomFlags = floorProps->roomFlags & ~(ROOM_FLAG_ALLOW_SECONDARY_TERRAIN);
    if (allowSecondaryTerrain)
        roomFlags |= ROOM_FLAG_ALLOW_SECONDARY_TERRAIN;
    floorProps->roomFlags = roomFlags;

    floorProps->floorConnectivity = (u8)DungeonSeedRng_NextRange(&rng,
                                                                 sSeededFloorGenConfig.floorConnectivityMin,
                                                                 sSeededFloorGenConfig.floorConnectivityMax + 1);
    floorProps->numExtraHallways = (u8)DungeonSeedRng_NextRange(&rng,
                                                                sSeededFloorGenConfig.extraHallwaysMin,
                                                                sSeededFloorGenConfig.extraHallwaysMax + 1);
    floorProps->allowDeadEnds = (DungeonSeedRng_NextRange(&rng, 0, 100) < sSeededFloorGenConfig.allowDeadEndsPercent);

    floorProps->monsterHouseChance = 0;
    floorProps->kecleonShopChance = 0;
    floorProps->buriedItemDensity = 0;
    difficulty = GetGameDifficultySetting();
    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    visRoll = (u32)DungeonSeedRng_NextRange(&rng, 0, 100);
    switch (difficulty) {
        case DIFFICULTY_HARD:
            if (visRoll < 70)
                visibility = 0;
            else if (visRoll < 85)
                visibility = 1;
            else
                visibility = 2;
            break;
        case DIFFICULTY_NIGHTMARE:
            if (visRoll < 60)
                visibility = 0;
            else if (visRoll < 80)
                visibility = 1;
            else
                visibility = 2;
            break;
        case DIFFICULTY_NORMAL:
        default:
            if (visRoll < 80)
                visibility = 0;
            else if (visRoll < 90)
                visibility = 1;
            else
                visibility = 2;
            break;
    }
    floorProps->visibilityRange = visibility;
    switch (difficulty) {
        case DIFFICULTY_HARD:
            floorProps->monsterHouseChance = 4;
            break;
        case DIFFICULTY_NIGHTMARE:
            floorProps->monsterHouseChance = 6;
            break;
        case DIFFICULTY_NORMAL:
        default:
            floorProps->monsterHouseChance = 2;
            break;
    }
    switch (difficulty) {
        case DIFFICULTY_HARD:
            floorProps->itemStickyChance = 2;
            break;
        case DIFFICULTY_NIGHTMARE:
            floorProps->itemStickyChance = 5;
            break;
        case DIFFICULTY_NORMAL:
        default:
            floorProps->itemStickyChance = 1;
            break;
    }
    {
        u8 budget = 0;
        if (DungeonSeedRng_NextRange(&rng, 0, 100) >= 50) {
            budget = (u8)DungeonSeedRng_NextRange(&rng, 1, 11);
        }
        floorProps->secondaryStructuresBudget = budget;
    }
    {
        u8 itemDensity = (u8)DungeonSeedRng_NextRange(&rng, SEEDED_ITEM_DENSITY_MIN, SEEDED_ITEM_DENSITY_MAX + 1);
        floorProps->itemDensity = itemDensity;
    }
    floorProps->standaloneLakeDensity = 0;

    roomCountForLog = (floorProps->roomDensity < 0) ? -floorProps->roomDensity : floorProps->roomDensity;
    layoutLabel = (layoutChoice.label != NULL) ? layoutChoice.label : "unknown";
    MGBA_Warnf("[FloorProps] seed=%d dungeon=%d floor=%d alt=%d layout=%d (%s) rooms=%d allowSecondary=%d roll=%u conn=%d extra=%d deadEnds=%d secondaryBudget=%d vis=%d visRoll=%u diff=%d sticky=%d itemDensity=%d",
               seed,
               dungeonId,
               floorId,
               useAlternate,
               layoutChoice.layout,
               layoutLabel,
               roomCountForLog,
               allowSecondaryTerrain,
               roll,
               floorProps->floorConnectivity,
               floorProps->numExtraHallways,
               floorProps->allowDeadEnds,
               floorProps->secondaryStructuresBudget,
               floorProps->visibilityRange,
               visRoll,
               difficulty,
               floorProps->itemStickyChance,
               floorProps->itemDensity);
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

static u8 GetMainTypeForTileset(u8 tileset)
{
    u8 type = TYPE_NONE;

    if (TypeSelection_HasActiveType())
        type = TypeSelection_GetActiveType();
    else if (TypeSelection_HasCommittedType())
        type = TypeSelection_GetCommittedType();

    if (type > TYPE_NONE && type < NUM_TYPES)
        return type;

    if (tileset < SEEDED_TILESET_COUNT)
        type = sTilesetTypeConfig[tileset].mainType;

    if (type > TYPE_NONE && type < NUM_TYPES)
        return type;

    return TYPE_NONE;
}

static u32 GetCombinedSpawnMask(u8 tileset, u32 mainTypeMask)
{
    u32 mask = mainTypeMask;

    if (tileset < SEEDED_TILESET_COUNT)
        mask |= sTilesetTypeConfig[tileset].spawnMask;

    return mask;
}

static s16 SelectSpeciesFromPool(DungeonSeedRng *rng, s16 *pool, s32 *poolCount, bool8 *selectedFlags)
{
    if (rng == NULL || pool == NULL || poolCount == NULL || selectedFlags == NULL)
        return MONSTER_NONE;

    while (*poolCount > 0) {
        s32 idx = DungeonSeedRng_NextRange(rng, 0, *poolCount);
        s16 species = pool[idx];
        pool[idx] = pool[*poolCount - 1];
        (*poolCount)--;

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (selectedFlags[species])
            continue;

        selectedFlags[species] = TRUE;
        return species;
    }

    return MONSTER_NONE;
}

static s16 SelectGenericSpecies(DungeonSeedRng *rng, bool8 *selectedFlags, bool8 *loggedFallback, u32 maskForLog)
{
    const SeedSpeciesPool *pool;
    s32 attempts = 0;

    if (rng == NULL || selectedFlags == NULL)
        return MONSTER_NONE;

    pool = &sSpeciesPools[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSpeciesPools))];
    while (attempts < 8) {
        s16 species = pool->species[DungeonSeedRng_NextRange(rng, 0, pool->count)];
        attempts++;

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (IsBossSpecies(species))
            continue;
        if (selectedFlags[species])
            continue;

        selectedFlags[species] = TRUE;
        if (loggedFallback != NULL && !*loggedFallback) {
            MGBA_Warnf("[SeedOverrides] Spawn fallback to generic pool (mask=0x%08x)", maskForLog);
            *loggedFallback = TRUE;
        }
        return species;
    }

    return MONSTER_NONE;
}

static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId)
{
    s32 entryCount;
    s32 i;
    u8 tileset = 0;
    u8 mainType = TYPE_NONE;
    u32 mainTypeMask = 0;
    u32 spawnTypeMask = 0;
    u32 combinedMask = 0;
    s16 mainCandidates[MONSTER_MAX];
    s16 spawnCandidates[MONSTER_MAX];
    s16 combinedCandidates[MONSTER_MAX];
    s32 mainCandidateCount = 0;
    s32 spawnCandidateCount = 0;
    s32 combinedCandidateCount = 0;
    s32 mainQuota;
    s32 spawnQuota;
    bool8 selectedFlags[MONSTER_MAX];
    bool8 useBulbasaurOnly = FALSE;
    bool8 loggedFallback = FALSE;
    bool8 loggedInvalid = FALSE;

    if (result != NULL)
        tileset = result->tileset;

    entryCount = SEEDED_FIXED_SPAWN_COUNT;
    if (entryCount > MONSTER_SPAWNS_ARR_COUNT)
        entryCount = MONSTER_SPAWNS_ARR_COUNT;

    mainType = GetMainTypeForTileset(tileset);
    if (mainType > TYPE_NONE && mainType < NUM_TYPES)
        mainTypeMask = (1u << mainType);

    if (tileset < SEEDED_TILESET_COUNT)
        spawnTypeMask = sTilesetTypeConfig[tileset].spawnMask & ~mainTypeMask;

    combinedMask = GetCombinedSpawnMask(tileset, mainTypeMask);

    if (mainTypeMask != 0)
        mainCandidateCount = BuildSpawnCandidates(mainTypeMask, mainCandidates, ARRAY_COUNT(mainCandidates));
    if (spawnTypeMask != 0)
        spawnCandidateCount = BuildSpawnCandidates(spawnTypeMask, spawnCandidates, ARRAY_COUNT(spawnCandidates));
    if (combinedMask != 0)
        combinedCandidateCount = BuildSpawnCandidates(combinedMask, combinedCandidates, ARRAY_COUNT(combinedCandidates));
    if (combinedMask != 0 && combinedCandidateCount == 0)
        useBulbasaurOnly = TRUE;

    for (i = 0; i < MONSTER_MAX; i++)
        selectedFlags[i] = FALSE;

    if (mainTypeMask == 0) {
        mainQuota = 0;
    } else {
        mainQuota = (entryCount * SEEDED_MAIN_TYPE_PERCENT + 99) / 100;
        if (mainQuota > entryCount)
            mainQuota = entryCount;
    }
    spawnQuota = entryCount - mainQuota;

    MGBA_Infof("[SeedOverrides] Spawn selection tileset=%d mainType=%d mainMask=0x%08x spawnMask=0x%08x combined=0x%08x mainCandidates=%d spawnCandidates=%d bulbaOnly=%d mainQuota=%d spawnQuota=%d",
               tileset,
               mainType,
               mainTypeMask,
               spawnTypeMask,
               combinedMask,
               mainCandidateCount,
               spawnCandidateCount,
               useBulbasaurOnly,
               mainQuota,
               spawnQuota);

    for (i = 0; i < entryCount; i++) {
        s16 species;
        s32 baseLevel;
        s32 levelVariance;
        s32 level;
        bool8 fillingMain = (i < mainQuota);

        if (useBulbasaurOnly) {
            species = MONSTER_BULBASAUR;
            selectedFlags[species] = TRUE;
        } else if (fillingMain) {
            species = SelectSpeciesFromPool(rng, mainCandidates, &mainCandidateCount, selectedFlags);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromPool(rng, combinedCandidates, &combinedCandidateCount, selectedFlags);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromPool(rng, spawnCandidates, &spawnCandidateCount, selectedFlags);
        } else {
            species = SelectSpeciesFromPool(rng, spawnCandidates, &spawnCandidateCount, selectedFlags);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromPool(rng, combinedCandidates, &combinedCandidateCount, selectedFlags);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromPool(rng, mainCandidates, &mainCandidateCount, selectedFlags);
        }

        if (species == MONSTER_NONE)
            species = SelectGenericSpecies(rng, selectedFlags, &loggedFallback, combinedMask);

        if (species <= MONSTER_NONE || species >= MONSTER_MAX) {
            species = MONSTER_BULBASAUR;
            if (!loggedInvalid) {
                MGBA_Warnf("[SeedOverrides] Spawn invalid species, forcing Bulbasaur (entry=%d)", i);
                loggedInvalid = TRUE;
            }
            selectedFlags[species] = TRUE;
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
    MGBA_Warnf("[BossWeather] species=%d config=%p enabled=%d",
               bossFight->bossSpecies, config, config ? config->enabled : -1);
    if (config == NULL || !config->enabled)
        return;

    difficulty = GetGameDifficultySetting();
    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    chance = config->chance[difficulty];
    MGBA_Warnf("[BossWeather] difficulty=%d chance=%d scale=%d weather=%d",
               difficulty, chance, BOSS_WEATHER_CHANCE_SCALE, config->weather);
    if (chance == 0)
        return;

    if (chance >= BOSS_WEATHER_CHANCE_SCALE) {
        bossFight->applyWeather = TRUE;
        bossFight->weather = config->weather;
        MGBA_Warnf("[BossWeather] Applied (100%% chance): weather=%d", config->weather);
        return;
    }

    if (DungeonSeedRng_NextRange(rng, 0, BOSS_WEATHER_CHANCE_SCALE) < chance) {
        bossFight->applyWeather = TRUE;
        bossFight->weather = config->weather;
        MGBA_Warnf("[BossWeather] Applied (rolled): weather=%d", config->weather);
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

    // Select boss room based on dungeon type (not boss type!)
    {
        u8 dungeonType = TYPE_NONE;
        const char *roomName = "unknown";

        // Get the current dungeon's type
        if (TypeSelection_HasActiveType())
            dungeonType = TypeSelection_GetActiveType();
        else if (TypeSelection_HasCommittedType())
            dungeonType = TypeSelection_GetCommittedType();

        // Choose tileset and layout based on dungeon type
        if (dungeonType == TYPE_STEEL) {
            // Steel dungeon -> Use Articuno's boss room (Frosty Grotto)
            result->bossFight.roomTileset = 68;        // Frosty Grotto boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 5;     // Articuno arena (Room 5)
            roomName = "Articuno";
        }
        else if (dungeonType == TYPE_FIGHTING) {
            // Fighting dungeon -> Use Skarmory's boss room (Mt. Steel)
            result->bossFight.roomTileset = 64;        // Mt. Steel boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 1;     // Skarmory arena (Room 1)
            roomName = "Skarmory";
        }
        else if (dungeonType == TYPE_BUG) {
            // Bug dungeon -> Use Sinister Woods boss room (extracted pattern)
            result->bossFight.roomTileset = 65;        // Sinister Woods boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 2;     // Team Meanies arena (Room 2)
            roomName = "SinisterWoods";
        }
        else if (dungeonType == TYPE_ELECTRIC) {
            // Electric dungeon -> Use Mt. Thunder boss room (extracted pattern)
            result->bossFight.roomTileset = 66;        // Mt. Thunder boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 3;     // Zapdos arena (Room 3)
            roomName = "MtThunder";
        }
        else if (dungeonType == TYPE_FIRE) {
            // Fire dungeon -> Use Mt. Blaze boss room (extracted pattern)
            result->bossFight.roomTileset = 67;        // Mt. Blaze boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 4;     // Moltres arena (Room 4)
            roomName = "MtBlaze";
        }
        else if (dungeonType == TYPE_ICE) {
            // Ice dungeon -> Use Articuno's boss room with Mt. Freeze visuals
            result->bossFight.roomTileset = 69;        // Mt. Freeze boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 5;     // Articuno arena (Room 5)
            roomName = "MtFreeze";
        }
        else if (dungeonType == TYPE_GROUND) {
            // Ground dungeon -> Use Groudon's boss room (Magma Cavern)
            result->bossFight.roomTileset = 70;        // Magma Cavern boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 7;     // Groudon arena (Room 7)
            roomName = "MagmaCavern";
        }
        else if (dungeonType == TYPE_FLYING) {
            // Flying dungeon -> Use Rayquaza's boss room (Sky Tower)
            result->bossFight.roomTileset = 71;        // Sky Tower boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 8;     // Rayquaza arena (Room 8)
            roomName = "SkyTower";
        }
        else if (dungeonType == TYPE_ROCK) {
            // Rock dungeon -> Use Ho-Oh's boss room (Mt. Faraway)
            result->bossFight.roomTileset = 72;        // Mt. Faraway boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 15;    // Ho-Oh arena (Room 15)
            roomName = "MtFaraway";
        }
        else if (dungeonType == TYPE_PSYCHIC) {
            // Psychic dungeon -> Use Lugia's boss room (Silver Trench)
            result->bossFight.roomTileset = 73;        // Silver Trench boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 21;    // Lugia arena (Room 21)
            roomName = "SilverTrench";
        }
        else if (dungeonType == TYPE_WATER) {
            // Water dungeon -> Use Kyogre's boss room (Stormy Sea)
            result->bossFight.roomTileset = 74;        // Stormy Sea boss tileset
            result->bossFight.useFixedRoomLayout = TRUE;
            result->bossFight.fixedRoomNumber = 22;    // Kyogre arena (Room 22)
            roomName = "StormySea";
        }
        else {
            // Other types -> Use custom boss arena (NOT fixed room reuse!)
            // Use the normal dungeon tileset (already set in result->tileset at line 275)
            result->bossFight.roomTileset = 0;         // Special value: keep normal tileset
            result->bossFight.useFixedRoomLayout = FALSE;  // Custom procedural arena
            result->bossFight.fixedRoomNumber = 0;     // Not used
            roomName = "CustomArena";
        }

        // Fire dungeons: avoid the back-line minion formation so adds don't spawn behind the boss
        if (dungeonType == TYPE_FIRE && result->bossFight.minionFormation == MINION_FORMATION_BACK) {
            result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
        }
        // Electric dungeons: same constraint as Fire, keep minions off the back line
        if (dungeonType == TYPE_ELECTRIC && result->bossFight.minionFormation == MINION_FORMATION_BACK) {
            result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
        }
        // Flying dungeons: also avoid back-line minions so adds don't spawn behind the boss
        if (dungeonType == TYPE_FLYING && result->bossFight.minionFormation == MINION_FORMATION_BACK) {
            result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
        }
        // Steel dungeons: keep minions off the back line in the Articuno room
        if (dungeonType == TYPE_STEEL && result->bossFight.minionFormation == MINION_FORMATION_BACK) {
            result->bossFight.minionFormation = MINION_FORMATION_DEFAULT;
        }

        MGBA_Warnf("[BossRoom] dungeonType=%d tileset=%d useFixed=%d layout=%d room=%s",
                   dungeonType, result->bossFight.roomTileset,
                   result->bossFight.useFixedRoomLayout,
                   result->bossFight.fixedRoomNumber, roomName);
    }

    // Set behavior for boss identification
    result->bossFight.monsterBehavior = 0;  // Will define this constant later

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
    // where X is the progression number (# completed + 1), not the dungeon's array index
    if (dungeonIndex != -1) {
        s32 progressionNumber = 1;  // Start at 1 for the first dungeon

        // Count how many dungeons have been conquered before this one
        for (i = 0; i < dungeonIndex; i++) {
            if (RescueScenarioConquered(sSequentialDungeonList[i])) {
                progressionNumber++;
            }
        }

        if (typeLabel != NULL) {
            sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d) %s",
                          progressionNumber, SEQUENTIAL_DUNGEON_COUNT, typeLabel);
        } else {
            sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d)",
                          progressionNumber, SEQUENTIAL_DUNGEON_COUNT);
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

static bool8 TrySpawnBossLoot(u16 itemId, s32 x, s32 y, const char *label)
{
    DungeonPos pos;
    Tile *tile;
    Item item;
    s32 terrain;

    if (x < 0 || y < 0 || x >= DUNGEON_MAX_SIZE_X || y >= DUNGEON_MAX_SIZE_Y) {
        MGBA_Warnf("[BossFaint] Skipping %s loot at (%d, %d): out of bounds", label, x, y);
        return FALSE;
    }

    tile = GetTileMut(x, y);
    terrain = GetTerrainType(tile);
    if (terrain == TERRAIN_TYPE_WALL || (tile->terrainFlags & TERRAIN_TYPE_STAIRS) || tile->object != NULL) {
        MGBA_Warnf("[BossFaint] Skipping %s loot at (%d, %d): blocked (terrain=%d, stairs=%d, object=%p)",
                   label, x, y, terrain, (tile->terrainFlags & TERRAIN_TYPE_STAIRS) != 0, tile->object);
        return FALSE;
    }

    ItemIdToItem(&item, itemId, 0);
    pos.x = x;
    pos.y = y;
    SpawnItem(&pos, &item, TRUE);
    MGBA_Warnf("[BossFaint] Spawned %s loot (itemId=%d) at (%d, %d)", label, itemId, x, y);
    return TRUE;
}

// Handle boss defeat - spawn stairs and drop loot
void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon)
{
    const BossFightConfig *bossFight;
    Tile *tile;
    s32 dropX;
    s32 dropY;

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
        dropX = sStairsSpawnX;
        dropY = sStairsSpawnY + 1;  // One tile in front of stairs
        TrySpawnBossLoot(bossFight->dropItem, dropX, dropY, "primary");
        TrySpawnBossLoot(BOSS_SECONDARY_LOOT_LEFT, dropX - 1, dropY, "secondary-left");
        TrySpawnBossLoot(BOSS_SECONDARY_LOOT_RIGHT, dropX + 1, dropY, "secondary-right");
    }

    // Update minimap and visibility
    UpdateTrapsVisibility();
    UpdateMinimap();
}

s32 DungeonSeedOverrides_GetSuperTrapFloor(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 maxNonBossFloor;
    s32 kecleonFloor;
    s32 selected;

    if (floorCount < 2)
        return 0;

    // Avoid the boss floor (last floor) so traps don't conflict with boss rooms
    // floorCount is "final floor + 1" so subtract 2 to drop the boss layer
    maxNonBossFloor = floorCount - 2;
    if (maxNonBossFloor <= 0)
        return 0;

    // Draw once; if it collides with the Kecleon shop floor, bump deterministically
    kecleonFloor = DungeonSeedOverrides_GetKecleonFloor(dungeonId, seed);
    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x54524150); // "TRAP"
    selected = DungeonSeedRng_NextRange(&rng, 0, maxNonBossFloor);
    if (selected == kecleonFloor) {
        selected = (selected + 1) % maxNonBossFloor;
    }
    return selected;
}

s32 DungeonSeedOverrides_GetGuaranteedMonsterHouseFloor(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 maxNonBossFloor;
    s32 kecleonFloor;
    s32 superTrapFloor;
    s32 candidates[SEEDED_MAX_FLOORS];
    s32 candidateCount = 0;
    s32 i;

    if (floorCount < 2)
        return 0;

    maxNonBossFloor = floorCount - 2;
    if (maxNonBossFloor <= 0)
        return 0;

    kecleonFloor = DungeonSeedOverrides_GetKecleonFloor(dungeonId, seed);
    superTrapFloor = DungeonSeedOverrides_GetSuperTrapFloor(dungeonId, seed);

    // Build a list of eligible floors that aren't reserved by other guarantees
    for (i = 0; i < maxNonBossFloor && i < SEEDED_MAX_FLOORS; i++) {
        if (i == kecleonFloor || i == superTrapFloor)
            continue;
        candidates[candidateCount++] = i;
    }

    if (candidateCount == 0) {
        MGBA_Warnf("[MonsterHouse] No open floors for guaranteed Monster House (dungeon=%d maxNonBoss=%d kec=%d trap=%d)",
                   dungeonId, maxNonBossFloor, kecleonFloor, superTrapFloor);
        return 0;
    }

    if (candidateCount == 1)
        return candidates[0];

    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x4D4F4E48); // "MONH"
    rng.state ^= (u32)kecleonFloor * 0x27D4EB2D;
    rng.state ^= (u32)superTrapFloor * 0x45D9F3B;
    return candidates[DungeonSeedRng_NextRange(&rng, 0, candidateCount)];
}

// Deterministically select which floor (0-indexed) should have a Kecleon shop
// Returns a floor index between 0 and (maxFloors - 2), excluding the boss floor
s32 DungeonSeedOverrides_GetKecleonFloor(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 maxNonBossFloor;

    // Ensure we have at least 2 floors (one for Kecleon, one for boss)
    if (floorCount <= 2)
        return 0;

    // Boss is on the final floor (floorCount - 1), so Kecleon can be on 0 to (floorCount - 3)
    // floorCount is "final floor + 1", so subtract 2 to remove the boss layer
    maxNonBossFloor = floorCount - 2;  // Exclusive upper bound for range function

    // Use a dedicated salt for Kecleon shop placement
    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x4B45434C);
    return DungeonSeedRng_NextRange(&rng, 0, maxNonBossFloor);
}
