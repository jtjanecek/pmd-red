#include "global.h"
#include "custom_fixed_rooms.h"
#include "dungeon_map_access.h"
#include "dungeon_util.h"
#include "dungeon_engine.h"
#include "dungeon_seed_overrides.h"
#include "structs/map.h"
#include "constants/dungeon.h"
#include "mgba_log.h"

// Fixed Room 1 - 9 rows x 17 columns
// Original source: Skarmory boss room pattern
// Tile types: 2=wall, 6=secondary wall, 10=water, 17=boss spawn, 16=player spawn, 4=stairs, 60=floor, 68=trap/item
static const u8 sFixedRoom1_Tiles[] = {
    // Row 0
    6,   2,   2,   2,   2,   2,   2,   2,   6,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 1
    6,   2,   2,   2,   2,   2,   2,   2,   6,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 2
    2,   2,   2,   2,   2,   2,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 3 - Boss spawns here at center (tile 17)
    2,   2,   2,  68,  17,  68,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 4 - Item spawns on sides
    68,   6,   6,   6,   6,   6,   6,   6,  68,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 5
    2,   6,   6,   6,   6,   6,   6,   6,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 6
    10,  10,  10,  10,  10,  10,  10,  10,  10,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 7
    10,  60,  60,  60,  60,  60,  60,  60,  10,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 8 - Stairs at center, player to the left
    60,  60,  60,  16,   4,  60,  60,  60,  60,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 9 - Empty floor (partner will spawn here)
    60,  60,  60,  60,  60,  60,  60,  60,  60,   0,   0,   0,   0,   0,   0,   0,   0
};

static const CustomFixedRoom sFixedRoom1 = {
    .width = 17,
    .height = 10,  // Now 10 rows (added row for stairs)
    .tiles = sFixedRoom1_Tiles
};

// Fixed Room 2 - 13 rows x 13 columns
// Original source: Sinister Woods Team Meanies boss room pattern
// Tile types: 2=wall, 4=stairs, 60=floor, 18-27=stairs components
static const u8 sFixedRoom2_Tiles[] = {
    // Row 0
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    // Row 1
    2,   2,   2,   2,   2,  60,  60,  60,   2,   2,   2,   2,   2,
    // Row 2
    2,   2,  60,   2,   2,  60,  60,  60,   2,   2,  60,   2,   2,
    // Row 3 - Stairs at center
    2,  60,  60,  60,  60,  60,   4,  60,  60,  60,  60,  60,   2,
    // Row 4
    2,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,   2,
    // Row 5 - Stairs decoration part 1
    2,  60,  60,  60,  60,  23,  22,  24,  60,  60,  60,   2,   2,
    // Row 6 - Stairs decoration part 2
    2,  60,  60,  60,  60,  26,  25,  27,  60,  60,  60,   2,   2,
    // Row 7
    2,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,   2,
    // Row 8 - Player spawn area
    2,  60,  60,  60,  60,  60,  16,  60,  60,  60,  60,  60,   2,
    // Row 9 - Stairs decoration bottom
    2,   2,  60,   2,   2,  20,  18,  19,   2,   2,  60,   2,   2,
    // Row 10
    2,   2,   2,   2,   2,  60,  60,  60,   2,   2,   2,   2,   2,
    // Row 11
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    // Row 12
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
};

static const CustomFixedRoom sFixedRoom2 = {
    .width = 13,
    .height = 13,
    .tiles = sFixedRoom2_Tiles
};

// Array of all custom rooms
static const CustomFixedRoom *sCustomRooms[] = {
    NULL,           // Index 0 unused
    &sFixedRoom1,   // Index 1 - Skarmory
    &sFixedRoom2,   // Index 2 - Sinister Woods (Team Meanies)
};

#define CUSTOM_ROOM_COUNT (sizeof(sCustomRooms) / sizeof(sCustomRooms[0]))

// Get tile value at (x, y) from flattened array
static u8 GetCustomTile(const CustomFixedRoom *room, s32 x, s32 y)
{
    if (room == NULL || x < 0 || y < 0 || x >= room->width || y >= room->height)
        return CUSTOM_TILE_UNUSED;

    return room->tiles[y * room->width + x];
}

