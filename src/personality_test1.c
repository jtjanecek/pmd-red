#include "global.h"
#include "globaldata.h"
#include "constants/emotions.h"
#include "constants/monster.h"
#include "bg_palette_buffer.h"
#include "confirm_name_menu.h"
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
#include "rescue_team_info.h"
#include "random.h"
#include "save.h"
#include "string_format.h"
#include "text_1.h"
#include "text_2.h"
#include "text_util.h"
#include "event_flag.h" // SetScriptVarValue, ScenarioCalc
#include "constants/ground_map.h" // MAP_TEAM_BASE_INSIDE
#include "ground_place.h" // GROUND_PLACE_TEAM_BASE_INSIDE
#include "save_write.h" // Prepare/Write/Finish save
#include "ground_main.h" // GroundMainGameEndRequest
#include "gba/syscall.h" // SoftReset, RESET_ALL
#include "constants/script_dungeon_id.h"
#include "constants/event_flag.h" // SCENARIO_MAIN, GROUND_ENTER, GROUND_ENTER_LINK
#include "code_80972F4.h"
// Starter kit/news helpers
#include "items.h"
#include "code_80958E8.h" // sub_80961B4

// Not exposed via a header: seed initial Pokemon News in mailbox
extern void sub_8096488(void);

// Forward declaration for dev mode level up function
extern void sub_8043FD0(void);

// Partner roster used by starter/partner selection menus
extern const s16 gPartners[NUM_PARTNERS];

enum
{
    PERSONALITY_SEED_PROMPT,
    PERSONALITY_SEED_HANDLE_SELECTION,
    PERSONALITY_SEED_CUSTOM_MESSAGE,
    PERSONALITY_SEED_BEGIN_INPUT,
    PERSONALITY_SEED_CUSTOM_INPUT,
    PERSONALITY_SKIP_BASIC_RESCUES_SELECTION,
    PERSONALITY_RECRUIT_ALL_SELECTION,
    PERSONALITY_DIFFICULTY_SELECTION,
    PERSONALITY_PLAYER_GENDER,
    PERSONALITY_ADVANCE_TO_STARTER_SELECTION,
    PERSONALITY_PLAYER_STARTER_SELECTION,
    PERSONALITY_ADVANCE_TO_STARTER_NICKNAME,
    PERSONALITY_STARTER_NICKNAME,
    PERSONALITY_STARTER_REVEAL,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_1,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_2,
    PERSONALITY_ADVANCE_TO_PARTNER_SELECTION_3,
    PERSONALITY_PARTNER_SELECTION,
    PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_1,
    PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_2,
    PERSONALITY_PARTNER_NICKNAME,
    PERSONALITY_TEAM_NAME_PROMPT,
    PERSONALITY_TEAM_NAME_ENTRY,
    PERSONALITY_END_INTRO,
    PERSONALITY_ADVANCE_TO_TEST_END,
    PERSONALITY_TEST_END,
    PERSONALITY_SKIP_BOOTSTRAP_SAVING,
};

static EWRAM_INIT PersonalityTestTracker *sPersonalityTestTracker = {NULL};

#define INT32_MAX_VALUE 2147483647
#define INT32_MIN_VALUE (-2147483647 - 1)
#define SEED_MENU_NORMAL 0
#define SEED_MENU_RANDOM 1
#define SEED_MENU_CUSTOM 2
#define NAMING_SCREEN_NUMERIC 6
#define NAMING_SCREEN_PLAYER 0
#define NAMING_SCREEN_TEAM 1
#define NAMING_SCREEN_PARTNER 3

#include "data/personality_test1.h"

