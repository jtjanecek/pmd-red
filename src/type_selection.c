#include "global.h"
#include "type_selection.h"

#include "constants/dungeon.h"
#include "constants/monster.h"
#include "dungeon_seed_overrides.h"
#include "mgba_log.h"
#include "memory.h"
#include "save.h"
#include "strings.h"

#define TYPE_SELECTION_MAX_TYPE_PICKS 2
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
static void SanitizeBossState(void);
static void SanitizeTilesetState(void);
static bool8 IsBossInPool(u8 type, s16 species);
static bool8 IsTilesetInPool(u8 type, u8 tilesetId);
static bool8 SelectBossForType(u8 type, u32 *rngState, s16 *bossOut);
static bool8 SelectTilesetForType(u8 type, u32 *rngState, u8 *tilesetOut);
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
    SanitizeBossState();
    SanitizeTilesetState();

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
    s32 seed = 0;
    // Keep the system active even if the seed is missing (-1) so we can
    // surface failures (Bulbasaur fallback) and still drive boss selection.
    if (DungeonSeedOverrides_IsEnabled(&seed))
        return TRUE;
    return (seed == -1);
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
    s16 chosenBoss = MONSTER_NONE;
    bool8 bossValid = FALSE;
    u8 chosenTileset = 0;
    bool8 tilesetValid = FALSE;
    u32 rng;
    u32 tilesetRng;

    if (!TypeSelection_IsFeatureEnabled())
        return FALSE;
    if (!sTypeSelectionState.initialized)
        TypeSelection_Init();

    if (!TypeSelection_ShouldPromptPlayer())
        return FALSE;
    SanitizeBossState();
    SanitizeTilesetState();
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

    bossValid = SelectBossForType(chosenType, &rng, &chosenBoss);
    if (!bossValid)
        MGBA_Warnf("[TypeSelection] No boss available for type %d", chosenType);

    tilesetRng = MixSeed((u32)sub_8011C34(), (sTypeSelectionState.data.completedDungeons << 4) ^ chosenType ^ 0x715E7D);
    tilesetValid = SelectTilesetForType(chosenType, &tilesetRng, &chosenTileset);
    if (!tilesetValid)
        MGBA_Warnf("[TypeSelection] No tileset available for type %d", chosenType);

    sTypeSelectionState.data.pickCount[chosenType]++;
    sTypeSelectionState.data.pendingHintCount = 0;
    ResetPendingHints();

    sTypeSelectionState.data.committedType = chosenType;
    sTypeSelectionState.data.committedTypeValid = TRUE;
    sTypeSelectionState.data.committedBoss = chosenBoss;
    sTypeSelectionState.data.committedBossValid = bossValid;
    sTypeSelectionState.data.committedTileset = chosenTileset;
    sTypeSelectionState.data.committedTilesetValid = tilesetValid;
    sTypeSelectionState.data.awaitingChoice = FALSE;
    sTypeSelectionState.data.completedDungeons++;

    if (chosenTypeOut != NULL)
        *chosenTypeOut = chosenType;

    MGBA_Warnf("[TypeSelection] Hint %d picked type %d (boss=%d, bossValid=%d, tileset=%d, tilesetValid=%d, seed=%d, dungeonCount=%d)", index, chosenType, chosenBoss, bossValid, chosenTileset, tilesetValid, sub_8011C34(), sTypeSelectionState.data.completedDungeons);
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

bool8 TypeSelection_HasCommittedTileset(void)
{
    return sTypeSelectionState.data.committedTilesetValid;
}

u8 TypeSelection_GetCommittedTileset(void)
{
    return sTypeSelectionState.data.committedTileset;
}

bool8 TypeSelection_HasActiveTileset(void)
{
    return sTypeSelectionState.data.activeTilesetValid;
}

u8 TypeSelection_GetActiveTileset(void)
{
    return sTypeSelectionState.data.activeTileset;
}

bool8 TypeSelection_HasCommittedBoss(void)
{
    return sTypeSelectionState.data.committedBossValid;
}

s16 TypeSelection_GetCommittedBoss(void)
{
    return sTypeSelectionState.data.committedBoss;
}

bool8 TypeSelection_HasActiveBoss(void)
{
    return sTypeSelectionState.data.activeBossValid;
}

