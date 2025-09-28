#include "global.h"
#include "globaldata.h"
#include "constants/emotions.h"
#include "bg_palette_buffer.h"
#include "code_801602C.h"
#include "random_mersenne_twister.h"
#include "code_8099360.h"
#include "game_options.h"
#include "input.h"
#include "main_loops.h"
#include "memory.h"
#include "menu_input.h"
#include "naming_screen.h"
#include "personality_test1.h"
#include "personality_test2.h"
#include "random.h"
#include "save.h"
#include "string_format.h"
#include "text_1.h"
#include "text_2.h"
#include "text_util.h"

enum
{
    PERSONALITY_SEED_PROMPT,
    PERSONALITY_SEED_HANDLE_SELECTION,
    PERSONALITY_SEED_CUSTOM_MESSAGE,
    PERSONALITY_SEED_BEGIN_INPUT,
    PERSONALITY_SEED_CUSTOM_INPUT,
    PERSONALITY_PLAYER_GENDER,
    PERSONALITY_ADVANCE_TO_STARTER_SELECTION,
    PERSONALITY_PLAYER_STARTER_SELECTION,
    PERSONALITY_STARTER_REVEAL,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_1,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_2,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_3,
    PERSONALITY_PARTNER_SELECTION,
    PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_1,
    PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_2,
    PERSONALITY_PARTNER_NICKNAME,
    PERSONALITY_END_INTRO,
    PERSONALITY_ADVANCE_TO_TEST_END,
    PERSONALITY_TEST_END,
};

static EWRAM_INIT PersonalityTestTracker *sPersonalityTestTracker = {NULL};

#define INT32_MAX_VALUE 2147483647
#define INT32_MIN_VALUE (-2147483647 - 1)
#define SEED_MENU_RANDOM 0
#define SEED_MENU_CUSTOM 1
#define NAMING_SCREEN_NUMERIC 6

#include "data/personality_test1.h"

static void AdvanceToPartnerNicknameScreen(void);
static void AdvanceToPartnerSelection(void);
static void AdvanceToPickPartnerPrompt(void);
static void AdvanceToTestEnd(void);
static void CallCreatePartnerSelectionMenu(void);
static void InitializeTestStats(void);
static void NicknamePartner(void);
static void PersonalityTest_DisplayStarterSprite(void);
static void PrintEndIntroText(void);
static void PromptForPartnerNickname(void);
static void PromptPickPartner(void);
static void RevealStarter(void);
static void SetPlayerGender(void);
static void PromptSeedSelection(void);
static void HandleSeedSelection(void);
static void WaitForSeedPromptAcknowledge(void);
static void StartCustomSeedInput(void);
static void HandleCustomSeedInput(void);
static void StartGenderSelection(void);
static void AdvanceToStarterSelection(void);
static void HandleStarterSelection(void);
static s32 GenerateRandomSeed(void);
static bool32 TryStoreCustomSeed(void);
static bool32 ParseSeedString(const u8 *text, s32 *seedOut);
static void CleanupSeedNamingScreen(void);

bool8 CreateTestTracker(void)
{
    sPersonalityTestTracker = MemoryAlloc(sizeof(PersonalityTestTracker), 8);
    ResetTouchScreenMenuInput(&sPersonalityTestTracker->input);
    InitializeTestStats();
    // sPersonalityTestTracker->unk4.StarterID = MONSTER_BULBASAUR;
    // sPersonalityTestTracker->TestState = PERSONALITY_STARTER_REVEAL;
    sub_8099690(1);
    return TRUE;
}

static void InitializeTestStats(void)
{
    sub_8001024(&sPersonalityTestTracker->unk4);
    sPersonalityTestTracker->FrameCounter = 0;
    sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
    sPersonalityTestTracker->playerGender = 0;
    sPersonalityTestTracker->rngSeed = 0;
    sPersonalityTestTracker->seedChosen = FALSE;
    sPersonalityTestTracker->usingCustomSeed = FALSE;
    MemoryFill8(sPersonalityTestTracker->seedBuffer, 0, PERSONALITY_TEST_SEED_BUFFER_SIZE);
}

