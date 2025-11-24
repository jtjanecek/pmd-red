#ifndef GUARD_DUNGEON_SEED_OVERRIDES_H
#define GUARD_DUNGEON_SEED_OVERRIDES_H

#include "global.h"
#include "structs/str_spawn_pokemon_data.h"
#include "structs/str_dungeon.h"

typedef struct BossFightConfig {
    bool8 enabled;                    // Is there a boss fight on this floor?
    s16 bossSpecies;                  // Boss monster species
    s16 bossHP;                       // Boss HP
    u16 bossMusic;                    // Music during fight
    u16 dropItem;                     // Item to drop (ITEM_NOTHING for none)
    u8 monsterBehavior;               // Boss behavior ID
    u8 minionCount;                   // Number of minions to spawn
    s16 minionSpecies[4];             // Minion species (up to 4)
    u8 roomTileset;                   // Override tileset for boss room
    u8 weather;                       // Weather to apply on boss floors
    bool8 applyWeather;               // Should boss floors override weather?
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

// Boss fight functions
bool8 DungeonSeedOverrides_IsCustomBoss(Entity *pokemon);
void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon);
void DungeonSeedOverrides_RegisterBossEntity(Entity *boss);
void DungeonSeedOverrides_SetStairsPosition(s32 x, s32 y);

#endif // GUARD_DUNGEON_SEED_OVERRIDES_H
