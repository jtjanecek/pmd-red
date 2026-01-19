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
#include "dungeon_util.h"
#include "run_dungeon.h"
#include "main_loops.h"
#include "mgba_log.h"
#include "dungeon_generation.h"
#include "pokemon.h"
#include "items.h"
#include "structs/str_items.h"
#include "structs/map.h"
#include "type_selection.h"
#include "rogue_item_tables.h"
#include "dungeon_random.h"
#include "string_format.h"
#include "dungeon_message.h"
#include "friend_area.h"
#include "structs/menu.h"

#define SEEDED_TILESET_COUNT 75  // Max valid tileset ID (gNaturePowerCalledMoves uses max 74)
#define SEEDED_ITEM_DENSITY_MIN 3
#define SEEDED_ITEM_DENSITY_MAX 6
#define SEEDED_MIN_FLOORS 3
#define SEEDED_MAX_FLOORS 100
#define SEEDED_MIN_SPAWNS 6
#define SEEDED_MAX_SPAWNS 16
#define SEEDED_FIXED_SPAWN_COUNT 32   // Max unique species per seeded dungeon (bounded by MONSTER_SPAWNS_ARR_COUNT)
#define SEEDED_SPAWN_TARGET_NUM 14    // Aim for ~1.4 species per floor (Purity Forest has ~1.39)
#define SEEDED_SPAWN_TARGET_DEN 10
#define SEEDED_MAIN_TYPE_PERCENT 80
#define SEEDED_LOW_BST_PERCENT 15
#define SEEDED_LOW_BST_FLAG_BYTES ((MONSTER_MAX + 7) / 8)
#define SEEDED_DUNGEON_NAME_MAX_LEN 32
#define SEEDED_DUNGEON_NAME_VERSION 2
#define SEEDED_PREFIX_BUFFER_LEN 16
#define SEEDED_TRAP_DENSITY_DEFAULT 15
#define SEEDED_TRAP_DENSITY_SUPER 56
#define SEEDED_TRAP_PERCENT_SUPER 56
#define SEEDED_FLOOR_WEATHER_CHANCE_PERCENT 20
#define SEEDED_SHOP_RARE_CHANCE_PERCENT 20
#define SEEDED_FLOOR_RARE_CHANCE_PERCENT 5

#define BOSS_SECONDARY_LOOT_LEFT ITEM_ORAN_BERRY
#define BOSS_SECONDARY_LOOT_RIGHT ITEM_MAX_ELIXIR

enum {
    BOSS_REWARD_NONE = 0,
    BOSS_REWARD_RARE_ITEMS = 1,
    BOSS_REWARD_MONEY = 2,
    BOSS_REWARD_RECRUIT = 3,
};

#define BOSS_REWARD_POKE_QUANTITY_3000 100

ALIGNED(4) static const u8 sBossRewardPrompt[] = _(
    "A strange voice eminates from somewhere...\n"
    "What reward would you like?"
);
ALIGNED(4) static const u8 sBossRewardRecruitFailText[] = _(
    "There is no space in the Friend Areas\n"
    "for a new recruit."
);
ALIGNED(4) static const u8 sBossRewardRecruitSuccessText[] = _(
    "{POKEMON_0} went to its Friend Area!"
);

static const MenuItem sBossRewardMenu[] = {
    { _("Rare items"), BOSS_REWARD_RARE_ITEMS },
    { _("Lots of money"), BOSS_REWARD_MONEY },
    { _("A recruit"), BOSS_REWARD_RECRUIT },
    { NULL, -1 },
};

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

static s32 GetSequentialDungeonCountForRun(void)
{
    s32 count = (s32)GetMaxDungeonsSetting();

    if (count <= 0 || count > SEQUENTIAL_DUNGEON_COUNT)
        count = SEQUENTIAL_DUNGEON_COUNT;

    return count;
}

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

typedef struct {
    s16 species;
    s16 start; // Inclusive, 0-based floor index within the dungeon
    s16 end;   // Inclusive, 0-based floor index within the dungeon
    u8 level;  // Cached level for this species band
} SeededSpawnRange;

typedef struct {
    bool8 valid;
    s32 seed;
    u8 dungeonId;
    u8 tileset;
    u32 combinedMask;
    s16 startFloorId;
    s16 floorCount;
    u8 rangeCount;
    SeededSpawnRange ranges[SEEDED_FIXED_SPAWN_COUNT];
} SeededSpawnRangeCache;

static void ClearFloorOverrides(DungeonSeedFloorOverrides *result);
static DungeonSeedRng DungeonSeedRng_Init(s32 seed, u8 dungeonId, s32 floorId, u32 salt);
static u32 DungeonSeedRng_Next(DungeonSeedRng *rng);
static s32 DungeonSeedRng_NextRange(DungeonSeedRng *rng, s32 min, s32 max);
static void ApplySeededFloorProperties(FloorProperties *floorProps, s32 seed, u8 dungeonId, s32 floorId);
static void ResetSeededItemState(void);
static void InitSeededItemState(s32 seed, u8 dungeonId, s32 floorId, bool8 isForcedKecleonFloor);
static u16 SelectItemFromPool(RogueItemPoolId poolId, DungeonSeedRng *rng);
static s32 GetDungeonNumberForFloorScaling(u8 dungeonId);
static s32 CalcSeededFloorCount(s32 seed, u8 dungeonId, s32 *desiredFloorsOut, s32 *multiplierOut);
static u8 SelectMinionFormation(s32 seed, u8 dungeonId, s32 floorId);
static s32 CalcSeededSpawnLevel(s32 seed, u8 dungeonId, s32 floorIndex, s32 floorCount);
static u8 SelectTileset(s32 floorId);
static SeededLayoutOption SelectAlternateLayout(DungeonSeedRng *rng);
static s8 SelectRoomDensity(DungeonSeedRng *rng, const SeededLayoutOption *option);
static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed);
static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result);
static void ResetSpawnRangeCache(void);
static bool8 SpawnRangeCacheMatches(s32 seed, u8 dungeonId, u32 combinedMask);
static void BuildSpawnRangesForDungeon(s32 seed, u8 dungeonId, u8 tileset, u32 mainTypeMask, u32 spawnTypeMask, u32 combinedMask);
static void EnsureSpawnRangeCache(s32 seed, u8 dungeonId, u8 tileset, u32 mainTypeMask, u32 spawnTypeMask, u32 combinedMask);
static s32 PopulateSpawnTableFromRanges(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, u32 mainTypeMask, u32 spawnTypeMask);
static s32 RollSpawnLevel(DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static s32 GetFloorIndexWithinDungeon(s32 floorId, s32 startFloorId, s32 floorCount);
static void EnsureSpawnRangesCoverDungeon(s32 floorCount);
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed);
static void BuildUniformTrapTableNoPitfall(u16 *trapTable);
static bool8 SelectWeatherForType(u8 dungeonType, DungeonSeedRng *rng, u8 *weatherOut);
static void MaybeApplyFloorWeather(DungeonSeedFloorOverrides *result, s32 seed, u8 dungeonId, s32 floorId);
static const SeedSpeciesPool* GetBossPool(s32 floorId);
static u16 SelectPrimaryBossLoot(DungeonSeedRng *rng);
static u16 SelectSecondaryBossLoot(DungeonSeedRng *rng);
static bool8 TryGetTypeSelectionBoss(s16 *bossSpecies);
static bool8 GetTypeBossMinions(s16 bossSpecies, s16 *minionsOut, u8 *minionCountOut);
static bool8 TrySpawnBossLoot(u16 itemId, s32 x, s32 y, const char *label);
static bool8 GetTypeBossMoves(s16 bossSpecies, u16 *movesOut);
static bool8 GetTypeBossMinionMoves(s16 bossSpecies, u16 minionMovesOut[][MAX_MON_MOVES], bool8 *minionHasCustomMovesOut);
static const BossWeatherConfig *GetBossWeatherConfigForSpecies(s16 species);
static void MaybeApplyBossWeather(BossFightConfig *bossFight, DungeonSeedRng *rng);
static bool8 IsBossSpecies(s16 species);
static bool8 SpeciesMatchesTypeMask(s16 species, u32 typeMask);
static s32 BuildSpawnCandidates(u32 typeMask, s16 *out, s32 outCapacity, bool8 restrictLowBst);
static u8 GetMainTypeForTileset(u8 tileset);
static u32 GetCombinedSpawnMask(u8 tileset, u32 mainTypeMask);
static s32 GetBaseStatTotal(s16 species);
static bool8 ShouldUseLowBstSpawns(u8 dungeonId);
static void EnsureLowBstSpeciesCache(void);
static bool8 IsLowBstSpecies(s16 species);
static u16 GetSeededStatTieBreak(s32 seed, u8 dungeonId, u8 tileset, s16 species);
static void SortSpawnCandidatesByBaseStats(s16 *pool, s32 count, s32 seed, u8 dungeonId, u8 tileset);
static s16 SelectSpeciesFromRankedPool(DungeonSeedRng *rng, const s16 *pool, s32 poolCount, bool8 *selectedFlags,
                                       s32 entryIndex, s32 entryCount);
static s16 SelectSpeciesFromRankedPoolWithReplacement(DungeonSeedRng *rng, const s16 *pool, s32 poolCount,
                                                      s32 entryIndex, s32 entryCount);
static s16 SelectGenericSpecies(DungeonSeedRng *rng, bool8 *selectedFlags, bool8 *loggedFallback, u32 maskForLog);
static s16 SelectLowBstFallbackSpecies(DungeonSeedRng *rng, bool8 *selectedFlags);

#ifdef DEV
static s32 GetSpawnTableCount(const SpawnPokemonData *spawnTable);
static const SeededSpawnRange *FindSpawnRangeForSpecies(s16 species);
void DungeonSeedOverrides_LogSeedDump(s32 seed, u8 dungeonId, s32 floorId, s32 startFloorId,
                                      const DungeonSeedFloorOverrides *overrides,
                                      SpawnPokemonData *spawnTable);
#endif

static const u8 sItemLimitsByDifficulty[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = INVENTORY_SIZE,
    [DIFFICULTY_HARD] = 10,
    [DIFFICULTY_NIGHTMARE] = 5,
};

static const s16 sRecruitBaseChancePercent[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = 8,
    [DIFFICULTY_HARD] = 5,
    [DIFFICULTY_NIGHTMARE] = 3,
};

static const s16 sRecruitFriendBowBonusPercent[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = 5,
    [DIFFICULTY_HARD] = 3,
    [DIFFICULTY_NIGHTMARE] = 1,
};

static const u8 sDifficultyLevelOffset[NUM_DIFFICULTY_SETTINGS] = {0, 0, 0};