u32 HandleTestTrackerState(void)
{

    sPersonalityTestTracker->FrameCounter++;

    switch (sPersonalityTestTracker->TestState) {
        case PERSONALITY_SEED_PROMPT:
            PromptSeedSelection();
            break;
        case PERSONALITY_SEED_HANDLE_SELECTION:
            HandleSeedSelection();
            break;
        case PERSONALITY_SEED_CUSTOM_MESSAGE:
            WaitForSeedPromptAcknowledge();
            break;
        case PERSONALITY_SEED_BEGIN_INPUT:
            StartCustomSeedInput();
            break;
        case PERSONALITY_SEED_CUSTOM_INPUT:
            HandleCustomSeedInput();
            break;
        case PERSONALITY_PLAYER_GENDER:
            SetPlayerGender();
            break;
        case PERSONALITY_ADVANCE_TO_STARTER_SELECTION:
            AdvanceToStarterSelection();
            break;
        case PERSONALITY_PLAYER_STARTER_SELECTION:
            HandleStarterSelection();
            break;
        case PERSONALITY_STARTER_REVEAL:
            RevealStarter();
            break;
        case PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_1:
            AdvanceToPickPartnerPrompt();
            break;
        case PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_2:
            PromptPickPartner();
            break;
        case PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_3:
            AdvanceToPartnerSelection();
            break;
        case PERSONALITY_PARTNER_SELECTION:
            CallCreatePartnerSelectionMenu();
            break;
        case PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_1:
            PromptForPartnerNickname();
            break;
        case PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_2:
            AdvanceToPartnerNicknameScreen();
            break;
        case PERSONALITY_PARTNER_NICKNAME:
            NicknamePartner();
            break;
        case PERSONALITY_END_INTRO:
            PrintEndIntroText();
            break;
        case PERSONALITY_ADVANCE_TO_TEST_END:
            AdvanceToTestEnd();
            break;
        case PERSONALITY_TEST_END:
            if (!sPersonalityTestTracker->seedChosen) {
                sPersonalityTestTracker->rngSeed = GenerateRandomSeed();
                sPersonalityTestTracker->seedChosen = TRUE;
            }
            sub_8011C40(sPersonalityTestTracker->rngSeed);
            return 3;
        default:
            break;
    }
    return 0;
}

void DeleteTestTracker(void)
{
    sub_8001044(&sPersonalityTestTracker->unk4);
    MemoryFree(sPersonalityTestTracker);
    sPersonalityTestTracker = NULL;
}

static void PromptSeedSelection(void)
{
    CreateMenuDialogueBoxAndPortrait(gSeedModePrompt, 0, 0, gSeedModeMenu, 0, 3, 0, 0, 0x101);
    sPersonalityTestTracker->TestState = PERSONALITY_SEED_HANDLE_SELECTION;
}

static void HandleSeedSelection(void)
{
    s32 selection;

    if (sub_80144A4(&selection))
        return;

    switch (selection) {
        case SEED_MENU_RANDOM:
            sPersonalityTestTracker->rngSeed = GenerateRandomSeed();
            sPersonalityTestTracker->seedChosen = TRUE;
            sPersonalityTestTracker->usingCustomSeed = FALSE;
            StartGenderSelection();
            break;
        case SEED_MENU_CUSTOM:
            sPersonalityTestTracker->usingCustomSeed = TRUE;
            CreateDialogueBoxAndPortrait(gSeedCustomPrompt, 0, 0, 0x101);
            sPersonalityTestTracker->TestState = PERSONALITY_SEED_CUSTOM_MESSAGE;
            break;
        default:
            sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
            break;
    }
}

static void WaitForSeedPromptAcknowledge(void)
{
    s32 unused;

    if (sub_80144A4(&unused))
        return;

    sPersonalityTestTracker->TestState = PERSONALITY_SEED_BEGIN_INPUT;
}

