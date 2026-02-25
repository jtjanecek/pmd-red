#include "global.h"
#include "globaldata.h"
#include "bg_palette_buffer.h"
#include "constants/emotions.h"
#include "constants/input.h"
#include "constants/type.h"
#include "music_util.h"
#include "input.h"
#include "memory.h"
#include "menu_input.h"
#include "personality_test2.h"
#include "pokemon.h"
#include "random.h"
#include "string_format.h"
#include "text_1.h"
#include "text_2.h"
#include "constants/monster.h"

EWRAM_INIT struct PersonalityStruct_203B404 *gUnknown_203B404 = {NULL};
static EWRAM_INIT bool8 sUseVanillaPartnerPool = {FALSE};
static EWRAM_INIT bool8 sIncludeRandomChoice = {TRUE};

#include "data/personality_test2.h"

static s32 GetValidPartners(void);
static s32 GetGlobalIndex(s32 localIndex);
static s32 GetPartnerIndexFromGlobalIndex(s32 globalIndex);
static void nullsub_135(void);
static void PersonalityTest_DisplayPartnerSprite(void);
static void RedrawPartnerSelectionMenu(void);
static s16 ChooseRandomPartner(void);
static bool8 UpdateSelectionMenuCursor(MenuInputStruct *menuInput);
static void ScrollSelectionMenu(MenuInputStruct *menuInput, bool8 moveRight);

static void sub_803CEAC(void);
static void sub_803CECC(void);
static void CreateSelectionMenuInternal(s16 starterID, bool8 selectingStarter, bool8 useVanillaPartnerPool);

static void CreateSelectionMenuInternal(s16 starterID, bool8 selectingStarter, bool8 useVanillaPartnerPool)
{
    s32 starterID_s32;
    starterID_s32 = starterID; // force an asr shift.. does lsr without it

    sub_803CEAC();
    gUnknown_203B404->selectingStarter = selectingStarter;
    sUseVanillaPartnerPool = useVanillaPartnerPool;
    sIncludeRandomChoice = selectingStarter || !useVanillaPartnerPool;
    gUnknown_203B404->StarterID = starterID_s32;
    gUnknown_203B404->s18.m.menuWinId = 0;
    gUnknown_203B404->s18.m.menuWindow = &gUnknown_203B404->s18.m.windows.id[0];

    gUnknown_203B404->s18.m.windows.id[0] = gUnknown_80F4290;
    gUnknown_203B404->s18.m.windows.id[1] = gUnknown_80F4278;
    gUnknown_203B404->s18.m.windows.id[2] = gUnknown_80F4278;
    gUnknown_203B404->s18.m.windows.id[3] = gUnknown_80F4278;

    gUnknown_203B404->s18.m.menuWindow->header = &gUnknown_203B404->s18.header;

    gUnknown_203B404->s18.header.count = 1;
    gUnknown_203B404->s18.header.currId = 0;
    gUnknown_203B404->s18.header.width = 6;
    gUnknown_203B404->s18.header.f3 = 0;
    ResetUnusedInputStruct();
    ShowWindows(&gUnknown_203B404->s18.m.windows, TRUE, TRUE);
    CreateMenuOnWindow(&gUnknown_203B404->s18.m.input, GetValidPartners(), 10, gUnknown_203B404->s18.m.menuWinId);
    RedrawPartnerSelectionMenu();
    PersonalityTest_DisplayPartnerSprite();
}

void CreatePartnerSelectionMenu(s16 starterID)
{
    CreateSelectionMenuInternal(starterID, FALSE, FALSE);
}

void CreateVanillaPartnerSelectionMenu(s16 starterID)
{
    CreateSelectionMenuInternal(starterID, FALSE, TRUE);
}

void CreateStarterSelectionMenu(void)
{
    CreateSelectionMenuInternal(MONSTER_NONE, TRUE, FALSE);
}

u16 HandlePartnerSelectionInput(void)
{
    s32 previousIndex;
    s32 keyPress;
    bool8 pageChanged;

    previousIndex = gUnknown_203B404->s18.m.input.menuIndex;
    gUnknown_203B404->unk16 = 0;

    pageChanged = UpdateSelectionMenuCursor(&gUnknown_203B404->s18.m.input);

    if (pageChanged)
        RedrawPartnerSelectionMenu();

    if (pageChanged || previousIndex != gUnknown_203B404->s18.m.input.menuIndex)
        PersonalityTest_DisplayPartnerSprite();

    keyPress = GetKeyPress(&gUnknown_203B404->s18.m.input);
    if (keyPress == INPUT_A_BUTTON) {
        s32 globalIndex = GetGlobalIndex(gUnknown_203B404->s18.m.input.menuIndex);
        s32 partnerIndex;

        PlayMenuSoundEffect(MENU_SFX_ACCEPT);

        if (sIncludeRandomChoice && globalIndex == 0)
            return ChooseRandomPartner();

        partnerIndex = GetPartnerIndexFromGlobalIndex(globalIndex);
        return gUnknown_203B404->PartnerArray[partnerIndex];
    }

    if (gUnknown_203B404->unk16 != 0) {
        return -2;
    }
    return -1;
}