static void AdvanceToPartnerNicknameScreen(void);
static void AdvanceToPartnerSelection(void);
static void AdvanceToPickPartnerPrompt(void);
static void AdvanceToTestEnd(void);
static void CallCreatePartnerSelectionMenu(void);
static void InitializeTestStats(void);
static void StartSkipBasicRescuesSelection(void);
static void HandleSkipBasicRescuesSelection(void);
static void StartRecruitAllSelection(void);
static void HandleRecruitAllSelection(void);
static bool8 ShouldEnableAutoRecruitAllDebug(void);
static void StartDifficultySelection(void);
static void NicknamePartner(void);
static void PromptTeamName(void);
static void HandleTeamNameEntry(void);
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
static void AdvanceToStarterNickname(void);
static void HandleStarterSelection(void);
static void HandleStarterNickname(void);
static s32 GenerateRandomSeed(void);
static bool32 TryStoreCustomSeed(void);
static bool32 ParseSeedString(const u8 *text, s32 *seedOut);
static void CleanupNamingScreen(void);
static void HandleDifficultySelection(void);
UNUSED static void ApplySkipStartMinimal(void);
// Skip-cutscene override is disabled for now; no postgame force.
static void ApplySkipPostgameBootstrap(void);
static s16 DevPickRandomMon(void);
static s16 DevPickRandomPartnerDistinctFrom(s16 starter);

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

static s16 DevPickRandomMon(void)
{
    // Draw from the full starter/partner roster
    return gPartners[RandInt(NUM_PARTNERS)];
}

static s16 DevPickRandomPartnerDistinctFrom(s16 starter)
{
    s16 selection;
    s32 attempts;

    // Try a bunch of random draws that avoid duplicating the starter
    for (attempts = 0; attempts < 256; attempts++) {
        selection = DevPickRandomMon();
        if (selection != starter)
            return selection;
    }

    // Deterministic fallback scan
    for (attempts = 0; attempts < NUM_PARTNERS; attempts++) {
        selection = gPartners[attempts];
        if (selection != starter)
            return selection;
    }

    // Worst case: allow a duplicate so we never return MONSTER_NONE
    return DevPickRandomMon();
}

static void InitializeTestStats(void)
{
    // s32 i; // unused

    ReadTeamBasicInfo(&sPersonalityTestTracker->unk4);
    sPersonalityTestTracker->FrameCounter = 0;
    sPersonalityTestTracker->playerGender = 0;
    sPersonalityTestTracker->rngSeed = 0;
    sPersonalityTestTracker->seedChosen = FALSE;
    sPersonalityTestTracker->usingCustomSeed = FALSE;
    sPersonalityTestTracker->unk4.difficulty = DIFFICULTY_NORMAL;
    sPersonalityTestTracker->unk4.skipBasicRescues = 0; // Default to No
    sPersonalityTestTracker->unk4.recruitAll = RECRUIT_ALL_NORMAL; // Default to No
    SetGameDifficultySetting(DIFFICULTY_NORMAL);
    MemoryFill8(sPersonalityTestTracker->seedBuffer, 0, PERSONALITY_TEST_SEED_BUFFER_SIZE);
    
    // DEV: Skip personality quiz and set dev defaults
    // To enable dev mode, compile with: make DEV=1
    // This will skip the personality quiz and set:
    // - Main: Random
    // - Solo: Yes
    // - Partner: Random (distinct from main when possible)
    // - Recruitment: No Recruitable
    // - Skip basic rescues: Yes
    // - Skip Cutscenes: Yes
    // - Difficulty: Normal
    #ifdef DEV
    sPersonalityTestTracker->TestState = PERSONALITY_TEST_END;
    sPersonalityTestTracker->unk4.StarterID = DevPickRandomMon();
    sPersonalityTestTracker->unk4.PartnerID = DevPickRandomPartnerDistinctFrom(sPersonalityTestTracker->unk4.StarterID);
    CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.StarterName, sPersonalityTestTracker->unk4.StarterID);
    CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.PartnerNick, sPersonalityTestTracker->unk4.PartnerID);
    sPersonalityTestTracker->unk4.recruitAll = RECRUIT_ALL_NONE;
    sPersonalityTestTracker->unk4.skipBasicRescues = 1; // Yes
    sPersonalityTestTracker->unk4.difficulty = DIFFICULTY_NIGHTMARE;
    SetRecruitAllSetting(RECRUIT_ALL_NONE);
    SetSkipBasicRescuesSetting(1);
    SetGameDifficultySetting(DIFFICULTY_NIGHTMARE);
    
    // Level up team to 100 in dev mode
    sub_8043FD0();
    #else
    sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
    #endif
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
        case PERSONALITY_SKIP_BASIC_RESCUES_SELECTION:
            HandleSkipBasicRescuesSelection();
            break;
        case PERSONALITY_RECRUIT_ALL_SELECTION:
            HandleRecruitAllSelection();
            break;
        case PERSONALITY_DIFFICULTY_SELECTION:
            HandleDifficultySelection();
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
        case PERSONALITY_ADVANCE_TO_STARTER_NICKNAME:
            AdvanceToStarterNickname();
            break;
        case PERSONALITY_STARTER_NICKNAME:
            HandleStarterNickname();
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
        case PERSONALITY_TEAM_NAME_PROMPT:
            PromptTeamName();
            break;
        case PERSONALITY_TEAM_NAME_ENTRY:
            HandleTeamNameEntry();
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
#ifdef DEV
                // In DEV builds, always use this seed
                sPersonalityTestTracker->rngSeed = 100;
