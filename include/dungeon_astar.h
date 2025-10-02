#ifndef DUNGEON_ASTAR_H
#define DUNGEON_ASTAR_H

#include "global.h"
#include "structs/map.h"

// A* pathfinding function
// Returns the next tile to move to from start towards goal
// Only considers T1 (walkable) tiles
DungeonPos AStarPathfind(DungeonPos start, DungeonPos goal);

// Get direction from current position to next step
s32 AStarGetDirection(DungeonPos current, DungeonPos next_step);

#endif // DUNGEON_ASTAR_H
