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
    if (species <= 0 || species > NUM_RECRUITABLE_MONSTERS)
        return FALSE;
    if (IS_CASTFORM_FORM_MONSTER(species) || IS_DEOXYS_FORM_MONSTER(species))
        return FALSE;
    if (species == MONSTER_DECOY || species == MONSTER_STATUE)
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
    s32 baseCost = 25;
    s32 rarity = GetRecruitRate(species);
    s32 level = SkarmoryRecruit_GetRecommendedLevel();
    s32 cost;
    u32 difficulty = GetGameDifficultySetting();
    static const s32 sDifficultySurcharge[] = {
        [DIFFICULTY_NORMAL] = 0,
        [DIFFICULTY_HARD] = 500,
        [DIFFICULTY_NIGHTMARE] = 1000,
    };

    if (rarity < 0)
        rarity = -rarity;
    if (rarity > 200)
        rarity = 200;

    cost = baseCost + rarity * 2 + level * 3;
    if (difficulty < ARRAY_COUNT(sDifficultySurcharge))
        cost += sDifficultySurcharge[difficulty];

    if (cost < 50)
        cost = 50;
    if (cost > MAX_TEAM_MONEY)
        cost = MAX_TEAM_MONEY;

    return cost;
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