#endif
                sPersonalityTestTracker->seedChosen = TRUE;
            }
            sPersonalityTestTracker->unk4.customSeed = sPersonalityTestTracker->rngSeed;
            SetGameDifficultySetting(sPersonalityTestTracker->unk4.difficulty);
            sub_8011C40(sPersonalityTestTracker->rngSeed);
            // Commit the chosen starter/partner/team-name into global state
            WriteTeamBasicInfo(&sPersonalityTestTracker->unk4);
            // Apply postgame flags and team initialization, then start save UI.
            ApplySkipPostgameBootstrap();
            // Mark continue mode for resume from title.
            SetScriptVarValue(NULL, START_MODE, 1); // MODE_CONTINUE_GAME
            // Persist to save so Continue is available on title.
            sub_8011C28(1);
            PrepareSavePakWrite(MONSTER_NONE);
            sPersonalityTestTracker->TestState = PERSONALITY_SKIP_BOOTSTRAP_SAVING;
            return 0;
        case PERSONALITY_SKIP_BOOTSTRAP_SAVING:
            // Drive the save UI to completion; then return to title.
            if (WriteSavePak())
                return 0;
            FinishWriteSavePak();
            // Perform a full soft reset so post-save behavior exactly matches
            // a fresh emulator boot. Continue will be available on the title.
            SoftReset(RESET_ALL);
            return 0;
        default:
            break;
    }
    return 0;
}

void DeleteTestTracker(void)
{
    WriteTeamBasicInfo(&sPersonalityTestTracker->unk4);
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
        case SEED_MENU_NORMAL:
        {
            //sPersonalityTestTracker->rngSeed = -1;
            sPersonalityTestTracker->unk4.customSeed = -1;
            sPersonalityTestTracker->seedChosen = TRUE;
            sPersonalityTestTracker->usingCustomSeed = FALSE;
            StartSkipBasicRescuesSelection();
            break;
        }
        case SEED_MENU_RANDOM:
        {
            s32 generatedSeed = GenerateRandomSeed();
#ifdef DEV
            // In DEV builds, always use seed -6903 for consistent testing
            generatedSeed = -6903;
#endif
            sPersonalityTestTracker->rngSeed = generatedSeed;
            sPersonalityTestTracker->unk4.customSeed = generatedSeed;
            sPersonalityTestTracker->seedChosen = TRUE;
            sPersonalityTestTracker->usingCustomSeed = FALSE;
            StartSkipBasicRescuesSelection();
            break;
        }
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
            CleanupNamingScreen();
            MemoryFill8(sPersonalityTestTracker->seedBuffer, 0, PERSONALITY_TEST_SEED_BUFFER_SIZE);
            sPersonalityTestTracker->usingCustomSeed = FALSE;
            sPersonalityTestTracker->TestState = PERSONALITY_SEED_PROMPT;
            break;
        case 3:
            if (TryStoreCustomSeed()) {
                CleanupNamingScreen();
                StartSkipBasicRescuesSelection();
            }
            break;
    }
}

static void CleanupNamingScreen(void)
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

