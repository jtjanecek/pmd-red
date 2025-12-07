#ifndef GUARD_SKARMORY_RECRUIT_H
#define GUARD_SKARMORY_RECRUIT_H

#include "global.h"
#include "pokemon.h"

#define SKARMORY_RECRUIT_COST 50

typedef struct SkarmoryRecruitSaveData
{
    s16 lastDungeonId;
    u16 available;
    u32 dungeonSeed;
    s32 personalitySeed;
} SkarmoryRecruitSaveData;

void SkarmoryRecruit_Init(void);
void SkarmoryRecruit_Reset(void);
void SkarmoryRecruit_ReadSaveData(const SkarmoryRecruitSaveData *data);
void SkarmoryRecruit_WriteSaveData(SkarmoryRecruitSaveData *data);
void SkarmoryRecruit_SetDungeonCompleted(s16 dungeonId);
bool8 SkarmoryRecruit_IsAvailable(void);
s16 SkarmoryRecruit_GetOfferedSpecies(void);
void SkarmoryRecruit_MarkUsed(void);
u8 SkarmoryRecruit_GetRecommendedLevel(void);
void SkarmoryRecruit_SetRecruitLevel(Pokemon *pokemon);
extern const void *const gSkarmoryRecruitLinkAnchor[];

#endif // GUARD_SKARMORY_RECRUIT_H
