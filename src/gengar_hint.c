#include "global.h"
#include "gengar_hint.h"

#include "debug.h"
#include "memory.h"
#include "mgba_log.h"

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
    bool8 result;
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId))
        return FALSE;

    result = (sGengarHintState.data.hintFlags[dungeonId / 8] >> (dungeonId % 8)) & 1;
    MGBA_Printf(0, "[GENGAR-HINT] HasHintForDungeon(%d) = %d", dungeonId, result);
    return result;
}

void GengarHint_MarkHintGiven(s32 dungeonId)
{
    MGBA_Printf(0, "[GENGAR-HINT] MarkHintGiven called for dungeonId=%d", dungeonId);
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId))
        return;

    MGBA_Printf(0, "[GENGAR-HINT] Marking hint as given for dungeon %d (byte %d, bit %d)", dungeonId, dungeonId / 8, dungeonId % 8);
    sGengarHintState.data.hintFlags[dungeonId / 8] |= (1 << (dungeonId % 8));
    MGBA_Printf(0, "[GENGAR-HINT] Hint marked as given for dungeon %d", dungeonId);
}

void GengarHint_ClearHintForDungeon(s32 dungeonId)
{
    MGBA_Printf(0, "[GENGAR-HINT] ClearHintForDungeon called with dungeonId=%d", dungeonId);
    EnsureInitialized();
    if (!IsDungeonIdValid(dungeonId)) {
        MGBA_Printf(0, "[GENGAR-HINT] Invalid dungeon ID %d, returning", dungeonId);
        return;
    }

    MGBA_Printf(0, "[GENGAR-HINT] Clearing hint flag for dungeon %d (byte %d, bit %d)", dungeonId, dungeonId / 8, dungeonId % 8);
    sGengarHintState.data.hintFlags[dungeonId / 8] &= ~(1 << (dungeonId % 8));
    MGBA_Printf(0, "[GENGAR-HINT] Hint flag cleared for dungeon %d", dungeonId);
}

void GengarHint_SetDungeonCompleted(void)
{
    MGBA_Printf(0, "[GENGAR-HINT] SetDungeonCompleted - setting flag to 1");
    EnsureInitialized();
    sGengarHintState.data.dungeonCompletedSinceLastHint = 1;
}

bool8 GengarHint_WasDungeonCompletedSinceLastHint(void)
{
    bool8 result;
    EnsureInitialized();
    result = sGengarHintState.data.dungeonCompletedSinceLastHint != 0;
    MGBA_Printf(0, "[GENGAR-HINT] WasDungeonCompletedSinceLastHint = %d", result);
    return result;
}

void GengarHint_ClearDungeonCompletedFlag(void)
{
    MGBA_Printf(0, "[GENGAR-HINT] ClearDungeonCompletedFlag - resetting flag to 0");
    EnsureInitialized();
    sGengarHintState.data.dungeonCompletedSinceLastHint = 0;
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
    (const void *)GengarHint_SetDungeonCompleted,
    (const void *)GengarHint_WasDungeonCompletedSinceLastHint,
    (const void *)GengarHint_ClearDungeonCompletedFlag,
};
