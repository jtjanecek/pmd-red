#include "global.h"
#include "type_selection.h"

#include "constants/dungeon.h"
#include "dungeon_seed_overrides.h"
#include "mgba_log.h"
#include "memory.h"
#include "save.h"
#include "strings.h"

#define TYPE_SELECTION_MAX_PER_TYPE 2
#define TYPE_SELECTION_MAX_DUNGEONS 20
#define TYPE_SELECTION_MAX_HINTS ((NUM_TYPES * (NUM_TYPES - 1)) / 2)

typedef struct TypeSelectionState
{
    TypeSelectionSaveData data;
    bool8 initialized;
} TypeSelectionState;

static EWRAM_DATA TypeSelectionState sTypeSelectionState = {0};

static void ResetPendingHints(void);
static bool8 IsTypeWithinBounds(u8 type);
static bool8 IsTypeAvailable(u8 type);
static bool8 ShouldGenerateHints(void);
static void SanitizePendingHints(void);
static u32 MixSeed(u32 seed, u32 salt);
static u32 NextRandom(u32 *state);
static s16 ChooseRandomIndex(u32 *state, s16 count);
const void *const gTypeSelectionLinkAnchor[];

void TypeSelection_Init(void)
{
    (void)gTypeSelectionLinkAnchor;
    MemoryFill8(&sTypeSelectionState, 0, sizeof(sTypeSelectionState));
    ResetPendingHints();
    sTypeSelectionState.initialized = TRUE;
}

void TypeSelection_ResetForNewRun(void)
{
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    MemoryFill8(&sTypeSelectionState.data, 0, sizeof(sTypeSelectionState.data));
    ResetPendingHints();
    sTypeSelectionState.data.awaitingChoice = TRUE;
}

void TypeSelection_ReadSaveData(const TypeSelectionSaveData *data)
{
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    if (data != NULL) {
        sTypeSelectionState.data = *data;
    }

    SanitizePendingHints();

    if (!sTypeSelectionState.data.awaitingChoice
        && !sTypeSelectionState.data.committedTypeValid
        && sTypeSelectionState.data.pendingHintCount == 0) {
        sTypeSelectionState.data.awaitingChoice = TRUE;
    }
}

void TypeSelection_WriteSaveData(TypeSelectionSaveData *data)
{
    if (data == NULL)
        return;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    *data = sTypeSelectionState.data;
}

bool8 TypeSelection_IsFeatureEnabled(void)
{
    return DungeonSeedOverrides_IsEnabled(NULL);
}

bool8 TypeSelection_ShouldPromptPlayer(void)
{
    if (!TypeSelection_IsFeatureEnabled())
        return FALSE;
    if (sTypeSelectionState.data.awaitingChoice == FALSE)
        return FALSE;
    if (sTypeSelectionState.data.committedTypeValid)
        return FALSE;
    if (sTypeSelectionState.data.completedDungeons >= TYPE_SELECTION_MAX_DUNGEONS)
        return FALSE;
    return TRUE;
}

bool8 TypeSelection_EnsurePendingHints(void)
{
    s32 i;
    s16 validIndices[TYPE_SELECTION_MAX_HINTS];
    s32 validCount = 0;
    u32 rng;

    if (!TypeSelection_IsFeatureEnabled())
        return FALSE;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();
    if (!ShouldGenerateHints())
        return FALSE;
    if (sTypeSelectionState.data.pendingHintCount == 2)
        return TRUE;

    for (i = 0; i < gTypeHintCount; i++) {
        const TypeHintDefinition *hint = &gTypeHintTable[i];
        if (IsTypeAvailable(hint->type1) && IsTypeAvailable(hint->type2)) {
            if (validCount < ARRAY_COUNT(validIndices)) {
                validIndices[validCount++] = i;
            }
        }
    }

    if (validCount < 2)
        return FALSE;

    rng = MixSeed((u32)sub_8011C34(), sTypeSelectionState.data.completedDungeons);
    sTypeSelectionState.data.pendingHintIds[0] = validIndices[ChooseRandomIndex(&rng, validCount)];
    do {
        sTypeSelectionState.data.pendingHintIds[1] = validIndices[ChooseRandomIndex(&rng, validCount)];
    } while (sTypeSelectionState.data.pendingHintIds[0] == sTypeSelectionState.data.pendingHintIds[1]);

    sTypeSelectionState.data.pendingHintCount = 2;
    sTypeSelectionState.data.awaitingChoice = TRUE;
    return TRUE;
}

