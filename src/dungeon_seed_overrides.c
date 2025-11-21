#include "global.h"
#include "dungeon_seed_overrides.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "constants/rescue_dungeon_id.h"
#include "constants/bg_music.h"
#include "constants/item.h"
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
#include "items.h"
#include "structs/map.h"

#define SEEDED_TILESET_COUNT 75  // Max valid tileset ID (gNaturePowerCalledMoves uses max 74)
#define SEEDED_MIN_FLOORS 3
#define SEEDED_MAX_FLOORS 60
#define SEEDED_MIN_SPAWNS 6
#define SEEDED_MAX_SPAWNS 16
#define SEEDED_DUNGEON_NAME_MAX_LEN 32
#define SEEDED_PREFIX_BUFFER_LEN 16

// List of rescue dungeon IDs that appear in the dungeon list, for sequential unlocking
// Exactly 30 dungeons - ONLY single-part dungeons (no peaks, summits, grottos, pits, or 2nd floors)
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
    RESCUE_DUNGEON_BURIED_RELIC,         // 20
    RESCUE_DUNGEON_MURKY_CAVE,           // 21
    RESCUE_DUNGEON_DESERT_REGION,        // 22
    RESCUE_DUNGEON_SOUTHERN_CAVERN,      // 23
    RESCUE_DUNGEON_WYVERN_HILL,          // 24
    RESCUE_DUNGEON_SOLAR_CAVE,           // 25
    RESCUE_DUNGEON_DARKNIGHT_RELIC,      // 26
    RESCUE_DUNGEON_GRAND_SEA,            // 27
    RESCUE_DUNGEON_WATERFALL_POND,       // 28
    RESCUE_DUNGEON_UNOWN_RELIC,          // 29
    RESCUE_DUNGEON_PURITY_FOREST,        // 30 - FINAL DUNGEON (credits after)
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
static u8 SelectTileset(s32 floorId);
static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result);
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static const SeedSpeciesPool* GetBossPool(s32 floorId);
static u16 SelectRandomLoot(DungeonSeedRng *rng, s32 floorId);

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

static u8 sSeededDungeonName1[NUM_DUNGEONS][SEEDED_DUNGEON_NAME_MAX_LEN];
static u8 sSeededDungeonName2[NUM_DUNGEONS][SEEDED_DUNGEON_NAME_MAX_LEN];
static bool8 sSeededDungeonNameValid[NUM_DUNGEONS];
static s32 sSeededDungeonNameSeed = -2;

static void ResetSeededDungeonNameCache(void);
static void GenerateSeededDungeonNames(u8 dungeonId, s32 seed);
static const char *SelectPrefixForDungeon(u8 dungeonId, DungeonSeedRng *rng, char *scratch, s32 scratchSize);
static bool8 CopyFirstTokenFromBaseName(u8 dungeonId, char *buffer, s32 bufferSize);

void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId, DungeonSeedFloorOverrides *result)
{
    DungeonSeedRng rng;

    if (result == NULL)
        return;

    ClearFloorOverrides(result);
    rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0xC0FFEE);

    // NEW: Procedurally generate boss fight configuration
    PopulateBossFightConfig(result, &rng, dungeonId, floorId);

    // If boss fight enabled, use boss tileset; otherwise normal generation
    if (result->bossFight.enabled) {
        result->tileset = result->bossFight.roomTileset;
        result->spawnCount = 0;  // No normal spawns in boss rooms
    } else {
        result->tileset = SelectTileset(floorId);
        PopulateSpawnTable(result, &rng, dungeonId, floorId);
        FinalizeSpawnWeights(result);
    }
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
    return DungeonSeedRng_Next(&rng) | 1;
}

bool8 DungeonSeedOverrides_IsEnabled(s32 *seedOut)
{
    s32 seed = sub_8011C34();

    if (seed == -1)
        return FALSE;
    if (seedOut != NULL)
        *seedOut = seed;
    return TRUE;
}

const u8 *DungeonSeedOverrides_GetDungeonName(u8 dungeonId, bool8 secondLine)
{
    s32 seed;

    if (dungeonId >= NUM_DUNGEONS)
        return NULL;
    if (dungeonId > DUNGEON_PURITY_FOREST)
        return NULL;
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return NULL;

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
    s32 i;

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
    result->bossFight.roomTileset = 0;
    for (i = 0; i < 4; i++) {
        result->bossFight.minionSpecies[i] = 0;
    }
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
    if (floorId < 0)
        floorId = 0;
    return (u8)(floorId % SEEDED_TILESET_COUNT);
}

