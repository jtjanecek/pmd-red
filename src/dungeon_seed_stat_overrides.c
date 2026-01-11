#include "global.h"
#include "dungeon_seed_stat_overrides.h"

#include "dungeon_util.h"
#include "type_selection.h"
#include "constants/monster.h"

#define BOSS_STAT_BASE 10
#define MINION_STAT_BASE 6
#define BOSS_STAT_DUNGEON_SCALE 3
#define MINION_STAT_DUNGEON_SCALE 2
#define STAT_TIER_LOW_BONUS 0
#define STAT_TIER_MEDIUM_BONUS 3
#define STAT_TIER_HIGH_BONUS 5
#define STAT_MAX 255
#define HP_MAX 999

static s32 ClampS32(s32 value, s32 min, s32 max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

static s32 GetTierBonus(u8 tier)
{
    switch (tier) {
        case STAT_TIER_LOW:
            return STAT_TIER_LOW_BONUS;
        case STAT_TIER_HIGH:
            return STAT_TIER_HIGH_BONUS;
        case STAT_TIER_MEDIUM:
        default:
            return STAT_TIER_MEDIUM_BONUS;
    }
}

static s32 CalcStatValue(s32 base, s32 scale, u8 dungeonNumber, u8 tier)
{
    s32 number = dungeonNumber;
    s32 value;

    if (number < 1)
        number = 1;

    value = base + scale * (number - 1) + GetTierBonus(tier);
    return ClampS32(value, 1, STAT_MAX);
}

void DungeonSeedStats_InitDefaultTiers(u8 tiers[TYPE_SELECTION_STAT_COUNT])
{
    s32 i;

    if (tiers == NULL)
        return;

    for (i = 0; i < TYPE_SELECTION_STAT_COUNT; i++)
        tiers[i] = STAT_TIER_MEDIUM;
}

u8 DungeonSeedStats_GetHighestTeamLevel(void)
{
    s32 i;
    u8 maxLevel = 1;

    if (gDungeon == NULL)
        return maxLevel;

    for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
        Entity *entity = gDungeon->teamPokemon[i];
        if (!EntityIsValid(entity))
            continue;
        if (GetEntInfo(entity)->level > maxLevel)
            maxLevel = GetEntInfo(entity)->level;
    }

    return maxLevel;
}

bool8 DungeonSeedStats_GetTypeBossStatTiers(
    s16 bossSpecies,
    u8 bossTiers[TYPE_SELECTION_STAT_COUNT],
    u8 minionTiers[TYPE_SELECTION_MINIONS_PER_BOSS][TYPE_SELECTION_STAT_COUNT])
{
    s32 type;
    s32 bossIdx;
    s32 minionIdx;
    s32 statIdx;

    DungeonSeedStats_InitDefaultTiers(bossTiers);
    if (minionTiers != NULL) {
        for (minionIdx = 0; minionIdx < TYPE_SELECTION_MINIONS_PER_BOSS; minionIdx++)
            DungeonSeedStats_InitDefaultTiers(minionTiers[minionIdx]);
    }

    if (bossSpecies <= MONSTER_NONE || bossSpecies >= MONSTER_MAX)
        return FALSE;

    for (type = 0; type < NUM_TYPES; type++) {
        const TypeBossPool *pool = &gTypeBossTable[type];
        for (bossIdx = 0; bossIdx < pool->count && bossIdx < TYPE_SELECTION_MAX_BOSSES_PER_TYPE; bossIdx++) {
            if (pool->species[bossIdx] != bossSpecies)
                continue;
            if (bossTiers != NULL) {
                for (statIdx = 0; statIdx < TYPE_SELECTION_STAT_COUNT; statIdx++)
                    bossTiers[statIdx] = pool->bossStatTiers[bossIdx][statIdx];
            }
            if (minionTiers != NULL) {
                for (minionIdx = 0; minionIdx < TYPE_SELECTION_MINIONS_PER_BOSS; minionIdx++) {
                    for (statIdx = 0; statIdx < TYPE_SELECTION_STAT_COUNT; statIdx++)
                        minionTiers[minionIdx][statIdx] = pool->minionStatTiers[bossIdx][minionIdx][statIdx];
                }
            }
            return TRUE;
        }
    }

    return FALSE;
}

void DungeonSeedStats_BuildBossProfile(u8 dungeonNumber, const u8 tiers[TYPE_SELECTION_STAT_COUNT], SeedStatProfile *out)
{
    if (out == NULL || tiers == NULL)
        return;

    out->hp = (s16)ClampS32(CalcStatValue(BOSS_STAT_BASE, BOSS_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_HP]), 1, HP_MAX);
    out->atk = (u8)CalcStatValue(BOSS_STAT_BASE, BOSS_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_ATK]);
    out->spAtk = (u8)CalcStatValue(BOSS_STAT_BASE, BOSS_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_SPATK]);
    out->def = (u8)CalcStatValue(BOSS_STAT_BASE, BOSS_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_DEF]);
    out->spDef = (u8)CalcStatValue(BOSS_STAT_BASE, BOSS_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_SPDEF]);
}

void DungeonSeedStats_BuildMinionProfile(u8 dungeonNumber, const u8 tiers[TYPE_SELECTION_STAT_COUNT], SeedStatProfile *out)
{
    if (out == NULL || tiers == NULL)
        return;

    out->hp = (s16)ClampS32(CalcStatValue(MINION_STAT_BASE, MINION_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_HP]), 1, HP_MAX);
    out->atk = (u8)CalcStatValue(MINION_STAT_BASE, MINION_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_ATK]);
    out->spAtk = (u8)CalcStatValue(MINION_STAT_BASE, MINION_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_SPATK]);
    out->def = (u8)CalcStatValue(MINION_STAT_BASE, MINION_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_DEF]);
    out->spDef = (u8)CalcStatValue(MINION_STAT_BASE, MINION_STAT_DUNGEON_SCALE, dungeonNumber, tiers[TYPE_STAT_SPDEF]);
}
