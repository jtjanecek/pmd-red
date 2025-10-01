#ifndef GUARD_DUNGEON_AUTO_EXPLORE_H
#define GUARD_DUNGEON_AUTO_EXPLORE_H

#include "structs/dungeon_entity.h"
#include "structs/str_position.h"

// Reset auto-explore state (call when entering a new floor)
void ResetAutoExplore(void);

// Set auto-explore active state
void SetAutoExploreActive(bool8 active);

// Check if auto-explore is currently active
bool8 IsAutoExploreActive(void);

// Check if stairs are visible on the map
bool8 AreStairsVisible(void);

// Get the auto-explore target position
bool8 GetAutoExploreTarget(Entity *leader, DungeonPos *outTarget);

// Get the next movement direction for auto-explore
s32 GetAutoExploreDirection(Entity *leader);

// Function to prevent linker from discarding auto-explore functions
void AutoExploreInit(void);

#endif // GUARD_DUNGEON_AUTO_EXPLORE_H