// Max per-dungeon floor ramp applied across the full depth; keeps 99F dungeons tame.
static const u8 sDifficultyFloorRampMax[NUM_DIFFICULTY_SETTINGS] = {
    [DIFFICULTY_NORMAL] = 2,
    [DIFFICULTY_HARD] = 2,
    [DIFFICULTY_NIGHTMARE] = 2,
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
static s32 sSeededDungeonNameVersion = 0;

static bool8 sSeededItemOverridesActive = FALSE;
static bool8 sSeededRareFloorItemPending = FALSE;
static bool8 sSeededRareFloorItemUsed = FALSE;
static u16 sSeededRareFloorItemId = ITEM_NOTHING;
static bool8 sSeededKecleonShopHasRare = FALSE;
static bool8 sSeededKecleonShopRareUsed = FALSE;
static u16 sSeededKecleonShopRareId = ITEM_NOTHING;
static bool8 sSeededIsForcedKecleonFloor = FALSE;
static SeededSpawnRangeCache sSpawnRangeCache = {0};
static bool8 sLowBstSpeciesReady = FALSE;
static u8 sLowBstSpeciesFlags[SEEDED_LOW_BST_FLAG_BYTES];
static s32 sLowBstSpeciesCount = 0;

static void ResetSeededDungeonNameCache(void);
static void GenerateSeededDungeonNames(u8 dungeonId, s32 seed);
UNUSED static const char *SelectPrefixForDungeon(u8 dungeonId, DungeonSeedRng *rng, char *scratch, s32 scratchSize);
UNUSED static bool8 CopyFirstTokenFromBaseName(u8 dungeonId, char *buffer, s32 bufferSize);
UNUSED static s32 GetSelectedTypeForDisplay(void);
static s16 GetSeededBossHP(u8 dungeonId);
static void SeedBossRewardLoot(BossFightConfig *bossFight, s32 seed, u8 dungeonId);

void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId, DungeonSeedFloorOverrides *result)
{
    DungeonSeedRng rng;
    bool8 isForcedKecleonFloor = FALSE;
    s32 kecleonFloors[SEEDED_KECLEON_SHOP_COUNT] = {0};
    s32 i;

    if (result == NULL)
        return;

    MGBA_Warnf("[SeedOverrides] GenFloor start: seed=%d dungeon=%d floor=%d", seed, dungeonId, floorId);
    ClearFloorOverrides(result);
    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0xC0FFEE);
    result->tileset = SelectTileset(floorId);
    // Use a seeded trap density and a uniform trap table; specific floors can override density below
    result->trapDensityOverride = SEEDED_TRAP_DENSITY_DEFAULT;
    result->hasTrapTable = TRUE;
    BuildUniformTrapTableNoPitfall(result->trapSpawnChances);
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

    {
        s32 kecleonFloorId;

        DungeonSeedOverrides_GetKecleonFloors(dungeonId, seed, &kecleonFloors[0], &kecleonFloors[1]);
        for (i = 0; i < SEEDED_KECLEON_SHOP_COUNT; i++) {
            kecleonFloorId = GetDungeonStartingFloor(dungeonId) + kecleonFloors[i] + 1; // Floors are 1-indexed in mapparam
            if (floorId == kecleonFloorId)
                isForcedKecleonFloor = TRUE;
        }
    }

    InitSeededItemState(seed, dungeonId, floorId, isForcedKecleonFloor);

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
        PopulateSpawnTable(result, &rng, dungeonId, floorId, seed);
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

void DungeonSeedOverrides_ResetItemPools(void)
{
    ResetSeededItemState();
}

static s32 GetDungeonNumberForFloorScaling(u8 dungeonId)
{
    s32 i;
    s32 count = GetSequentialDungeonCountForRun();

    if (dungeonId >= NUM_DUNGEONS)
        return 1;

    for (i = 0; i < count; i++) {
        u8 listDungeonId = RescueDungeonToDungeonId(sSequentialDungeonList[i]);
        if (listDungeonId == dungeonId)
            return i + 1; // 1-indexed progression number
    }

    // Fallback: treat unknown dungeons as the first slot
    return 1;
}

static s32 CalcSeededFloorCount(s32 seed, u8 dungeonId, s32 *desiredFloorsOut, s32 *multiplierOut)
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

    if (desiredFloorsOut != NULL)
        *desiredFloorsOut = desiredFloors;
    if (multiplierOut != NULL)
        *multiplierOut = multiplier;

    return floorCount;
}

s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId)
{
    s32 multiplier = 0;
    s32 desiredFloors = 0;
    s32 floorCount = CalcSeededFloorCount(seed, dungeonId, &desiredFloors, &multiplier);

    MGBA_Warnf("[FloorCount] seed=%d dungeon=%d number=%d mult=%d floors=%d (engineCount=%d)",
               seed, dungeonId, GetDungeonNumberForFloorScaling(dungeonId), multiplier, desiredFloors, floorCount);
    return floorCount;
}

u32 DungeonSeedOverrides_GetDungeonRngSeed(s32 seed, u8 dungeonId, s32 floorId)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0x5EED5EED);
    u32 rngSeed = DungeonSeedRng_Next(&rng) | 1;
    MGBA_Warnf("[SeedOverrides] RngSeed: seed=%d dungeon=%d floor=%d rngSeed=%u", seed, dungeonId, floorId, rngSeed);
    return rngSeed;
}

s32 DungeonSeedOverrides_GetDungeonNumberForDisplay(u8 dungeonId)
{
    return GetDungeonNumberForFloorScaling(dungeonId);
}

