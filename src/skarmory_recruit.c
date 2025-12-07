#include "global.h"
#include "skarmory_recruit.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
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
static u32 MixSeeds(u32 dungeonSeed, s32 personalitySeed);
static bool8 IsLegendarySpecies(s16 species);
static bool8 IsEligibleSpecies(s16 species);

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
    s32 i;

    EnsureInitialized();
    if (!SkarmoryRecruit_IsAvailable())
        return -1;

    mixedSeed = MixSeeds(sSkarmoryRecruitState.data.dungeonSeed, sSkarmoryRecruitState.data.personalitySeed);

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

// Mix the dungeon-specific seed with the personality quiz seed so Skarmory's
// offer changes with both.
static u32 MixSeeds(u32 dungeonSeed, s32 personalitySeed)
{
    u32 mixed = dungeonSeed ^ ((u32)personalitySeed * 0x9E3779B9);

    mixed ^= mixed >> 16;
    mixed *= 0x85EBCA6B;
    mixed ^= mixed >> 13;
    mixed *= 0xC2B2AE35;
    mixed ^= mixed >> 16;

    return mixed;
}

static bool8 IsLegendarySpecies(s16 species)
{
    switch (species) {
        case MONSTER_ARTICUNO:
        case MONSTER_ZAPDOS:
        case MONSTER_MOLTRES:
        case MONSTER_GROUDON:
        case MONSTER_RAYQUAZA:
        case MONSTER_RAYQUAZA_CUTSCENE:
        case MONSTER_KYOGRE:
        case MONSTER_LUGIA:
        case MONSTER_MEWTWO:
        case MONSTER_JIRACHI:
        case MONSTER_MEW:
        case MONSTER_LATIAS:
        case MONSTER_LATIOS:
        case MONSTER_ENTEI:
        case MONSTER_RAIKOU:
        case MONSTER_SUICUNE:
        case MONSTER_HO_OH:
        case MONSTER_REGIROCK:
        case MONSTER_REGICE:
        case MONSTER_REGISTEEL:
        case MONSTER_CELEBI:
            return TRUE;
    }
    return FALSE;
}

static bool8 IsEligibleSpecies(s16 species)
{
    if (species <= 0 || species > NUM_RECRUITABLE_MONSTERS)
        return FALSE;
    if (IS_CASTFORM_FORM_MONSTER(species) || IS_DEOXYS_FORM_MONSTER(species))
        return FALSE;
    if (species == MONSTER_UNOWN_EMARK || species == MONSTER_UNOWN_QMARK)
        return FALSE;
    if (species == MONSTER_DECOY || species == MONSTER_STATUE)
        return FALSE;
    if (IsLegendarySpecies(species))
        return FALSE;
    if (!IsExclusivePokemonUnlocked(species))
        return FALSE;

    return TRUE;
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