// Place a tile based on custom tile type
static void PlaceCustomTile(Tile *tile, u8 tileType, s32 worldX, s32 worldY)
{
    if (tile == NULL)
        return;

    switch (tileType) {
        case CUSTOM_TILE_WALL:
        case CUSTOM_TILE_SECONDARY_WALL:
            // Solid wall - impassable
            tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
            SetTerrainWall(tile);
            tile->room = 0;
            break;

        case CUSTOM_TILE_WATER:
            // Water tile - use secondary terrain (water/lava depending on dungeon)
            SetTerrainType(tile, TERRAIN_TYPE_SECONDARY);
            tile->room = 0;
            break;

        case CUSTOM_TILE_FLOOR:
            // Normal walkable floor
            SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
            tile->room = 0;
            break;

        case CUSTOM_TILE_PLAYER_SPAWN:
            // Player spawn - mark as normal floor, set spawn position
            SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
            tile->room = 0;
            gDungeon->playerSpawn.x = worldX;
            gDungeon->playerSpawn.y = worldY;
            MGBA_Warnf("[CustomRoom] Player spawn set: (%d, %d)", worldX, worldY);
            break;

        case CUSTOM_TILE_STAIRS_DOWN:
        case CUSTOM_TILE_STAIRS_UP:
            // Stairs - mark as normal floor (stairs spawn AFTER boss defeat)
            SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
            tile->room = 0;
            // Store position but DON'T set spawn flags yet (happens after boss defeat)
            gDungeon->stairsSpawn.x = worldX;
            gDungeon->stairsSpawn.y = worldY;
            MGBA_Warnf("[CustomRoom] Stairs position marked: (%d, %d)", worldX, worldY);
            break;

        case CUSTOM_TILE_STAIRS_19:
        case CUSTOM_TILE_STAIRS_20:
        case CUSTOM_TILE_STAIRS_21:
        case CUSTOM_TILE_STAIRS_PART_1:
        case CUSTOM_TILE_STAIRS_PART_2:
        case CUSTOM_TILE_STAIRS_PART_3:
        case CUSTOM_TILE_STAIRS_PART_4:
        case CUSTOM_TILE_STAIRS_PART_5:
        case CUSTOM_TILE_STAIRS_PART_6:
            // Stair decoration tiles - treat as normal floor
            SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
            tile->room = 0;
            break;

        case CUSTOM_TILE_TRAP_ITEM:
        case CUSTOM_TILE_SPECIAL:
            // Special tiles - treat as normal floor for now
            SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
            tile->room = 0;
            break;

        case CUSTOM_TILE_UNUSED:
        default:
            // Unused space - make it a wall to prevent access
            tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
            SetTerrainWall(tile);
            tile->room = 0;
            break;
    }
}

// Load a custom fixed room layout
void LoadCustomFixedRoom(u8 roomId, bool8 spawnEntities)
{
    const CustomFixedRoom *room;
    s32 x, y;
    s32 worldX, worldY;
    s32 offsetX, offsetY;
    u8 tileType;
    Tile *tile;

    (void)spawnEntities;  // Not used yet

    MGBA_Warnf("[CustomRoom] Loading custom room %d", roomId);

    if (roomId >= CUSTOM_ROOM_COUNT || sCustomRooms[roomId] == NULL) {
        MGBA_Warnf("[CustomRoom] ERROR: Invalid room ID %d", roomId);
        return;
    }

    room = sCustomRooms[roomId];
    MGBA_Warnf("[CustomRoom] Room size: %dx%d", room->width, room->height);

    // Center the room in the dungeon grid
    // Standard offset is (5, 5) like the game's fixed rooms
    offsetX = 5;
    offsetY = 5;

    // Place all tiles
    for (y = 0; y < room->height; y++) {
        for (x = 0; x < room->width; x++) {
            tileType = GetCustomTile(room, x, y);

            // Skip unused tiles
            if (tileType == CUSTOM_TILE_UNUSED)
                continue;

            worldX = offsetX + x;
            worldY = offsetY + y;

            // Bounds check
            if (worldX < 0 || worldX >= DUNGEON_MAX_SIZE_X ||
                worldY < 0 || worldY >= DUNGEON_MAX_SIZE_Y)
                continue;

            tile = GetTileMut(worldX, worldY);
            PlaceCustomTile(tile, tileType, worldX, worldY);
        }
    }

    MGBA_Warnf("[CustomRoom] Room loaded successfully");
    MGBA_Warnf("[CustomRoom] Final positions - Player: (%d, %d), Stairs: (%d, %d)",
               gDungeon->playerSpawn.x, gDungeon->playerSpawn.y,
               gDungeon->stairsSpawn.x, gDungeon->stairsSpawn.y);

    // Store stairs position for post-boss spawning
    DungeonSeedOverrides_SetStairsPosition(gDungeon->stairsSpawn.x, gDungeon->stairsSpawn.y);
}