s32 DungeonSeedOverrides_GetSequentialDungeonCount(void)
{
    return GetSequentialDungeonCountForRun();
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

static bool8 SelectShoppableItemFromPool(const RogueItemPool *pool, u8 *itemIdOut)
{
    s32 i;
    s32 shoppableCount = 0;
    s32 pick;

    if (pool == NULL || pool->items == NULL || pool->count == 0 || itemIdOut == NULL)
        return FALSE;

    for (i = 0; i < pool->count; i++) {
        if (IsShoppableItem(pool->items[i]))
            shoppableCount++;
    }

    if (shoppableCount == 0)
        return FALSE;

    pick = DungeonRandInt(shoppableCount);
    for (i = 0; i < pool->count; i++) {
        if (!IsShoppableItem(pool->items[i]))
            continue;
        if (pick == 0) {
            *itemIdOut = (u8)pool->items[i];
            return TRUE;
        }
        pick--;
    }

    return FALSE;
}

static void EnsureMissingKecleonShopRareItem(void)
{
    u8 itemId = ITEM_NOTHING;

    if (!sSeededIsForcedKecleonFloor)
        return;
    if (DungeonFloorSpawns_ShouldSpawnKecleonShopkeeper())
        return;
    if (gRogueItemPools[ROGUE_ITEM_POOL_KECLEON_RARE].count == 0)
        return;
    if (sSeededKecleonShopHasRare && sSeededKecleonShopRareId != ITEM_NOTHING &&
        IsShoppableItem(sSeededKecleonShopRareId)) {
        return;
    }

    if (!SelectShoppableItemFromPool(&gRogueItemPools[ROGUE_ITEM_POOL_KECLEON_RARE], &itemId)) {
        MGBA_Warnf("[ItemPools] Missing Kecleon shop has no shoppable rare items");
        return;
    }

    sSeededKecleonShopHasRare = TRUE;
    sSeededKecleonShopRareUsed = FALSE;
    sSeededKecleonShopRareId = itemId;
    MGBA_Warnf("[ItemPools] Missing Kecleon shop forced rare item id=%d", sSeededKecleonShopRareId);
}

bool8 DungeonSeedOverrides_SelectFloorItem(s32 spawnType, u8 *itemIdOut)
{
    const RogueItemPool *pool = NULL;

    if (!sSeededItemOverridesActive || itemIdOut == NULL)
        return FALSE;

    switch (spawnType) {
        case ITEM_SPAWN_IN_SHOP:
            EnsureMissingKecleonShopRareItem();
            if (sSeededKecleonShopHasRare && !sSeededKecleonShopRareUsed && sSeededKecleonShopRareId != ITEM_NOTHING) {
                if (IsShoppableItem(sSeededKecleonShopRareId)) {
                    *itemIdOut = (u8)sSeededKecleonShopRareId;
                    sSeededKecleonShopRareUsed = TRUE;
                    MGBA_Warnf("[ItemPools] Using rare Kecleon item id=%d (forced=%d)", *itemIdOut, sSeededIsForcedKecleonFloor);
                    return TRUE;
                }
                sSeededKecleonShopRareUsed = TRUE;
                MGBA_Warnf("[ItemPools] Skipped non-shoppable Kecleon rare item id=%d", sSeededKecleonShopRareId);
            }
            pool = &gRogueItemPools[ROGUE_ITEM_POOL_KECLEON_COMMON];
            break;
        case ITEM_SPAWN_IN_MONSTER_HOUSE:
            pool = &gRogueItemPools[ROGUE_ITEM_POOL_MONSTER_HOUSE];
            break;
        case ITEM_SPAWN_WALL:
        case ITEM_SPAWN_NORMAL:
            if (sSeededRareFloorItemPending && !sSeededRareFloorItemUsed && sSeededRareFloorItemId != ITEM_NOTHING) {
                *itemIdOut = (u8)sSeededRareFloorItemId;
                sSeededRareFloorItemUsed = TRUE;
                MGBA_Warnf("[ItemPools] Spawning rare floor item id=%d", *itemIdOut);
                return TRUE;
            }
            pool = &gRogueItemPools[ROGUE_ITEM_POOL_NORMAL];
            break;
        default:
            return FALSE;
    }

    if (pool == NULL || pool->items == NULL || pool->count == 0)
        return FALSE;

    if (spawnType == ITEM_SPAWN_IN_SHOP)
        return SelectShoppableItemFromPool(pool, itemIdOut);

    *itemIdOut = (u8)pool->items[DungeonRandInt(pool->count)];
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

    if (sSeededDungeonNameVersion != SEEDED_DUNGEON_NAME_VERSION) {
        sSeededDungeonNameVersion = SEEDED_DUNGEON_NAME_VERSION;
        ResetSeededDungeonNameCache();
    }

#ifdef DEV
    displayType = GetSelectedTypeForDisplay();
#else
    displayType = TYPE_NONE;
#endif
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
    result->bossFight.dropItem = ITEM_NOTHING;
    result->bossFight.secondaryDropLeft = ITEM_NOTHING;
    result->bossFight.secondaryDropRight = ITEM_NOTHING;
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

static void BuildUniformTrapTableNoPitfall(u16 *trapTable)
{
    s32 i;
    u32 accum = 0;
    u32 activeTraps = NUM_TRAPS - 1;
    u32 base = 10000 / activeTraps;
    u32 remainder = 10000 % activeTraps;

    if (trapTable == NULL)
        return;

    for (i = 0; i < NUM_TRAPS; i++) {
        u32 increment = base;
        if (i == TRAP_PITFALL_TRAP) {
            trapTable[i] = (u16)accum;
            continue;
        }
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

static void ResetSeededItemState(void)
{
    sSeededItemOverridesActive = FALSE;
    sSeededRareFloorItemPending = FALSE;
    sSeededRareFloorItemUsed = FALSE;
    sSeededRareFloorItemId = ITEM_NOTHING;
    sSeededKecleonShopHasRare = FALSE;
    sSeededKecleonShopRareUsed = FALSE;
    sSeededKecleonShopRareId = ITEM_NOTHING;
    sSeededIsForcedKecleonFloor = FALSE;
}

static u16 SelectItemFromPool(RogueItemPoolId poolId, DungeonSeedRng *rng)
{
    const RogueItemPool *pool;

    if (poolId < 0 || poolId >= ROGUE_ITEM_POOL_COUNT)
        return ITEM_NOTHING;

    pool = &gRogueItemPools[poolId];
    if (pool->items == NULL || pool->count == 0)
        return ITEM_NOTHING;

    if (rng != NULL)
        return pool->items[DungeonSeedRng_NextRange(rng, 0, pool->count)];
    return pool->items[DungeonRandInt(pool->count)];
}

static void InitSeededItemState(s32 seed, u8 dungeonId, s32 floorId, bool8 isForcedKecleonFloor)
{
    DungeonSeedRng rng;

    ResetSeededItemState();
    sSeededItemOverridesActive = TRUE;
    sSeededIsForcedKecleonFloor = isForcedKecleonFloor;

    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0x4954454D); // "ITEM"

    if (gRogueItemPools[ROGUE_ITEM_POOL_RARE].count > 0 &&
        DungeonSeedRng_NextRange(&rng, 0, 100) < SEEDED_FLOOR_RARE_CHANCE_PERCENT) {
        sSeededRareFloorItemPending = TRUE;
        sSeededRareFloorItemUsed = FALSE;
        sSeededRareFloorItemId = SelectItemFromPool(ROGUE_ITEM_POOL_RARE, &rng);
        MGBA_Warnf("[ItemPools] Rare floor item selected: id=%d", sSeededRareFloorItemId);
    }

    if (isForcedKecleonFloor &&
        gRogueItemPools[ROGUE_ITEM_POOL_KECLEON_RARE].count > 0 &&
        DungeonSeedRng_NextRange(&rng, 0, 100) < SEEDED_SHOP_RARE_CHANCE_PERCENT) {
        sSeededKecleonShopHasRare = TRUE;
        sSeededKecleonShopRareUsed = FALSE;
        sSeededKecleonShopRareId = SelectItemFromPool(ROGUE_ITEM_POOL_KECLEON_RARE, &rng);
        MGBA_Warnf("[ItemPools] Kecleon rare item selected: id=%d", sSeededKecleonShopRareId);
    }
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

    // Normalize cutscene/forms so boss variants like Rayquaza Cutscene are filtered too.
    species = GetBaseSpecies(species);

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
    // Kecleon is reserved for shops; never treat it as a normal floor spawn
    if (species == MONSTER_KECLEON)
        return FALSE;
    // Decoy/statue are placeholder actors and should never be wild spawns
    if (species == MONSTER_DECOY || species == MONSTER_STATUE)
        return FALSE;

    for (i = 0; i < 2; i++) {
        u8 type = GetPokemonType(species, (u32)i);
        if (type > TYPE_NONE && type < NUM_TYPES && (typeMask & (1u << type)))
            return TRUE;
    }
    return FALSE;
}

static s32 BuildSpawnCandidates(u32 typeMask, s16 *out, s32 outCapacity, bool8 restrictLowBst)
{
    s32 count = 0;
    s32 species;

    if (out == NULL || outCapacity <= 0 || typeMask == 0)
        return 0;

    for (species = 1; species < MONSTER_MAX && count < outCapacity; species++) {
        if (!SpeciesMatchesTypeMask((s16)species, typeMask))
            continue;
        if (restrictLowBst && !IsLowBstSpecies((s16)species))
            continue;
        out[count++] = (s16)species;
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

static s32 GetBaseStatTotal(s16 species)
{
    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return 0;

    return GetBaseHP(species)
        + GetBaseOffensiveStat(species, OFFENSE_NRM)
        + GetBaseOffensiveStat(species, OFFENSE_SP)
        + GetBaseDefensiveStat(species, 0)
        + GetBaseDefensiveStat(species, 1);
}

static bool8 ShouldUseLowBstSpawns(u8 dungeonId)
{
    s32 i;
    s32 count = GetSequentialDungeonCountForRun();

    if (dungeonId >= NUM_DUNGEONS)
        return FALSE;

    for (i = 0; i < count; i++) {
        u8 listDungeonId = RescueDungeonToDungeonId(sSequentialDungeonList[i]);
        if (listDungeonId == dungeonId)
            return (i < 2);
    }

    return FALSE;
}

static void EnsureLowBstSpeciesCache(void)
{
    s16 speciesList[MONSTER_MAX];
    s32 speciesCount = 0;
    s32 bottomCount;
    s32 i;

    if (sLowBstSpeciesReady)
        return;

    for (i = 0; i < ARRAY_COUNT(sLowBstSpeciesFlags); i++)
        sLowBstSpeciesFlags[i] = 0;

    for (i = 1; i < MONSTER_MAX; i++) {
        s32 total = GetBaseStatTotal((s16)i);
        if (total <= 0)
            continue;
        if (i == MONSTER_KECLEON || i == MONSTER_DECOY || i == MONSTER_STATUE)
            continue;
        speciesList[speciesCount++] = (s16)i;
    }

    for (i = 1; i < speciesCount; i++) {
        s16 key = speciesList[i];
        s32 keyTotal = GetBaseStatTotal(key);
        s32 j = i - 1;

        while (j >= 0) {
            s16 current = speciesList[j];
            s32 currentTotal = GetBaseStatTotal(current);

            if (currentTotal < keyTotal)
                break;
            if (currentTotal == keyTotal && current <= key)
                break;

            speciesList[j + 1] = speciesList[j];
            j--;
        }
        speciesList[j + 1] = key;
    }

    bottomCount = (speciesCount * SEEDED_LOW_BST_PERCENT + 99) / 100;
    if (bottomCount < 1)
        bottomCount = 1;
    if (bottomCount > speciesCount)
        bottomCount = speciesCount;

    for (i = 0; i < bottomCount; i++) {
        s16 species = speciesList[i];
        sLowBstSpeciesFlags[(u16)species >> 3] |= (1u << ((u16)species & 7));
    }

    sLowBstSpeciesCount = bottomCount;
    sLowBstSpeciesReady = TRUE;
}

static bool8 IsLowBstSpecies(s16 species)
{
    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return FALSE;

    EnsureLowBstSpeciesCache();
    return (sLowBstSpeciesFlags[(u16)species >> 3] & (1u << ((u16)species & 7))) != 0;
}

static u16 GetSeededStatTieBreak(s32 seed, u8 dungeonId, u8 tileset, s16 species)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, species, 0x53544154 ^ ((u32)tileset << 16)); // "STAT"

    return (u16)DungeonSeedRng_Next(&rng);
}

static void SortSpawnCandidatesByBaseStats(s16 *pool, s32 count, s32 seed, u8 dungeonId, u8 tileset)
{
    s32 i;

    if (pool == NULL || count <= 1)
        return;

    for (i = 1; i < count; i++) {
        s16 key = pool[i];
        s32 keyTotal = GetBaseStatTotal(key);
        u16 keyTie = GetSeededStatTieBreak(seed, dungeonId, tileset, key);
        s32 j = i - 1;

        while (j >= 0) {
            s16 current = pool[j];
            s32 currentTotal = GetBaseStatTotal(current);
            u16 currentTie;

            if (currentTotal < keyTotal)
                break;
            if (currentTotal == keyTotal) {
                currentTie = GetSeededStatTieBreak(seed, dungeonId, tileset, current);
                if (currentTie <= keyTie)
                    break;
            }

            pool[j + 1] = pool[j];
            j--;
        }
        pool[j + 1] = key;
    }
}

static s32 GetRankTargetIndex(s32 entryIndex, s32 entryCount, s32 poolCount)
{
    if (poolCount <= 0)
        return 0;
    if (entryCount <= 1)
        return poolCount / 2;
    if (poolCount <= 1)
        return 0;
    if (entryIndex < 0)
        entryIndex = 0;
    if (entryIndex > entryCount - 1)
        entryIndex = entryCount - 1;

    return (entryIndex * (poolCount - 1)) / (entryCount - 1);
}

static s16 SelectSpeciesFromRankedPool(DungeonSeedRng *rng, const s16 *pool, s32 poolCount, bool8 *selectedFlags,
                                       s32 entryIndex, s32 entryCount)
{
    s32 target;
    s32 window;
    s32 start;
    s32 end;
    s32 attempts;
    s32 i;

    if (rng == NULL || pool == NULL || selectedFlags == NULL || poolCount <= 0)
        return MONSTER_NONE;

    target = GetRankTargetIndex(entryIndex, entryCount, poolCount);
    window = poolCount / entryCount;
    if (window < 1)
        window = 1;
    if (window > poolCount - 1)
        window = poolCount - 1;

    start = target - window;
    end = target + window;
    if (start < 0)
        start = 0;
    if (end >= poolCount)
        end = poolCount - 1;

    attempts = (end - start + 1) + 2;
    for (i = 0; i < attempts; i++) {
        s32 idx = DungeonSeedRng_NextRange(rng, start, end + 1);
        s16 species = pool[idx];

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (selectedFlags[species])
            continue;

        selectedFlags[species] = TRUE;
        return species;
    }

    {
        s32 left = target;
        s32 right = target + 1;
        bool8 rightFirst = DungeonSeedRng_NextRange(rng, 0, 2);

        while (left >= 0 || right < poolCount) {
            s32 idx;
            s16 species;

            if (rightFirst) {
                if (right < poolCount) {
                    idx = right++;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !selectedFlags[species]) {
                        selectedFlags[species] = TRUE;
                        return species;
                    }
                }
                if (left >= 0) {
                    idx = left--;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !selectedFlags[species]) {
                        selectedFlags[species] = TRUE;
                        return species;
                    }
                }
            } else {
                if (left >= 0) {
                    idx = left--;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !selectedFlags[species]) {
                        selectedFlags[species] = TRUE;
                        return species;
                    }
                }
                if (right < poolCount) {
                    idx = right++;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !selectedFlags[species]) {
                        selectedFlags[species] = TRUE;
                        return species;
                    }
                }
            }
        }
    }

    return MONSTER_NONE;
}

static s16 SelectSpeciesFromRankedPoolWithReplacement(DungeonSeedRng *rng, const s16 *pool, s32 poolCount,
                                                      s32 entryIndex, s32 entryCount)
{
    s32 target;
    s32 window;
    s32 start;
    s32 end;
    s32 attempts;
    s32 i;

    if (rng == NULL || pool == NULL || poolCount <= 0)
        return MONSTER_NONE;

    target = GetRankTargetIndex(entryIndex, entryCount, poolCount);
    window = poolCount / entryCount;
    if (window < 1)
        window = 1;
    if (window > poolCount - 1)
        window = poolCount - 1;

    start = target - window;
    end = target + window;
    if (start < 0)
        start = 0;
    if (end >= poolCount)
        end = poolCount - 1;

    attempts = (end - start + 1) + 4;
    for (i = 0; i < attempts; i++) {
        s32 idx = DungeonSeedRng_NextRange(rng, start, end + 1);
        s16 species = pool[idx];

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (IsBossSpecies(species))
            continue;

        return species;
    }

    {
        s32 left = target;
        s32 right = target + 1;
        bool8 rightFirst = DungeonSeedRng_NextRange(rng, 0, 2);

        while (left >= 0 || right < poolCount) {
            s32 idx;
            s16 species;

            if (rightFirst) {
                if (right < poolCount) {
                    idx = right++;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !IsBossSpecies(species))
                        return species;
                }
                if (left >= 0) {
                    idx = left--;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !IsBossSpecies(species))
                        return species;
                }
            } else {
                if (left >= 0) {
                    idx = left--;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !IsBossSpecies(species))
                        return species;
                }
                if (right < poolCount) {
                    idx = right++;
                    species = pool[idx];
                    if (species > MONSTER_NONE && species < MONSTER_MAX && !IsBossSpecies(species))
                        return species;
                }
            }
        }
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

static s16 SelectLowBstFallbackSpecies(DungeonSeedRng *rng, bool8 *selectedFlags)
{
    s32 unselectedCount = 0;
    s32 target;
    s32 seen;
    s32 i;

    if (rng == NULL)
        return MONSTER_NONE;

    EnsureLowBstSpeciesCache();
    if (sLowBstSpeciesCount <= 0)
        return MONSTER_NONE;

    if (selectedFlags != NULL) {
        for (i = 1; i < MONSTER_MAX; i++) {
            if (!IsLowBstSpecies((s16)i))
                continue;
            if (!selectedFlags[i])
                unselectedCount++;
        }

        if (unselectedCount > 0) {
            target = DungeonSeedRng_NextRange(rng, 0, unselectedCount);
            seen = 0;
            for (i = 1; i < MONSTER_MAX; i++) {
                if (!IsLowBstSpecies((s16)i))
                    continue;
                if (selectedFlags[i])
                    continue;
                if (seen == target) {
                    selectedFlags[i] = TRUE;
                    return (s16)i;
                }
                seen++;
            }
        }
    }

    target = DungeonSeedRng_NextRange(rng, 0, sLowBstSpeciesCount);
    seen = 0;
    for (i = 1; i < MONSTER_MAX; i++) {
        if (!IsLowBstSpecies((s16)i))
            continue;
        if (seen == target) {
            if (selectedFlags != NULL)
                selectedFlags[i] = TRUE;
            return (s16)i;
        }
        seen++;
    }

    return MONSTER_NONE;
}