const TypeHintDefinition *TypeSelection_GetPendingHint(u32 index)
{
    if (index >= sTypeSelectionState.data.pendingHintCount)
        return NULL;
    if (index >= ARRAY_COUNT(sTypeSelectionState.data.pendingHintIds))
        return NULL;

    if (sTypeSelectionState.data.pendingHintIds[index] < 0)
        return NULL;

    if (sTypeSelectionState.data.pendingHintIds[index] >= gTypeHintCount)
        return NULL;

    return &gTypeHintTable[sTypeSelectionState.data.pendingHintIds[index]];
}

bool8 TypeSelection_SelectHint(u32 index, u8 *chosenTypeOut)
{
    const TypeHintDefinition *hint;
    u8 chosenType;
    u8 fallbackType;
    u32 rng;

    if (!TypeSelection_IsFeatureEnabled())
        return FALSE;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    if (!TypeSelection_ShouldPromptPlayer())
        return FALSE;
    if (index >= sTypeSelectionState.data.pendingHintCount)
        return FALSE;

    hint = TypeSelection_GetPendingHint(index);
    if (hint == NULL)
        return FALSE;

    rng = MixSeed((u32)sub_8011C34(), sTypeSelectionState.data.completedDungeons + index + 1);
    chosenType = (NextRandom(&rng) & 1) ? hint->type1 : hint->type2;
    fallbackType = (chosenType == hint->type1) ? hint->type2 : hint->type1;

    if (!IsTypeAvailable(chosenType) && IsTypeAvailable(fallbackType)) {
        chosenType = fallbackType;
    }

    if (!IsTypeAvailable(chosenType))
        return FALSE;

    sTypeSelectionState.data.pickCount[chosenType]++;
    sTypeSelectionState.data.pendingHintCount = 0;
    ResetPendingHints();

    sTypeSelectionState.data.committedType = chosenType;
    sTypeSelectionState.data.committedTypeValid = TRUE;
    sTypeSelectionState.data.awaitingChoice = FALSE;
    sTypeSelectionState.data.completedDungeons++;

    if (chosenTypeOut != NULL)
        *chosenTypeOut = chosenType;

    MGBA_Infof("[TypeSelection] Hint %d picked type %d", index, chosenType);
    return TRUE;
}

bool8 TypeSelection_HasCommittedType(void)
{
    return sTypeSelectionState.data.committedTypeValid;
}

u8 TypeSelection_GetCommittedType(void)
{
    return sTypeSelectionState.data.committedType;
}

bool8 TypeSelection_HasActiveType(void)
{
    return sTypeSelectionState.data.activeTypeValid;
}

u8 TypeSelection_GetActiveType(void)
{
    return sTypeSelectionState.data.activeType;
}

bool8 TypeSelection_EnsureInitialCommittedType(void)
{
    u32 rng;
    s32 selection;

    if (!TypeSelection_IsFeatureEnabled())
        return FALSE;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    // Only apply for the very first dungeon, before any runs are counted.
    if (sTypeSelectionState.data.completedDungeons != 0)
        return FALSE;
    if (sTypeSelectionState.data.committedTypeValid)
        return TRUE;

    if (!TypeSelection_EnsurePendingHints())
        return FALSE;
    if (sTypeSelectionState.data.pendingHintCount == 0)
        return FALSE;

    // Deterministic hint selection based on the custom seed and current dungeon index (0).
    rng = MixSeed((u32)sub_8011C34(), sTypeSelectionState.data.completedDungeons);
    selection = ChooseRandomIndex(&rng, sTypeSelectionState.data.pendingHintCount);

    return TypeSelection_SelectHint((u32)selection, NULL);
}

