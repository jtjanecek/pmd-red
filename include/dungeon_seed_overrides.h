#ifndef GUARD_DUNGEON_SEED_OVERRIDES_H
#define GUARD_DUNGEON_SEED_OVERRIDES_H

#include "global.h"
#include "structs/str_spawn_pokemon_data.h"
#include "structs/str_dungeon.h"

typedef enum {
    MINION_FORMATION_DEFAULT = 0,
    MINION_FORMATION_WIDE,           // Shift minions 1 tile farther left/right
    MINION_FORMATION_FORWARD,        // Move minions 1 tile down toward the player
    MINION_FORMATION_BACK,           // Move minions 1 tile up
    MINION_FORMATION_WIDE_FORWARD,   // Shift left/right and down
    MINION_FORMATION_COUNT
} BossMinionFormation;

typedef struct BossFightConfig {
    bool8 enabled;                    // Is there a boss fight on this floor?
    s16 bossSpecies;                  // Boss monster species
    s16 bossHP;                       // Boss HP
    u16 bossMusic;                    // Music during fight
    u16 dropItem;                     // Item to drop (ITEM_NOTHING for none)
    u8 monsterBehavior;               // Boss behavior ID
    u8 minionCount;                   // Number of minions to spawn
    u8 minionFormation;               // Seeded minion layout selection
    s16 minionSpecies[4];             // Minion species (up to 4)
    u16 bossMoves[MAX_MON_MOVES];     // Custom move set for the boss (MOVE_NOTHING entries are ignored)
    u16 minionMoves[4][MAX_MON_MOVES]; // Custom move sets for minions (aligned to minionSpecies)
    bool8 useCustomMoves;             // Should the boss use the custom moves above?
    bool8 minionUseCustomMoves[4];    // Should minions use their custom moves?
    u8 roomTileset;                   // Override tileset for boss room
    u8 weather;                       // Weather to apply on boss floors
    bool8 applyWeather;               // Should boss floors override weather?
    bool8 useFixedRoomLayout;         // Use a fixed room layout instead of simple rectangle
    u8 fixedRoomNumber;               // Which fixed room to load (e.g., 1 = Skarmory)
} BossFightConfig;

typedef struct DungeonSeedFloorOverrides {
    u8 tileset;                                          // Tileset selected for this floor
    s32 spawnCount;                                      // Number of entries populated in spawns[]
    SpawnPokemonData spawns[MONSTER_SPAWNS_ARR_COUNT];   // Generated spawn table
    BossFightConfig bossFight;                           // Boss fight configuration (procedurally generated)
} DungeonSeedFloorOverrides;

void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId, DungeonSeedFloorOverrides *result);
s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId);
u32 DungeonSeedOverrides_GetDungeonRngSeed(s32 seed, u8 dungeonId, s32 floorId);
bool8 DungeonSeedOverrides_IsEnabled(s32 *seedOut);
const u8 *DungeonSeedOverrides_GetDungeonName(u8 dungeonId, bool8 secondLine);
bool8 DungeonSeedOverrides_IsInSequentialList(s16 rescueDungeonId);
s16 DungeonSeedOverrides_GetCurrentDungeon(void);
bool8 DungeonSeedOverrides_CanEnterDungeon(s16 rescueDungeonId);
bool8 DungeonSeedOverrides_ShouldTriggerCredits(void);
s32 DungeonSeedOverrides_GetItemLimit(void);
s32 DungeonSeedOverrides_ApplyItemLimit(void);

// Boss fight functions
bool8 DungeonSeedOverrides_IsCustomBoss(Entity *pokemon);
void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon);
void DungeonSeedOverrides_RegisterBossEntity(Entity *boss);
void DungeonSeedOverrides_SetStairsPosition(s32 x, s32 y);

// Kecleon shop functions
s32 DungeonSeedOverrides_GetKecleonFloor(u8 dungeonId, s32 seed);

#endif // GUARD_DUNGEON_SEED_OVERRIDES_H