static void ResetSpawnRangeCache(void)
{
    sSpawnRangeCache.valid = FALSE;
    sSpawnRangeCache.rangeCount = 0;
    sSpawnRangeCache.seed = -1;
    sSpawnRangeCache.dungeonId = 0;
    sSpawnRangeCache.tileset = 0;
    sSpawnRangeCache.startFloorId = 0;
    sSpawnRangeCache.floorCount = 0;
}

static bool8 SpawnRangeCacheMatches(s32 seed, u8 dungeonId, u32 combinedMask)
{
    if (!sSpawnRangeCache.valid)
        return FALSE;

    return (sSpawnRangeCache.seed == seed &&
            sSpawnRangeCache.dungeonId == dungeonId &&
            sSpawnRangeCache.combinedMask == combinedMask);
}

static s32 GetFloorIndexWithinDungeon(s32 floorId, s32 startFloorId, s32 floorCount)
{
    s32 floorIndex = floorId - startFloorId - 1;

    if (floorIndex < 0)
        floorIndex = 0;
    if (floorCount > 0 && floorIndex >= floorCount)
        floorIndex = floorCount - 1;

    return floorIndex;
}

// Deterministic level curve: dungeon progression + floor depth + difficulty + a tiny seeded jitter.
static s32 CalcSeededSpawnLevel(s32 seed, u8 dungeonId, s32 floorIndex, s32 floorCount)
{
    s32 dungeonNumber = GetDungeonNumberForFloorScaling(dungeonId);
    u32 difficulty = GetGameDifficultySetting();
    s32 baseLevel;
    s32 level;
    s32 denom;
    s32 jitter = 0;
    DungeonSeedRng rng;

    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;
    if (dungeonNumber < 1)
        dungeonNumber = 1;
    if (floorCount < 1)
        floorCount = 1;
    if (floorIndex < 0)
        floorIndex = 0;
    if (floorIndex >= floorCount)
        floorIndex = floorCount - 1;

    baseLevel = 1 + (dungeonNumber - 1);
    baseLevel += sDifficultyLevelOffset[difficulty];

    denom = floorCount - 1;
    if (denom < 1)
        denom = 1;
    level = baseLevel + (floorIndex * sDifficultyFloorRampMax[difficulty]) / denom;

    rng = DungeonSeedRng_Init(seed, dungeonId, floorIndex, 0x4C564C43); // "LVLC"
    jitter = DungeonSeedRng_NextRange(&rng, 0, 3) - 1; // -1 to +1
    level += jitter;

    if (level < 1)
        level = 1;
    if (level > 90)
        level = 90;

    return level;
}

static s32 RollSpawnLevel(DungeonSeedRng *rng, s32 dungeonId, s32 floorId)
{
    s32 baseLevel = 3 + (dungeonId % 10) + floorId;
    s32 levelVariance = DungeonSeedRng_NextRange(rng, 0, 6);
    s32 level = baseLevel + levelVariance;

    if (level > 90)
        level = 90;

    return level;
}

static void EnsureSpawnRangesCoverDungeon(s32 floorCount)
{
    s32 i, j;
    u8 order[SEEDED_FIXED_SPAWN_COUNT];
    SeededSpawnRange *ranges = sSpawnRangeCache.ranges;

    if (sSpawnRangeCache.rangeCount == 0)
        return;
    if (floorCount <= 0)
        return;

    for (i = 0; i < sSpawnRangeCache.rangeCount; i++)
        order[i] = (u8)i;

    // Simple selection sort by start
    for (i = 0; i < sSpawnRangeCache.rangeCount; i++) {
        for (j = i + 1; j < sSpawnRangeCache.rangeCount; j++) {
            if (ranges[order[j]].start < ranges[order[i]].start) {
                u8 tmp = order[i];
                order[i] = order[j];
                order[j] = tmp;
            }
        }
    }

    // Force first range to start at floor 0, preserve length if possible
    {
        SeededSpawnRange *first = &ranges[order[0]];
        s32 length = first->end - first->start;
        if (length < 0)
            length = 0;
        first->start = 0;
        first->end = first->start + length;
        if (first->end >= floorCount)
            first->end = floorCount - 1;
    }

    // Fix gaps by sliding subsequent ranges to start after the previous end
    for (i = 1; i < sSpawnRangeCache.rangeCount; i++) {
        SeededSpawnRange *prev = &ranges[order[i - 1]];
        SeededSpawnRange *curr = &ranges[order[i]];
        s32 length = curr->end - curr->start;
        if (length < 0)
            length = 0;

        if (prev->end + 1 < curr->start) {
            curr->start = prev->end + 1;
            curr->end = curr->start + length;
        }

        if (curr->end >= floorCount) {
            curr->end = floorCount - 1;
            curr->start = curr->end - length;
            if (curr->start < 0)
                curr->start = 0;
        }
    }

    // Force last range to end at final floor, adjust start to keep length if possible
    {
        SeededSpawnRange *last = &ranges[order[sSpawnRangeCache.rangeCount - 1]];
        s32 length = last->end - last->start;
        if (length < 0)
            length = 0;
        last->end = floorCount - 1;
        last->start = last->end - length;
        if (last->start < 0)
            last->start = 0;
    }

    // Refresh cached levels against the adjusted ranges
    if (sSpawnRangeCache.seed != -1) {
        for (i = 0; i < sSpawnRangeCache.rangeCount; i++) {
            SeededSpawnRange *range = &ranges[i];
            s32 mid = range->start + (range->end - range->start) / 2;
            range->level = (u8)CalcSeededSpawnLevel(sSpawnRangeCache.seed,
                                                    sSpawnRangeCache.dungeonId,
                                                    mid,
                                                    floorCount);
        }
    }
}