UNUSED static void sub_803CE34(bool8 cursorSprite)
{
    gUnknown_203B404->s18.m.input.totalEntriesCount = GetValidPartners();
    MenuUpdatePagesData(&gUnknown_203B404->s18.m.input);
    RedrawPartnerSelectionMenu();
    PersonalityTest_DisplayPartnerSprite();

    if (cursorSprite)
        AddMenuCursorSprite(&gUnknown_203B404->s18.m.input);
}

void sub_803CE6C(void)
{
    gUnknown_203B404->s18.m.windows.id[gUnknown_203B404->s18.m.menuWinId] = gUnknown_80F4278;
    ResetUnusedInputStruct();
    ShowWindows(&gUnknown_203B404->s18.m.windows, TRUE, TRUE);
    gUnknown_203B404->selectingStarter = FALSE;
    sUseVanillaPartnerPool = FALSE;
    sIncludeRandomChoice = TRUE;
    sub_803CECC();
}

static void sub_803CEAC(void)
{
    gUnknown_203B404 = MemoryAlloc(sizeof(struct PersonalityStruct_203B404), MEMALLOC_GROUP_8);
    nullsub_135();
}

static void nullsub_135(void)
{
}

static void sub_803CECC(void)
{
    if (gUnknown_203B404 != NULL) {
        nullsub_135();
        MemoryFree(gUnknown_203B404);
        gUnknown_203B404 = NULL;
    }
}

static void RedrawPartnerSelectionMenu(void)
{
    u32 yCoord;
    const u8 *monName;
    s32 monCounter;

    UPDATE_MENU_WINDOW_HEIGHT(gUnknown_203B404->s18.m);

    CallPrepareTextbox_8008C54(gUnknown_203B404->s18.m.menuWinId);
    sub_80073B8(gUnknown_203B404->s18.m.menuWinId);
    if (gUnknown_203B404->selectingStarter)
        PrintStringOnWindow(12, 0, gStarterSelectionHeaderText, gUnknown_203B404->s18.m.menuWinId, 0);
    else
        PrintStringOnWindow(12, 0, gPartnerSelectionHeaderText, gUnknown_203B404->s18.m.menuWinId, 0);

    monCounter = 0;
    while (monCounter < gUnknown_203B404->s18.m.input.currPageEntries) {
        s32 globalIndex;
        s32 partnerIndex;

        yCoord = GetMenuEntryYCoord(&gUnknown_203B404->s18.m.input, monCounter);
        globalIndex = GetGlobalIndex(monCounter);

        if (sIncludeRandomChoice && globalIndex == 0) {
            PrintStringOnWindow(8, yCoord, gMenuRandomSelectionText, gUnknown_203B404->s18.m.menuWinId, 0);
        }
        else {
            partnerIndex = GetPartnerIndexFromGlobalIndex(globalIndex);
            monName = GetMonSpecies(gUnknown_203B404->PartnerArray[partnerIndex]);
            PrintStringOnWindow(8, yCoord, monName, gUnknown_203B404->s18.m.menuWinId, 0);
        }
        monCounter++;
    }
    sub_80073E0(gUnknown_203B404->s18.m.menuWinId);
    gUnknown_203B404->unk16 = 1;
}

static void PersonalityTest_DisplayPartnerSprite(void)
{
    gUnknown_203B404->unk16 = 1;
}

