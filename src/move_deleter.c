#include "global.h"
#include "globaldata.h"
#include "move_deleter.h"

#include "gulpin_shop.h"
#include "memory.h"
#include "moves.h"
#include "pokemon.h"
#include "string_format.h"
#include "text_1.h"
#include "constants/common_strings_gulpin_shop.h"

enum MoveDeleterState {
    MOVE_DELETER_STATE_SELECT_MON,
    MOVE_DELETER_STATE_GULPIN_SHOP,
    MOVE_DELETER_STATE_APPLY_CHANGES,
    MOVE_DELETER_STATE_FINISHED,
    MOVE_DELETER_STATE_CANCELLED,
};

typedef struct MoveDeleterWork {
    s32 state;
    s32 teamIndices[MAX_TEAM_MEMBERS];
    s32 teamCount;
    s32 defaultAction;
    s32 chosenPokemon;
    Pokemon *pokeStruct;
    MenuItem menuItems[MAX_TEAM_MEMBERS + 1];
    Move moves[MAX_MON_MOVES * 2];
} MoveDeleterWork;

static MoveDeleterWork *sMoveDeleterWork = NULL;

static const u8 sMoveDeletePrompt[] = _(" Whose move should I erase?");
static const u8 sMoveDeletedText[] = _(" The move has been erased.");

static void MoveDeleterSetState(s32 newState);
static void MoveDeleterBuildMenu(void);

bool8 CreateXatuMoveDeleter(void)
{
    sMoveDeleterWork = MemoryAlloc(sizeof(MoveDeleterWork), 8);
    if (sMoveDeleterWork == NULL) {
        return FALSE;
    }

    MemoryFill8(sMoveDeleterWork, 0, sizeof(MoveDeleterWork));
    sMoveDeleterWork->teamCount = sub_808D580(sMoveDeleterWork->teamIndices);
    if (sMoveDeleterWork->teamCount <= 0) {
        DestroyXatuMoveDeleter();
        return FALSE;
    }

    sMoveDeleterWork->defaultAction = 4; // First entry in the menu
    MoveDeleterSetState(MOVE_DELETER_STATE_SELECT_MON);
    return TRUE;
}

u32 XatuMoveDeleterCallback(void)
{
    s32 menuAction;

    switch (sMoveDeleterWork->state) {
        case MOVE_DELETER_STATE_SELECT_MON:
            if (sub_80144A4(&menuAction)) {
                return 0;
            }

            if (menuAction == 1) {
                MoveDeleterSetState(MOVE_DELETER_STATE_CANCELLED);
                return 0;
            }

            sMoveDeleterWork->chosenPokemon = sMoveDeleterWork->teamIndices[menuAction - 4];
            sMoveDeleterWork->pokeStruct = &gRecruitedPokemonRef->pokemon[sMoveDeleterWork->chosenPokemon];
            unk_CopyMoves4To8(sMoveDeleterWork->moves, sMoveDeleterWork->pokeStruct->moves);
            CreateGulpinShop(GULPIN_SHOP_MODE_UNK2, sMoveDeleterWork->chosenPokemon, sMoveDeleterWork->moves);
            MoveDeleterSetState(MOVE_DELETER_STATE_GULPIN_SHOP);
            return 0;
        case MOVE_DELETER_STATE_GULPIN_SHOP:
            switch (sub_801E8C0()) {
                case 3:
                    DestroyGulpinShop();
                    MoveDeleterSetState(MOVE_DELETER_STATE_APPLY_CHANGES);
                    break;
                case 2:
                    DestroyGulpinShop();
                    MoveDeleterSetState(MOVE_DELETER_STATE_CANCELLED);
                    break;
            }
            return 0;
        case MOVE_DELETER_STATE_APPLY_CHANGES:
            sub_8094060(sMoveDeleterWork->moves, sMoveDeleterWork->pokeStruct->moves);
            CreateDialogueBoxAndPortrait(sMoveDeletedText, 0, 0, 0x121);
            MoveDeleterSetState(MOVE_DELETER_STATE_FINISHED);
            return 0;
        case MOVE_DELETER_STATE_FINISHED:
            if (sub_80144A4(&menuAction) == 0) {
                return 3;
            }
            return 0;
        case MOVE_DELETER_STATE_CANCELLED:
            return 2;
        default:
            return 3;
    }
}

void DestroyXatuMoveDeleter(void)
{
    if (sMoveDeleterWork != NULL) {
        MemoryFree(sMoveDeleterWork);
        sMoveDeleterWork = NULL;
    }
}

static void MoveDeleterSetState(s32 newState)
{
    sMoveDeleterWork->state = newState;

    switch (newState) {
        case MOVE_DELETER_STATE_SELECT_MON:
            MoveDeleterBuildMenu();
            CreateMenuDialogueBoxAndPortrait(sMoveDeletePrompt, 0, sMoveDeleterWork->defaultAction,
                sMoveDeleterWork->menuItems, 0, 4, 0, NULL, 32);
            break;
        default:
            break;
    }
}

static void MoveDeleterBuildMenu(void)
{
    s32 i;
    u8 *bufferPtr;

    for (i = 0; i < sMoveDeleterWork->teamCount; i++) {
        bufferPtr = gFormatBuffer_Monsters[i];
        PrintPokeNameToBuffer(bufferPtr, &gRecruitedPokemonRef->pokemon[sMoveDeleterWork->teamIndices[i]]);
        sMoveDeleterWork->menuItems[i].text = bufferPtr;
        sMoveDeleterWork->menuItems[i].menuAction = i + 4;
    }

    sMoveDeleterWork->menuItems[i].text = NULL;
    sMoveDeleterWork->menuItems[i].menuAction = 1;
}