static void BuildSpawnRangesForDungeon(s32 seed, u8 dungeonId, u8 tileset, u32 mainTypeMask, u32 spawnTypeMask, u32 combinedMask)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, tileset, 0x5352414E); // "SRAN"
    s16 mainCandidates[MONSTER_MAX];
    s16 mainCandidatesCopy[MONSTER_MAX];
    s16 spawnCandidates[MONSTER_MAX];
    s16 combinedCandidates[MONSTER_MAX];
    s16 normalCandidates[MONSTER_MAX];
    bool8 selectedFlags[MONSTER_MAX];
    s32 mainCandidateCount = 0;
    s32 mainCandidateCopyCount = 0;
    s32 spawnCandidateCount = 0;
    s32 combinedCandidateCount = 0;
    s32 normalCandidateCount = 0;
    s32 orderedCandidateCount = 0;
    s32 orderedMainCount = 0;
    s32 orderedSpawnCount = 0;
    s32 orderedNormalCount = 0;
    s32 mainQuota;
    s32 spawnQuota;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 engineFloorCount = GetDungeonFloorCount(dungeonId);
    s32 startFloorId = GetDungeonStartingFloor(dungeonId);
    s32 lengthMin;
    s32 lengthMax;
    bool8 useBulbasaurOnly = FALSE;
    bool8 loggedFallback = FALSE;
    bool8 loggedInvalid = FALSE;
    bool8 useLowBstSelection = FALSE;
    bool8 entryIsMain[SEEDED_FIXED_SPAWN_COUNT];
    s32 i;

    // Scale spawn variety with dungeon length (~1.4 species per floor) but cap to engine limits.
    s32 entryCount = (floorCount * SEEDED_SPAWN_TARGET_NUM + (SEEDED_SPAWN_TARGET_DEN - 1)) / SEEDED_SPAWN_TARGET_DEN;
    if (entryCount < SEEDED_MIN_SPAWNS)
        entryCount = SEEDED_MIN_SPAWNS;

    if (entryCount > MONSTER_SPAWNS_ARR_COUNT)
        entryCount = MONSTER_SPAWNS_ARR_COUNT;
    if (floorCount <= 0)
        floorCount = 1;
    if (engineFloorCount > 0 && floorCount > engineFloorCount)
        floorCount = engineFloorCount;

    useLowBstSelection = ShouldUseLowBstSpawns(dungeonId);

    if (mainTypeMask != 0)
        mainCandidateCount = BuildSpawnCandidates(mainTypeMask, mainCandidates, ARRAY_COUNT(mainCandidates), useLowBstSelection);
    if (spawnTypeMask != 0)
        spawnCandidateCount = BuildSpawnCandidates(spawnTypeMask, spawnCandidates, ARRAY_COUNT(spawnCandidates), useLowBstSelection);
    if (combinedMask != 0)
        combinedCandidateCount = BuildSpawnCandidates(combinedMask, combinedCandidates, ARRAY_COUNT(combinedCandidates), useLowBstSelection);
    if (useLowBstSelection)
        normalCandidateCount = BuildSpawnCandidates(1u << TYPE_NORMAL, normalCandidates, ARRAY_COUNT(normalCandidates), TRUE);
    if (combinedMask != 0 && combinedCandidateCount == 0 && !useLowBstSelection)
        useBulbasaurOnly = TRUE;
    if (mainCandidateCount > 0)
        SortSpawnCandidatesByBaseStats(mainCandidates, mainCandidateCount, seed, dungeonId, tileset);
    if (spawnCandidateCount > 0)
        SortSpawnCandidatesByBaseStats(spawnCandidates, spawnCandidateCount, seed, dungeonId, tileset);
    if (combinedCandidateCount > 0)
        SortSpawnCandidatesByBaseStats(combinedCandidates, combinedCandidateCount, seed, dungeonId, tileset);
    if (normalCandidateCount > 0)
        SortSpawnCandidatesByBaseStats(normalCandidates, normalCandidateCount, seed, dungeonId, tileset);
    if (mainCandidateCount > 0) {
        mainCandidateCopyCount = mainCandidateCount;
        for (i = 0; i < mainCandidateCount; i++)
            mainCandidatesCopy[i] = mainCandidates[i];
    }

    for (i = 0; i < MONSTER_MAX; i++)
        selectedFlags[i] = FALSE;
    if (useLowBstSelection) {
        orderedCandidateCount = 0;
        for (i = 0; i < mainCandidateCount && orderedCandidateCount < ARRAY_COUNT(combinedCandidates); i++) {
            s16 species = mainCandidates[i];
            if (species <= MONSTER_NONE || species >= MONSTER_MAX)
                continue;
            if (selectedFlags[species])
                continue;
            combinedCandidates[orderedCandidateCount++] = species;
            selectedFlags[species] = TRUE;
        }
        orderedMainCount = orderedCandidateCount;
        for (i = 0; i < spawnCandidateCount && orderedCandidateCount < ARRAY_COUNT(combinedCandidates); i++) {
            s16 species = spawnCandidates[i];
            if (species <= MONSTER_NONE || species >= MONSTER_MAX)
                continue;
            if (selectedFlags[species])
                continue;
            combinedCandidates[orderedCandidateCount++] = species;
            selectedFlags[species] = TRUE;
        }
        orderedSpawnCount = orderedCandidateCount - orderedMainCount;
        for (i = 0; i < normalCandidateCount && orderedCandidateCount < ARRAY_COUNT(combinedCandidates); i++) {
            s16 species = normalCandidates[i];
            if (species <= MONSTER_NONE || species >= MONSTER_MAX)
                continue;
            if (selectedFlags[species])
                continue;
            combinedCandidates[orderedCandidateCount++] = species;
            selectedFlags[species] = TRUE;
        }
        orderedNormalCount = orderedCandidateCount - orderedMainCount - orderedSpawnCount;

        for (i = 0; i < MONSTER_MAX; i++)
            selectedFlags[i] = FALSE;
    }

    if (useLowBstSelection) {
        mainQuota = entryCount;
    } else if (mainTypeMask == 0) {
        mainQuota = 0;
    } else {
        mainQuota = (entryCount * SEEDED_MAIN_TYPE_PERCENT + 99) / 100;
        if (mainQuota > entryCount)
            mainQuota = entryCount;
    }
    spawnQuota = entryCount - mainQuota;

    ResetSpawnRangeCache();
    sSpawnRangeCache.seed = seed;
    sSpawnRangeCache.dungeonId = dungeonId;
    sSpawnRangeCache.tileset = tileset;
    sSpawnRangeCache.combinedMask = combinedMask;
    sSpawnRangeCache.startFloorId = (s16)startFloorId;
    sSpawnRangeCache.floorCount = (s16)floorCount;
    sSpawnRangeCache.rangeCount = 0;

    // Scale band lengths with dungeon length while keeping a reasonable minimum.
    lengthMin = floorCount / entryCount;
    if (lengthMin < 1)
        lengthMin = 1;
    lengthMax = lengthMin + 3;
    if (lengthMin < 4)
        lengthMin = 4;
    if (lengthMax < lengthMin)
        lengthMax = lengthMin;
    if (lengthMax > floorCount)
        lengthMax = floorCount;
    if (lengthMin > floorCount)
        lengthMin = floorCount;

    MGBA_Infof("[SeedOverrides] Spawn ranges build tileset=%d mainTypeMask=0x%08x spawnMask=0x%08x combined=0x%08x floors=%d entryCount=%d startFloor=%d",
               tileset,
               mainTypeMask,
               spawnTypeMask,
               combinedMask,
               floorCount,
               entryCount,
               startFloorId);

    // Shuffle which bands are main/off-type so mains are not front-loaded.
    for (i = 0; i < entryCount; i++)
        entryIsMain[i] = (i < mainQuota);
    for (i = entryCount - 1; i > 0; i--) {
        s32 swapIdx = DungeonSeedRng_NextRange(&rng, 0, i + 1);
        bool8 tmp = entryIsMain[i];
        entryIsMain[i] = entryIsMain[swapIdx];
        entryIsMain[swapIdx] = tmp;
    }

    for (i = 0; i < entryCount; i++) {
        s16 species;
        s32 length;
        s32 bucketStart;
        s32 bucketEnd;
        s32 bucketSpan;
        s32 mid;
        s32 startIndex = 0;
        s32 endIndex;
        bool8 fillingMain = entryIsMain[i];
        bool8 pickedWithReplacement = FALSE;
        SeededSpawnRange *range;

        if (useBulbasaurOnly) {
            species = MONSTER_BULBASAUR;
            selectedFlags[species] = TRUE;
        } else if (useLowBstSelection) {
            if (orderedCandidateCount > 0) {
                s32 index = i;
                if (index < orderedCandidateCount) {
                    species = combinedCandidates[index];
                } else if (orderedNormalCount > 0) {
                    s32 normalStart = orderedMainCount + orderedSpawnCount;
                    species = combinedCandidates[normalStart + ((index - orderedCandidateCount) % orderedNormalCount)];
                } else if (orderedSpawnCount > 0) {
                    s32 spawnStart = orderedMainCount;
                    species = combinedCandidates[spawnStart + ((index - orderedMainCount) % orderedSpawnCount)];
                } else if (orderedMainCount > 0) {
                    species = combinedCandidates[index % orderedMainCount];
                } else {
                    species = MONSTER_NONE;
                }
            } else {
                species = SelectLowBstFallbackSpecies(&rng, selectedFlags);
            }
        } else if (fillingMain) {
            species = SelectSpeciesFromRankedPool(&rng, mainCandidates, mainCandidateCount, selectedFlags, i, entryCount);
            if (species == MONSTER_NONE && mainCandidateCopyCount > 0) {
                species = SelectSpeciesFromRankedPoolWithReplacement(&rng, mainCandidatesCopy, mainCandidateCopyCount, i, entryCount);
                pickedWithReplacement = (species != MONSTER_NONE);
            }
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromRankedPool(&rng, combinedCandidates, combinedCandidateCount, selectedFlags, i, entryCount);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromRankedPool(&rng, spawnCandidates, spawnCandidateCount, selectedFlags, i, entryCount);
        } else {
            species = SelectSpeciesFromRankedPool(&rng, spawnCandidates, spawnCandidateCount, selectedFlags, i, entryCount);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromRankedPool(&rng, combinedCandidates, combinedCandidateCount, selectedFlags, i, entryCount);
            if (species == MONSTER_NONE)
                species = SelectSpeciesFromRankedPool(&rng, mainCandidates, mainCandidateCount, selectedFlags, i, entryCount);
        }

        if (species == MONSTER_NONE) {
            if (useLowBstSelection)
                species = SelectLowBstFallbackSpecies(&rng, selectedFlags);
            else
                species = SelectGenericSpecies(&rng, selectedFlags, &loggedFallback, combinedMask);
        }

        if (species <= MONSTER_NONE || species >= MONSTER_MAX) {
            species = MONSTER_BULBASAUR;
            if (!loggedInvalid) {
                MGBA_Warnf("[SeedOverrides] Spawn invalid species, forcing Bulbasaur (entry=%d)", i);
                loggedInvalid = TRUE;
            }
            selectedFlags[species] = TRUE;
        } else if (pickedWithReplacement) {
            selectedFlags[species] = TRUE;
        }

        // Pick a scaled band (min 4 floors) and center it within an even bucketed midpoint.
        length = DungeonSeedRng_NextRange(&rng, lengthMin, lengthMax + 1);
        if (length > floorCount)
            length = floorCount;

        bucketStart = (i * floorCount) / entryCount;
        bucketEnd = ((i + 1) * floorCount) / entryCount - 1;
        if (bucketEnd < bucketStart)
            bucketEnd = bucketStart;
        bucketSpan = bucketEnd - bucketStart;

        // Random midpoint within this bucket (stratified distribution)
        if (bucketSpan > 0)
            mid = bucketStart + DungeonSeedRng_NextRange(&rng, 0, bucketSpan + 1);
        else
            mid = bucketStart;

        startIndex = mid - length / 2;
        if (startIndex < 0)
            startIndex = 0;
        endIndex = startIndex + length - 1;
        if (endIndex >= floorCount) {
            endIndex = floorCount - 1;
            startIndex = endIndex - length + 1;
            if (startIndex < 0)
                startIndex = 0;
        }

        if (sSpawnRangeCache.rangeCount < SEEDED_FIXED_SPAWN_COUNT) {
            range = &sSpawnRangeCache.ranges[sSpawnRangeCache.rangeCount++];
            range->species = species;
            range->start = (s16)startIndex;
            range->end = (s16)endIndex;
            range->level = (u8)CalcSeededSpawnLevel(seed, dungeonId, mid, floorCount);
            MGBA_Warnf("[SeedOverrides] Range %d: species=%d len=%d mid=%d floors=%d-%d level=%d",
                       i,
                       species,
                       endIndex - startIndex + 1,
                       mid + startFloorId + 1,
                       startIndex + startFloorId + 1,
                       endIndex + startFloorId + 1,
                       range->level);
        }
    }

    EnsureSpawnRangesCoverDungeon(floorCount);
    sSpawnRangeCache.valid = TRUE;
}

static void EnsureSpawnRangeCache(s32 seed, u8 dungeonId, u8 tileset, u32 mainTypeMask, u32 spawnTypeMask, u32 combinedMask)
{
    if (SpawnRangeCacheMatches(seed, dungeonId, combinedMask))
        return;

    BuildSpawnRangesForDungeon(seed, dungeonId, tileset, mainTypeMask, spawnTypeMask, combinedMask);
}

static s32 PopulateSpawnTableFromRanges(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, u32 mainTypeMask, u32 spawnTypeMask)
{
    s32 count = 0;
    s32 floorIndex;
    s32 i;
    s16 mainList[SEEDED_FIXED_SPAWN_COUNT];
    u8 mainLevel[SEEDED_FIXED_SPAWN_COUNT];
    s16 otherList[SEEDED_FIXED_SPAWN_COUNT];
    u8 otherLevel[SEEDED_FIXED_SPAWN_COUNT];
    s32 mainCount = 0;
    s32 otherCount = 0;
    s32 entryCount = SEEDED_FIXED_SPAWN_COUNT;
    s32 targetMain;

    if (result == NULL || rng == NULL)
        return 0;
    if (!sSpawnRangeCache.valid || sSpawnRangeCache.rangeCount == 0)
        return 0;

    floorIndex = GetFloorIndexWithinDungeon(floorId, sSpawnRangeCache.startFloorId, sSpawnRangeCache.floorCount);

    for (i = 0; i < sSpawnRangeCache.rangeCount; i++) {
        const SeededSpawnRange *range = &sSpawnRangeCache.ranges[i];
        s16 species = range->species;

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (floorIndex < range->start || floorIndex > range->end)
            continue;

        if (SpeciesMatchesTypeMask(species, mainTypeMask)) {
            if (mainCount < SEEDED_FIXED_SPAWN_COUNT) {
                mainList[mainCount++] = species;
                mainLevel[mainCount - 1] = range->level;
            }
        } else {
            if (otherCount < SEEDED_FIXED_SPAWN_COUNT) {
                otherList[otherCount++] = species;
                otherLevel[otherCount - 1] = range->level;
            }
        }
    }

    targetMain = (entryCount * SEEDED_MAIN_TYPE_PERCENT + 99) / 100;
    if (targetMain > entryCount)
        targetMain = entryCount;

    // Fill main slots first
    for (i = 0; i < targetMain && count < entryCount; i++) {
        s16 species;
        s32 level = 0;
        if (i < mainCount)
            species = mainList[i], level = mainLevel[i];
        else if (mainCount > 0) {
            s32 idx = i % mainCount;
            species = mainList[idx];
            level = mainLevel[idx];
        }
        else if (otherCount > 0) {
            s32 idx = i % otherCount;
            species = otherList[idx];
            level = otherLevel[idx];
        }
        else
            break;
        if (level <= 0 && sSpawnRangeCache.valid)
            level = CalcSeededSpawnLevel(sSpawnRangeCache.seed, dungeonId, floorIndex, sSpawnRangeCache.floorCount);
        if (level <= 0)
            level = RollSpawnLevel(rng, dungeonId, floorId);
        SetSpeciesLevelToExtract(&result->spawns[count], level, species);
        count++;
    }

    // Fill remaining slots with other type (or reuse main if needed)
    for (; count < entryCount; count++) {
        s16 species;
        s32 idx = count - targetMain;
        s32 level = 0;
        if (idx < otherCount) {
            species = otherList[idx];
            level = otherLevel[idx];
        }
        else if (otherCount > 0) {
            s32 wrapped = idx % otherCount;
            species = otherList[wrapped];
            level = otherLevel[wrapped];
        }
        else if (mainCount > 0) {
            s32 wrapped = idx % mainCount;
            species = mainList[wrapped];
            level = mainLevel[wrapped];
        }
        else {
            // Last resort: first range species
            if (sSpawnRangeCache.rangeCount > 0) {
                species = sSpawnRangeCache.ranges[0].species;
                level = sSpawnRangeCache.ranges[0].level;
            }
            else
                species = MONSTER_BULBASAUR;
        }
        if (level <= 0 && sSpawnRangeCache.valid)
            level = CalcSeededSpawnLevel(sSpawnRangeCache.seed, dungeonId, floorIndex, sSpawnRangeCache.floorCount);
        if (level <= 0)
            level = RollSpawnLevel(rng, dungeonId, floorId);
        SetSpeciesLevelToExtract(&result->spawns[count], level, species);
    }

    result->spawnCount = entryCount;
    return entryCount;
}

