#ifndef GUARD_GENGAR_HINT_H
#define GUARD_GENGAR_HINT_H

#include "global.h"
#include "constants/dungeon.h"

#define GENGAR_HINT_COST 1000
#define GENGAR_HINT_FLAG_BYTES ((NUM_DUNGEONS + 7) / 8)

typedef struct GengarHintSaveData
{
    u8 hintFlags[GENGAR_HINT_FLAG_BYTES];
    u8 reserved[3];
} GengarHintSaveData;

void GengarHint_Init(void);
void GengarHint_ResetAll(void);
void GengarHint_ReadSaveData(const GengarHintSaveData *data);
void GengarHint_WriteSaveData(GengarHintSaveData *data);
bool8 GengarHint_HasHintForDungeon(s32 dungeonId);
void GengarHint_MarkHintGiven(s32 dungeonId);
void GengarHint_ClearHintForDungeon(s32 dungeonId);
extern const void *const gGengarHintLinkAnchor[];

#endif // GUARD_GENGAR_HINT_H