static void StartCustomSeedInput(void)
{
    if (!sPersonalityTestTracker->usingCustomSeed) {
        sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
        return;
    }

    MemoryFill8(sPersonalityTestTracker->seedBuffer, 0, PERSONALITY_TEST_SEED_BUFFER_SIZE);
    NamingScreen_Init(NAMING_SCREEN_NUMERIC, sPersonalityTestTracker->seedBuffer);
    sPersonalityTestTracker->TestState = PERSONALITY_SEED_CUSTOM_INPUT;
}

static void HandleCustomSeedInput(void)
{
    u32 result = NamingScreen_HandleInput();

    switch (result) {
        case 0:
            break;
        case 2:
            CleanupSeedNamingScreen();
            MemoryFill8(sPersonalityTestTracker->seedBuffer, 0, PERSONALITY_TEST_SEED_BUFFER_SIZE);
            sPersonalityTestTracker->usingCustomSeed = FALSE;
            sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
            break;
        case 3:
            if (TryStoreCustomSeed()) {
                CleanupSeedNamingScreen();
                StartGenderSelection();
            }
            break;
    }
}

static void CleanupSeedNamingScreen(void)
{
    NamingScreen_Free();
    ResetUnusedInputStruct();
    ShowWindows(NULL, TRUE, TRUE);
}

static void StartGenderSelection(void)
{
    CreateMenuDialogueBoxAndPortrait(sGender0, 0, 0, gGenderMenu, 0, 3, 0, 0, 0x101);
    sPersonalityTestTracker->TestState = PERSONALITY_PLAYER_GENDER;
}

static void AdvanceToStarterSelection(void)
{
    s32 temp;

    if (sub_80144A4(&temp) != 0)
        return;

    CreateStarterSelectionMenu();
    sPersonalityTestTracker->TestState = PERSONALITY_PLAYER_STARTER_SELECTION;
}

static void HandleStarterSelection(void)
{
    u16 chosen;

    chosen = HandlePartnerSelectionInput();

    if (chosen == 0xFFFF || chosen == 0xFFFE)
        return;

    sub_803CE6C();
    sPersonalityTestTracker->unk4.StarterID = chosen;
    CopyMonsterNameToBuffer(gFormatBuffer_Monsters[0], chosen);
    sPersonalityTestTracker->TestState = PERSONALITY_STARTER_REVEAL;
}

static s32 GenerateRandomSeed(void)
{
    s32 seed;

    do {
        seed = Rand32Bit();
    } while (seed == -1);

    return seed;
}

static bool32 TryStoreCustomSeed(void)
{
    s32 seed;

    if (!ParseSeedString(sPersonalityTestTracker->seedBuffer, &seed))
        return FALSE;

    sPersonalityTestTracker->rngSeed = seed;
    sPersonalityTestTracker->seedChosen = TRUE;
    sPersonalityTestTracker->usingCustomSeed = TRUE;
    return TRUE;
}

static bool32 ParseSeedString(const u8 *text, s32 *seedOut)
{
    const u8 *ptr = text;
    bool8 negative = FALSE;
    s64 value = 0;
    s32 digitCount = 0;

    while (*ptr == ' ')
        ptr++;

    if (*ptr == '\0')
        return FALSE;

    if (*ptr == '-') {
        negative = TRUE;
        ptr++;
        if (*ptr == '\0')
            return FALSE;
    }

    while (*ptr != '\0') {
        if (*ptr < '0' || *ptr > '9')
            return FALSE;

        value = value * 10 + (*ptr - '0');
        digitCount++;
        if (!negative && value > INT32_MAX_VALUE)
            return FALSE;
        if (negative && value > (s64)INT32_MAX_VALUE + 1)
            return FALSE;
        ptr++;
    }

    if ((!negative && digitCount > 10) || (negative && digitCount > 10))
        return FALSE;

    if (negative)
        value = -value;

    if (value < INT32_MIN_VALUE || value > INT32_MAX_VALUE)
        return FALSE;

    if (value == -1)
        return FALSE;

    *seedOut = (s32)value;
    return TRUE;
}


