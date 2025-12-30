#ifndef GUARD_DUNGEON_MAIN_H
#define GUARD_DUNGEON_MAIN_H

#include "structs/dungeon_entity.h"

struct UnkMenuBitsStruct {
    u8 a0_8;
    u8 a0_16;
    u8 a0_24;
    u8 a0_32;
};

void DungeonHandlePlayerInput(void);
void sub_805E804(void);
void CheckLeaderTile(void);
void sub_805EFB4(Entity *a0, bool8 a1);
void sub_805F02C(void);
ActionContainer *GetLeaderActionContainer(void);
u16 GetLeaderActionId(void);
bool8 DungeonGiveNameToRecruitedMon(u8 *name);
void CheckTileDebugNotification(Entity *leader);
void InitializeJunctionT1Tiles(void);

// Auto-explore functions
bool8 IsAutoExploreActive(void);
void SetAutoExploreActive(bool8 active);
bool8 ShouldExitAutoExploreOnInput(void);
// Auto leader swap
bool8 IsAutoLeaderSwapActive(void);
void SetAutoLeaderSwapActive(bool8 active);
void ResetAutoLeaderSwapChain(void);
void QueueAutoLeaderSwapAfterAction(Entity *leader, u16 action);
void ApplyPendingAutoLeaderSwap(void);
void ApplyAutoLeaderSwapReturn(void);
bool8 AutoLeaderSwapHasActedIndex(s32 index);

#endif // GUARD_DUNGEON_MAIN_H
