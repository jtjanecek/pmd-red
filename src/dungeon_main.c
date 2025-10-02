#include "global.h"
#include "globaldata.h"
#include "constants/dungeon.h"
#include "constants/dungeon_action.h"
#include "constants/iq_skill.h"
#include "constants/status.h"
#include "constants/tactic.h"
#include "dungeon_astar.h"
#include "dungeon_pathfinding_attack.h"
#include "code_8066D04.h"
#include "constants/move_id.h"
#include "dungeon_message.h"
#include "structs/map.h"
#include "structs/str_dungeon.h"
#include "structs/str_text.h"
#include "text_1.h"
#include "text_3.h"
#include "bg_control.h"
#include "dungeon_move_util.h"
#include "code_800D090.h"
#include "code_801602C.h"
#include "code_801B3C0.h"
#include "dungeon_vram.h"
#include "dungeon_tilemap.h"
#include "dungeon_action.h"
#include "code_8066D04.h"
#include "code_806CD90.h"
#include "dungeon_action.h"
#include "dungeon_ai_movement.h"
#include "dungeon_logic.h"
#include "position_util.h"
#include "dungeon_items.h"
#include "dungeon_range.h"
#include "dungeon_main.h"
#include "dungeon_map.h"
#include "dungeon_random.h"
#include "run_dungeon.h"
#include "dungeon_map_access.h"
#include "dungeon_menu_items.h"
#include "dungeon_menu_moves.h"
#include "dungeon_menu_others.h"
#include "dungeon_menu_stairs.h"
#include "dungeon_menu_team.h"
#include "dungeon_menu_tile.h"
#include "dungeon_message.h"
#include "dungeon_message_log.h"
#include "dungeon_misc.h"
#include "dungeon_music.h"
#include "dungeon_random.h"
#include "dungeon_util.h"
#include "game_options.h"
#include "input.h"
#include "items.h"
#include "menu_input.h"
#include "moves.h"
#include "music.h"
#include "number_util.h"
#include "play_time.h"
#include "pokemon.h"
#include "pokemon_3.h"
#include "sprite.h"
#include "string_format.h"
#include "trap.h"
#include "weather.h"
#include "dungeon_pos_data.h"
#include "dungeon_kecleon_shop.h"
#include "dungeon_engine.h"
#include "dungeon_item_action.h"
#include "dungeon_strings.h"

extern void HandleUnsetItemAction(Entity *,bool8);
extern void TryTriggerTrap(Entity *pokemon, DungeonPos *pos, int param_3, char param_4);
void TryPointCameraToMonster(Entity *a0, u8 a1);
bool8 sub_80701A4(Entity *a0);
void sub_8075680(u32);
void ClearUnpaidFlagFromAllItems(void);
void sub_806A914(u8 a0, u8 a1, u8 a2);
u16 GetLeaderActionId(void);
void sub_80978C8(s16 a0);
void HandleTalkFieldAction(Entity *);
s32 GetTeamMemberEntityIndex(Entity *pokemon);
bool8 sub_8070F80(Entity * pokemon, s32 direction);
void sub_806752C(ActionContainer *a0);
void sub_8067768(ActionContainer *a0);
extern bool8 sub_8071A8C(Entity *pokemon);
extern void sub_8041AD0(Entity *pokemon);
extern void sub_8041AE0(Entity *pokemon);

static EWRAM_DATA bool8 sInDiagonalMode = 0;
static EWRAM_DATA bool8 sInRotateMode = 0;
// Frames counter for arrows in diagonal/rotate mode.
static EWRAM_DATA s16 sArrowsFrames = 0;
// If both of these are set to TRUE, there are 3 arrows visible instead of 1 in rotate mode
static EWRAM_DATA bool8 sShowThreeArrows1 = 0;
static EWRAM_DATA bool8 sShowThreeArrows2 = 0;

static void TryCreateModeArrows(Entity *leader);
static void sub_805E738(Entity *a0);
static bool8 sub_805E874(void);
static bool8 sub_805EC2C(Entity *a0, s32 x, s32 y);
static bool8 sub_805EC4C(Entity *a0, u8 a1);
static bool8 sub_805EF60(Entity *a0, EntityInfo *a1);
static void ShowMainMenu(bool8 fromBPress, bool8 a1);
static void PrintOnMainMenu(bool8 printAll);
static bool8 AreStairsInCurrentRoom(DungeonPos *playerPos);
static bool8 ArePlayerAndTargetInSameRoom(DungeonPos *playerPos, DungeonPos *targetPos);

// Auto-explore state
EWRAM_DATA bool8 gAutoExploreActive = FALSE;
EWRAM_DATA DungeonPos gAutoExploreTarget = {0, 0};
EWRAM_DATA bool8 gAutoExploreHasTarget = FALSE;
EWRAM_DATA DungeonPos gAutoExploreLastTarget = {0, 0};
EWRAM_DATA bool8 gAutoExploreTargetPreserved = FALSE;

// Debug position tracking
EWRAM_DATA s32 gLastDebugX = -1;
EWRAM_DATA s32 gLastDebugY = -1;

// Auto-crawl target position for minimap display
EWRAM_DATA DungeonPos gAutoCrawlTargetPos = {-1, -1};

// Path for minimap display (show next 10 steps for debugging)
// Junction storage variables removed - using A* pathfinding instead

// Junction T1 highlighting system - using existing path step variables

// Complete path for full path visualization (using existing step variables)
// We'll extend the existing 10 steps to show more of the path

// Forward declarations
static void CalculateSimplePath(Entity *leader, DungeonPos *target);
static void CalculateFullPath(Entity *leader, DungeonPos *target); // DEV
static bool8 CanMoveInDirectionIgnoreMonsters(Entity *pokemon, u32 direction);

