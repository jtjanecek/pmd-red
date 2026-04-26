#include "global.h"
#include "skarmory_recruit.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "constants/item.h"
#include "dungeon_seed_overrides.h"
#include "exclusive_pokemon.h"
#include "memory.h"
#include "pokemon.h"
#include "save.h"
#include "constants/spawn.h"

typedef struct SkarmoryRecruitState {
    SkarmoryRecruitSaveData data;
    bool8 initialized;
} SkarmoryRecruitState;

static EWRAM_DATA SkarmoryRecruitState sSkarmoryRecruitState = {0};

static void EnsureInitialized(void);
static u32 MixSeeds(u32 dungeonNumber, s32 personalitySeed);
static bool8 IsEligibleSpecies(s16 species);
static u8 GetMaxRecruitedLevel(void);
static void ApplyLevelGains(Pokemon *mon, u8 targetLevel);
static s32 GetBaseStatTotal(s16 species);
static s32 GetMaxRecruitBst(void);

void SkarmoryRecruit_Init(void)
{
    MemoryFill8(&sSkarmoryRecruitState, 0, sizeof(sSkarmoryRecruitState));
    sSkarmoryRecruitState.data.lastDungeonId = -1;
    sSkarmoryRecruitState.initialized = TRUE;
}

void SkarmoryRecruit_Reset(void)
{
    SkarmoryRecruit_Init();
}

void SkarmoryRecruit_ReadSaveData(const SkarmoryRecruitSaveData *data)
{
    EnsureInitialized();
    if (data != NULL) {
        sSkarmoryRecruitState.data = *data;
    }
}

void SkarmoryRecruit_WriteSaveData(SkarmoryRecruitSaveData *data)
{
    if (data == NULL)
        return;

    EnsureInitialized();
    *data = sSkarmoryRecruitState.data;
}

void SkarmoryRecruit_SetDungeonCompleted(s16 dungeonId)
{
    s32 personalitySeed;
    u32 dungeonSeed;

    EnsureInitialized();

    if (dungeonId < 0 || dungeonId >= NUM_DUNGEONS) {
        sSkarmoryRecruitState.data.available = 0;
        sSkarmoryRecruitState.data.lastDungeonId = -1;
        return;
    }

    personalitySeed = sub_8011C34();
    if (personalitySeed == -1) {
        personalitySeed = 0;
    }

    dungeonSeed = DungeonSeedOverrides_GetDungeonRngSeed(personalitySeed, (u8)dungeonId, 0);

    sSkarmoryRecruitState.data.lastDungeonId = dungeonId;
    sSkarmoryRecruitState.data.dungeonSeed = dungeonSeed;
    sSkarmoryRecruitState.data.personalitySeed = personalitySeed;
    sSkarmoryRecruitState.data.available = 1;
}

bool8 SkarmoryRecruit_IsAvailable(void)
{
    EnsureInitialized();
    return (sSkarmoryRecruitState.data.available != 0) && (sSkarmoryRecruitState.data.lastDungeonId >= 0);
}

s16 SkarmoryRecruit_GetOfferedSpecies(void)
{
    s16 candidates[NUM_RECRUITABLE_MONSTERS];
    s32 candidateCount = 0;
    u32 mixedSeed;
    s32 dungeonNumber = 1;
    s32 i;

    EnsureInitialized();
    if (!SkarmoryRecruit_IsAvailable())
        return -1;

    if (sSkarmoryRecruitState.data.lastDungeonId >= 0
        && sSkarmoryRecruitState.data.lastDungeonId < NUM_DUNGEONS) {
        dungeonNumber = DungeonSeedOverrides_GetDungeonNumberForDisplay(
            (u8)sSkarmoryRecruitState.data.lastDungeonId);
    }

    // Pick deterministically from the run seed + dungeon number so offers are stable per run.
    mixedSeed = MixSeeds((u32)dungeonNumber, sSkarmoryRecruitState.data.personalitySeed);

    for (i = 1; i <= NUM_RECRUITABLE_MONSTERS; i++) {
        if (IsEligibleSpecies((s16)i)) {
            candidates[candidateCount++] = (s16)i;
        }
    }

    if (candidateCount == 0)
        return -1;

    return candidates[mixedSeed % candidateCount];
}

void SkarmoryRecruit_MarkUsed(void)
{
    EnsureInitialized();
    sSkarmoryRecruitState.data.available = 0;
}

static void EnsureInitialized(void)
{
    if (!sSkarmoryRecruitState.initialized)
        SkarmoryRecruit_Init();
}

// Mix the dungeon number with the personality quiz seed so Skarmory's
// offer is stable per run and per dungeon.
static u32 MixSeeds(u32 dungeonNumber, s32 personalitySeed)
{
    u32 mixed = dungeonNumber ^ ((u32)personalitySeed * 0x9E3779B9);

    mixed ^= mixed >> 16;
    mixed *= 0x85EBCA6B;
    mixed ^= mixed >> 13;
    mixed *= 0xC2B2AE35;
    mixed ^= mixed >> 16;

    return mixed;
}

