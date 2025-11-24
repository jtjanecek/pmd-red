#ifndef GUARD_TYPE_SELECTION_H
#define GUARD_TYPE_SELECTION_H

#include "global.h"
#include "constants/difficulty.h"
#include "constants/type.h"
#include "constants/weather.h"

#define TYPE_SELECTION_MAX_BOSSES_PER_TYPE 2
#define TYPE_SELECTION_MINIONS_PER_BOSS 2
#define TYPE_SELECTION_MAX_TILESETS_PER_TYPE 8
#define BOSS_WEATHER_CHANCE_SCALE 1000

typedef struct TypeHintDefinition
{
    const u8 *message;
    u8 type1;
    u8 type2;
} TypeHintDefinition;

typedef struct TypeBossPool
{
    s16 species[TYPE_SELECTION_MAX_BOSSES_PER_TYPE];
    s16 minions[TYPE_SELECTION_MAX_BOSSES_PER_TYPE][TYPE_SELECTION_MINIONS_PER_BOSS];
    u8 count;
} TypeBossPool;

typedef struct BossWeatherConfig
{
    bool8 enabled;
    u8 weather;
    u16 chance[NUM_DIFFICULTY_SETTINGS]; // Probability scaled by BOSS_WEATHER_CHANCE_SCALE
} BossWeatherConfig;

typedef struct TypeBossWeatherPool
{
    BossWeatherConfig bosses[TYPE_SELECTION_MAX_BOSSES_PER_TYPE];
    u8 count;
} TypeBossWeatherPool;

typedef struct TypeTilesetPool
{
    u8 tilesets[TYPE_SELECTION_MAX_TILESETS_PER_TYPE];
    u8 count;
} TypeTilesetPool;

typedef struct TypeSelectionSaveData
{
    u8 pickCount[NUM_TYPES];
    u8 bossMask[NUM_TYPES];
    u8 tilesetMask[NUM_TYPES];
    s16 pendingHintIds[2];
    s16 committedBoss;
    s16 activeBoss;
    u8 pendingHintCount;
    u8 completedDungeons;
    u8 committedTileset;
    u8 committedTilesetValid;
    u8 activeTileset;
    u8 activeTilesetValid;
    u8 committedType;
    u8 committedTypeValid;
    u8 activeType;
    u8 activeTypeValid;
    u8 committedBossValid;
    u8 activeBossValid;
    u8 awaitingChoice;
    u8 reserved;
} TypeSelectionSaveData;

extern const TypeHintDefinition gTypeHintTable[];
extern const u32 gTypeHintCount;
extern const TypeBossPool gTypeBossTable[NUM_TYPES];
extern const TypeBossWeatherPool gTypeBossWeatherTable[NUM_TYPES];
extern const TypeTilesetPool gTypeTilesetTable[NUM_TYPES];

void TypeSelection_Init(void);
void TypeSelection_ResetForNewRun(void);
void TypeSelection_ReadSaveData(const TypeSelectionSaveData *data);
void TypeSelection_WriteSaveData(TypeSelectionSaveData *data);
bool8 TypeSelection_IsFeatureEnabled(void);
bool8 TypeSelection_ShouldPromptPlayer(void);
bool8 TypeSelection_EnsurePendingHints(void);
const TypeHintDefinition *TypeSelection_GetPendingHint(u32 index);
bool8 TypeSelection_SelectHint(u32 index, u8 *chosenTypeOut);
bool8 TypeSelection_HasCommittedType(void);
u8 TypeSelection_GetCommittedType(void);
bool8 TypeSelection_HasActiveType(void);
u8 TypeSelection_GetActiveType(void);
bool8 TypeSelection_HasCommittedTileset(void);
u8 TypeSelection_GetCommittedTileset(void);
bool8 TypeSelection_HasActiveTileset(void);
u8 TypeSelection_GetActiveTileset(void);
bool8 TypeSelection_HasCommittedBoss(void);
s16 TypeSelection_GetCommittedBoss(void);
bool8 TypeSelection_HasActiveBoss(void);
s16 TypeSelection_GetActiveBoss(void);
void TypeSelection_HandleDungeonStart(void);
bool8 TypeSelection_EnsureInitialCommittedType(void);

bool8 TypeSelectionMenu_Begin(void);
bool8 TypeSelectionMenu_Update(void);
void TypeSelectionMenu_Reset(void);
extern const u8 gTypeSelectionFallbackText[];
extern const void *const gTypeSelectionLinkAnchor[];
extern const void *const gTypeSelectionUiLinkAnchor[];

#endif // GUARD_TYPE_SELECTION_H
