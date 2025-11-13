#include "global.h"
#include "dungeon_seed_overrides.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "pokemon_3.h"
#include "save.h"
#include "code_800D090.h"
#include "strings.h"

#define SEEDED_TILESET_COUNT 80
#define SEEDED_MIN_FLOORS 3
#define SEEDED_MAX_FLOORS 60
#define SEEDED_MIN_SPAWNS 6
#define SEEDED_MAX_SPAWNS 16
#define SEEDED_DUNGEON_NAME_MAX_LEN 32
#define SEEDED_PREFIX_BUFFER_LEN 16

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
static u8 SelectTileset(DungeonSeedRng *rng);
static void PopulateSpawnTable(DungeonSeedFloorOverrides *result, DungeonSeedRng *rng, s32 dungeonId, s32 floorId);
static void FinalizeSpawnWeights(DungeonSeedFloorOverrides *result);

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
    result->tileset = SelectTileset(&rng);
    PopulateSpawnTable(result, &rng, dungeonId, floorId);
    FinalizeSpawnWeights(result);
}

s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, 0, 0xF00DBABE);
    s32 band = DungeonSeedRng_NextRange(&rng, 0, ARRAY_COUNT(sFloorBandTable));
    s32 baseFloors = sFloorBandTable[band];
    s32 swing = DungeonSeedRng_NextRange(&rng, 0, 5);
    s32 extra = dungeonId % 4;
    s32 total = baseFloors + swing + extra;

    if (total < SEEDED_MIN_FLOORS)
        total = SEEDED_MIN_FLOORS;
    if (total > SEEDED_MAX_FLOORS)
        total = SEEDED_MAX_FLOORS;
    return total;
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

static u8 SelectTileset(DungeonSeedRng *rng)
{
    return (u8)DungeonSeedRng_NextRange(rng, 0, SEEDED_TILESET_COUNT);
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
    const char *suffixPrimary;
    const char *suffixSecondary;
    char prefixBuffer[SEEDED_PREFIX_BUFFER_LEN];

    prefix = SelectPrefixForDungeon(dungeonId, &rng, prefixBuffer, ARRAY_COUNT(prefixBuffer));
    suffixPrimary = sSeededSuffixTable[DungeonSeedRng_NextRange(&rng, 0, ARRAY_COUNT(sSeededSuffixTable))];
    suffixSecondary = suffixPrimary;

    if (ARRAY_COUNT(sSeededSuffixTable) > 1) {
        do {
            suffixSecondary = sSeededSuffixTable[DungeonSeedRng_NextRange(&rng, 0, ARRAY_COUNT(sSeededSuffixTable))];
        } while (suffixSecondary == suffixPrimary);
    }

    sprintfStatic((char *)sSeededDungeonName1[dungeonId], "%s %s", prefix, suffixPrimary);
    sprintfStatic((char *)sSeededDungeonName2[dungeonId], "%s %s", prefix, suffixSecondary);
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
