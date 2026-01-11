#ifndef GUARD_DUNGEON_SEED_STAT_OVERRIDES_H
#define GUARD_DUNGEON_SEED_STAT_OVERRIDES_H

#include "global.h"
#include "type_selection.h"

typedef struct SeedStatProfile
{
    s16 hp;
    u8 atk;
    u8 spAtk;
    u8 def;
    u8 spDef;
} SeedStatProfile;

void DungeonSeedStats_InitDefaultTiers(u8 tiers[TYPE_SELECTION_STAT_COUNT]);
u8 DungeonSeedStats_GetHighestTeamLevel(void);
bool8 DungeonSeedStats_GetTypeBossStatTiers(
    s16 bossSpecies,
    u8 bossTiers[TYPE_SELECTION_STAT_COUNT],
    u8 minionTiers[TYPE_SELECTION_MINIONS_PER_BOSS][TYPE_SELECTION_STAT_COUNT]);
void DungeonSeedStats_BuildBossProfile(u8 dungeonNumber, const u8 tiers[TYPE_SELECTION_STAT_COUNT], SeedStatProfile *out);
void DungeonSeedStats_BuildMinionProfile(u8 dungeonNumber, const u8 tiers[TYPE_SELECTION_STAT_COUNT], SeedStatProfile *out);

#endif // GUARD_DUNGEON_SEED_STAT_OVERRIDES_H