static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId)
{
    s32 entryCount;
    s32 i;

    entryCount = DungeonSeedRng_NextRange(rng, SEEDED_MIN_SPAWNS, SEEDED_MAX_SPAWNS + 1);
    if (entryCount > MONSTER_SPAWNS_ARR_COUNT)
        entryCount = MONSTER_SPAWNS_ARR_COUNT;

    for (i = 0; i < entryCount; i++) {
        const SeedSpeciesPool *pool = &sSpeciesPools[DungeonSeedRng_NextRange(rng, 0, ARRAY_COUNT(sSpeciesPools))];
        s16 species = pool->species[DungeonSeedRng_NextRange(rng, 0, pool->count)];
        s32 baseLevel = 3 + (dungeonId % 10) + floorId;
        s32 levelVariance = DungeonSeedRng_NextRange(rng, 0, 6);
        s32 level = baseLevel + levelVariance;

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

// Procedurally generate boss fight configuration from seed
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId)
{
    const SeedSpeciesPool *bossPool;
    const SeedSpeciesPool minionPool = {sPoolMinions, ARRAY_COUNT(sPoolMinions)};
    s32 bossIndex, i;

    (void)dungeonId;  // May use for dungeon-specific logic later

    // Procedurally determine if this floor has a boss
    // TESTING: Always spawn boss on floors >= 2 (normally would be 20% chance)
    if (floorId < 2) {
        result->bossFight.enabled = FALSE;
        return;
    }

    result->bossFight.enabled = TRUE;

    // Procedurally select boss species from tier-appropriate pool
    bossPool = GetBossPool(floorId);
    bossIndex = DungeonSeedRng_NextRange(rng, 0, bossPool->count);
    result->bossFight.bossSpecies = bossPool->species[bossIndex];

    // Procedurally set HP scaling with floor
    result->bossFight.bossHP = 300 + (floorId * 25);

    // Procedurally select music
    result->bossFight.bossMusic = MUS_BOSS_BATTLE;

    // Procedurally select loot drop
    result->bossFight.dropItem = SelectRandomLoot(rng, floorId);

    // Procedurally determine minion count (0-3)
    result->bossFight.minionCount = DungeonSeedRng_NextRange(rng, 0, 4);
    if (result->bossFight.minionCount > 3)
        result->bossFight.minionCount = 3;

    // Procedurally select minion species
    for (i = 0; i < result->bossFight.minionCount; i++) {
        s32 minionIdx = DungeonSeedRng_NextRange(rng, 0, minionPool.count);
        result->bossFight.minionSpecies[i] = minionPool.species[minionIdx];
    }

    // Procedurally select arena tileset (19 or 33)
    result->bossFight.roomTileset = DungeonSeedRng_NextRange(rng, 0, 2) == 0 ? 19 : 33;

    // Set behavior for boss identification
    result->bossFight.monsterBehavior = 0;  // Will define this constant later
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

    prefix = SelectPrefixForDungeon(dungeonId, &rng, prefixBuffer, ARRAY_COUNT(prefixBuffer));
    suffix = sSeededSuffixTable[DungeonSeedRng_NextRange(&rng, 0, ARRAY_COUNT(sSeededSuffixTable))];

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
        sprintfStatic((char *)sSeededDungeonName1[dungeonId], "Dungeon %d/%d",
                      dungeonIndex + 1, SEQUENTIAL_DUNGEON_COUNT);
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

// Check if this dungeon is the final one (Dungeon 30) to trigger credits
bool8 DungeonSeedOverrides_ShouldTriggerCredits(s16 rescueDungeonId)
{
    s32 seed;

    // Only check if overrides are enabled
    if (!DungeonSeedOverrides_IsEnabled(&seed))
        return FALSE;

    // Check if this is the last dungeon in our sequential list (Dungeon 30)
    if (SEQUENTIAL_DUNGEON_COUNT == 0)
        return FALSE;

    return (rescueDungeonId == sSequentialDungeonList[SEQUENTIAL_DUNGEON_COUNT - 1]);
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
    sStairsSpawnX = x;
    sStairsSpawnY = y;
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

    if (pokemon != sCustomBossEntity)
        return;

    DungeonSeedOverrides_RegisterBossEntity(NULL);

    bossFight = DungeonFloorSpawns_GetBossFightConfig();
    if (bossFight == NULL)
        return;

    // Spawn stairs at marked position
    tile = GetTileMut(sStairsSpawnX, sStairsSpawnY);
    if (tile != NULL) {
        tile->terrainFlags |= TERRAIN_TYPE_STAIRS;
        gDungeon->stairsSpawn.x = sStairsSpawnX;
        gDungeon->stairsSpawn.y = sStairsSpawnY;
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