// Simple debug notification when player moves to a new tile
void CheckTileDebugNotification(Entity *leader)
{
    if (leader != NULL) {
        s32 currentX = leader->pos.x;
        s32 currentY = leader->pos.y;
        const Tile *tile;
        // u8 message[64];
        u16 terrainType;
        u8 roomType;
        
        // Only show debug info for the actual player/leader, not enemies or partners
        // Check if this is the leader monster (player), not just any monster
        if (GetEntityType(leader) != ENTITY_MONSTER || leader != GetLeader()) {
            return;
        }
        
        // Only show notification when position changes
        if (gLastDebugX != currentX || gLastDebugY != currentY) {
            gLastDebugX = currentX;
            gLastDebugY = currentY;
            
            tile = GetTile(currentX, currentY);
            
            // Get terrain type
            terrainType = tile->terrainFlags & (TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
            roomType = tile->room;
            
            // Create detailed message with both terrain and tile type
            // if (tile->terrainFlags & TERRAIN_TYPE_NATURAL_JUNCTION) {
            //     sprintf(message, "Junction! T:%d R:%d", terrainType, roomType);
            // } else if (tile->terrainFlags & TERRAIN_TYPE_STAIRS) {
            //     sprintf(message, "Stairs! T:%d R:%d", terrainType, roomType);
            // } else if (tile->terrainFlags & TERRAIN_TYPE_SHOP) {
            //     sprintf(message, "Shop! T:%d R:%d", terrainType, roomType);
            // } else if (tile->terrainFlags & TERRAIN_TYPE_IMPASSABLE_WALL) {
            //     sprintf(message, "Impassable! T:%d R:%d", terrainType, roomType);
            // } else if (tile->terrainFlags & TERRAIN_TYPE_UNBREAKABLE) {
            //     sprintf(message, "Unbreakable! T:%d R:%d", terrainType, roomType);
            // } else if (tile->terrainFlags & TERRAIN_TYPE_SECONDARY) {
            //     sprintf(message, "Water/Lava! T:%d R:%d", terrainType, roomType);
            // } else if (terrainType == 0) {
            //     sprintf(message, "Wall! T:%d R:%d", terrainType, roomType);
            // } else if (roomType == CORRIDOR_ROOM) {
            //     sprintf(message, "Corridor! T:%d R:%d", terrainType, roomType);
            // } else {
            //     sprintf(message, "Room! T:%d R:%d", terrainType, roomType);
            // }
            
            // Display the notification only once per tile change
            // LogMessageByIdWithPopupCheckUser(leader, message);
        }
    }
}

// Auto-explore functions
void ResetAutoExplore(void)
{
    gAutoExploreActive = FALSE;
    gAutoExploreHasTarget = FALSE;
    gAutoExploreLastTarget.x = 0;
    gAutoExploreLastTarget.y = 0;
    gAutoExploreTargetPreserved = FALSE;
    
    // Junction storage variables removed - using A* pathfinding instead
}

void SetAutoExploreActive(bool8 active)
{
    gAutoExploreActive = active;
    if (!active) {
        // When deactivating, preserve the target for potential reuse
        gAutoExploreTargetPreserved = gAutoExploreHasTarget;
        gAutoExploreHasTarget = FALSE;
    } else {
        // When reactivating, check if we should preserve the target
        if (gAutoExploreTargetPreserved) {
            Entity *leader = GetLeader();
            if (leader != NULL) {
                // Check if we should switch targets (stairs in room OR same room as target)
                if (AreStairsInCurrentRoom(&leader->pos) || 
                    ArePlayerAndTargetInSameRoom(&leader->pos, &gAutoExploreTarget)) {
                    // We should switch targets, so don't restore the old one
                    gAutoExploreTargetPreserved = FALSE;
                } else {
                    // Keep the same target
                    gAutoExploreHasTarget = TRUE;
                    gAutoCrawlTargetPos = gAutoExploreTarget;
                    gAutoExploreTargetPreserved = FALSE;
                    
                    // Recalculate the path for the preserved target
                    CalculateSimplePath(leader, &gAutoExploreTarget);
                    // DEV: Also calculate the complete path for visualization
                    CalculateFullPath(leader, &gAutoExploreTarget);
                    UpdateMinimap(); // Force minimap update to show new path icons
                }
            } else {
                // No leader available, clear preserved target
                gAutoExploreTargetPreserved = FALSE;
            }
        }
    }
}

bool8 IsAutoExploreActive(void)
{
    return gAutoExploreActive;
}

bool8 AreStairsVisible(void)
{
    DungeonPos stairsPos;
    const Tile *tile;
    
    stairsPos = gDungeon->stairsSpawn;
    
    if (stairsPos.x < 0 || stairsPos.y < 0)
        return FALSE;
    
    if (stairsPos.x >= DUNGEON_MAX_SIZE_X || stairsPos.y >= DUNGEON_MAX_SIZE_Y)
        return FALSE;
    
    tile = GetTile(stairsPos.x, stairsPos.y);
    
    // Check if the stairs tile has been revealed
    return (tile->spawnOrVisibilityFlags.visibility & VISIBILITY_FLAG_REVEALED) != 0;
}

// Helper function to check if stairs are in the current room
static bool8 AreStairsInCurrentRoom(DungeonPos *playerPos)
{
    const Tile *playerTile = GetTile(playerPos->x, playerPos->y);
    const Tile *stairsTile = GetTile(gDungeon->stairsSpawn.x, gDungeon->stairsSpawn.y);
    
    return (playerTile->room == stairsTile->room) && (stairsTile->room != CORRIDOR_ROOM);
}

// Helper function to check if player and target are in the same room
static bool8 ArePlayerAndTargetInSameRoom(DungeonPos *playerPos, DungeonPos *targetPos)
{
    const Tile *playerTile = GetTile(playerPos->x, playerPos->y);
    const Tile *targetTile = GetTile(targetPos->x, targetPos->y);
    
    return (playerTile->room == targetTile->room) && (playerTile->room != CORRIDOR_ROOM);
}


// Find unexplored junction tiles (corridor entrances) that lead to unexplored corridors

// Find a random undiscovered room and return its position
static bool8 FindRandomUndiscoveredRoom(DungeonPos *outTarget)
{
    s32 x, y;
    s32 attempts = 0;
    const Tile *tile;
    bool8 found = FALSE;
    
    // Try up to 250 random positions to find an undiscovered room
    while (attempts < 250 && !found) {
        x = DungeonRandInt(DUNGEON_MAX_SIZE_X);
        y = DungeonRandInt(DUNGEON_MAX_SIZE_Y);
        tile = GetTile(x, y);
        
        // Check if this is a room tile (not corridor or anchor)
        if (tile->room != CORRIDOR_ROOM && tile->room != ROOM_0xFE) {
            // Check if this specific tile is not revealed (simple check)
            if (!(tile->spawnOrVisibilityFlags.visibility & VISIBILITY_FLAG_REVEALED)) {
                outTarget->x = x;
                outTarget->y = y;
                found = TRUE;
            }
        }
        attempts++;
    }
    
    return found;
}

bool8 GetAutoExploreTarget(Entity *leader, DungeonPos *outTarget)
{
    const Tile *currentTile;
    
    // Check for debug notifications when player moves
    CheckTileDebugNotification(leader);
    
    // PRIORITY 1: If stairs in the current room -> path to the stairs (ALWAYS takes priority)
    if (AreStairsInCurrentRoom(&leader->pos)) {
        // LogMessageByIdWithPopupCheckUser(leader, "Going to stairs!");
        outTarget->x = gDungeon->stairsSpawn.x;
        outTarget->y = gDungeon->stairsSpawn.y;
        gAutoExploreHasTarget = TRUE;
        gAutoExploreTarget = *outTarget;
        gAutoCrawlTargetPos = *outTarget; // Set target for minimap display
        
        // Don't reset auto-explore when stairs are detected - keep it active
        gAutoExploreActive = TRUE;
        gAutoExploreHasTarget = TRUE;
        gAutoExploreTarget = *outTarget;
        gAutoCrawlTargetPos = *outTarget;
        
        // Calculate the full path
        CalculateSimplePath(leader, outTarget);
        // DEV: Also calculate the complete path for visualization
        CalculateFullPath(leader, outTarget);
        
        // LogMessageByIdWithPopupCheckUser(leader, "Stairs target set!");
        // Debug: Show target coordinates
        // LogMessageByIdWithPopupCheckUser(leader, "Target coords set!");
        UpdateMinimap(); // Force minimap update
        return TRUE;
    }
    
    // If we already have a target, check if we've reached it
    if (gAutoExploreHasTarget) {
        // Check if we've reached the current target
        if (leader->pos.x == gAutoExploreTarget.x && leader->pos.y == gAutoExploreTarget.y) {
            LogMessageByIdWithPopupCheckUser(leader, "Reached target!");
            // Mark the current room as discovered
            currentTile = GetTile(leader->pos.x, leader->pos.y);
            if (currentTile->room != CORRIDOR_ROOM && currentTile->room != ROOM_0xFE) {
                // This is a room tile, mark it as visited
                GetTileMut(leader->pos.x, leader->pos.y)->spawnOrVisibilityFlags.visibility |= VISIBILITY_FLAG_VISITED;
            }
            // Clear the current target so we can find a new one
            gAutoExploreHasTarget = FALSE;
            gAutoCrawlTargetPos.x = -1;
            gAutoCrawlTargetPos.y = -1;
            UpdateMinimap();
        } else {
            // Keep the current target
            *outTarget = gAutoExploreTarget;
            return TRUE;
        }
    }
    
    // Logic 2: Pick a random room that is undiscovered, and path there
    if (FindRandomUndiscoveredRoom(outTarget)) {
        // LogMessageByIdWithPopupCheckUser(leader, "Found undiscovered room!");
        gAutoExploreHasTarget = TRUE;
        gAutoExploreTarget = *outTarget;
        gAutoCrawlTargetPos = *outTarget; // Set target for minimap display
        
        // Don't reset auto-explore when stairs are detected - keep it active
        gAutoExploreActive = TRUE;
        gAutoExploreHasTarget = TRUE;
        gAutoExploreTarget = *outTarget;
        gAutoCrawlTargetPos = *outTarget;
        
        // Calculate the full path
        CalculateSimplePath(leader, outTarget);
        // DEV: Also calculate the complete path for visualization
        CalculateFullPath(leader, outTarget);
        
        // LogMessageByIdWithPopupCheckUser(leader, "Room target set!");
        // Debug: Show target coordinates
        // LogMessageByIdWithPopupCheckUser(leader, "Target coords set!");
        UpdateMinimap(); // Force minimap update
        return TRUE;
    }
    
    // No valid target found
    // LogMessageByIdWithPopupCheckUser(leader, "No target found!");
    gAutoExploreHasTarget = FALSE;
    gAutoCrawlTargetPos.x = -1; // Clear target for minimap display
    gAutoCrawlTargetPos.y = -1;
    return FALSE;
}

// Find the best path to a target using improved pathfinding
static s32 FindBestPathToTarget(Entity *leader, DungeonPos *target)
{
    s32 direction;
    s32 bestDirection = -1;
    s32 bestScore = -9999;
    s32 score;
    DungeonPos testPos;
    s32 currentDistance;
    u16 terrainType;
    s32 testDistance;
    const Tile *targetTile;
    
    currentDistance = GetDistance(&leader->pos, target);
    
    // Try primary direction first
    direction = GetDirectionTowardsPosition(&leader->pos, target);
    if (CanMoveInDirectionIgnoreMonsters(leader, direction)) {
        return direction;
    }
    
    // Try all 8 directions and evaluate them with pathfinding
    for (direction = 0; direction < 8; direction++) {
        if (!CanMoveInDirectionIgnoreMonsters(leader, direction)) {
            continue;
        }
        
        // Additional check: ensure we can move to NATURAL_JUNCTION tiles
        testPos.x = leader->pos.x + gAdjacentTileOffsets[direction].x;
        testPos.y = leader->pos.y + gAdjacentTileOffsets[direction].y;
        targetTile = GetTile(testPos.x, testPos.y);
        
        // Check for blocking terrain types first
        if (targetTile->terrainFlags & TERRAIN_TYPE_IMPASSABLE_WALL) {
            // Skip impassable walls
            continue;
        } else if (targetTile->terrainFlags & TERRAIN_TYPE_UNREACHABLE_FROM_STAIRS) {
            // Skip unreachable tiles (dead ends, isolated areas)
            continue;
        } else if (targetTile->terrainFlags & TERRAIN_TYPE_UNBREAKABLE) {
            // Skip unbreakable tiles (key doors, permanent barriers)
            continue;
        } else if (testPos.x < 0 || testPos.y < 0 || 
                   testPos.x >= DUNGEON_MAX_SIZE_X || testPos.y >= DUNGEON_MAX_SIZE_Y) {
            // Skip out-of-bounds tiles
            continue;
        }
        
        // Check if tile is walkable (not a wall) regardless of room/junction/corridor type
        // A tile can be a room tile, junction tile, or corridor tile but still be a wall
        terrainType = targetTile->terrainFlags & (TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
        if (terrainType == 0) {
            // This is a wall tile (terrain type 0 = TERRAIN_TYPE_WALL)
            // Skip it even if it's marked as room/junction/corridor
            continue;
        }
        
        // Allow room tiles, junction tiles, and corridor tiles ONLY if they are not walls
        // NATURAL_JUNCTION tiles are valid movement targets (they're connection points)
        // Don't skip tiles with monsters - they're not blockers for auto-crawl
        // The player can walk through/over monsters
        testDistance = GetDistance(&testPos, target);
        
        // Start with basic distance improvement
        score = currentDistance - testDistance;
        
        // Big bonus for getting closer to target
        if (testDistance < currentDistance) {
            score += 20;
        }
        
        // Small penalty for getting further from target
        if (testDistance > currentDistance) {
            score -= 2;
        }
        
        // Bonus for directions that are closer to the target direction
        {
            s32 targetDirection = GetDirectionTowardsPosition(&leader->pos, target);
            s32 directionDiff = (direction - targetDirection + 8) % 8;
            if (directionDiff > 4) directionDiff = 8 - directionDiff; // Wrap around
            score += (4 - directionDiff) * 3; // Bonus for being closer to target direction
        }
        
        // Small bonus for any valid movement
        score += 1;
        
        if (score > bestScore) {
            bestScore = score;
            bestDirection = direction;
        }
    }
    
    return bestDirection;
}

// Note: FindBestPathToTargetWithVisited function removed - now using simple pathfinding

// Simple movement check that only blocks truly impassable areas for auto-crawl
static bool8 CanMoveInDirectionIgnoreMonsters(Entity *pokemon, u32 direction)
{
    const Tile *targetTile = GetTile(pokemon->pos.x + gAdjacentTileOffsets[direction].x,
        pokemon->pos.y + gAdjacentTileOffsets[direction].y);
    s32 testX = pokemon->pos.x + gAdjacentTileOffsets[direction].x;
    s32 testY = pokemon->pos.y + gAdjacentTileOffsets[direction].y;
    u16 terrainType;

    // Block out-of-bounds tiles
    if (testX < 0 || testY < 0 || 
        testX >= DUNGEON_MAX_SIZE_X || testY >= DUNGEON_MAX_SIZE_Y)
        return FALSE;

    // Only block truly impassable areas - ignore regular walls
    if (targetTile->terrainFlags & TERRAIN_TYPE_IMPASSABLE_WALL)
        return FALSE;
    
    // Block unreachable tiles (dead ends, isolated areas)
    if (targetTile->terrainFlags & TERRAIN_TYPE_UNREACHABLE_FROM_STAIRS)
        return FALSE;
    
    // Block unbreakable tiles (key doors, permanent barriers)
    if (targetTile->terrainFlags & TERRAIN_TYPE_UNBREAKABLE)
        return FALSE;

    // Only allow tiles with terrain type 1 (T1) - TERRAIN_TYPE_NORMAL
    terrainType = targetTile->terrainFlags & (TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
    if (terrainType != TERRAIN_TYPE_NORMAL) {
        // Only allow T1 tiles - block everything else
        return FALSE;
    }

    // Don't block monsters for auto-crawl - the player can walk through/over monsters
    // Don't block regular walls - auto-crawl should be able to navigate through them
    // This allows auto-crawl to work even when there are monsters and walls in the way

    return TRUE;
}

// Simple pathfinding: move to the tile closest to destination
static void CalculateSimplePath(Entity *leader, DungeonPos *target)
{
    // Using A* pathfinding instead of junction storage
}

// DEV: Calculate the complete path from start to target (using existing step variables)
static void CalculateFullPath(Entity *leader, DungeonPos *target)
{
    // Using A* pathfinding instead of junction highlighting
}

s32 GetAutoExploreDirection(Entity *leader)
{
    DungeonPos target;
    DungeonPos nextStep;
    s32 direction;
    
    if (!gAutoExploreActive)
        return -1;
    
    // Check if we've reached our current target
    if (gAutoExploreHasTarget) {
        if (leader->pos.x == gAutoExploreTarget.x && leader->pos.y == gAutoExploreTarget.y) {
            // Store the target we just reached to avoid immediately re-targeting it
            gAutoExploreLastTarget = gAutoExploreTarget;
            gAutoExploreHasTarget = FALSE;
        }
    }
    
    // Get a new target if we don't have one
    if (!GetAutoExploreTarget(leader, &target)) {
        // No more targets - deactivate auto-explore
        SetAutoExploreActive(FALSE);
        return -1;
    }
    
    // Use A* pathfinding to get the next step
    nextStep = AStarPathfind(leader->pos, target);
    
    // Enable fast movement animation for A* pathfinding (like running)
    gDungeon->unk644.unk28 = 1;
    
    // Check for enemy attack before moving
    if (nextStep.x != -1 && nextStep.y != -1) {
        const Tile *target_tile = GetTile(nextStep.x, nextStep.y);
        
        // If there's an enemy on the target tile, attack it (but not allies/partners)
        if (target_tile->monster != NULL && GetEntityType(target_tile->monster) == ENTITY_MONSTER) {
            EntityInfo *targetInfo = GetEntInfo(target_tile->monster);
            
            // Only attack if it's an actual enemy (not an ally/partner)
            if (targetInfo->isNotTeamMember) {
                EntityInfo *leaderInfo = GetEntInfo(leader);
                s32 direction = GetDirectionTowardsPosition(&leader->pos, &target_tile->monster->pos);
                
                // Check if we can attack in this direction
                if (CanAttackInDirection(leader, direction) && !CannotAttack(leader, FALSE)) {
                    // Set up basic attack action
                    SetMonsterActionFields(&leaderInfo->action, ACTION_REGULAR_ATTACK);
                    leaderInfo->action.direction = direction & DIRECTION_MASK;
                    TargetTileInFront(leader);
                    // Execute the attack directly
                    sub_8067904(leader, MOVE_REGULAR_ATTACK);
                    // LogMessageByIdWithPopupCheckUser(leader, "Attacking enemy!");
                    return -1; // Don't move, just attack
                }
            }
        }
        
        // Calculate direction from current position to next step
        direction = AStarGetDirection(leader->pos, nextStep);
        
        // Verify this direction is valid
        if (!CanMoveInDirectionIgnoreMonsters(leader, direction)) {
            // Fallback to original pathfinding if A* step is invalid
            direction = FindBestPathToTarget(leader, &target);
        }
    } else {
        // No path found with A*, use original pathfinding
        direction = FindBestPathToTarget(leader, &target);
    }
    
    if (direction < 0) {
        // Can't find a path to target - clear it and try again next time
        LogMessageByIdWithPopupCheckUser(leader, "Path not able to be found!");
        gAutoExploreHasTarget = FALSE;
        return -1;
    }
    
    UpdateMinimap(); // Force minimap update to show updated path
    
    return direction;
}

void DungeonHandlePlayerInput(void)
{
    struct UnkMenuBitsStruct r6;
    bool8 triggers[5]; // Always FALSE, if one of these is TRUE - they can open various menus or cause an item throw. Used in Blue's touch screen.
    s32 frames;
    s32 var_38;
    UnkDungeonGlobal_unk181E8_sub *unkPtr;

    unkPtr = &gDungeon->unk181e8;
    var_38 = 3;
    gDungeon->unk12 = 0;
    TryPointCameraToMonster(GetLeader(), 1);
    if (sub_80701A4(GetLeader())) {
        sub_803E708(60, 16);
        return;
    }

    gDungeon->unk644.unk2F = 0;
    ResetMapPlayerDotFrames();
    if (gDungeon->unk1 != 0) {
        gDungeon->unk1 = 0;
        if (!ShouldMonsterRunAwayAndShowEffect(GetLeader(), TRUE)) {
            SetLeaderActionToNothing(TRUE);
            sub_805E804();
            
            // DEV: Auto-proceed on stairs during auto-navigate
            if (IsAutoExploreActive()) {
                // Automatically choose "Go Down" option (ACTION_STAIRS)
                ActionContainer *action = &GetEntInfo(GetLeader())->action;
                SetMonsterActionFields(action, ACTION_STAIRS);
                action->actionParameters[0].actionUseIndex = 0;
                action->actionParameters[0].itemPos.x = 0;
                action->actionParameters[0].itemPos.y = 0;
                action->actionParameters[1].actionUseIndex = 0;
                action->actionParameters[1].itemPos.x = 0;
                action->actionParameters[1].itemPos.y = 0;
            } else {
                ShowDungeonStairsMenu(GetLeader());
            }
            
            ResetRepeatTimers();
            ResetUnusedInputStruct();
            if (GetLeaderActionId() != ACTION_NOTHING) {
                return;
            }
        }
    }

    sub_806A914(1, 1, 1);
    while (1) {
        Entity *leader = GetLeader();
        EntityInfo *leaderInfo = GetEntInfo(leader);

        sub_80978C8(leaderInfo->id);
        if (gDungeon->unk644.unk28 != 0) {
            if (sub_805E874()) {
                leaderInfo->action.action = 2;
                leaderInfo->action.actionParameters[0].actionUseIndex = 0;
                break;
            }
            sub_805E804();
        }
        sInRotateMode = FALSE;
        sInDiagonalMode = FALSE;
        if (gDungeon->unk5C0 >= 0) {
            r6.a0_8 = 1;
            r6.a0_16 = 0;
            r6.a0_24 = 0;
        }
        else {
            r6.a0_8 = 0;
            r6.a0_16 = 0;
            r6.a0_24 = 0;
        }

        frames = 0;
        SetLeaderActionFields(ACTION_NOTHING);
        sShowThreeArrows1 = FALSE;
        sShowThreeArrows2 = FALSE;

        while (r6.a0_8 == 0) {
            u32 dpadDiagonal, dpadSimple;
            bool32 highlightTiles, tryItemThrow;
            bool32 bPress, rPress, unkBool; // Always FALSE, might've been used in Blue.
            s32 directionNew;

            sArrowsFrames++;
            if (unkPtr->unk1821A != 0) {
                frames = 0;
            }
            else {
                frames++;
            }

            if (var_38 != 0 && --var_38 == 0) {
                sub_8075680(0);
            }

            TryCreateModeArrows(leader);
            unkBool = FALSE;
            {
                s32 i;
                for (i = 0; i < 5; i++) {
                    triggers[i] = FALSE;
                }
            }

            if (gRealInputs.held & A_BUTTON && gRealInputs.held & B_BUTTON && FixedPointToInt(leaderInfo->belly) != 0) {
                SetLeaderActionFields(ACTION_PASS_TURN);
                gDungeon->unk644.unk2F = 1;
                break;
            }

            bPress = FALSE;
            rPress = FALSE;

            if (gRealInputs.pressed & A_BUTTON) {
                if (gRealInputs.held & B_BUTTON) {
                    if (FixedPointToInt(leaderInfo->belly) != 0) {
                        SetLeaderActionFields(ACTION_PASS_TURN);
                        gDungeon->unk644.unk2F = 1;
                        break;
                    }
                }
                else if (ShouldMonsterRunAwayAndShowEffect(leader, TRUE)) {
                    LogMessageByIdWithPopupCheckUser(leader, gUnknown_80FD4B0);
                    SetLeaderActionFields(ACTION_PASS_TURN);
                    gDungeon->unk644.unk2F = 1;
                    break;
                }
                else if (gRealInputs.held & L_BUTTON) {
                    bool32 canUseMove;
                    s32 i, j;

                    for (i = 0; i < MAX_MON_MOVES; i++) {
                        if (MoveFlagExists(&leaderInfo->moves.moves[i]) && MoveFlagSet(&leaderInfo->moves.moves[i])) {
                            break;
                        }
                    }
                    if (i == MAX_MON_MOVES) {
                        LogMessageByIdWithPopupCheckUser(leader, gUnknown_80F8A28);
                        break;
                    }

                    for (j = 0; j < MAX_MON_MOVES; j++) {
                        if (MoveFlagExists(&leaderInfo->moves.moves[j])) {
                            if (leaderInfo->moves.moves[j].PP != 0)
                                break;
                        }
                    }
                    if (j == MAX_MON_MOVES) {
                        SetMonsterActionFields(&leaderInfo->action, ACTION_STRUGGLE);
                        break;
                    }

                    canUseMove = FALSE;
                    for (j = i; j < MAX_MON_MOVES; j++) {
                        if (j != i && !(leaderInfo->moves.moves[j].moveFlags & MOVE_FLAG_SUBSEQUENT_IN_LINK_CHAIN)) {
                            break;
                        }
                        if (leaderInfo->moves.moves[j].PP != 0) {
                            canUseMove = TRUE;
                            break;
                        }
                    }
                    if (!canUseMove) {
                        LogMessageByIdWithPopupCheckUser(leader, gUnknown_80F8A4C);
                    }
                    else {
                        SetMonsterActionFields(&leaderInfo->action, ACTION_USE_MOVE_PLAYER);
                        leaderInfo->action.actionParameters[0].actionUseIndex = GetTeamMemberEntityIndex(leader);
                        leaderInfo->action.actionParameters[1].actionUseIndex = i;
                    }
                    break;
                }
                else {
                    if (!sub_805EF60(leader, leaderInfo)) {
                        SetMonsterActionFields(&leaderInfo->action, ACTION_REGULAR_ATTACK);
                    }
                    break;
                }
            }

            if (gRealInputs.shortPress & B_BUTTON) {
                r6.a0_8 = 1;
                r6.a0_16 = 0;
                r6.a0_24 = 0;
                break;
            }
            else if (triggers[1]) { // Opens moves menu
                gDungeon->unk5C0 = 0;
                r6.a0_8 = 1;
                r6.a0_16 = 0;
                r6.a0_24 = 1;
                break;
            }
            else if (triggers[2]) { // Opens item menu
                gDungeon->unk5C0 = 1;
                r6.a0_8 = 1;
                r6.a0_16 = 0;
                r6.a0_24 = 1;
                break;
            }
            else if (triggers[3]) { // Opens pokemon menu
                gDungeon->unk5C0 = 2;
                r6.a0_8 = 1;
                r6.a0_16 = 0;
                r6.a0_24 = 1;
                break;
            }
            else if (triggers[4]) { // Opens regular menu
                r6.a0_8 = 1;
                r6.a0_16 = 0;
                r6.a0_24 = 1;
                break;
            }
            else if (frames > 0x707) { // Opens simple menu when idling
                r6.a0_8 = 1;
                r6.a0_16 = 1;
                r6.a0_24 = 0;
                break;
            }

            if (gGameOptionsRef->controls == CONTROLS_GBA
                && (gRealInputs.pressed & B_BUTTON || (!unkBool && bPress))
                && unkPtr->unk1821A != 0)
            {
                sub_804AA60();
                sInRotateMode = FALSE;
                ResetRepeatTimers();
                ResetUnusedInputStruct();
            }

            if (gRealInputs.held & L_BUTTON) {
                if (gRealInputs.pressed & B_BUTTON) {
                    DisplayMessageLog();
                    ResetRepeatTimers();
                    ResetUnusedInputStruct();
                }
            }

            // Auto-explore mode: L+R to toggle ON auto-navigate
            // Support both: Hold L + Press R, or Hold R + Press L
            if ((gRealInputs.held & L_BUTTON) && (gRealInputs.pressed & R_BUTTON)) {
                //LogMessageByIdWithPopupCheckUser(leader, "Hold L + Press R!");
                if (!IsAutoExploreActive()) {
                    SetAutoExploreActive(TRUE);
                    LogMessageByIdWithPopupCheckUser(leader, "Auto-navigate ON!");
                } else {
                    LogMessageByIdWithPopupCheckUser(leader, "Already active!");
                }
            }
            else if ((gRealInputs.held & R_BUTTON) && (gRealInputs.pressed & L_BUTTON)) {
                //LogMessageByIdWithPopupCheckUser(leader, "Hold R + Press L!");
                if (!IsAutoExploreActive()) {
                    SetAutoExploreActive(TRUE);
                    LogMessageByIdWithPopupCheckUser(leader, "Auto-navigate ON!");
                } else {
                    LogMessageByIdWithPopupCheckUser(leader, "Already active!");
                }
            }
            
            // B button to cancel auto-navigate (only when auto-navigate is active)
            if ((gRealInputs.pressed & B_BUTTON) && IsAutoExploreActive()) {
                SetAutoExploreActive(FALSE);
                LogMessageByIdWithPopupCheckUser(leader, "Auto-navigate OFF!");
            }

            tryItemThrow = FALSE;
            if (gRealInputs.held & R_BUTTON) {
                if (!sInDiagonalMode) {
                    sArrowsFrames = 0;
                }
                sInDiagonalMode = TRUE;
            }
            else {
                sInDiagonalMode = FALSE;
            }

            highlightTiles = FALSE;
            if (gGameOptionsRef->controls == CONTROLS_GBA) {
                if (gRealInputs.shortPress & R_BUTTON || rPress || gRealInputs.pressed & START_BUTTON) {
                    highlightTiles = TRUE;
                }
            }
            if (highlightTiles) {
                sub_805E738(leader);
                sInRotateMode = TRUE;
                unkPtr->rotateModeDirection = leaderInfo->action.direction;
                unkPtr->prevRotateModeDirection = 0xFF;
                ResetRepeatTimers();
            }

            if ((gRealInputs.held & L_BUTTON) == L_BUTTON && (gRealInputs.pressed & R_BUTTON) == R_BUTTON) {
                tryItemThrow = TRUE;
            }
            if (triggers[0]) {
                tryItemThrow = TRUE;
            }
            if (tryItemThrow) {
                s32 i;
                for (i = 0; i < INVENTORY_SIZE; i++) {
                    if (ItemExists(&gTeamInventoryRef->teamItems[i]) && ItemSet(&gTeamInventoryRef->teamItems[i])) {
                        SetLeaderActionFields(ACTION_THROW_ITEM_PLAYER);
                        leaderInfo->action.actionParameters[0].actionUseIndex = i +1;
                        leaderInfo->action.actionParameters[0].itemPos.x = 0;
                        leaderInfo->action.actionParameters[0].itemPos.y = 0;
                        break;
                    }
                }
                if (leaderInfo->action.action != 0) {
                    break;
                }
            }

            // SELECT button
            if (!gDungeon->unk181e8.blinded && gGameOptionsRef->mapOption != 6 && gRealInputs.pressed & SELECT_BUTTON) {
                s32 prevMapOption = gGameOptionsRef->mapOption;
                gShowMonsterDotsInDungeonMap = TRUE;
                gDungeon->unk181e8.inFloorMapMode = TRUE;
                if (!GameOptions_ShowMiniMap()) {
                    GameOptions_SetTransparentMiniMap();
                }
                sub_8052210(1);
                UpdateMinimap();
                SetBGOBJEnableFlags(0x1E);
                sub_803E708(0xA, 0x2F);
                while (1) {
                    DungeonRunFrameActions(0x2F);
                    if (gRealInputs.pressed & SELECT_BUTTON)
                        break;
                    if (gRealInputs.pressed & B_BUTTON)
                        break;

                    if (gRealInputs.pressed & A_BUTTON) {
                        gShowMonsterDotsInDungeonMap = (gShowMonsterDotsInDungeonMap == FALSE) ? TRUE : FALSE; // Flip
                        UpdateMinimap();
                    }
                }
                gDungeon->unk181e8.inFloorMapMode = FALSE;
                gGameOptionsRef->mapOption = prevMapOption;
                gShowMonsterDotsInDungeonMap = TRUE;
                UpdateMinimap();
                SetBGOBJEnableFlags(0);
                DungeonRunFrameActions(0x2F);
                DungeonRunFrameActions(0x2F);
            }

            // Handle auto-explore movement
            if (IsAutoExploreActive() && !sInRotateMode) {
                s32 autoExploreDir = GetAutoExploreDirection(leader);
                if (autoExploreDir >= 0) {
                    u8 canMoveFlags = 0;
                    const u8 *immobilizedMsg = NULL;

                    leaderInfo->action.direction = autoExploreDir & DIRECTION_MASK;

                    if (sub_805EC4C(leader, 1))
                        break;

                    if (leaderInfo->frozenClassStatus.status == STATUS_SHADOW_HOLD) {
                        immobilizedMsg = gUnknown_80F8A84, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_CONSTRICTION) {
                        immobilizedMsg = gUnknown_80F8A6C, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_INGRAIN) {
                        immobilizedMsg = gUnknown_80F8AB0, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_WRAP) {
                        immobilizedMsg = gUnknown_80F8ADC, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_WRAPPED) {
                        immobilizedMsg = gUnknown_80F8B0C, canMoveFlags |= 1;
                    }

                    if (!CanMoveInDirection(leader, autoExploreDir))
                        canMoveFlags |= 2;

                    sub_806CDD4(leader, sub_806CEBC(leader), autoExploreDir);

                    if (!(canMoveFlags & 2)) {
                        if (canMoveFlags & 1) {
                            if (immobilizedMsg != NULL) {
                                LogMessageByIdWithPopupCheckUser(leader, immobilizedMsg);
                            }
                            SetLeaderActionFields(ACTION_PASS_TURN);
                            gDungeon->unk644.unk2F = 1;
                        }
                        else {
                            SetLeaderActionFields(ACTION_WALK);
                            leaderInfo->action.actionParameters[0].actionUseIndex = 1;
                        }
                        break;
                    }
                    else {
                        // Can't move in auto-explore direction, stop auto-explore
                        SetAutoExploreActive(FALSE);
                    }
                }
            }

            if (gDungeon->unk644.unk29 != 0 && !sInDiagonalMode) {
                dpadDiagonal = dpadSimple = gRealInputs.pressed;
            }
            else {
                dpadDiagonal = gRealInputs.held;
                dpadSimple = (unkPtr->unk1821A == 0) ? gRealInputs.held : gRealInputs.pressed;
            }

            dpadDiagonal &= DPAD_ANY;
            dpadSimple &= DPAD_ANY;
            directionNew = -1;
            if (dpadDiagonal == (DPAD_UP | DPAD_RIGHT))
                directionNew = DIRECTION_NORTHEAST;
            if (dpadDiagonal == (DPAD_UP | DPAD_LEFT))
                directionNew = DIRECTION_NORTHWEST;
            if (dpadDiagonal == (DPAD_DOWN | DPAD_RIGHT))
                directionNew = DIRECTION_SOUTHEAST;
            if (dpadDiagonal == (DPAD_DOWN | DPAD_LEFT))
                directionNew = DIRECTION_SOUTHWEST;

            if (dpadSimple == DPAD_UP)
                directionNew = DIRECTION_NORTH;
            if (dpadSimple == DPAD_DOWN)
                directionNew = DIRECTION_SOUTH;
            if (dpadSimple == DPAD_RIGHT)
                directionNew = DIRECTION_EAST;
            if (dpadSimple == DPAD_LEFT)
                directionNew = DIRECTION_WEST;

            if (directionNew >= 0 && (!sInDiagonalMode || (directionNew & 1))) {
                bool32 directionChanged = (leaderInfo->action.direction != directionNew);
                leaderInfo->action.direction = directionNew & DIRECTION_MASK;
                if (sInRotateMode) {
                    unkPtr->rotateModeDirection = directionNew;
                    sub_806CDD4(leader, sub_806CEBC(leader), directionNew);
                }
                else {
                    u8 canMoveFlags = 0;
                    const u8 *immobilizedMsg = NULL;

                    if (sub_805EC4C(leader, 1))
                        break;

                    if (leaderInfo->frozenClassStatus.status == STATUS_SHADOW_HOLD) {
                        immobilizedMsg = gUnknown_80F8A84, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_CONSTRICTION) {
                        immobilizedMsg = gUnknown_80F8A6C, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_INGRAIN) {
                        immobilizedMsg = gUnknown_80F8AB0, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_WRAP) {
                        immobilizedMsg = gUnknown_80F8ADC, canMoveFlags |= 1;
                    }
                    else if (leaderInfo->frozenClassStatus.status == STATUS_WRAPPED) {
                        immobilizedMsg = gUnknown_80F8B0C, canMoveFlags |= 1;
                    }

                    if (!CanMoveInDirection(leader, directionNew))
                        canMoveFlags |= 2;

                    if (directionChanged) {
                        sub_806CDD4(leader, sub_806CEBC(leader), directionNew);
                    }

                    if (!(canMoveFlags & 2)) {
                        if (canMoveFlags & 1) {
                            if (immobilizedMsg != NULL) {
                                LogMessageByIdWithPopupCheckUser(leader, immobilizedMsg);
                            }
                            SetLeaderActionFields(ACTION_PASS_TURN);
                            gDungeon->unk644.unk2F = 1;
                        }
                        else {
                            SetLeaderActionFields(ACTION_WALK);
                            if ((gRealInputs.held & B_BUTTON || bPress) && FixedPointToInt(leaderInfo->belly) != 0) {
                                if (GetEntInfo(leader)->cringeClassStatus.status != STATUS_CONFUSED) {
                                    gDungeon->unk644.unk28 = 1;
                                }
                                leaderInfo->action.actionParameters[0].actionUseIndex = 0;
                            }
                            else {
                                leaderInfo->action.actionParameters[0].actionUseIndex = 1;
                            }
                        }
                        break;
                    }
                    else if (canMoveFlags & 1) {
                        sub_803E724(0x23);
                    }

                }
            }
            DungeonRunFrameActions(0xF);
        }

        if (unkPtr->unk1821A != 0) {
            sub_804AA60();
        }

        if (leaderInfo->action.action == 0x2D || leaderInfo->action.action == 0x13) {
            HandleTalkFieldAction(leader);
            if (IsFloorOver())
                break;
            SetLeaderActionFields(ACTION_NOTHING);
        }
        else if ((r6.a0_8) == 0) {
            gDungeon->unk644.unk29 = 0;
            if (leaderInfo->action.action != 0) {
                if (!IsNotAttacking(leader, FALSE)) {
                    DungeonRunFrameActions(0xF);
                }
                break;
            }
            DungeonRunFrameActions(0xF);
        }
        else {
            DungeonRunFrameActions(0xF);
            ClearUnpaidFlagFromAllItems();
            ShowMainMenu((r6.a0_16 == 0), r6.a0_24);
            ResetRepeatTimers();
            ResetUnusedInputStruct();
            sInRotateMode = FALSE;
            unkPtr->unk1821A = 0;
            sub_804AA60();
            if (IsFloorOver())
                break;
            if (leaderInfo->action.action != 0) {
                if (leaderInfo->action.action == 0x2B) {
                    gDungeon->unk4 = 1;
                    gDungeon->unk3 = 1;
                }
                if (leaderInfo->action.action == 0x2E) {
                    gDungeon->unk4 = 1;
                    gDungeon->unk3 = 0;
                }
                break;
            }
            DungeonRunFrameActions(0xF);
            if (gDungeon->unk4 != 0)
                break;
        }
    }
}

struct DiagonalArrowInfo
{
    s16 x;
    s16 y;
    bool8 hFlip;
    bool8 vFlip;
};

static const struct DiagonalArrowInfo sDiagonalArrowsInfo[] =
{
    {-1, -1, TRUE, FALSE},
    {-1, 1, TRUE, TRUE},
    {1, 1, FALSE, TRUE},
    {1, -1, FALSE, FALSE},
};

struct RotateArrowInfo
{
    s16 x;
    s16 y;
    u32 tilemapNum;
    bool8 hFlip;
    bool8 vFlip;
};

static const struct RotateArrowInfo sRotateArrowsInfo[] =
{
    {0, 1,   0x212, FALSE, TRUE},
    {1, 1,   0x213, FALSE, TRUE},
    {1, 0,   0x214, FALSE, FALSE},
    {1, -1,  0x213, FALSE, FALSE},
    {0, -1,  0x212, TRUE, FALSE},
    {-1, -1, 0x213, TRUE, FALSE},
    {-1, 0,  0x214, TRUE, TRUE},
    {-1, 1,  0x213, TRUE, TRUE},
};

// Creates arrow sprites which are used when in rotate or diagonal modes.
static void TryCreateModeArrows(Entity *leader)
{
    UnkDungeonGlobal_unk181E8_sub *unkPtr = &gDungeon->unk181e8;

    if (sInDiagonalMode) {
        s32 i;
        SpriteOAM sprite;

        for (i = 0; i < 4; i++) {
            u32 flips;
            s32 x, xMul, x2;
            s32 y, yMul, y2;

            SpriteSetAffine1(&sprite, 0);
            SpriteSetAffine2(&sprite, 0);
            SpriteSetObjMode(&sprite, 1);
            SpriteSetMosaic(&sprite, 0);
            SpriteSetBpp(&sprite, 0);
            SpriteSetShape(&sprite, 0);

            flips = 0;
            if (sDiagonalArrowsInfo[i].hFlip)
                flips += (1 << SPRITEOAM_SHIFT_H_FLIP_MATRIXNUM);
            if (sDiagonalArrowsInfo[i].vFlip)
                flips += (1 << SPRITEOAM_SHIFT_V_FLIP_MATRIXNUM);

            SpriteSetMatrixNumFlips(&sprite, flips);
            SpriteSetSize(&sprite, 0);

            SpriteSetTileNum(&sprite, 0x213);
            SpriteSetPriority(&sprite, 2);
            SpriteSetPalNum(&sprite, 0);

            SpriteSetUnk6_0(&sprite, 0);
            SpriteSetUnk6_1(&sprite, 0);

            xMul = sDiagonalArrowsInfo[i].x * 10;
            x2 = (sArrowsFrames / 2) & 7;
            x = (x2 * sDiagonalArrowsInfo[i].x) + xMul + 116;
            SpriteSetX(&sprite, x);

            yMul = sDiagonalArrowsInfo[i].y * 10;
            y2 = (sArrowsFrames / 2) & 7;
            y = (y2 * sDiagonalArrowsInfo[i].y) + yMul + 82;
            SpriteSetY(&sprite, y);

            AddSprite(&sprite, 0x100, NULL, NULL);
        }
    }

    else if (unkPtr->unk1821A) {
        s32 i;
        SpriteOAM sprite;
        s32 direction = unkPtr->rotateModeDirection;

        if (unkPtr->rotateModeDirection < NUM_DIRECTIONS) {
            s32 x, xMul, x2;
            s32 y, yMul, y2;
            s32 to = (sShowThreeArrows2 != FALSE && sShowThreeArrows1 != FALSE) ? 3 : 1;

            xMul = sRotateArrowsInfo[direction].x * 10;
            x2 = (sArrowsFrames / 2) & 7;
            x = (sRotateArrowsInfo[direction].x * x2) + xMul + 116;

            yMul = sRotateArrowsInfo[direction].y * 10;
            y2 = (sArrowsFrames / 2) & 7;
            y = (y2 * sRotateArrowsInfo[direction].y) + yMul + 82;
            for (i = 0; i < to; i++) {
                u32 flips;

                SpriteSetAffine1(&sprite, 0);
                SpriteSetAffine2(&sprite, 0);
                SpriteSetObjMode(&sprite, 1);
                SpriteSetMosaic(&sprite, 0);
                SpriteSetBpp(&sprite, 0);
                SpriteSetShape(&sprite, 0);

                flips = 0;
                if (sRotateArrowsInfo[direction].hFlip)
                    flips += (1 << SPRITEOAM_SHIFT_H_FLIP_MATRIXNUM);
                if (sRotateArrowsInfo[direction].vFlip)
                    flips += (1 << SPRITEOAM_SHIFT_V_FLIP_MATRIXNUM);

                SpriteSetMatrixNumFlips(&sprite, flips);
                SpriteSetSize(&sprite, 0);

                SpriteSetTileNum(&sprite, sRotateArrowsInfo[direction].tilemapNum);
                SpriteSetPriority(&sprite, 2);
                SpriteSetPalNum(&sprite, 0);

                SpriteSetUnk6_0(&sprite, 0);
                SpriteSetUnk6_1(&sprite, 0);

                SpriteSetX(&sprite, x);
                SpriteSetY(&sprite, y);

                AddSprite(&sprite, 0x100, NULL, NULL);
                x += sRotateArrowsInfo[direction].x * 4;
                y += sRotateArrowsInfo[direction].y * 4;
            }
        }
    }

    if (sInRotateMode && unkPtr->prevRotateModeDirection != unkPtr->rotateModeDirection) {
        unkPtr->prevRotateModeDirection = unkPtr->rotateModeDirection;
        ChangeDungeonCameraPos(&leader->pos, unkPtr->rotateModeDirection, 0, sInRotateMode);
    }
}

static void sub_805E738(Entity *a0)
{
    const Tile *tile;
    s32 i, j;
    EntityInfo *entityInfo = GetEntInfo(a0);
    if (entityInfo->blinkerClassStatus.status != 1 && entityInfo->blinkerClassStatus.status != 2) {
        // What???
        for (i = 0; i < 1; i++) {
            bool8 r9 = FALSE;
            u32 direction = entityInfo->action.direction;
            direction++;
            for (j = 1; j < 8; j++, direction++) {
                direction &= DIRECTION_MASK;
                tile = GetTile(a0->pos.x + gAdjacentTileOffsets[direction].x, a0->pos.y + gAdjacentTileOffsets[direction].y);
                if (tile->monster != NULL && GetEntityType(tile->monster) == ENTITY_MONSTER) {
                    EntityInfo *tileMonsterInfo = GetEntInfo(tile->monster);
                    if (CanSeeTarget(a0, tile->monster)) {
                        if (i != 0 || tileMonsterInfo->isNotTeamMember) {
                            r9 = TRUE;
                            break;
                        }
                    }
                }
            }
            if (r9) {
                GetEntInfo(a0)->action.direction = direction & DIRECTION_MASK;
                sub_806CDD4(a0, sub_806CEBC(a0), direction);
                break;
            }
        }
    }
}

void sub_805E804(void)
{
    gDungeon->unk644.unk29 |= gDungeon->unk644.unk28;
    gDungeon->unk644.unk28 = 0;
    while (gDungeon->unk644.unk29 != 0 && gRealInputs.held & R_BUTTON) {
        DungeonRunFrameActions(0x54);
    }
}

static bool8 sub_805E874(void)
{
    u8 r7, r8, r0;
    s32 i, j, k;
    s32 xArray[3];
    s32 yArray[3];
    Dungeon *dungeon = gDungeon;
    Entity *leader = GetLeader();
    s32 direction = GetEntInfo(leader)->action.direction;
    s32 x = leader->pos.x;
    s32 y = leader->pos.y;
    const Tile *leaderTile = GetTile(x, y);
    s32 xAdjacent = x + gAdjacentTileOffsets[direction].x;
    s32 yAdjacent = y + gAdjacentTileOffsets[direction].y;
    s32 room;

    if (dungeon->unk644.unk28 == 0)
        return FALSE;
    if (leaderTile->object != NULL)
        return FALSE;
    if (sub_805EC2C(leader, x, y))
        return FALSE;
    if (!sub_8070F14(leader, direction))
        return FALSE;

    room = leaderTile->room;
    if (room == 0xFF) {
        if (GetTile(xAdjacent, yAdjacent)->room != 0xFF)
            return FALSE;
    }
    else {
        if (leaderTile->terrainFlags & TERRAIN_TYPE_NATURAL_JUNCTION)
            return FALSE;
    }

    for (j = -1; j < 2; j++) {
        const Tile *tile = GetTile(x + gAdjacentTileOffsets[(direction + j) & 7].x, y + gAdjacentTileOffsets[(direction + j) & 7].y);
        if (tile->monster != NULL)
            return FALSE;
        if (tile->terrainFlags & TERRAIN_TYPE_STAIRS)
            return FALSE;
    }

    xArray[0] = x + gAdjacentTileOffsets[(direction + 3) & 7].x;
    xArray[1] = x + gAdjacentTileOffsets[(direction + 4) & 7].x;
    xArray[2] = x + gAdjacentTileOffsets[(direction + 5) & 7].x;

    yArray[0] = y + gAdjacentTileOffsets[(direction + 3) & 7].y;
    yArray[1] = y + gAdjacentTileOffsets[(direction + 4) & 7].y;
    yArray[2] = y + gAdjacentTileOffsets[(direction + 5) & 7].y;

    for (i = -1; i < 2; i++) {
        for (j = -1; j < 2; j++) {
            const Tile *tile = GetTile(x + i, y + j);
            if (tile->object != NULL) {
                for (k = 0; k < 3; k++) {
                    if (x + i == xArray[k] && y + j == yArray[k])
                        break;
                }
                if (k == 3 && GetEntityType(tile->object) == ENTITY_ITEM)
                    return FALSE;
            }

            if (direction & 1) {
                // This doesn't really have to be called two times...
                if (tile->object != NULL && GetEntityType(tile->object) == ENTITY_TRAP && GetEntityType(tile->object) == ENTITY_TRAP) {
                    if (tile->object->isVisible)
                        return FALSE;
                    if (gDungeon->unk181e8.showInvisibleTrapsMonsters)
                        return FALSE;
                }
            }
            else if (i == 0 || j == 0) {
                if (tile->object != NULL && GetEntityType(tile->object) == ENTITY_TRAP) {
                    if (tile->object->isVisible)
                        return FALSE;
                    if (gDungeon->unk181e8.showInvisibleTrapsMonsters)
                        return FALSE;
                }
            }

            if (i != 0 && j != 0) continue;
            if (i == 0 && j == 0) continue;

            if ((xArray[1] != x + i || yArray[1] != y + j) && !sub_805EC2C(leader, x + i, y + j) && room != 0xFF && room != tile->room)
                return FALSE;
        }
    }

    if (!(direction & 1)) {
        if (direction == 0 || direction == 4) {
            r8 = sub_805EC2C(leader, x - 1, y - 1);
            r7 = sub_805EC2C(leader, x - 1, y);
            r0 = sub_805EC2C(leader, x - 1, y + 1);
            if (r7 == 0) {
                if (r8 != 0 || r7 != r0)
                    return FALSE;
            }

            r8 = sub_805EC2C(leader, x + 1, y - 1);
            r7 = sub_805EC2C(leader, x + 1, y);
            r0 = sub_805EC2C(leader, x + 1, y + 1);
            if (r7 == 0) {
                if (r8 != 0 || r7 != r0)
                    return FALSE;
            }
        }
        else {
            r8 = sub_805EC2C(leader, x - 1, y - 1);
            r7 = sub_805EC2C(leader, x, y - 1);
            r0 = sub_805EC2C(leader, x + 1, y - 1);
            if (r7 == 0) {
                if (r8 != 0 || r7 != r0)
                    return FALSE;
            }

            r8 = sub_805EC2C(leader, x - 1, y + 1);
            r7 = sub_805EC2C(leader, x, y + 1);
            r0 = sub_805EC2C(leader, x + 1, y + 1);
            if (r7 == 0) {
                if (r8 != 0 || r7 != r0)
                    return FALSE;
            }
        }
    }

    return TRUE;
}

static bool8 sub_805EC2C(Entity *a0, s32 x, s32 y)
{
    DungeonPos pos = {.x = x, .y = y};
    return sub_8070564(a0, &pos);
}

static bool8 sub_805EC4C(Entity *a0, u8 a1)
{
    DungeonPos pos;
    const Tile *tile;
    EntityInfo *tileMonsterInfo;
    Entity *tileMonster;
    EntityInfo *entityInfo = GetEntInfo(a0);

    pos.x = a0->pos.x + gAdjacentTileOffsets[entityInfo->action.direction].x;
    pos.y = a0->pos.y + gAdjacentTileOffsets[entityInfo->action.direction].y;
    tile = GetTile(pos.x, pos.y);
    tileMonster = tile->monster;

    if (tileMonster == NULL) return FALSE;
    if (GetEntityType(tileMonster) != ENTITY_MONSTER) return FALSE;

    tileMonsterInfo = GetEntInfo(tileMonster);
    if (tileMonsterInfo->isNotTeamMember
        && (tileMonsterInfo->shopkeeper != 1 && tileMonsterInfo->shopkeeper != 2)
        && !IsExperienceLocked(tileMonsterInfo->joinedAt.id)
        && tileMonsterInfo->monsterBehavior != BEHAVIOR_RESCUE_TARGET) {
        return FALSE;
    }

    if (entityInfo->frozenClassStatus.status == STATUS_SHADOW_HOLD) return FALSE;
    if (entityInfo->frozenClassStatus.status == STATUS_FROZEN) return FALSE;
    if (entityInfo->frozenClassStatus.status == STATUS_CONSTRICTION) return FALSE;
    if (entityInfo->frozenClassStatus.status == STATUS_INGRAIN) return FALSE;
    if (entityInfo->frozenClassStatus.status == STATUS_WRAP) return FALSE;
    if (entityInfo->frozenClassStatus.status == STATUS_WRAPPED) return FALSE;

    if (tileMonsterInfo->frozenClassStatus.status == STATUS_SHADOW_HOLD) return FALSE;
    if (tileMonsterInfo->frozenClassStatus.status == STATUS_FROZEN) return FALSE;
    if (tileMonsterInfo->frozenClassStatus.status == STATUS_CONSTRICTION) return FALSE;
    if (tileMonsterInfo->frozenClassStatus.status == STATUS_INGRAIN) return FALSE;
    if (tileMonsterInfo->frozenClassStatus.status == STATUS_WRAP) return FALSE;
    if (tileMonsterInfo->frozenClassStatus.status == STATUS_WRAPPED) return FALSE;

    if (entityInfo->cringeClassStatus.status == STATUS_CONFUSED) return FALSE;
    if (tileMonsterInfo->cringeClassStatus.status == STATUS_CONFUSED) return FALSE;

    if (tileMonsterInfo->sleepClassStatus.status != STATUS_NONE && tileMonsterInfo->sleepClassStatus.status != STATUS_SLEEPLESS && tileMonsterInfo->sleepClassStatus.status != STATUS_YAWNING)  return FALSE;
    if (entityInfo->sleepClassStatus.status != STATUS_NONE      && entityInfo->sleepClassStatus.status != STATUS_SLEEPLESS      && entityInfo->sleepClassStatus.status != STATUS_YAWNING)       return FALSE;

    if (IsChargingAnyTwoTurnMove(tileMonster, FALSE)) return FALSE;
    if (!sub_8070F80(a0, entityInfo->action.direction)) return FALSE;

    if (a1 != 0 && sub_807049C(tileMonster, &a0->pos) && !DisplayDungeonYesNoMessage(0, gUnknown_8100208, 0)) return FALSE;

    SetMonsterActionFields(&entityInfo->action, ACTION_WALK);
    if (gRealInputs.held & B_BUTTON) {
        entityInfo->action.actionParameters[0].actionUseIndex = 0;
    }
    else {
        entityInfo->action.actionParameters[0].actionUseIndex = 1;
    }
    entityInfo->flags |= 0x8000;

    SetMonsterActionFields(&tileMonsterInfo->action, ACTION_WALK);
    tileMonsterInfo->action.actionParameters[0].actionUseIndex = 0;
    tileMonsterInfo->action.direction = (entityInfo->action.direction + 4) & 7;
    tileMonsterInfo->flags |= 0x8000;
    tileMonsterInfo->targetPos.x = tileMonster->pos.x;
    tileMonsterInfo->targetPos.y = tileMonster->pos.y;
    gDungeon->unkE = 1;
    return TRUE;
}

void CheckLeaderTile(void)
{
    Entity *tileObject;
    Tile *tile;
    Entity *leader = GetLeader();
    if (leader == NULL)
        return;
    if (IsFloorOver())
        return;

    tile = GetTileAtEntitySafe(leader);
    if (IQSkillIsEnabled(leader, IQ_SUPER_MOBILE) && GetEntInfo(leader)->invisibleClassStatus.status != STATUS_MOBILE && !HasHeldItem(leader, ITEM_MOBILE_SCARF))
        sub_804AE84(&leader->pos);
    if (tile->terrainFlags & TERRAIN_TYPE_STAIRS)
        gDungeon->unk1 = 1;

    tileObject = tile->object;
    if (tileObject == NULL)
        return;
    switch (GetEntityType(tileObject))
    {
        case ENTITY_TRAP: {
            Trap *trap = GetTrapInfo(tileObject);
            bool32 r8 = FALSE;
            bool32 r7 = FALSE;
            if (IQSkillIsEnabled(leader, IQ_TRAP_SEER) && !tileObject->isVisible) {
                tileObject->isVisible = TRUE;
                UpdateTrapsVisibility();
                r7 = TRUE;
            }
            if (trap->unk1 != 0) {
                if (trap->unk1 == 1)
                    break;
                if (trap->unk1 == 2)
                    r8 = TRUE;
                if (r8 == FALSE)
                    break;
            }
            if (!r7) {
                TryTriggerTrap(leader, &leader->pos, 0, 1);
            }
        }
        break;
        case ENTITY_ITEM: {
            Item *item = GetItemInfo(tileObject);
            if (!(item->flags & ITEM_FLAG_IN_SHOP)) {
                TryLeaderItemPickUp(&leader->pos, 1);
            }
            else {
                gDungeon->unk5C0 = 4;
            }
        }
        break;
        case ENTITY_NOTHING:
        case ENTITY_MONSTER:
        case ENTITY_UNK_4:
        case ENTITY_UNK_5:
        default:
            break;
    }
}

static bool8 sub_805EF60(Entity *a0, EntityInfo *a1)
{
    Entity *r4 = sub_80696A8(a0);

    if (r4 == NULL)
        return FALSE;
    if (GetEntityType(r4) != ENTITY_MONSTER)
        return FALSE;
    if (!sub_8070BC0(a0))
        return FALSE;
    if (GetEntInfo(r4)->isNotTeamMember && GetEntInfo(r4)->monsterBehavior != BEHAVIOR_RESCUE_TARGET && GetEntInfo(r4)->shopkeeper != 1)
        return FALSE;

    SetMonsterActionFields(&a1->action, ACTION_TALK_FIELD);
    return TRUE;
}

void sub_805EFB4(Entity *a0, bool8 a1)
{
    s32 i;
    EntityInfo *leaderInfo = GetLeaderInfo();
    if (a1 && leaderInfo->cringeClassStatus.status == STATUS_COWERING) {
        leaderInfo->action.direction += 4;
        leaderInfo->action.direction &= 7;
    }
    else if (leaderInfo->cringeClassStatus.status == STATUS_CONFUSED) {
        s32 rnd = DungeonRandInt(8);
        for (i = 0; i < 8; i++) {
            if (a1 || CanMoveInDirection(a0, rnd)) {
                leaderInfo->action.direction = rnd & 7;
                return;
            }
            rnd = (rnd + 1) & 7;
        }
    }
}

void sub_805F02C(void)
{
    s32 i;
    Entity *r7 = gDungeon->unkBC;
    Entity *leader = GetLeader();
    EntityInfo *r8 = GetEntInfo(r7);
    EntityInfo *leaderInfo = GetEntInfo(leader);

    if (r8->isTeamLeader) {
        DisplayDungeonLoggableMessageTrue(r7, gUnknown_80F9BD8);
    }
    else if (PlayerHasItemWithFlag(ITEM_FLAG_IN_SHOP) || sub_807EF48()) {
        DisplayDungeonLoggableMessageTrue(r7, gUnknown_80F9C08);
    }
    else if (gDungeon->unk644.unk2A) {
        DisplayDungeonLoggableMessageTrue(r7, gUnknown_80F9C2C);
    }
    else {
        gDungeon->unk644.emptyBellyAlert = 0;
        r8->isTeamLeader = TRUE;
        leaderInfo->isTeamLeader = FALSE;
        for (i = 0; i < 4; i++) {
            DungeonMon *mon = &gRecruitedPokemonRef->dungeonTeam[i];
            Pokemon *r5 = NULL;

            if ((u8)mon->flags & 1) {
                if (!sub_806A538(mon->recruitedPokemonId)) {
                    r5 = &gRecruitedPokemonRef->pokemon[mon->recruitedPokemonId];
                }
                if (i == r8->teamIndex) {
                    mon->isTeamLeader = TRUE;
                    if (r5 != NULL) {
                        r5->isTeamLeader = TRUE;
                    }
                }
                else {
                    mon->isTeamLeader = FALSE;
                    if (r5 != NULL) {
                        r5->isTeamLeader = FALSE;
                    }
                }
            }
        }
        gLeaderPointer = NULL;
        r8->action = leaderInfo->action;
        PointCameraToMonster(r7);
        sub_8041AD0(leader);
        sub_8041AE0(GetLeader());
        SubstitutePlaceholderStringTags(gFormatBuffer_Monsters[0], r7, 0);
        LogMessageByIdWithPopupCheckUser(r7, gUnknown_80F9BB0);
        sub_807EC28(FALSE);
        r8->unk64 = 0;
        leaderInfo->unk64 = 0;
        sub_806A6E8(leader);
        sub_806A6E8(r7);
    }
}

ActionContainer *GetLeaderActionContainer(void)
{
    return &GetLeaderInfo()->action;
}

u16 GetLeaderActionId(void)
{
    return GetLeaderInfo()->action.action;
}

enum
{
    MAIN_MENU_MOVES,
    MAIN_MENU_ITEMS,
    MAIN_MENU_TEAM,
    MAIN_MENU_OTHERS,
    MAIN_MENU_GROUND,
};

static void ShowMainMenu(bool8 fromBPress, bool8 a1)
{
    Item *item;
    s32 r10;
    bool8 printAll = fromBPress;
    s32 chosenOption;
    s32 var_24;
    struct UnkMenuBitsStruct var_30;
    struct UnkMenuBitsStruct var_34;

    r10 = gDungeon->unk5C0;
    chosenOption = 0;
    var_24 = (gDungeon->unk5C0 > - 1);
    gDungeon->unk5C0 = -1;
    if (r10 >= 0) {
        chosenOption = r10;
    }

    if (a1) {
        PlayFanfareSE(0x137, 0x100);
    }
    else {
        PlayDungeonStartButtonSE();
    }

    while (1) {
        if (r10 < 0) {
            SetLeaderActionToNothing(TRUE);
            gTeamMenuChosenId = -1;
            PrintOnMainMenu(printAll);
            TryPointCameraToMonster(GetLeader(), 0);
            while (1) {
                AddMenuCursorSprite(&gDungeonMenu);
                DungeonRunFrameActions(0x1D);
                if (gRealInputs.repeated & DPAD_DOWN) {
                    PlayDungeonCursorSE(1);
                    MoveMenuCursorDownWrapAround(&gDungeonMenu, TRUE);
                }
                if (gRealInputs.repeated & DPAD_UP) {
                    PlayDungeonCursorSE(1);
                    MoveMenuCursorUpWrapAround(&gDungeonMenu, TRUE);
                }
                if ((gRealInputs.pressed & A_BUTTON || gDungeonMenu.touchScreen.a_button)) {
                    if (gUnknown_202749A[gDungeonMenu.menuIndex + 1] == 7) {
                        PlayDungeonConfirmationSE();
                        chosenOption = gDungeonMenu.menuIndex;
                        break;
                    }
                    PlayDungeonCancelSE();
                }
                if ((gRealInputs.pressed & B_BUTTON) || gDungeonMenu.touchScreen.b_button) {
                    PlayDungeonCancelSE();
                    chosenOption = -1;
                    break;
                }
            }
            AddMenuCursorSprite(&gDungeonMenu);
            DungeonRunFrameActions(0x1D);
            printAll = TRUE;
        }

        r10 = chosenOption;
        if (chosenOption == MAIN_MENU_ITEMS) {
            u16 action;

            SetLeaderActionToNothing(TRUE);
            var_34.a0_8 = 0;
            var_34.a0_16 = 1;
            var_34.a0_24 = 0;
            var_34.a0_32 = 0;
            if (ShowDungeonItemsMenu(GetLeader(), &var_34)) {
                r10 = -1;
            }
            if (sub_805FD3C(&var_34) && ShowDungeonItemsMenu(GetLeader(), &var_34)) {
                SetLeaderActionToNothing(TRUE);
            }
            action = GetLeaderActionId();
            if (action == ACTION_SHOW_INFO) {
                sub_8044D90(GetLeader(), 0, 12)->flags |= ITEM_FLAG_UNPAID;
                DungeonShowItemDescription(GetLeaderActionContainer());
                SetLeaderActionToNothing(TRUE);
            }
            else if (action == ACTION_UNK35) {
                item = sub_8044D90(GetLeader(), 0, 13);
                if (!sub_8048A68(GetLeader(), item)) {
                    SetLeaderActionToNothing(TRUE);
                }
            }
            else if (action == ACTION_UNK10) {
                item = sub_8044D90(GetLeader(), 0, 14);
                if (!sub_8048950(GetLeader(), item)) {
                    SetLeaderActionToNothing(TRUE);
                }
            }
            else if (action == ACTION_USE_LINK_BOX) {
                item = sub_8044D90(GetLeader(), 0, 15);
                if (!sub_8048B9C(GetLeader(), item)) {
                    SetLeaderActionToNothing(TRUE);
                }
            }
            else if (action == ACTION_SET_ITEM) {
                HandleSetItemAction(GetLeader(), TRUE);
                SetLeaderActionToNothing(TRUE);
                sub_803E708(0x50, 0x4D);
                sub_8052210(0);
                break;
            }
            else if (action == ACTION_UNSET_ITEM) {
                HandleUnsetItemAction(GetLeader(), TRUE);
                SetLeaderActionToNothing(TRUE);
                sub_803E708(0x50, 0x4D);
                sub_8052210(0);
                break;
            }

            if (GetLeaderActionId() != ACTION_NOTHING)
                break;
        }
        else if (chosenOption == MAIN_MENU_TEAM) {
            SetLeaderActionToNothing(TRUE);
            if (ShowDungeonTeamMenu(GetLeader())) {
                r10 = -1;
            }

            if (GetLeaderActionId() == ACTION_CHECK_SUMMARY) {
                ShowDungeonSummaryOrIQMenu(GetLeaderActionContainer(), FALSE);
                SetLeaderActionToNothing(TRUE);
            }
            else if (GetLeaderActionId() == ACTION_TALK_MENU) {
                sub_806752C(GetLeaderActionContainer());
                SetLeaderActionToNothing(TRUE);
            }
            else if (GetLeaderActionId() == ACTION_UNK34) {
                sub_8067768(GetLeaderActionContainer());
                SetLeaderActionToNothing(TRUE);
            }
            else if (GetLeaderActionId() == ACTION_CHANGE_TACTICS) {
                ShowDungeonTacticsMenu(GetLeaderActionContainer());
                SetLeaderActionToNothing(TRUE);
            }
            else if (GetLeaderActionId() == ACTION_VIEW_IQ) {
                ShowDungeonSummaryOrIQMenu(GetLeaderActionContainer(), TRUE);
                SetLeaderActionToNothing(TRUE);
            }
            else if (GetLeaderActionId() == ACTION_CHECK_MOVES) {
                s32 i, count;

                ShowMovesFromTeamMenu(GetLeaderActionContainer());
                count = 0;
                for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
                    Entity *teamMon = gDungeon->teamPokemon[i];
                    if (EntityIsValid(teamMon)) {
                        if (i == GetLeaderActionContainer()->actionParameters[0].actionUseIndex) {
                            gTeamMenuChosenId = count;
                            if (GetLeaderActionId() != ACTION_NOTHING) {
                                TryPointCameraToMonster(teamMon, 0);
                            }
                            break;
                        }
                        count++;
                    }
                }
            }

            if (GetLeaderActionId() != ACTION_NOTHING)
                break;
        }
        else if (chosenOption == MAIN_MENU_MOVES) {
            s32 i, currMonId, teamMonsCount, r9;
            Entity *currEntity;

            currMonId = 0;
            SetLeaderActionToNothing(TRUE);
            for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
                Entity *teamMon = gDungeon->teamPokemon[i];
                if (EntityIsValid(teamMon)) {
                    if (GetEntInfo(teamMon)->isTeamLeader) {
                        currMonId = i;
                        break;
                    }
                }
            }

            while (1) {
                SetLeaderActionToNothing(0);
            LOOP_START_NO_CALL: // Actions 6 and 7 don't call SetLeaderActionToNothing
                currEntity = NULL;
                r9 = 0;
                teamMonsCount = 0;
                for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
                    Entity *teamMon = gDungeon->teamPokemon[i];
                    if (sub_8071A8C(teamMon)) {
                        if (i == currMonId) {
                            r9 = teamMonsCount;
                            currEntity = teamMon;
                        }
                        teamMonsCount++;
                    }
                }
                if (currEntity == NULL) {
                    currEntity = GetLeader();
                }

                TryPointCameraToMonster(currEntity, 0);
                ChangeDungeonCameraPos(&currEntity->pos, 0, 1, 1);
                GetLeaderInfo()->action.actionParameters[0].actionUseIndex = currMonId;
                SetLeaderActionToNothing(FALSE);
                if (ShowDungeonMovesMenu(currEntity, 0, 1, r9, teamMonsCount)) {
                    r10 = -1;
                }

                if (GetLeaderActionId() == ACTION_MOVES_MENU_NEXT_MON) {
                    s32 prevMonId = currMonId;
                    for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
                        if (++currMonId >= MAX_TEAM_MEMBERS) {
                            currMonId = 0;
                        }
                        currEntity = gDungeon->teamPokemon[currMonId];
                        if (sub_8071A8C(currEntity))
                            break;
                    }
                    GetLeaderInfo()->action.actionParameters[0].actionUseIndex = currMonId;
                    if (prevMonId != currMonId) {
                        PlayDungeonCursorSE(0);
                    }
                    goto LOOP_START_NO_CALL;
                }
                else if (GetLeaderActionId() == ACTION_MOVES_MENU_PREV_MON) {
                    s32 prevMonId = currMonId;
                    for (i = 0; i < MAX_TEAM_MEMBERS; i++) {
                        if (--currMonId < 0) {
                            currMonId = MAX_TEAM_MEMBERS - 1;
                        }
                        currEntity = gDungeon->teamPokemon[currMonId];
                        if (sub_8071A8C(currEntity))
                            break;
                    }
                    GetLeaderInfo()->action.actionParameters[0].actionUseIndex = currMonId;
                    if (prevMonId != currMonId) {
                        PlayDungeonCursorSE(0);
                    }
                    goto LOOP_START_NO_CALL;
                }
                else if (GetLeaderActionId() == ACTION_MOVE_INFO) {
                    ActionShowMoveInfo(GetLeaderActionContainer());
                }
                else if (GetLeaderActionId() == ACTION_SET_MOVE || GetLeaderActionId() == ACTION_UNSET_MOVE) {
                    ActionSetOrUnsetMove(GetLeaderActionContainer(), FALSE);
                }
                else if (GetLeaderActionId() == ACTION_SWITCH_AI_MOVE) {
                    ActionToggleMoveUsableForAi(GetLeaderActionContainer());
                }
                else if (GetLeaderActionId() == ACTION_LINK_MOVES) {
                    ActionLinkMoves(GetLeaderActionContainer());
                }
                else if (GetLeaderActionId() == ACTION_DELINK_MOVES) {
                    ActionDelinkMoves(GetLeaderActionContainer(), FALSE);
                }
                else {
                    break;
                }
            }
            TryPointCameraToMonster(GetLeader(), 0);
            ChangeDungeonCameraPos(&GetLeader()->pos, 0, 1, 1);
            if (GetLeaderActionId() != ACTION_NOTHING)
                break;
        }
        else if (chosenOption == MAIN_MENU_GROUND) {
            Entity *leader = GetLeader();
            const Tile *tile = GetTile(leader->pos.x, leader->pos.y);
            Entity *tileObject = tile->object;
            if (tileObject != NULL) {
                if (GetEntityType(tileObject) == ENTITY_ITEM) {
                    u16 action;

                    SetLeaderActionToNothing(TRUE);
                    var_30.a0_8 = 0;
                    var_30.a0_16 = 1;
                    var_30.a0_24 = 1;
                    var_30.a0_32 = 1;
                    if (ShowDungeonItemsMenu(GetLeader(), &var_30)) {
                        // This actually doesn't do anything, it's just there to make the code match as the compiler does a `lsl r0, r0, #0x10, mov r0, r4`
                        ASM_MATCH_TRICK(leader);
                    }
                    if (sub_805FD3C(&var_30) && ShowDungeonItemsMenu(GetLeader(), &var_30)) {
                        SetLeaderActionToNothing(TRUE);
                    }

                    action = GetLeaderActionId();
                    if (action == ACTION_SHOW_INFO) {
                        sub_8044D90(GetLeader(), 0, 0x10)->flags |= ITEM_FLAG_UNPAID;
                        DungeonShowItemDescription(GetLeaderActionContainer());
                        SetLeaderActionToNothing(TRUE);
                    }
                    else if (action == ACTION_UNK35) {
                        item = sub_8044D90(GetLeader(), 0, 0x11);
                        if (!sub_8048A68(GetLeader(), item)) {
                            SetLeaderActionToNothing(TRUE);
                        }
                    }
                    else if (action == ACTION_UNK10) {
                        item = sub_8044D90(GetLeader(), 0, 0x12);
                        if (!sub_8048950(GetLeader(), item)) {
                            SetLeaderActionToNothing(TRUE);
                        }
                    }
                    else if (action == ACTION_USE_LINK_BOX) {
                        item = sub_8044D90(GetLeader(), 0, 0x13);
                        if (!sub_8048B9C(GetLeader(), item)) {
                            SetLeaderActionToNothing(TRUE);
                        }
                    }
                    if (GetLeaderActionId() != ACTION_NOTHING)
                        break;
                }
                else if (GetEntityType(tileObject) == ENTITY_TRAP) {
                    SetLeaderActionToNothing(TRUE);
                    ShowDungeonTileMenu(GetLeader());
                    if (GetLeaderActionId() != ACTION_NOTHING)
                        break;
                }
            }
            else if (tile->terrainFlags & TERRAIN_TYPE_STAIRS) {
                SetLeaderActionToNothing(TRUE);
                
                // DEV: Auto-proceed on stairs during auto-navigate
                if (IsAutoExploreActive()) {
                    // Automatically choose "Go Down" option (ACTION_STAIRS)
                    ActionContainer *action = &GetEntInfo(GetLeader())->action;
                    SetMonsterActionFields(action, ACTION_STAIRS);
                    action->actionParameters[0].actionUseIndex = 0;
                    action->actionParameters[0].itemPos.x = 0;
                    action->actionParameters[0].itemPos.y = 0;
                    action->actionParameters[1].actionUseIndex = 0;
                    action->actionParameters[1].itemPos.x = 0;
                    action->actionParameters[1].itemPos.y = 0;
                } else {
                    ShowDungeonStairsMenu(GetLeader());
                }
                
                if (GetLeaderActionId() != ACTION_NOTHING)
                    break;
            }
            else {
                SubstitutePlaceholderStringTags(gFormatBuffer_Monsters[0], GetLeader(), 0);
                DisplayDungeonMessage(0, gUnknown_80FDE18, 1);
            }

            r10 = -1;
        }
        else if (chosenOption == MAIN_MENU_OTHERS) {
            ShowDungeonOthersMenu();
            if (gDungeon->unk4)
                break;
            if (GetLeaderActionId() != ACTION_NOTHING)
                break;
            r10 = -1;
        }

        // B button
        if (chosenOption < 0)
            break;

        if (var_24 == 0) {
            ResetRepeatTimers();
            ResetUnusedInputStruct();
        }
        else {
            TryPointCameraToMonster(GetLeader(), 0);
            break;
        }
    }

    sub_803EAF0(0, NULL);
    ResetRepeatTimers();
    ResetUnusedInputStruct();
}