static void StartSkipBasicRescuesSelection(void)
{
    CreateMenuDialogueBoxAndPortrait(gSkipBasicRescuesPrompt, 0, 0, gSkipBasicRescuesMenu, 0, 3, 0, 0, 0x101);
    sPersonalityTestTracker->TestState = PERSONALITY_SKIP_BASIC_RESCUES_SELECTION;
}

static void StartRecruitAllSelection(void)
{
    CreateMenuDialogueBoxAndPortrait(gRecruitAllPrompt, 0, 0, gRecruitAllMenu, 0, 3, 0, 0, 0x101);
    sPersonalityTestTracker->TestState = PERSONALITY_RECRUIT_ALL_SELECTION;
}

static bool8 ShouldEnableAutoRecruitAllDebug(void)
{
    // Hidden debug toggle: hold L+R+SELECT while confirming "All Recruitable"
    // to set the AutoRecruitAll preset without exposing it on the menu.
    u16 held = gRealInputs.held;
    return (held & (L_BUTTON | R_BUTTON | SELECT_BUTTON)) == (L_BUTTON | R_BUTTON | SELECT_BUTTON);
}

// PlaySolo selection removed; always proceed to partner selection.

static void StartDifficultySelection(void)
{
    CreateMenuDialogueBoxAndPortrait(gDifficultyPrompt, 0, 0, gDifficultyMenu, 0, 3, 0, 0, 0x101);
    sPersonalityTestTracker->TestState = PERSONALITY_DIFFICULTY_SELECTION;
}

static void AdvanceToStarterSelection(void)
{
    s32 temp;

    if (sub_80144A4(&temp) != 0)
        return;

    CreateStarterSelectionMenu();
    sPersonalityTestTracker->TestState = PERSONALITY_PLAYER_STARTER_SELECTION;
}

static void AdvanceToStarterNickname(void)
{
    s32 temp;

    if (sub_80144A4(&temp) != 0)
        return;

    NamingScreen_Init(NAMING_SCREEN_PLAYER, sPersonalityTestTracker->unk4.StarterName);
    sPersonalityTestTracker->TestState = PERSONALITY_STARTER_NICKNAME;
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
    CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.StarterName, chosen);
    CreateDialogueBoxAndPortrait(gStarterNickPrompt, 0, 0, 0x301);
    sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_STARTER_NICKNAME;
}

static void HandleStarterNickname(void)
{
    u32 result = NamingScreen_HandleInput();

    if (result == 0)
        return;

    if (result == 2) {
        CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.StarterName, sPersonalityTestTracker->unk4.StarterID);
    }
    else if (result == 3 && sPersonalityTestTracker->unk4.StarterName[0] == '\0') {
        CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.StarterName, sPersonalityTestTracker->unk4.StarterID);
    }

    CleanupNamingScreen();
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
    sPersonalityTestTracker->unk4.customSeed = seed;
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

static void HandleSkipBasicRescuesSelection(void)
{
    s32 selection;

    if (sub_80144A4(&selection) != 0)
        return;

    if (selection < 0 || selection > 1)
        selection = 0; // Default to No

    sPersonalityTestTracker->unk4.skipBasicRescues = (u8)selection;
    SetSkipBasicRescuesSetting((u8)selection);
    StartRecruitAllSelection();
}

static void HandleRecruitAllSelection(void)
{
    s32 selection;

    if (sub_80144A4(&selection) != 0)
        return;

    if (selection < 0 || selection > RECRUIT_ALL_NONE)
        selection = RECRUIT_ALL_NORMAL; // Default to Normal

    if (selection == RECRUIT_ALL_ALL && ShouldEnableAutoRecruitAllDebug()) {
        selection = RECRUIT_ALL_AUTO;
    }

    sPersonalityTestTracker->unk4.recruitAll = (u8)selection;
    SetRecruitAllSetting((u8)selection);
    StartDifficultySelection();
}

static void HandleDifficultySelection(void)
{
    s32 selection;

    if (sub_80144A4(&selection) != 0)
        return;

    if (selection < 0 || selection >= NUM_DIFFICULTY_SETTINGS)
        selection = DIFFICULTY_NORMAL;

    sPersonalityTestTracker->unk4.difficulty = selection;
    SetGameDifficultySetting(selection);
    StartGenderSelection();
}

