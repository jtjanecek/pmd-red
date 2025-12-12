#ifndef INCLUDE_FRIEND_LIST_MENU_H
#define INCLUDE_FRIEND_LIST_MENU_H

bool8 CreateFriendListMenu(s32 param_1);
u32 sub_8025354(void);
u8 sub_802540C(void);
void CleanFriendListMenu(void);
bool8 FriendListMenu_DebugIsMovesState(void);
bool8 FriendListMenu_DebugIsActive(void);
s32 FriendListMenu_DebugGetState(void);
s16 FriendListMenu_DebugGetSpeciesSlot(void);

#endif
