/* dungeon_pathfinding_attack.c — External attack logic for A* pathfinding */
#include "global.h"
#include "globaldata.h"
#include "constants/dungeon.h"
#include "constants/direction.h"
#include "structs/map.h"
#include "structs/str_dungeon.h"
#include "dungeon_map_access.h"
#include "dungeon_astar.h"
#include "dungeon_action.h"
#include "dungeon_logic.h"
#include "dungeon_util.h"
#include "position_util.h"
#include "dungeon_misc.h"
#include "dungeon_pos_data.h"
#include "code_8066D04.h"
#include "constants/move_id.h"
#include "dungeon_action_execution.h"
#include "dungeon_message.h"

/* Check if there's an enemy at the specified position and attack it if possible */
bool8 AttackEnemyAtPosition(Entity *attacker, s32 x, s32 y)
{
    const Tile *tile = GetTile(x, y);
    Entity *enemy = tile->monster;
    
    if (enemy != NULL && GetEntityType(enemy) == ENTITY_MONSTER) {
        EntityInfo *attackerInfo = GetEntInfo(attacker);
        s32 direction = GetDirectionTowardsPosition(&attacker->pos, &enemy->pos);
        
        /* Check if we can attack in this direction */
        if (CanAttackInDirection(attacker, direction)) {
            /* Check if the attacker can actually attack */
            if (CannotAttack(attacker, FALSE)) {
                return FALSE;
            }
            
            /* Set up basic attack action */
            SetMonsterActionFields(&attackerInfo->action, ACTION_REGULAR_ATTACK);
            attackerInfo->action.direction = direction & DIRECTION_MASK;
            /* Set the target position for the attack */
            TargetTileInFront(attacker);
            /* Try to execute the attack directly */
            sub_8067904(attacker, MOVE_REGULAR_ATTACK);
            /* Debug: Show that we're trying to attack */
            LogMessageByIdWithPopupCheckUser(attacker, "Attacking enemy!");
            return TRUE;
        }
    }
    return FALSE;
}

/* Enhanced pathfinding that handles enemy attacks externally */
DungeonPos AStarPathfindWithAttack(DungeonPos start, DungeonPos goal, Entity *attacker)
{
    DungeonPos next_step;
    EntityInfo *attackerInfo = GetEntInfo(attacker);
    
    /* First, check if there's an enemy directly in front of where the player is facing */
    s32 front_x = start.x + gAdjacentTileOffsets[attackerInfo->action.direction].x;
    s32 front_y = start.y + gAdjacentTileOffsets[attackerInfo->action.direction].y;
    
    /* Check if there's an enemy directly in front */
    if (front_x >= 0 && front_y >= 0 && front_x < DUNGEON_MAX_SIZE_X && front_y < DUNGEON_MAX_SIZE_Y) {
        const Tile *front_tile = GetTile(front_x, front_y);
        if (front_tile->monster != NULL && GetEntityType(front_tile->monster) == ENTITY_MONSTER) {
            /* Attack the enemy directly in front */
            if (AttackEnemyAtPosition(attacker, front_x, front_y)) {
                /* Return current position to execute the attack */
                next_step.x = start.x;
                next_step.y = start.y;
                return next_step;
            }
        }
    }
    
    /* No enemy in front, use normal A* pathfinding */
    next_step = AStarPathfind(start, goal);
    
    /* If we got a valid next step, check if there's an enemy there */
    if (next_step.x != -1 && next_step.y != -1) {
        const Tile *target_tile = GetTile(next_step.x, next_step.y);
        
        /* If there's an enemy on the target tile, attack it */
        if (target_tile->monster != NULL && GetEntityType(target_tile->monster) == ENTITY_MONSTER) {
            if (AttackEnemyAtPosition(attacker, next_step.x, next_step.y)) {
                /* Return current position to execute the attack */
                next_step.x = start.x;
                next_step.y = start.y;
            } else {
                /* Can't attack the enemy, return invalid position */
                next_step.x = -1;
                next_step.y = -1;
            }
        }
    }
    
    return next_step;
}