void TypeSelection_HandleDungeonStart(void)
{
    if (!TypeSelection_IsFeatureEnabled())
        return;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    // Ensure a deterministic initial type if none has been picked yet.
    (void)TypeSelection_EnsureInitialCommittedType();

    if (sTypeSelectionState.data.committedTypeValid) {
        sTypeSelectionState.data.activeType = sTypeSelectionState.data.committedType;
        sTypeSelectionState.data.activeTypeValid = TRUE;
        sTypeSelectionState.data.committedTypeValid = FALSE;
        sTypeSelectionState.data.awaitingChoice = TRUE;
    } else {
        sTypeSelectionState.data.activeTypeValid = FALSE;
    }
}

static void ResetPendingHints(void)
{
    sTypeSelectionState.data.pendingHintIds[0] = -1;
    sTypeSelectionState.data.pendingHintIds[1] = -1;
    sTypeSelectionState.data.pendingHintCount = 0;
}

static void SanitizePendingHints(void)
{
    if (sTypeSelectionState.data.pendingHintCount > ARRAY_COUNT(sTypeSelectionState.data.pendingHintIds))
        sTypeSelectionState.data.pendingHintCount = 0;

    if (sTypeSelectionState.data.pendingHintCount == 0) {
        ResetPendingHints();
        return;
    }

    if (sTypeSelectionState.data.pendingHintIds[0] < 0
        || sTypeSelectionState.data.pendingHintIds[0] >= gTypeHintCount
        || sTypeSelectionState.data.pendingHintIds[1] < 0
        || sTypeSelectionState.data.pendingHintIds[1] >= gTypeHintCount) {
        ResetPendingHints();
    }
}

static bool8 IsTypeWithinBounds(u8 type)
{
    return (type > TYPE_NONE && type < NUM_TYPES);
}

static bool8 IsTypeAvailable(u8 type)
{
    if (!IsTypeWithinBounds(type))
        return FALSE;
    if (sTypeSelectionState.data.pickCount[type] >= TYPE_SELECTION_MAX_PER_TYPE)
        return FALSE;
    return TRUE;
}

static bool8 ShouldGenerateHints(void)
{
    if (!TypeSelection_ShouldPromptPlayer())
        return FALSE;
    return TRUE;
}

static u32 MixSeed(u32 seed, u32 salt)
{
    u32 mixed = seed ^ (salt * 0x9E3779B9u) ^ 0xA36111C3u;
    mixed ^= mixed >> 16;
    mixed *= 0x7FEB352Du;
    mixed ^= mixed >> 15;
    mixed *= 0x846CA68Bu;
    mixed ^= mixed >> 16;
    return mixed;
}

static u32 NextRandom(u32 *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static s16 ChooseRandomIndex(u32 *state, s16 count)
{
    s32 value;

    if (count <= 0)
        return 0;

    value = NextRandom(state) & 0x7FFFFFFF;
    return (s16)(value % count);
}

const void *const gTypeSelectionLinkAnchor[] = {
    (const void *)TypeSelection_Init,
    (const void *)TypeSelection_ResetForNewRun,
    (const void *)TypeSelection_ReadSaveData,
    (const void *)TypeSelection_WriteSaveData,
    (const void *)TypeSelection_IsFeatureEnabled,
    (const void *)TypeSelection_ShouldPromptPlayer,
    (const void *)TypeSelection_EnsurePendingHints,
    (const void *)TypeSelection_GetPendingHint,
    (const void *)TypeSelection_SelectHint,
    (const void *)TypeSelection_HasCommittedType,
    (const void *)TypeSelection_GetCommittedType,
    (const void *)TypeSelection_HasActiveType,
    (const void *)TypeSelection_GetActiveType,
    (const void *)TypeSelection_HandleDungeonStart,
};
