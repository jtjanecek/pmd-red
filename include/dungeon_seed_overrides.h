#ifndef GUARD_DUNGEON_SEED_OVERRIDES_H
#define GUARD_DUNGEON_SEED_OVERRIDES_H

#include "structs/str_spawn_pokemon_data.h"
#include "structs/str_dungeon.h"

typedef struct DungeonSeedFloorOverrides {
    u8 tileset;                                          // Tileset selected for this floor
    s32 spawnCount;                                      // Number of entries populated in spawns[]
    SpawnPokemonData spawns[MONSTER_SPAWNS_ARR_COUNT];   // Generated spawn table
} DungeonSeedFloorOverrides;

void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId, DungeonSeedFloorOverrides *result);
s32 DungeonSeedOverrides_GetFloorCount(s32 seed, u8 dungeonId);
u32 DungeonSeedOverrides_GetDungeonRngSeed(s32 seed, u8 dungeonId, s32 floorId);

#endif // GUARD_DUNGEON_SEED_OVERRIDES_H