static bool8 IsEligibleSpecies(s16 species)
{
    u32 spawn = GetGameSpawnSetting();

    if (species <= 0 || species > NUM_RECRUITABLE_MONSTERS)
        return FALSE;
    if (IS_CASTFORM_FORM_MONSTER(species) || IS_DEOXYS_FORM_MONSTER(species))
        return FALSE;
    if (species == MONSTER_DECOY || species == MONSTER_STATUE)
        return FALSE;
    if (species >= MONSTER_WEAVILE && spawn == SPAWN_CLASSIC)
        return FALSE;
    if (!IsExclusivePokemonUnlocked(species))
        return FALSE;

    return TRUE;
}

u8 SkarmoryRecruit_GetRecommendedLevel(void)
{
    u8 maxLevel = GetMaxRecruitedLevel();
    u32 scaled = (maxLevel * 8) / 10;

    if (scaled < 1)
        scaled = 1;
    if (scaled > 100)
        scaled = 100;

    return (u8)scaled;
}

void SkarmoryRecruit_SetRecruitLevel(Pokemon *pokemon)
{
    if (pokemon == NULL)
        return;

    ApplyLevelGains(pokemon, SkarmoryRecruit_GetRecommendedLevel());
}

static u8 GetMaxRecruitedLevel(void)
{
    s32 i;
    u8 maxLevel = 1;

    if (gRecruitedPokemonRef == NULL)
        return maxLevel;

    for (i = 0; i < NUM_MONSTERS; i++) {
        Pokemon *mon = &gRecruitedPokemonRef->pokemon[i];
        if (PokemonExists(mon) && mon->level > maxLevel) {
            maxLevel = mon->level;
        }
    }

    return maxLevel;
}

// Lift a freshly-created level 1 recruit up to the target level while keeping stats consistent
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

s32 SkarmoryRecruit_GetCostForSpecies(s16 species)
{
    s32 bst = GetBaseStatTotal(species);
    s32 cost = 0;
    s32 bstScale = 0;
    u32 difficulty = GetGameDifficultySetting();
    static const s32 sDifficultySurcharge[] = {
        [DIFFICULTY_NORMAL] = 1000,
        [DIFFICULTY_HARD] = 2000,
        [DIFFICULTY_NIGHTMARE] = 3000,
    };
    s32 surcharge = 0;
    s32 maxBst = 1;
    s32 dungeonNumber = 1;
    enum { BST_SCALE_MAX = 10000, BST_SCALE_MIN = 100 };

    if (bst < 0)
        bst = 0;

    if (difficulty < ARRAY_COUNT(sDifficultySurcharge))
        surcharge = sDifficultySurcharge[difficulty];

    if (sSkarmoryRecruitState.data.lastDungeonId >= 0
        && sSkarmoryRecruitState.data.lastDungeonId < NUM_DUNGEONS
        && DungeonSeedOverrides_IsEnabled(NULL)) {
        dungeonNumber = DungeonSeedOverrides_GetDungeonNumberForDisplay(
            (u8)sSkarmoryRecruitState.data.lastDungeonId);
        if (dungeonNumber < 1)
            dungeonNumber = 1;
    }

    maxBst = GetMaxRecruitBst();
    if (maxBst < 1)
        maxBst = 1;

    bstScale = (bst * BST_SCALE_MAX + maxBst / 2) / maxBst;
    if (bstScale < BST_SCALE_MIN)
        bstScale = BST_SCALE_MIN;
    if (bstScale > BST_SCALE_MAX)
        bstScale = BST_SCALE_MAX;
    if (species == MONSTER_STATUE)
        bstScale = BST_SCALE_MIN;

    cost = (surcharge * bstScale + (BST_SCALE_MAX / 2)) / BST_SCALE_MAX;
    cost += dungeonNumber * 75;
    if (cost > MAX_TEAM_MONEY)
        cost = MAX_TEAM_MONEY;

    return cost;
}

static s32 GetBaseStatTotal(s16 species)
{
    s32 hp = GetBaseHP(species);
    s32 atk = GetBaseOffensiveStat(species, OFFENSE_NRM);
    s32 spAtk = GetBaseOffensiveStat(species, OFFENSE_SP);
    s32 def = GetBaseDefensiveStat(species, OFFENSE_NRM);
    s32 spDef = GetBaseDefensiveStat(species, OFFENSE_SP);

    return hp + atk + spAtk + def + spDef;
}

static s32 GetMaxRecruitBst(void)
{
    s32 species;
    s32 maxBst = 0;

    for (species = 0; species < MONSTER_MAX; species++) {
        s32 bst;

        if (species == MONSTER_STATUE)
            continue;

        bst = GetBaseStatTotal(species);
        if (bst > maxBst)
            maxBst = bst;
    }

    if (maxBst < 1)
        maxBst = 1;

    return maxBst;
}

const void *const gSkarmoryRecruitLinkAnchor[] = {
    (const void *)SkarmoryRecruit_Init,
    (const void *)SkarmoryRecruit_Reset,
    (const void *)SkarmoryRecruit_ReadSaveData,
    (const void *)SkarmoryRecruit_WriteSaveData,
    (const void *)SkarmoryRecruit_SetDungeonCompleted,
    (const void *)SkarmoryRecruit_IsAvailable,
    (const void *)SkarmoryRecruit_GetOfferedSpecies,
    (const void *)SkarmoryRecruit_MarkUsed,
};