// Strong override for SkipCutscenes=ON: behave as if postgame is unlocked
// and all main-story missions are complete. Spawn in Team Base Inside,
// seed basic items/news, and clear any pending GO flags.
// (skipCutscenes postgame bootstrap removed)
UNUSED static void ApplySkipStartMinimal(void)
{
    // Intentionally no-op: SkipCutscenes no longer alters start state.
}

// Minimal state setup to mirror a "postgame" start, used when SkipCutscenes=ON
// after finishing the personality quiz. This marks the main scenario as
// postgame, spawns the player inside the Team Base, and seeds some initial
// mailbox/news so the world looks sane on first load.
static void ApplySkipPostgameBootstrap(void)
{
    // Ensure the chosen hero/partner/team-name are committed to the global
    // personality state used by team creation, then materialize the team.
    // This mirrors what DeleteTestTracker() would do in the normal flow.
    WriteTeamBasicInfo(&sPersonalityTestTracker->unk4);
    sub_8001064();

    // Scenario: set main scenario & sub-scenarios to match the
    // story_flow.md "Normal Post Game Example" snapshot.
    // Main: SCEN=[19,2]
    ScenarioCalc(SCENARIO_MAIN, 19, 2);
    // Subs: S1=[31,1] S2=[35,0] S3=[37,0] S4=[43,2] S5=[45,0]
    //       S6=[48,0] S7=[50,0] S8=[52,0] S9=[55,3]
    ScenarioCalc(SCENARIO_SUB1, 31, 1);
    ScenarioCalc(SCENARIO_SUB2, 35, 0);
    ScenarioCalc(SCENARIO_SUB3, 37, 0);
    ScenarioCalc(SCENARIO_SUB4, 43, 2);
    ScenarioCalc(SCENARIO_SUB5, 45, 0);
    ScenarioCalc(SCENARIO_SUB6, 48, 0);
    ScenarioCalc(SCENARIO_SUB7, 50, 0);
    ScenarioCalc(SCENARIO_SUB8, 52, 0);
    ScenarioCalc(SCENARIO_SUB9, 55, 3);

    // Use the final base interior for postgame look-and-feel.
    SetScriptVarValue(NULL, BASE_LEVEL, 2);

    // Enter: resume into the Team Base (inside) when continuing from title.
    SetScriptVarValue(NULL, GROUND_ENTER, MAP_TEAM_BASE_INSIDE);
    SetScriptVarValue(NULL, GROUND_ENTER_LINK, 0);
    SetScriptVarValue(NULL, GROUND_GETOUT, MAP_TEAM_BASE_INSIDE);
    SetScriptVarValue(NULL, GROUND_MAP, -1);
    SetScriptVarValue(NULL, GROUND_PLACE, GROUND_PLACE_TEAM_BASE_INSIDE);

    // Clear any dungeon selection/resolution state. Use the neutral default
    // (-1) so world-map/dungeon selection logic can set it explicitly.
    SetScriptVarValue(NULL, DUNGEON_SELECT, -1);
    SetScriptVarValue(NULL, DUNGEON_ENTER, 0);
    SetScriptVarValue(NULL, DUNGEON_ENTER_INDEX, -1);
    SetScriptVarValue(NULL, DUNGEON_RESULT, 0);

    // Do not force-open or pre-clear every dungeon. Let the world
    // behave like vanilla postgame: Pelipper jobs and normal story
    // checks will populate GO lists and dungeon availability.

    // Basic QoL: seed Pelipper jobs and Pokémon News so the mailbox isn't empty.
    sub_80961B4();     // seed Pelipper jobs
    sub_8096488();     // seed Pokémon News

    // Early-story boss/event flags: mark as completed so dungeons/rooms
    // naturally route to their postgame branches without custom gating.
    // This mirrors a file that has finished the main story and avoids
    // re-triggering fights like Skarmory at Mt. Steel.
    {
        // All "post" flags used by the fixed-room cutscene selector table
        // (gUnknown_8107234[].unk4). Marking these ensures param_1->unk5
        // (post branch) is selected everywhere relevant.
        static const u8 kPostgameExclusiveFlags[] = {
            0x01, 0x03, 0x05, 0x07, 0x09, 0x0A, 0x0C, 0x0D,
            0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1A, 0x1C,
            0x1E, 0x20, 0x21
        };
        s32 i;
        for (i = 0; i < (s32)(sizeof(kPostgameExclusiveFlags)/sizeof(kPostgameExclusiveFlags[0])); i++) {
            sub_8097FD0(kPostgameExclusiveFlags[i]);
        }
    }

    // Continue-screen legendary icon: mark Purity Forest (Celebi) as conquered
    // so the Celebi mini-portrait appears alongside the others, matching
    // the postgame example.
    // Mapping: load_screen uses sub_80023E4(0x1B) which checks
    // RescueScenarioConquered(0x29). 0x29 corresponds to Purity Forest.
    sub_8097418(0x29, 1);
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
    CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.PartnerNick, selectedPartner);
            CreateDialogueBoxAndPortrait(gPartnerNickPrompt, 0, 0, 0x301);
            sPersonalityTestTracker->TestState = PERSONALITY_ADVANCE_TO_PARTNER_NICKNAME_2;
        }
    }
}