s16 TypeSelection_GetActiveBoss(void)
{
    return sTypeSelectionState.data.activeBoss;
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

    MGBA_Warnf("[TypeSelection] Auto-selecting initial hint=%d seed=%d", selection, sub_8011C34());
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
    SanitizeBossState();
    SanitizeTilesetState();

    if (sTypeSelectionState.data.committedTypeValid) {
        sTypeSelectionState.data.activeType = sTypeSelectionState.data.committedType;
        sTypeSelectionState.data.activeTypeValid = TRUE;
        sTypeSelectionState.data.committedTypeValid = FALSE;
        sTypeSelectionState.data.committedType = TYPE_NONE;
        sTypeSelectionState.data.awaitingChoice = TRUE;

        if (sTypeSelectionState.data.committedBossValid
            && IsBossInPool(sTypeSelectionState.data.activeType, sTypeSelectionState.data.committedBoss)) {
            sTypeSelectionState.data.activeBoss = sTypeSelectionState.data.committedBoss;
            sTypeSelectionState.data.activeBossValid = TRUE;
        } else {
            sTypeSelectionState.data.activeBoss = MONSTER_NONE;
            sTypeSelectionState.data.activeBossValid = FALSE;
        }

        sTypeSelectionState.data.committedBossValid = FALSE;
        sTypeSelectionState.data.committedBoss = MONSTER_NONE;

        if (sTypeSelectionState.data.committedTilesetValid
            && IsTilesetInPool(sTypeSelectionState.data.activeType, sTypeSelectionState.data.committedTileset)) {
            sTypeSelectionState.data.activeTileset = sTypeSelectionState.data.committedTileset;
            sTypeSelectionState.data.activeTilesetValid = TRUE;
        } else {
            sTypeSelectionState.data.activeTileset = 0;
            sTypeSelectionState.data.activeTilesetValid = FALSE;
        }

        sTypeSelectionState.data.committedTilesetValid = FALSE;
        sTypeSelectionState.data.committedTileset = 0;
    } else {
        sTypeSelectionState.data.activeTypeValid = FALSE;
        sTypeSelectionState.data.activeBossValid = FALSE;
        sTypeSelectionState.data.activeType = TYPE_NONE;
        sTypeSelectionState.data.activeBoss = MONSTER_NONE;
        sTypeSelectionState.data.activeTilesetValid = FALSE;
        sTypeSelectionState.data.activeTileset = 0;
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

static void SanitizeBossState(void)
{
    s32 i;
    const u8 validMask = (1 << TYPE_SELECTION_MAX_BOSSES_PER_TYPE) - 1;

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeBossPool *pool = &gTypeBossTable[i];
        u8 poolMask = validMask;

        if (pool->count == 0)
            poolMask = 0;
        else if (pool->count < TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
            poolMask = (1 << pool->count) - 1;

        sTypeSelectionState.data.bossMask[i] &= poolMask;
    }

    if (!sTypeSelectionState.data.committedTypeValid
        || !IsBossInPool(sTypeSelectionState.data.committedType, sTypeSelectionState.data.committedBoss)) {
        sTypeSelectionState.data.committedBossValid = FALSE;
        sTypeSelectionState.data.committedBoss = MONSTER_NONE;
    }

    if (!sTypeSelectionState.data.activeTypeValid
        || !IsBossInPool(sTypeSelectionState.data.activeType, sTypeSelectionState.data.activeBoss)) {
        sTypeSelectionState.data.activeBossValid = FALSE;
        sTypeSelectionState.data.activeBoss = MONSTER_NONE;
    }
}

static void SanitizeTilesetState(void)
{
    s32 i;
    const u8 validMask = (1 << TYPE_SELECTION_MAX_TILESETS_PER_TYPE) - 1;

    for (i = 0; i < NUM_TYPES; i++) {
        const TypeTilesetPool *pool = &gTypeTilesetTable[i];
        u8 poolMask = 0;

        if (pool->count >= TYPE_SELECTION_MAX_TILESETS_PER_TYPE)
            poolMask = validMask;
        else if (pool->count > 0)
            poolMask = (1 << pool->count) - 1;

        sTypeSelectionState.data.tilesetMask[i] &= poolMask;
    }

    if (!sTypeSelectionState.data.committedTypeValid
        || !IsTilesetInPool(sTypeSelectionState.data.committedType, sTypeSelectionState.data.committedTileset)) {
        sTypeSelectionState.data.committedTilesetValid = FALSE;
        sTypeSelectionState.data.committedTileset = 0;
    }

    if (!sTypeSelectionState.data.activeTypeValid
        || !IsTilesetInPool(sTypeSelectionState.data.activeType, sTypeSelectionState.data.activeTileset)) {
        sTypeSelectionState.data.activeTilesetValid = FALSE;
        sTypeSelectionState.data.activeTileset = 0;
    }
}

static bool8 IsTypeWithinBounds(u8 type)
{
    return (type > TYPE_NONE && type < NUM_TYPES);
}

static bool8 IsBossInPool(u8 type, s16 species)
{
    s32 i;
    const TypeBossPool *pool;
    s32 poolCount;

    if (!IsTypeWithinBounds(type))
        return FALSE;
    if (species <= MONSTER_NONE || species >= MONSTER_MAX)
        return FALSE;

    pool = &gTypeBossTable[type];
    poolCount = pool->count;
    if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
        poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

    for (i = 0; i < poolCount; i++) {
        if (pool->species[i] == species)
            return TRUE;
    }

    return FALSE;
}

static bool8 IsTilesetInPool(u8 type, u8 tilesetId)
{
    s32 i;
    const TypeTilesetPool *pool;
    s32 poolCount;

    if (!IsTypeWithinBounds(type))
        return FALSE;

    pool = &gTypeTilesetTable[type];
    poolCount = pool->count;
    if (poolCount > TYPE_SELECTION_MAX_TILESETS_PER_TYPE)
        poolCount = TYPE_SELECTION_MAX_TILESETS_PER_TYPE;

    for (i = 0; i < poolCount; i++) {
        if (pool->tilesets[i] == tilesetId)
            return TRUE;
    }

    return FALSE;
}

static bool8 IsTypeAvailable(u8 type)
{
    if (!IsTypeWithinBounds(type))
        return FALSE;
    if (sTypeSelectionState.data.pickCount[type] >= TYPE_SELECTION_MAX_TYPE_PICKS)
        return FALSE;
    return TRUE;
}

static bool8 ShouldGenerateHints(void)
{
    if (!TypeSelection_ShouldPromptPlayer())
        return FALSE;
    return TRUE;
}

static bool8 SelectBossForType(u8 type, u32 *rngState, s16 *bossOut)
{
    const TypeBossPool *pool;
    u8 availableIndices[TYPE_SELECTION_MAX_BOSSES_PER_TYPE];
    s32 availableCount = 0;
    u32 rngLocal;
    s32 poolCount;
    s32 choice;

    if (bossOut == NULL)
        return FALSE;
    if (!IsTypeWithinBounds(type))
        return FALSE;

    pool = &gTypeBossTable[type];
    poolCount = pool->count;
    if (poolCount <= 0)
        return FALSE;
    if (poolCount > TYPE_SELECTION_MAX_BOSSES_PER_TYPE)
        poolCount = TYPE_SELECTION_MAX_BOSSES_PER_TYPE;

    sTypeSelectionState.data.bossMask[type] &= (1 << TYPE_SELECTION_MAX_BOSSES_PER_TYPE) - 1;

    if (rngState != NULL)
        rngLocal = *rngState;
    else
        rngLocal = MixSeed((u32)sub_8011C34(), sTypeSelectionState.data.completedDungeons + type);

    for (choice = 0; choice < poolCount; choice++) {
        if (!(sTypeSelectionState.data.bossMask[type] & (1 << choice))) {
            availableIndices[availableCount++] = (u8)choice;
        }
    }

    if (availableCount == 0) {
        availableIndices[availableCount++] = (u8)(poolCount - 1);
    }

    choice = ChooseRandomIndex(&rngLocal, (s16)availableCount);
    if (choice < 0 || choice >= availableCount)
        return FALSE;

    sTypeSelectionState.data.bossMask[type] |= (1 << availableIndices[choice]);
    *bossOut = pool->species[availableIndices[choice]];

    if (rngState != NULL)
        *rngState = rngLocal;

    return TRUE;
}

static bool8 SelectTilesetForType(u8 type, u32 *rngState, u8 *tilesetOut)
{
    const TypeTilesetPool *pool;
    u8 availableIndices[TYPE_SELECTION_MAX_TILESETS_PER_TYPE];
    s32 availableCount = 0;
    u32 rngLocal;
    s32 poolCount;
    s32 choice;

    if (tilesetOut == NULL)
        return FALSE;
    if (!IsTypeWithinBounds(type))
        return FALSE;

    pool = &gTypeTilesetTable[type];
    poolCount = pool->count;
    if (poolCount <= 0)
        return FALSE;
    if (poolCount > TYPE_SELECTION_MAX_TILESETS_PER_TYPE)
        poolCount = TYPE_SELECTION_MAX_TILESETS_PER_TYPE;

    sTypeSelectionState.data.tilesetMask[type] &= (1 << poolCount) - 1;

    if (rngState != NULL)
        rngLocal = *rngState;
    else
        rngLocal = MixSeed((u32)sub_8011C34(), (sTypeSelectionState.data.completedDungeons << 8) ^ type ^ 0x715E7D);

    for (choice = 0; choice < poolCount; choice++) {
        if (!(sTypeSelectionState.data.tilesetMask[type] & (1 << choice))) {
            availableIndices[availableCount++] = (u8)choice;
        }
    }

    if (availableCount == 0) {
        availableIndices[availableCount++] = (u8)(poolCount - 1);
    }

    choice = ChooseRandomIndex(&rngLocal, (s16)availableCount);
    if (choice < 0 || choice >= availableCount)
        return FALSE;

    sTypeSelectionState.data.tilesetMask[type] |= (1 << availableIndices[choice]);
    *tilesetOut = pool->tilesets[availableIndices[choice]];

    if (rngState != NULL)
        *rngState = rngLocal;

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
