#ifndef GUARD_CUSTOM_FIXED_ROOMS_H
#define GUARD_CUSTOM_FIXED_ROOMS_H

#include "global.h"

// Custom fixed room structure
typedef struct CustomFixedRoom {
    u8 width;
    u8 height;
    const u8 *tiles;  // Flattened 2D array
} CustomFixedRoom;

// Tile type constants (from extracted patterns)
#define CUSTOM_TILE_UNUSED          0
#define CUSTOM_TILE_WALL            2
#define CUSTOM_TILE_STAIRS_DOWN     4
#define CUSTOM_TILE_SECONDARY_WALL  6
#define CUSTOM_TILE_WATER           10
#define CUSTOM_TILE_PLAYER_SPAWN    16
#define CUSTOM_TILE_SPECIAL         17
#define CUSTOM_TILE_STAIRS_UP       18
#define CUSTOM_TILE_STAIRS_PART_1   22
#define CUSTOM_TILE_STAIRS_PART_2   23
#define CUSTOM_TILE_STAIRS_PART_3   24
#define CUSTOM_TILE_STAIRS_PART_4   25
#define CUSTOM_TILE_STAIRS_PART_5   26
#define CUSTOM_TILE_STAIRS_PART_6   27
#define CUSTOM_TILE_FLOOR           60
#define CUSTOM_TILE_TRAP_ITEM       68

// Load a custom fixed room layout
void LoadCustomFixedRoom(u8 roomId, bool8 spawnEntities);

#endif // GUARD_CUSTOM_FIXED_ROOMS_H