static void PrintOnMainMenu(bool8 printAll)
{
    s32 i, x, y, yLoop;

    gDungeonMenu.menuIndex = 0;
    gDungeonMenu.currPageEntries = 5;
    gDungeonMenu.entriesPerPage = 5;
    gDungeonMenu.currPage = 0;
    gDungeonMenu.unk4 = 0;
    gDungeonMenu.firstEntryY = 2;
    gDungeonMenu.leftRightArrowsPos.x = 0;
    gDungeonMenu.leftRightArrowsPos.y = 0;
    gDungeonMenu.windowId = 0;
    gDungeonMenu.unk14.x = 0;
    ResetTouchScreenMenuInput(&gDungeonMenu.touchScreen);
    sub_80137B0(&gDungeonMenu, 0x38);
    if (printAll) {
        sub_803EAF0(7, NULL);
    }
    else {
        sub_803EAF0(6, NULL);
    }

    sub_80073B8(0);
    if (ShouldMonsterRunAwayAndShowEffect(GetLeader(), TRUE)) {
        gUnknown_202749A[1] = 2;
        gUnknown_202749A[2] = 2;
        gUnknown_202749A[3] = 2;
        gUnknown_202749A[4] = 7;
        gUnknown_202749A[5] = 2;
    }
    else
    {
        gUnknown_202749A[1] = 7;
        gUnknown_202749A[2] = 7;
        gUnknown_202749A[3] = 7;
        gUnknown_202749A[4] = 7;
        gUnknown_202749A[5] = 7;
    }

    y = GetMenuEntryYCoord(&gDungeonMenu, 0);
    PrintStringOnWindow(8, y, gFieldMenuMovesPtr, 0, 0);

    y = GetMenuEntryYCoord(&gDungeonMenu, 1);
    PrintStringOnWindow(8, y, gFieldMenuItemsPtr, 0, 0);

    y = GetMenuEntryYCoord(&gDungeonMenu, 2);
    PrintStringOnWindow(8, y, gFieldMenuTeamPtr, 0, 0);

    y = GetMenuEntryYCoord(&gDungeonMenu, 3);
    PrintStringOnWindow(8, y, gFieldMenuOthersPtr, 0, 0);

    y = GetMenuEntryYCoord(&gDungeonMenu, 4);
    PrintStringOnWindow(8, y, gFieldMenuGroundPtr, 0, 0);

    sub_80073E0(0);
    if (printAll) {
        u32 hours, minutes, seconds;
        EntityInfo *leaderInfo = GetEntInfo(GetLeader());
        const u8 *dungeonName = GetCurrentDungeonName();

        x = (136 - GetStringLineWidth(dungeonName)) / 2;
        sub_80073B8(1);
        PrintStringOnWindow(x, 2, dungeonName, 1, 0);
        sub_80073E0(1);
        sub_80073B8(2);
        DeconstructPlayTime(gPlayTimeRef, &hours, &minutes, &seconds);

        gFormatArgs[0] = FixedPointToInt(leaderInfo->belly);
        gFormatArgs[1] = FixedPointToInt(leaderInfo->maxBelly);
        PrintFormattedStringOnWindow(0x73, 0, gUnknown_80F9174, 2, 0);

        gFormatArgs[0] = gTeamInventoryRef->teamMoney;
        PrintFormattedStringOnWindow(0x73, 12, gUnknown_80F9190, 2, 0);

        GetWeatherName(gFormatBuffer_Monsters[0], GetApparentWeather(NULL));
        PrintFormattedStringOnWindow(0x73, 24, gUnknown_80F91A8, 2, 0);

        gFormatArgs[0] = hours;
        gFormatArgs[1] = minutes;
        gFormatArgs[2] = seconds;
        PrintFormattedStringOnWindow(0x73, 36, gUnknown_80F91C8, 2, 0);
        for (yLoop = 0, i = 0; i < MAX_TEAM_MEMBERS; i++) {
            Entity *teamMon = gDungeon->teamPokemon[i];
            if (EntityIsValid(teamMon)) {
                EntityInfo *monInfo = GetEntInfo(teamMon);
                SubstitutePlaceholderStringTags(gFormatBuffer_Monsters[0], teamMon, 0);
                gFormatArgs[0] = monInfo->HP;
                gFormatArgs[1] = monInfo->maxHPStat;
                PrintFormattedStringOnWindow(4, yLoop, gUnknown_80F91E0, 2, 0);
                yLoop += 12;
                if (yLoop >= 12 * MAX_TEAM_MEMBERS)
                    break;
            }
        }
        sub_80073E0(2);
    }
}

bool8 DungeonGiveNameToRecruitedMon(u8 *name)
{
    s32 r4;
    sub_803EAF0(8, name);
    do
    {
        DungeonRunFrameActions(0xE);
        DrawDialogueBoxString();
        r4 = sub_8016080();
    } while (r4 == 0);
    CleanConfirmNameMenu();
    DungeonRunFrameActions(0xE);
    sub_803EAF0(0, NULL);
    if (r4 == 3 && *name != '\0')
        return TRUE;

    return FALSE;
}

// Junction T1 highlighting functions removed - using A* pathfinding instead