static s32 GetValidPartners(void)
{
    s32 PlayerType[2];
    s32 currentPartnerTypes[2];
    s32 CurrentPartnerID;
    s32 i;
    s32 ValidPartnerCounter;

    if (gUnknown_203B404->selectingStarter) {
        for (i = 0; i < NUM_PARTNERS; i++)
            gUnknown_203B404->PartnerArray[i] = gPartners[i];
        return NUM_PARTNERS + sIncludeRandomChoice;
    }

    if (sUseVanillaPartnerPool) {
        PlayerType[0] = GetPokemonType(gUnknown_203B404->StarterID, 0);
        PlayerType[1] = GetPokemonType(gUnknown_203B404->StarterID, 1);

        ValidPartnerCounter = 0;
        for (i = 0; i < ARRAY_COUNT(gVanillaPartners); i++) {
            CurrentPartnerID = gVanillaPartners[i];
            currentPartnerTypes[0] = GetPokemonType(CurrentPartnerID, 0);
            currentPartnerTypes[1] = GetPokemonType(CurrentPartnerID, 1);

            if ((currentPartnerTypes[0] == TYPE_NONE || (currentPartnerTypes[0] != PlayerType[0] && currentPartnerTypes[0] != PlayerType[1]))
            && ((currentPartnerTypes[1] == TYPE_NONE || (currentPartnerTypes[1] != PlayerType[0] && currentPartnerTypes[1] != PlayerType[1])))) {
                gUnknown_203B404->PartnerArray[ValidPartnerCounter] = CurrentPartnerID;
                ValidPartnerCounter++;
            }
        }

        return ValidPartnerCounter + sIncludeRandomChoice;
    }

    ValidPartnerCounter = 0;

    for (i = 0; i < NUM_PARTNERS; i++) {
        gUnknown_203B404->PartnerArray[ValidPartnerCounter] = gPartners[i];
        ValidPartnerCounter++;
    }

    return ValidPartnerCounter + sIncludeRandomChoice;
}

static s32 GetGlobalIndex(s32 localIndex)
{
    MenuInputStruct *input = &gUnknown_203B404->s18.m.input;
    return input->currPage * input->entriesPerPage + localIndex;
}

static s32 GetPartnerIndexFromGlobalIndex(s32 globalIndex)
{
    return globalIndex - sIncludeRandomChoice;
}

static s16 ChooseRandomPartner(void)
{
    s16 selection;
    s32 availableCount = gUnknown_203B404->s18.m.input.totalEntriesCount - sIncludeRandomChoice;

    if (availableCount <= 0)
        return MONSTER_NONE;

    if (!gUnknown_203B404->selectingStarter && gUnknown_203B404->StarterID != MONSTER_NONE && availableCount > 1) {
        s32 attempts;

        for (attempts = 0; attempts < 256; attempts++) {
            selection = gUnknown_203B404->PartnerArray[RandInt(availableCount)];
            if (selection != gUnknown_203B404->StarterID)
                return selection;
        }

        for (attempts = 0; attempts < availableCount; attempts++) {
            selection = gUnknown_203B404->PartnerArray[attempts];
            if (selection != gUnknown_203B404->StarterID)
                return selection;
        }
    }

    selection = gUnknown_203B404->PartnerArray[RandInt(availableCount)];
    return selection;
}

static bool8 UpdateSelectionMenuCursor(MenuInputStruct *menuInput)
{
    s32 previousPage = menuInput->currPage;
    s32 previousIndex = menuInput->menuIndex;
    bool8 movedWithinPage = FALSE;
    s32 key;

    AddMenuCursorSprite(menuInput);

    key = GetKeyPress(menuInput);
    switch (key) {
        case INPUT_DPAD_UP:
            menuInput->unk24 = 0;
            if (menuInput->currPageEntries > 0) {
                if (menuInput->menuIndex <= 0)
                    menuInput->menuIndex = menuInput->currPageEntries - 1;
                else
                    menuInput->menuIndex--;
                movedWithinPage = TRUE;
            }
            break;
        case INPUT_DPAD_DOWN:
            menuInput->unk24 = 0;
            if (menuInput->currPageEntries > 0) {
                if (menuInput->menuIndex >= menuInput->currPageEntries - 1)
                    menuInput->menuIndex = 0;
                else
                    menuInput->menuIndex++;
                movedWithinPage = TRUE;
            }
            break;
        case INPUT_DPAD_LEFT:
            menuInput->unk24 = 0;
            ScrollSelectionMenu(menuInput, FALSE);
            break;
        case INPUT_DPAD_RIGHT:
            menuInput->unk24 = 0;
            ScrollSelectionMenu(menuInput, TRUE);
            break;
    }

    if (menuInput->currPage != previousPage) {
        PlayMenuSoundEffect(4);
        return TRUE;
    }

    if (movedWithinPage && menuInput->menuIndex != previousIndex)
        PlayMenuSoundEffect(3);

    return FALSE;
}

static void ScrollSelectionMenu(MenuInputStruct *menuInput, bool8 moveRight)
{
    if (menuInput->pagesCount <= 0)
        return;

    if (moveRight) {
        if (menuInput->currPage < menuInput->pagesCount - 1)
            menuInput->currPage++;
        else
            menuInput->currPage = 0;
    }
    else {
        if (menuInput->currPage <= 0)
            menuInput->currPage = menuInput->pagesCount - 1;
        else
            menuInput->currPage--;
    }

    MenuUpdatePagesData(menuInput);
}
