#include "global.h"
#include "gengar_hint.h"

#include "memory.h"

typedef struct GengarHintState
{
    GengarHintSaveData data;
    bool8 initialized;
} GengarHintState;

static EWRAM_DATA GengarHintState sGengarHintState = {0};

static bool8 IsDungeonIdValid(s32 dungeonId);
static void EnsureInitialized(void);

void GengarHint_Init(void)
{
    MemoryFill8(&sGengarHintState, 0, sizeof(sGengarHintState));
    sGengarHintState.initialized = TRUE;
}

void GengarHint_ResetAll(void)
{
    GengarHint_Init();
}

void GengarHint_ReadSaveData(const GengarHintSaveData *data)
{
    EnsureInitialized();

    if (data != NULL) {
        sGengarHintState.data = *data;
    }
}

void GengarHint_WriteSaveData(GengarHintSaveData *data)
{
    if (data == NULL)
        return;

    EnsureInitialized();
    *data = sGengarHintState.data;
}

bool8 GengarHint_HasHintForDungeon(s32 dungeonId)
{
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId))
        return FALSE;

    return (sGengarHintState.data.hintFlags[dungeonId / 8] >> (dungeonId % 8)) & 1;
}

void GengarHint_MarkHintGiven(s32 dungeonId)
{
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId))
        return;

    sGengarHintState.data.hintFlags[dungeonId / 8] |= (1 << (dungeonId % 8));
}

void GengarHint_ClearHintForDungeon(s32 dungeonId)
{
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId))
        return;

    sGengarHintState.data.hintFlags[dungeonId / 8] &= ~(1 << (dungeonId % 8));
}

static bool8 IsDungeonIdValid(s32 dungeonId)
{
    if (dungeonId < 0 || dungeonId >= NUM_DUNGEONS)
        return FALSE;
    return TRUE;
}

static void EnsureInitialized(void)
{
    if (!sGengarHintState.initialized)
        GengarHint_Init();
}

const void *const gGengarHintLinkAnchor[] = {
    (const void *)GengarHint_Init,
    (const void *)GengarHint_ResetAll,
    (const void *)GengarHint_ReadSaveData,
    (const void *)GengarHint_WriteSaveData,
    (const void *)GengarHint_HasHintForDungeon,
    (const void *)GengarHint_MarkHintGiven,
    (const void *)GengarHint_ClearHintForDungeon,
};