static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId, s32 seed)
{
    u8 tileset = 0;
    u8 mainType = TYPE_NONE;
    u32 mainTypeMask = 0;
    u32 spawnTypeMask = 0;
    u32 combinedMask = 0;

    if (result != NULL)
        tileset = result->tileset;

    mainType = GetMainTypeForTileset(tileset);
    if (mainType > TYPE_NONE && mainType < NUM_TYPES)
        mainTypeMask = (1u << mainType);

    if (tileset < SEEDED_TILESET_COUNT)
        spawnTypeMask = sTilesetTypeConfig[tileset].spawnMask & ~mainTypeMask;

    combinedMask = GetCombinedSpawnMask(tileset, mainTypeMask);

    EnsureSpawnRangeCache(seed, dungeonId, tileset, mainTypeMask, spawnTypeMask, combinedMask);

    MGBA_Infof("[SeedOverrides] Spawn selection tileset=%d mainType=%d mainMask=0x%08x spawnMask=0x%08x combined=0x%08x ranges=%d",
               tileset,
               mainType,
               mainTypeMask,
               spawnTypeMask,
               combinedMask,
               sSpawnRangeCache.rangeCount);

    PopulateSpawnTableFromRanges(result, rng, dungeonId, floorId, mainTypeMask, spawnTypeMask);
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

static u16 SelectPrimaryBossLoot(DungeonSeedRng *rng)
{
    static const u16 sPrimaryFallback[] = {
        ITEM_ORAN_BERRY,
        ITEM_SITRUS_BERRY,
        ITEM_REVIVER_SEED,
        ITEM_APPLE,
        ITEM_MAX_ELIXIR,
    };
    u16 itemId = SelectItemFromPool(ROGUE_ITEM_POOL_PRIMARY_LOOT, rng);

    if (itemId != ITEM_NOTHING)
        return itemId;

    if (rng == NULL)
        return sPrimaryFallback[DungeonRandInt(ARRAY_COUNT(sPrimaryFallback))];
    return sPrimaryFallback[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sPrimaryFallback))];
}

static u16 SelectSecondaryBossLoot(DungeonSeedRng *rng)
{
    static const u16 sSecondaryFallback[] = {BOSS_SECONDARY_LOOT_LEFT, BOSS_SECONDARY_LOOT_RIGHT};
    u16 itemId = SelectItemFromPool(ROGUE_ITEM_POOL_SECONDARY_LOOT, rng);

    if (itemId != ITEM_NOTHING)
        return itemId;

    if (rng == NULL)
        return sSecondaryFallback[DungeonRandInt(ARRAY_COUNT(sSecondaryFallback))];
    return sSecondaryFallback[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSecondaryFallback))];
}

static void SeedBossRewardLoot(BossFightConfig *bossFight, s32 seed, u8 dungeonId)
{
    DungeonSeedRng rng;

    if (bossFight == NULL)
        return;

    if (seed < 0)
        seed = 0;

    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x4C4F4F54); // "LOOT"
    bossFight->dropItem = SelectPrimaryBossLoot(&rng);
    bossFight->secondaryDropLeft = SelectSecondaryBossLoot(&rng);
    bossFight->secondaryDropRight = SelectSecondaryBossLoot(&rng);
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
        result->bossFight.bossHP = GetSeededBossHP(dungeonId);
        result->bossFight.bossMusic = MUS_BOSS_BATTLE;
        SeedBossRewardLoot(&result->bossFight, seed, dungeonId);
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

    // Procedurally set HP based on dungeon progression and difficulty.
    result->bossFight.bossHP = GetSeededBossHP(dungeonId);

    // Procedurally select music
    result->bossFight.bossMusic = MUS_BOSS_BATTLE;

    // Procedurally select loot drops
    SeedBossRewardLoot(&result->bossFight, seed, dungeonId);

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
        bool8 isDeoxysBoss = (result->bossFight.bossSpecies == MONSTER_DEOXYS_NORMAL
            || result->bossFight.bossSpecies == MONSTER_DEOXYS_ATTACK
            || result->bossFight.bossSpecies == MONSTER_DEOXYS_DEFENSE
            || result->bossFight.bossSpecies == MONSTER_DEOXYS_SPEED);

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
            if (isDeoxysBoss) {
                // Deoxys boss -> Use custom procedural arena (not Lugia's fixed room)
                result->bossFight.roomTileset = 0;         // Special value: keep normal tileset
                result->bossFight.useFixedRoomLayout = FALSE;
                result->bossFight.fixedRoomNumber = 0;     // Not used
                roomName = "CustomArena-Deoxys";
            }
            else {
                // Psychic dungeon -> Use Lugia's boss room (Silver Trench)
                result->bossFight.roomTileset = 73;        // Silver Trench boss tileset
                result->bossFight.useFixedRoomLayout = TRUE;
                result->bossFight.fixedRoomNumber = 21;    // Lugia arena (Room 21)
                roomName = "SilverTrench";
            }
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

static s16 GetSeededBossHP(u8 dungeonId)
{
    s32 dungeonNumber = GetDungeonNumberForFloorScaling(dungeonId);
    s32 difficulty = GetGameDifficultySetting();
    s32 difficultyBonus = 0;

    if (dungeonNumber < 1)
        dungeonNumber = 1;

    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    switch (difficulty) {
        case DIFFICULTY_HARD:
            difficultyBonus = 50;
            break;
        case DIFFICULTY_NIGHTMARE:
            difficultyBonus = 100;
            break;
        case DIFFICULTY_NORMAL:
        default:
            difficultyBonus = 0;
            break;
    }

    return (s16)(100 * dungeonNumber + difficultyBonus);
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
    s32 progressionNumber = GetDungeonNumberForFloorScaling(dungeonId);
    s32 dungeonCount = GetSequentialDungeonCountForRun();
    const char *typeLabel = NULL;
#ifdef DEV
    char typeBuffer[20];
    const char *bannerTypeLabel = NULL;
    char bannerTypeBuffer[20];
#endif

#ifdef DEV
    if (sSeededDungeonNameType > TYPE_NONE && sSeededDungeonNameType < NUM_TYPES) {
        sprintfStatic(typeBuffer, "[%s]", gUnformattedTypeStrings[sSeededDungeonNameType]);
        typeLabel = typeBuffer;
        sprintfStatic(bannerTypeBuffer, "%s", gUnformattedTypeStrings[sSeededDungeonNameType]);
        bannerTypeLabel = bannerTypeBuffer;
    }
#else
    (void)typeLabel;
#endif

    (void)seed;

    if (progressionNumber < 1)
        progressionNumber = 1;

    if (dungeonCount < 1)
        dungeonCount = 1;

    if (typeLabel != NULL) {
        sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d) %s",
                      progressionNumber, dungeonCount, typeLabel);
    } else {
        sprintfStatic((char *)sSeededDungeonName1[dungeonId], "D (%d/%d)",
                      progressionNumber, dungeonCount);
    }

#ifdef DEV
    if (bannerTypeLabel != NULL) {
        sprintfStatic((char *)sSeededDungeonName2[dungeonId], "D %d of %d %s",
                      progressionNumber, dungeonCount, bannerTypeLabel);
    } else
#endif
    {
        sprintfStatic((char *)sSeededDungeonName2[dungeonId], "D %d of %d",
                      progressionNumber, dungeonCount);
    }

    sSeededDungeonNameValid[dungeonId] = TRUE;
}

UNUSED static const char *SelectPrefixForDungeon(u8 dungeonId, DungeonSeedRng *rng, char *scratch, s32 scratchSize)
{
    if (scratch != NULL && scratchSize > 1 && CopyFirstTokenFromBaseName(dungeonId, scratch, scratchSize))
        return scratch;

    return sSeededPrefixTable[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSeededPrefixTable))];
}

UNUSED static bool8 CopyFirstTokenFromBaseName(u8 dungeonId, char *buffer, s32 bufferSize)
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

