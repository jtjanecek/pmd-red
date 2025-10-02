#ifndef DUNGEON_PATHFINDING_ATTACK_H
#define DUNGEON_PATHFINDING_ATTACK_H

#include "global.h"
#include "structs/map.h"

// Check if there's an enemy at the specified position and attack it if possible
bool8 AttackEnemyAtPosition(Entity *attacker, s32 x, s32 y);

// Enhanced pathfinding that handles enemy attacks externally
// Uses A* to find path, then checks for enemies and attacks them if needed
DungeonPos AStarPathfindWithAttack(DungeonPos start, DungeonPos goal, Entity *attacker);

#endif // DUNGEON_PATHFINDING_ATTACK_H