static void SetPlayerGender(void)
{
    s32 gender;

    if (sub_80144A4(&gender) != 0)
        return;

    if (gender == MALE) {
        sPersonalityTestTracker->playerGender = MALE;
        gGameOptionsRef->playerGender = MALE;
    }
    else {
        sPersonalityTestTracker->playerGender = FEMALE;
        gGameOptionsRef->playerGender = FEMALE;
    }

    sub_8099690(0);
    CreateDialogueBoxAndPortrait(gStarterPrompt, 0, 0, 0x301);
    sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_STARTER_SELECTION;
}

static void RevealStarter(void)
{
    s32 temp;

    if (sub_80144A4(&temp) == 0) {
        CreateDialogueBoxAndPortrait(gStarterReveal, 0, 0, 0x101);
        PersonalityTest_DisplayStarterSprite();
        sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_1;
    }
}

static void AdvanceToPickPartnerPrompt(void)
{
    s32 temp;

    if (sub_80144A4(&temp) == 0)
        sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_2;
}

static void PromptPickPartner(void)
{
    CreateDialogueBoxAndPortrait(gPartnerPrompt, 0, 0, 0x301);
    sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_3;
}

static void AdvanceToPartnerSelection(void)
{
    s32 temp;

    if (sub_80144A4(&temp) == 0)
        sPersonalityTestTracker->TestState = PERSONALITY_PARTNER_SELECTION;
}

static void CallCreatePartnerSelectionMenu(void)
{
    CreatePartnerSelectionMenu(sPersonalityTestTracker->unk4.StarterID);
    sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_1;
}

static void PromptForPartnerNickname(void)
{
    u16 selectedPartner;

    selectedPartner = HandlePartnerSelectionInput();

    if (selectedPartner != 0xFFFF) {
        if (selectedPartner != 0xFFFE) {
            sub_803CE6C();
            sPersonalityTestTracker->unk4.PartnerID = selectedPartner;
            CreateDialogueBoxAndPortrait(gPartnerNickPrompt, 0, 0, 0x301);
            sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_2;
        }
    }
}

static void AdvanceToPartnerNicknameScreen(void)
{
    s32 temp;

    if (sub_80144A4(&temp) == 0)
        sPersonalityTestTracker->TestState = PERSONALITY_PARTNER_NICKNAME;
}

static void NicknamePartner(void)
{
    CopyStringtoBuffer(sPersonalityTestTracker->unk4.PartnerNick, GetMonSpecies(sPersonalityTestTracker->unk4.PartnerID));
    // Skip nickname UI; accept default and continue
    CreateDialogueBoxAndPortrait(gEndIntroText, 0, 0, 0x301);
    sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_TEST_END;
}

static void PrintEndIntroText(void)
{
    if (sub_8016080()) {
        CleanConfirmNameMenu();
        CreateDialogueBoxAndPortrait(gEndIntroText, 0, 0, 0x301);
        sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_TEST_END;
    }
}

static void AdvanceToTestEnd(void)
{
    s32 temp;

    if (sub_80144A4(&temp) == 0)
        sPersonalityTestTracker->TestState = PERSONALITY_TEST_END;
}

static void PersonalityTest_DisplayStarterSprite(void)
{
    s32 starterID;
    struct OpenedFile *faceFile;
    s32 paletteIndex;
    s32 emotionId;
    const u8 *gfx;
    WindowTemplates stackArray;

    starterID = sPersonalityTestTracker->unk4.StarterID;
    RestoreSavedWindows(&stackArray);
    stackArray.id[1] = sUnknown_80F4244;
    ResetUnusedInputStruct();
    ShowWindows(&stackArray, TRUE, FALSE);
    CallPrepareTextbox_8008C54(1);
    sub_80073B8(1);

    faceFile = GetDialogueSpriteDataPtr(starterID);
    gfx = ((PortraitGfx *)(faceFile->data))->sprites[EMOTION_HAPPY].gfx;
    emotionId = EMOTION_HAPPY;
    for (paletteIndex = 0; paletteIndex < 0x10; paletteIndex++) {
        SetBGPaletteBufferColorArray(paletteIndex + 0xE0, &((PortraitGfx *)(faceFile->data))->sprites[emotionId].pal[paletteIndex]);
    }

    DisplayMonPortraitSpriteFlipped(1, gfx, 14);
    CloseFile(faceFile);
    sub_80073E0(1);
}