UNUSED static s32 GetSelectedTypeForDisplay(void)
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
    s32 count = GetSequentialDungeonCountForRun();

    for (i = 0; i < count; i++) {
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
    s32 count;

    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return -1;

    // Find the first unconquered dungeon in the sequential list
    count = GetSequentialDungeonCountForRun();
    for (i = 0; i < count; i++) {
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
static Entity *sCustomBossMinions[4] = {NULL};
static u8 sCustomBossMinionCount = 0;
static bool8 sCustomBossDefeated = FALSE;
static bool8 sCustomBossRewardsSpawned = FALSE;
static s32 sStairsSpawnX = 0;
static s32 sStairsSpawnY = 0;

void DungeonSeedOverrides_ResetBossFightState(void)
{
    s32 i;

    sCustomBossEntity = NULL;
    sCustomBossMinionCount = 0;
    sCustomBossDefeated = FALSE;
    sCustomBossRewardsSpawned = FALSE;

    for (i = 0; i < ARRAY_COUNT(sCustomBossMinions); i++)
        sCustomBossMinions[i] = NULL;
}

// Register the boss entity for tracking
void DungeonSeedOverrides_RegisterBossEntity(Entity *boss)
{
    sCustomBossEntity = boss;
}

void DungeonSeedOverrides_RegisterBossMinion(Entity *minion)
{
    s32 i;

    if (!EntityIsValid(minion))
        return;

    for (i = 0; i < sCustomBossMinionCount; i++) {
        if (sCustomBossMinions[i] == minion)
            return;
    }

    if (sCustomBossMinionCount >= ARRAY_COUNT(sCustomBossMinions))
        return;

    sCustomBossMinions[sCustomBossMinionCount++] = minion;
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

bool8 DungeonSeedOverrides_IsCustomBossMinion(Entity *pokemon)
{
    s32 i;

    if (pokemon == NULL)
        return FALSE;

    for (i = 0; i < sCustomBossMinionCount; i++) {
        if (sCustomBossMinions[i] == pokemon)
            return TRUE;
    }

    return FALSE;
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

static bool8 TrySpawnBossMoney(s32 x, s32 y, u8 quantity, const char *label)
{
    DungeonPos pos;
    Tile *tile;
    Item item;
    s32 terrain;

    if (x < 0 || y < 0 || x >= DUNGEON_MAX_SIZE_X || y >= DUNGEON_MAX_SIZE_Y) {
        MGBA_Warnf("[BossFaint] Skipping %s money at (%d, %d): out of bounds", label, x, y);
        return FALSE;
    }

    tile = GetTileMut(x, y);
    terrain = GetTerrainType(tile);
    if (terrain == TERRAIN_TYPE_WALL || (tile->terrainFlags & TERRAIN_TYPE_STAIRS) || tile->object != NULL) {
        MGBA_Warnf("[BossFaint] Skipping %s money at (%d, %d): blocked (terrain=%d, stairs=%d, object=%p)",
                   label, x, y, terrain, (tile->terrainFlags & TERRAIN_TYPE_STAIRS) != 0, tile->object);
        return FALSE;
    }

    item.flags = ITEM_FLAG_EXISTS;
    item.id = ITEM_POKE;
    item.quantity = quantity;
    pos.x = x;
    pos.y = y;
    SpawnItem(&pos, &item, TRUE);
    MGBA_Warnf("[BossFaint] Spawned %s money (quantity=%d) at (%d, %d)", label, quantity, x, y);
    return TRUE;
}

static bool8 AreBossMinionsDefeated(void)
{
    s32 i;

    for (i = 0; i < sCustomBossMinionCount; i++) {
        Entity *minion = sCustomBossMinions[i];

        if (minion != NULL && EntityIsValid(minion)) {
            EntityInfo *info = GetEntInfo(minion);

            if (info != NULL && info->isNotTeamMember)
                return FALSE;
        }
    }

    return TRUE;
}

static u8 GetStrongestTeamLevel(void)
{
    s32 i;
    u8 maxLevel = 1;

    if (gRecruitedPokemonRef == NULL)
        return maxLevel;

    for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
        DungeonMon *mon = &gRecruitedPokemonRef->dungeonTeam[i];
        if (DungeonMonExists(mon) && mon->level > maxLevel)
            maxLevel = mon->level;
    }

    return maxLevel;
}

// Lift a level 1 recruit to the target level while keeping stats consistent.
static void ApplyLevelGains(Pokemon *mon, u8 targetLevel)
{
    u8 currentLevel;
    s32 level;
    LevelData levelData;

    if (targetLevel < 1)
        targetLevel = 1;
    if (targetLevel > 100)
        targetLevel = 100;

    currentLevel = mon->level;
    if (currentLevel < 1)
        currentLevel = 1;

    if (targetLevel <= currentLevel) {
        mon->level = targetLevel;
        return;
    }

    for (level = currentLevel; level < targetLevel; level++) {
        s32 newStat;
        GetLvlUpEntry(&levelData, mon->speciesNum, level + 1);

        newStat = mon->pokeHP + levelData.gainHP;
        mon->pokeHP = (newStat > 0xFFFF) ? 0xFFFF : (u16)newStat;

        newStat = mon->offense.att[0] + levelData.gainAtt[0];
        mon->offense.att[0] = (newStat > 0xFF) ? 0xFF : (u8)newStat;

        newStat = mon->offense.att[1] + levelData.gainAtt[1];
        mon->offense.att[1] = (newStat > 0xFF) ? 0xFF : (u8)newStat;

        newStat = mon->offense.def[0] + levelData.gainDef[0];
        mon->offense.def[0] = (newStat > 0xFF) ? 0xFF : (u8)newStat;

        newStat = mon->offense.def[1] + levelData.gainDef[1];
        mon->offense.def[1] = (newStat > 0xFF) ? 0xFF : (u8)newStat;
    }

    mon->level = targetLevel;
    GetLvlUpEntry(&levelData, mon->speciesNum, targetLevel);
    mon->currExp = levelData.expRequired;
}

static bool8 HasFriendAreaSpaceForSpecies(s16 species)
{
    FriendAreaCapacity capacity;
    u8 friendArea = GetFriendArea(species);

    GetFriendAreaCapacity2(friendArea, &capacity, FALSE, FALSE);
    return capacity.hasFriendArea && capacity.currNoPokemon < capacity.maxPokemon;
}

static s16 SelectBossRewardRecruitSpecies(void)
{
    DungeonSeedRng rng;
    s32 seed = sub_8011C34();
    u8 dungeonId = gDungeon->unk644.dungeonLocation.id;
    s32 attempts;

    if (seed < 0)
        seed = 0;

    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x52454352); // "RECR"
    for (attempts = 0; attempts < 200; attempts++) {
        s16 species = (s16)DungeonSeedRng_NextRange(&rng, MONSTER_NONE + 1, MONSTER_MAX);

        if (species <= MONSTER_NONE || species >= MONSTER_MAX)
            continue;
        if (species == MONSTER_DECOY || species == MONSTER_STATUE)
            continue;
        if (IS_CASTFORM_FORM_MONSTER(species) || IS_DEOXYS_FORM_MONSTER(species))
            continue;
        if (!HasFriendAreaSpaceForSpecies(species))
            continue;

        return species;
    }

    return MONSTER_NONE;
}

static bool8 TryAddBossRewardRecruit(void)
{
    Pokemon recruit;
    DungeonLocation location;
    s16 species;
    u32 scaledLevel;

    if (GetFriendSum_808D480() >= MAX_RECRUITED_POKEMON)
        return FALSE;

    species = SelectBossRewardRecruitSpecies();
    if (species <= MONSTER_NONE)
        return FALSE;

    location = gDungeon->unk644.dungeonLocation;
    CreateLevel1Pokemon(&recruit, species, NULL, 0, &location, NULL);

    scaledLevel = (GetStrongestTeamLevel() * 80) / 100;
    if (scaledLevel < 1)
        scaledLevel = 1;
    if (scaledLevel > 100)
        scaledLevel = 100;

    ApplyLevelGains(&recruit, (u8)scaledLevel);

    if (TryAddPokemonToRecruited(&recruit) == NULL)
        return FALSE;

    CopyMonsterNameToBuffer(gFormatBuffer_Monsters[0], species);
    DisplayDungeonMessage(NULL, sBossRewardRecruitSuccessText, TRUE);
    return TRUE;
}

static s32 PromptBossRewardChoice(void)
{
    s32 choice;

    do {
        choice = DisplayDungeonMenuMessage(NULL, sBossRewardPrompt, sBossRewardMenu, 0x701);
    } while (choice < BOSS_REWARD_RARE_ITEMS);

    return choice;
}

static void TrySpawnBossRewards(void)
{
    const BossFightConfig *bossFight;
    Tile *tile;
    s32 dropX;
    s32 dropY;
    s32 rewardChoice;

    if (sCustomBossRewardsSpawned || !sCustomBossDefeated || !AreBossMinionsDefeated())
        return;

    bossFight = DungeonFloorSpawns_GetBossFightConfig();
    if (bossFight == NULL) {
        MGBA_Warnf("[BossFaint] ERROR: bossFight is NULL!");
        return;
    }

    sCustomBossRewardsSpawned = TRUE;

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
    dropX = sStairsSpawnX;
    dropY = sStairsSpawnY + 1;  // One tile in front of stairs

    while (1) {
        rewardChoice = PromptBossRewardChoice();
        if (rewardChoice == BOSS_REWARD_RECRUIT) {
            if (TryAddBossRewardRecruit())
                break;
            DisplayDungeonMessage(NULL, sBossRewardRecruitFailText, TRUE);
            continue;
        }
        break;
    }

    if (rewardChoice == BOSS_REWARD_RARE_ITEMS) {
        if (bossFight->dropItem != ITEM_NOTHING)
            TrySpawnBossLoot(bossFight->dropItem, dropX, dropY, "primary");
        if (bossFight->secondaryDropLeft != ITEM_NOTHING)
            TrySpawnBossLoot(bossFight->secondaryDropLeft, dropX - 1, dropY, "secondary-left");
        if (bossFight->secondaryDropRight != ITEM_NOTHING)
            TrySpawnBossLoot(bossFight->secondaryDropRight, dropX + 1, dropY, "secondary-right");
    } else if (rewardChoice == BOSS_REWARD_MONEY) {
        TrySpawnBossMoney(dropX, dropY, BOSS_REWARD_POKE_QUANTITY_3000, "primary");
    }

    // Update minimap and visibility
    UpdateTrapsVisibility();
    UpdateMinimap();
}

// Handle boss defeat - spawn stairs and drop loot
void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon)
{
    MGBA_Warnf("[BossFaint] HandleBossFaint called for entity %p (boss=%p)", pokemon, sCustomBossEntity);

    if (pokemon != sCustomBossEntity) {
        MGBA_Warnf("[BossFaint] Entity mismatch - not our custom boss, returning");
        return;
    }

    sCustomBossDefeated = TRUE;
    MGBA_Warnf("[BossFaint] Boss defeated! Checking minions before spawning stairs");
    DungeonSeedOverrides_RegisterBossEntity(NULL);

    if (!AreBossMinionsDefeated())
        MGBA_Warnf("[BossFaint] Boss defeated but minions remain - delaying stairs");

    TrySpawnBossRewards();
}

void DungeonSeedOverrides_HandleBossMinionFaint(Entity *pokemon)
{
    s32 i;

    if (!DungeonSeedOverrides_IsCustomBossMinion(pokemon))
        return;

    for (i = 0; i < sCustomBossMinionCount; i++) {
        if (sCustomBossMinions[i] == pokemon) {
            sCustomBossMinions[i] = NULL;
            break;
        }
    }

    MGBA_Warnf("[BossFaint] Boss minion defeated; checking stairs spawn");
    TrySpawnBossRewards();
}

s32 DungeonSeedOverrides_GetSuperTrapFloor(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 eligibleCount;
    s32 kecleonFloors[SEEDED_KECLEON_SHOP_COUNT] = {0};
    s32 candidates[SEEDED_MAX_FLOORS];
    s32 candidateCount = 0;
    s32 i;

    if (floorCount < 2)
        return 0;

    // Avoid the boss floor (last floor) so traps don't conflict with boss rooms
    // floorCount is "final floor + 1" so subtract 2 to drop the boss layer
    eligibleCount = floorCount - 2;
    if (eligibleCount <= 0)
        return 0;

    DungeonSeedOverrides_GetKecleonFloors(dungeonId, seed, &kecleonFloors[0], &kecleonFloors[1]);
    for (i = 0; i < eligibleCount && i < SEEDED_MAX_FLOORS; i++) {
        if (i == kecleonFloors[0] || i == kecleonFloors[1])
            continue;
        candidates[candidateCount++] = i;
    }

    if (candidateCount == 0) {
        MGBA_Warnf("[SuperTrap] No open floors for guaranteed traps (dungeon=%d eligible=%d kec1=%d kec2=%d)",
                   dungeonId, eligibleCount, kecleonFloors[0], kecleonFloors[1]);
        return 0;
    }

    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x54524150); // "TRAP"
    return candidates[DungeonSeedRng_NextRange(&rng, 0, candidateCount)];
}

s32 DungeonSeedOverrides_GetGuaranteedMonsterHouseFloor(u8 dungeonId, s32 seed)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 eligibleCount;
    s32 kecleonFloors[SEEDED_KECLEON_SHOP_COUNT] = {0};
    s32 superTrapFloor;
    s32 candidates[SEEDED_MAX_FLOORS];
    s32 candidateCount = 0;
    s32 i;

    if (floorCount < 2)
        return 0;

    eligibleCount = floorCount - 2;
    if (eligibleCount <= 0)
        return 0;

    DungeonSeedOverrides_GetKecleonFloors(dungeonId, seed, &kecleonFloors[0], &kecleonFloors[1]);
    superTrapFloor = DungeonSeedOverrides_GetSuperTrapFloor(dungeonId, seed);

    // Build a list of eligible floors that aren't reserved by other guarantees
    for (i = 0; i < eligibleCount && i < SEEDED_MAX_FLOORS; i++) {
        if (i == kecleonFloors[0] || i == kecleonFloors[1] || i == superTrapFloor)
            continue;
        candidates[candidateCount++] = i;
    }

    if (candidateCount == 0) {
        MGBA_Warnf("[MonsterHouse] No open floors for guaranteed Monster House (dungeon=%d eligible=%d kec1=%d kec2=%d trap=%d)",
                   dungeonId, eligibleCount, kecleonFloors[0], kecleonFloors[1], superTrapFloor);
        return 0;
    }

    if (candidateCount == 1)
        return candidates[0];

    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x4D4F4E48); // "MONH"
    rng.state ^= (u32)kecleonFloors[0] * 0x27D4EB2D;
    rng.state ^= (u32)kecleonFloors[1] * 0xA511E9B5;
    rng.state ^= (u32)superTrapFloor * 0x45D9F3B;
    return candidates[DungeonSeedRng_NextRange(&rng, 0, candidateCount)];
}

// Deterministically select which floors (0-indexed) should have Kecleon shops
// Floors are drawn without replacement from 0 to (floorCount - 3), excluding the boss floor and penultimate floor
void DungeonSeedOverrides_GetKecleonFloors(u8 dungeonId, s32 seed, s32 *floor0Out, s32 *floor1Out)
{
    DungeonSeedRng rng;
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 eligibleCount;
    s32 first = 0;
    s32 second = 0;

    if (floor0Out != NULL)
        *floor0Out = 0;
    if (floor1Out != NULL)
        *floor1Out = 0;

    // Ensure we have at least 2 floors (one for Kecleon, one for boss)
    if (floorCount <= 2)
        return;

    // Boss is on the final floor (floorCount - 1), so Kecleon can be on 0 to (floorCount - 3)
    // floorCount is "final floor + 1", so subtract 2 to remove the boss layer
    eligibleCount = floorCount - 2;  // Exclusive upper bound for range function

    // Use a dedicated salt for Kecleon shop placement
    rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0x4B45434C);
    first = DungeonSeedRng_NextRange(&rng, 0, eligibleCount);

    // Draw a second, distinct floor if possible
    if (eligibleCount > 1) {
        s32 secondRoll = DungeonSeedRng_NextRange(&rng, 0, eligibleCount - 1);

        if (secondRoll >= first)
            secondRoll++;
        second = secondRoll;
    } else {
        second = first;
    }

    if (floor0Out != NULL)
        *floor0Out = first;
    if (floor1Out != NULL)
        *floor1Out = second;

    MGBA_Warnf("[Kecleon] Shop floors selected: dungeon=%d seed=%d floorA=%d floorB=%d eligible=%d",
               dungeonId, seed, first, second, eligibleCount);
}