static void AdvanceToPartnerNicknameScreen(void)
{
    s32 temp;

    if (sub_80144A4(&temp) != 0)
        return;

    NamingScreen_Init(NAMING_SCREEN_PARTNER, sPersonalityTestTracker->unk4.PartnerNick);
    sPersonalityTestTracker->TestState = PERSONALITY_PARTNER_NICKNAME;
}

static void NicknamePartner(void)
{
    u32 result = NamingScreen_HandleInput();

    if (result == 0)
        return;

    if (result == 2) {
        CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.PartnerNick, sPersonalityTestTracker->unk4.PartnerID);
    }
    else if (result == 3 && sPersonalityTestTracker->unk4.PartnerNick[0] == '\0') {
        CopyMonsterNameToBuffer(sPersonalityTestTracker->unk4.PartnerNick, sPersonalityTestTracker->unk4.PartnerID);
    }

    CleanupNamingScreen();
    CreateDialogueBoxAndPortrait(gTeamNamePrompt, 0, 0, 0x301);
    sPersonalityTestTracker->TestState = PERSONALITY_TEAM_NAME_PROMPT;
}

static void PromptTeamName(void)
{
    s32 temp;

    if (sub_80144A4(&temp) != 0)
        return;

    if (sPersonalityTestTracker->unk4.TeamName[0] == '\0') {
        sub_80920D8(sPersonalityTestTracker->unk4.TeamName);
    }

    NamingScreen_Init(NAMING_SCREEN_TEAM, sPersonalityTestTracker->unk4.TeamName);
    sPersonalityTestTracker->TestState = PERSONALITY_TEAM_NAME_ENTRY;
}

static void HandleTeamNameEntry(void)
{
    u32 result = NamingScreen_HandleInput();

    if (result == 0)
        return;

    if (result == 2) {
        sub_80920D8(sPersonalityTestTracker->unk4.TeamName);
    }
    else if (result == 3 && sPersonalityTestTracker->unk4.TeamName[0] == '\0') {
        sub_80920D8(sPersonalityTestTracker->unk4.TeamName);
    }

    CleanupNamingScreen();
    SetRescueTeamName(sPersonalityTestTracker->unk4.TeamName);
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
    WindowTemplates stackArray;

    RestoreSavedWindows(&stackArray);
    stackArray.id[1].width = 0;
    stackArray.id[1].height = 0;
    stackArray.id[1].unk10 = 0;
    stackArray.id[1].unk12 = 0;
    stackArray.id[1].header = NULL;
    ResetUnusedInputStruct();
    ShowWindows(&stackArray, TRUE, FALSE);
    CallPrepareTextbox_8008C54(1);
    sub_80073B8(1);
    sub_80073E0(1);
}