// Legacy wrapper: returns the primary Kecleon floor (first draw)
s32 DungeonSeedOverrides_GetKecleonFloor(u8 dungeonId, s32 seed)
{
    s32 floor0 = 0;

    DungeonSeedOverrides_GetKecleonFloors(dungeonId, seed, &floor0, NULL);
    return floor0;
}

#ifdef DEV
static s32 GetSpawnTableCount(const SpawnPokemonData *spawnTable)
{
    s32 i;

    if (spawnTable == NULL)
        return 0;

    for (i = 0; i < MONSTER_SPAWNS_ARR_COUNT; i++) {
        if (ExtractSpeciesIndex((SpawnPokemonData *)&spawnTable[i]) == 0)
            break;
    }

    return i;
}

static const SeededSpawnRange *FindSpawnRangeForSpecies(s16 species)
{
    s32 i;

    if (!sSpawnRangeCache.valid)
        return NULL;

    for (i = 0; i < sSpawnRangeCache.rangeCount; i++) {
        if (sSpawnRangeCache.ranges[i].species == species)
            return &sSpawnRangeCache.ranges[i];
    }

    return NULL;
}

static s32 AdjustSeedDumpSpawnLevel(s32 level, bool8 bossEnabled)
{
    s32 adjusted = level;
    u32 difficulty = GetGameDifficultySetting();

    if (adjusted < 1)
        adjusted = 1;
    if (bossEnabled)
        return adjusted;

    if (difficulty >= NUM_DIFFICULTY_SETTINGS)
        difficulty = DIFFICULTY_NORMAL;

    switch (difficulty) {
        case DIFFICULTY_NORMAL:
            adjusted = (adjusted * 60) / 100;
            break;
        case DIFFICULTY_HARD:
            adjusted = (adjusted * 80) / 100;
            break;
        case DIFFICULTY_NIGHTMARE:
        default:
            break;
    }

    if (adjusted < 1)
        adjusted = 1;
    return adjusted;
}

static void GetSpeciesNameForLog(char *out, size_t outLen, s16 species)
{
    const char *name = GetMonSpecies(species);

    if (outLen == 0)
        return;

    if (name != NULL) {
        sprintf(out, "%.*s", (int)(outLen - 1), name);
    }
    else {
        sprintf(out, "species_%d", species);
    }
}

static void GetItemNameForLog(char *out, size_t outLen, u16 itemId)
{
    const char *name = (itemId < NUMBER_OF_ITEM_IDS) ? gItemParametersData[itemId].name : NULL;
    size_t i;
    size_t j = 0;

    if (outLen == 0)
        return;

    if (name == NULL) {
        sprintf(out, "item_%d", itemId);
        return;
    }

    for (i = 0; name[i] != '\0' && j + 1 < outLen; i++) {
        u8 c = (u8)name[i];
        // Game strings include control/icon bytes; replace non-ASCII with '?' for logs.
        if (c < 0x20 || c >= 0x7F)
            c = '?';
        out[j++] = (char)c;
    }
    out[j] = '\0';
}

void DungeonSeedOverrides_LogSeedDump(s32 seed, u8 dungeonId, s32 floorId, s32 startFloorId,
                                      const DungeonSeedFloorOverrides *overrides,
                                      SpawnPokemonData *spawnTable)
{
    // Use the 1-based dungeon progression number in seed dump output.
    s32 dungeonNumber = GetDungeonNumberForFloorScaling(dungeonId);
    s32 floorCount = DungeonSeedOverrides_GetFloorCount(seed, dungeonId);
    s32 floorIndex = GetFloorIndexWithinDungeon(floorId, startFloorId, floorCount);
    s32 spawnCount = GetSpawnTableCount(spawnTable);
    s32 bossFloorId = (floorCount > 0) ? (floorCount - 1) : floorId;
    s32 kecleonFloors[SEEDED_KECLEON_SHOP_COUNT] = {0};
    s32 superTrapFloor = 0;
    s32 monsterHouseFloor = 0;
    s32 rangeCount = 0;
    u8 tileset = 0;
    u8 bossEnabled = FALSE;
    s32 i;
    char speciesName[32];
    TypeSelectionSaveData typeSelectionData;

    if (overrides != NULL) {
        tileset = overrides->tileset;
        bossEnabled = overrides->bossFight.enabled;
    }

    if (overrides != NULL) {
        u8 mainType = GetMainTypeForTileset(tileset);
        u32 mainTypeMask = 0;
        u32 spawnTypeMask = 0;
        u32 combinedMask = 0;

        if (mainType > TYPE_NONE && mainType < NUM_TYPES)
            mainTypeMask = (1u << mainType);

        if (tileset < SEEDED_TILESET_COUNT)
            spawnTypeMask = sTilesetTypeConfig[tileset].spawnMask & ~mainTypeMask;

        combinedMask = GetCombinedSpawnMask(tileset, mainTypeMask);
        EnsureSpawnRangeCache(seed, dungeonId, tileset, mainTypeMask, spawnTypeMask, combinedMask);
    }

    if (sSpawnRangeCache.valid)
        rangeCount = sSpawnRangeCache.rangeCount;

    // Use WARN level so the host emulator log captures these rows (INFO is often filtered out).
    MGBA_Warnf("SEED_DUMP_HEADER,meta,seed,dungeon_id,floor_id,start_floor_id,floor_index,floor_count,tileset,spawn_count,range_count,boss_enabled");
    MGBA_Warnf("SEED_DUMP,meta,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
               seed, dungeonNumber, floorId, startFloorId, floorIndex, floorCount, tileset, spawnCount, rangeCount, bossEnabled);

    TypeSelection_WriteSaveData(&typeSelectionData);
    MGBA_Warnf("SEED_DUMP_HEADER,save_overrides,seed,diff,skip,recruit,tc,ta,tsc,tsa,bc,ba,done,await");
    MGBA_Warnf("SEED_DUMP,save_overrides,%d,%u,%u,%u,%d,%d,%d,%d,%d,%d,%d,%u",
               sub_8011C34(),
               GetGameDifficultySetting(),
               GetSkipBasicRescuesSetting(),
               GetRecruitAllSetting(),
               typeSelectionData.committedTypeValid ? typeSelectionData.committedType : -1,
               typeSelectionData.activeTypeValid ? typeSelectionData.activeType : -1,
               typeSelectionData.committedTilesetValid ? typeSelectionData.committedTileset : -1,
               typeSelectionData.activeTilesetValid ? typeSelectionData.activeTileset : -1,
               typeSelectionData.committedBossValid ? typeSelectionData.committedBoss : -1,
               typeSelectionData.activeBossValid ? typeSelectionData.activeBoss : -1,
               typeSelectionData.completedDungeons,
               typeSelectionData.awaitingChoice);

    DungeonSeedOverrides_GetKecleonFloors(dungeonId, seed, &kecleonFloors[0], &kecleonFloors[1]);
    superTrapFloor = DungeonSeedOverrides_GetSuperTrapFloor(dungeonId, seed);
    monsterHouseFloor = DungeonSeedOverrides_GetGuaranteedMonsterHouseFloor(dungeonId, seed);

    MGBA_Warnf("SEED_DUMP_HEADER,special_floors,dungeon_id,kec_floor0_index,kec_floor0_num,kec_floor1_index,kec_floor1_num,super_trap_index,super_trap_num,monster_house_index,monster_house_num");
    MGBA_Warnf("SEED_DUMP,special_floors,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
               dungeonNumber,
               kecleonFloors[0], kecleonFloors[0] + 1,
               kecleonFloors[1], kecleonFloors[1] + 1,
               superTrapFloor, superTrapFloor + 1,
               monsterHouseFloor, monsterHouseFloor + 1);

    {
        DungeonSeedFloorOverrides bossOverrides;
        DungeonSeedRng bossRng = DungeonSeedRng_Init(seed, dungeonId, bossFloorId, 0xC0FFEE);
        char primaryName[64];
        char secondaryLeftName[64];
        char secondaryRightName[64];

        ClearFloorOverrides(&bossOverrides);
        PopulateBossFightConfig(&bossOverrides, &bossRng, dungeonId, bossFloorId, seed);
        MGBA_Warnf("SEED_DUMP_HEADER,boss_loot,dungeon_id,boss_floor_id,boss_enabled,boss_species,boss_species_name,primary,primary_name,secondary_left,secondary_left_name,secondary_right,secondary_right_name");
        GetSpeciesNameForLog(speciesName, sizeof(speciesName), bossOverrides.bossFight.bossSpecies);
        GetItemNameForLog(primaryName, sizeof(primaryName), bossOverrides.bossFight.dropItem);
        GetItemNameForLog(secondaryLeftName, sizeof(secondaryLeftName), bossOverrides.bossFight.secondaryDropLeft);
        GetItemNameForLog(secondaryRightName, sizeof(secondaryRightName), bossOverrides.bossFight.secondaryDropRight);
        MGBA_Warnf("SEED_DUMP,boss_loot,%d,%d,%d,%d,%s,%d,%s,%d,%s,%d,%s",
                   dungeonNumber,
                   bossFloorId,
                   bossOverrides.bossFight.enabled,
                   bossOverrides.bossFight.bossSpecies,
                   speciesName,
                   bossOverrides.bossFight.dropItem,
                   primaryName,
                   bossOverrides.bossFight.secondaryDropLeft,
                   secondaryLeftName,
                   bossOverrides.bossFight.secondaryDropRight,
                   secondaryRightName);
    }

    if (sSpawnRangeCache.valid) {
        MGBA_Warnf("SEED_DUMP_HEADER,spawn_range,dungeon_id,index,species,species_name,level,start_idx,end_idx,start_flr,end_flr");
        for (i = 0; i < sSpawnRangeCache.rangeCount; i++) {
            const SeededSpawnRange *range = &sSpawnRangeCache.ranges[i];
            s32 level = AdjustSeedDumpSpawnLevel(range->level, bossEnabled);
            GetSpeciesNameForLog(speciesName, sizeof(speciesName), range->species);
            MGBA_Warnf("SEED_DUMP,spawn_range,%d,%d,%d,%s,%d,%d,%d,%d,%d",
                       dungeonNumber,
                       i,
                       range->species,
                       speciesName,
                       level,
                       range->start,
                       range->end,
                       range->start + 1,
                       range->end + 1);
        }
    }

    if (spawnTable != NULL) {
        MGBA_Warnf("SEED_DUMP_HEADER,spawn_entry,dungeon_id,index,species,species_name,level,weight1,weight2,range_start_idx,range_end_idx,range_start_flr,range_end_flr");
        for (i = 0; i < spawnCount; i++) {
            SpawnPokemonData *entry = &spawnTable[i];
            s16 species = ExtractSpeciesIndex(entry);
            s32 level = AdjustSeedDumpSpawnLevel(ExtractLevel(entry), bossEnabled);
            const SeededSpawnRange *range = FindSpawnRangeForSpecies(species);
            s32 rangeStart = range ? range->start : -1;
            s32 rangeEnd = range ? range->end : -1;
            s32 rangeStartFloor = (rangeStart >= 0) ? (rangeStart + 1) : -1;
            s32 rangeEndFloor = (rangeEnd >= 0) ? (rangeEnd + 1) : -1;

            GetSpeciesNameForLog(speciesName, sizeof(speciesName), species);
            MGBA_Warnf("SEED_DUMP,spawn_entry,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d",
                       dungeonNumber,
                       i,
                       species,
                       speciesName,
                       level,
                       entry->randNum[0],
                       entry->randNum[1],
                       rangeStart,
                       rangeEnd,
                       rangeStartFloor,
                       rangeEndFloor);
        }
    }
}
#endif
