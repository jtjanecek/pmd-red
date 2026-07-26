#include "global.h"
#include "globaldata.h"
#include "mgba_log.h"
#include "constants/friend_area.h"
#include "bg_palette_buffer.h"
#include "code_800D090.h"
#include "code_801D014.h"
#include "code_80227B8.h"
#include "friend_list.h"
#include "code_8024458.h"
#include "code_80958E8.h"
#include "common_strings.h"
#include "def_filearchives.h"
#include "event_flag.h"
#include "friend_area.h"
#include "friend_area_action_menu.h"
#include "friend_list_menu.h"
#include "ground_lives.h"
#include "ground_main.h"
#include "ground_map.h"
#include "input.h"
#include "iq_skill_menu.h"
#include "main_loops.h"
#include "memory.h"
#include "menu_input.h"
#include "naming_screen.h"
#include "options_menu1.h"
#include "party_list_menu.h"
#include "personality_test1.h"
#include "rescue_team_info.h"
#include "save.h"
#include "save_write.h"
#include "gba/syscall.h" // SoftReset, RESET_ALL
#include "string_format.h"
#include "text_1.h"
#include "text_2.h"
#include "text_3.h"
#include "wigglytuff_shop2.h"
#include "wonder_mail_802C4C8.h"
#include "wonder_mail_802C860.h"

static EWRAM_INIT struct unk_203B250 *sUnknown_203B250 = {NULL};
static EWRAM_INIT u32 sUnknown_203B254 = {0};
static EWRAM_INIT u32 sFriendListMovesHeartbeat = {0};
static EWRAM_INIT u32 sFriendListActiveHeartbeat = {0};

// "Change Seed" menu: numeric keypad seed entry, then save + return to title.
#define CHANGE_SEED_MENU_ACTION   13
#define CHANGE_SEED_STATE_INPUT   13
#define CHANGE_SEED_STATE_SAVING  14
#define NAMING_SCREEN_NUMERIC     6
#define CHANGE_SEED_BUFFER_SIZE   12
#define CHANGE_SEED_INT32_MAX     2147483647
#define CHANGE_SEED_INT32_MIN     (-2147483647 - 1)

static EWRAM_INIT u8 sChangeSeedBuffer[CHANGE_SEED_BUFFER_SIZE] = {0};

#include "data/code_801D014.h"

static void LoadTeamRankBadge(u32, u32, u32);

static void HandleChangeSeedInput(void);
static void HandleChangeSeedSaving(void);
static bool8 ParseChangeSeed(const u8 *text, s32 *seedOut);
static void CommitChangeSeed(s32 seed);

static void sub_801D208(u32 newState);
static void sub_801D220(void);
static void sub_801D3A8(void);
static void sub_801D4C0(void);
static void sub_801D680(void);
static void sub_801D760(void);
static void sub_801D77C(void);
static void sub_801D798(void);
static void sub_801D7CC(void);
static void sub_801D808(void);
static void sub_801D824(void);
static void sub_801D840(void);
static void sub_801D85C(void);
static void sub_801D878(void);
static void sub_801D894(void);

bool8 sub_801D014(Pokemon *a0)
{
    s32 index;
    Pokemon *pokemon;
    struct unk_203B250 *preload;

    ResetUnusedInputStruct();
    ShowWindows(NULL, TRUE, TRUE);

    sUnknown_203B250 = MemoryAlloc(sizeof(struct unk_203B250), 8);
    sUnknown_203B250->menuAction = sUnknown_203B254;
    sUnknown_203B250->pokeStruct = a0;

    if (sUnknown_203B250->pokeStruct != NULL) {
        for (index = 0; index < NUM_MONSTERS; index++) {
            preload = sUnknown_203B250;
            pokemon = &gRecruitedPokemonRef->pokemon[(s16)index]; // cast is needed here

            if (preload->pokeStruct == pokemon) {
                preload->index = index;
                break;
            }
        }
    }
    else
        sUnknown_203B250->index = NUM_MONSTERS;

    sUnknown_203B250->currFriendAreaLocation = MapIdToFriendAreaId(GetGroundMapID());
    sUnknown_203B250->unk8 = 0;
    sUnknown_203B250->unk9 = 0;
    sUnknown_203B250->unkC = GetPlayerPokemonStruct();
    sUnknown_203B250->unk7 = 0;

    if (sUnknown_203B250->pokeStruct != NULL)
        sub_801D208(5);
    else
        sub_801D208(0);

    return TRUE;
}

u32 sub_801D0DC(void)
{
    u32 retval;

    MGBA_Warnf("[code_801D014] sub_801D0DC: entered, state=%d", sUnknown_203B250->state);
    if (FriendListMenu_DebugIsActive()) {
        sFriendListActiveHeartbeat++;
        MGBA_Warnf("[code_801D014][Heartbeat] FriendList active frame=%u state=%d", sFriendListActiveHeartbeat, sUnknown_203B250->state);
    }
    if (FriendListMenu_DebugIsMovesState()) {
        sFriendListMovesHeartbeat++;
        MGBA_Warnf("[code_801D014][Heartbeat] FriendList MOVES frame=%u", sFriendListMovesHeartbeat);
    }
    else {
        if (sFriendListMovesHeartbeat != 0 && sUnknown_203B250->state != 0xF) {
            MGBA_Warnf("[code_801D014][Heartbeat] MOVES heartbeat reset (state=%d)", sUnknown_203B250->state);
            sFriendListMovesHeartbeat = 0;
        }
    }
    MGBA_Warnf("[code_801D014] sub_801D0DC: dispatching state switch");
    switch (sUnknown_203B250->state) {
        case 0:
        case 1:
            sub_801D680();
            break;
        case 3:
            sub_801D760();
            break;
        case 4:
            sub_801D77C();
            break;
        case 5:
            sub_801D798();
        break;
    case 6:
    case 7:
        MGBA_Warnf("[code_801D014] sub_801D0DC: calling sub_801D7CC");
        sub_801D7CC();
        MGBA_Warnf("[code_801D014] sub_801D0DC: sub_801D7CC returned");
        break;
        case 8:
            sub_801D808();
            break;
        case 9:
            sub_801D824();
            break;
        case 10:
            sub_801D840();
            break;
        case 11:
            sub_801D85C();
            break;
        case 12:
            sub_801D878();
            break;
        case CHANGE_SEED_STATE_INPUT:
            HandleChangeSeedInput();
            break;
        case CHANGE_SEED_STATE_SAVING:
            HandleChangeSeedSaving();
            break;
        default:
            MGBA_Warnf("[code_801D014] sub_801D0DC: unknown state, returning 3");
            return 3;
    }
    retval = 0;
    MGBA_Warnf("[code_801D014] sub_801D0DC: returning %d", retval);
    return retval;
}

u32 sub_801D178(void)
{
    if (sUnknown_203B250->unk9 != 0)
        return 2;

    if (sUnknown_203B250->unk7 != 0)
        return 3;

    if (sUnknown_203B250->unk8 != 0)
        return 1;

    if (sUnknown_203B250->unkC != GetPlayerPokemonStruct())
        return 4;

    if ((s16) sub_80A7AE8(7) < 0)
        return 0;

    if (PokemonFlag2(sub_808D3F8()))
        return 0;

    return 4;
}

u8 sub_801D1D4(void)
{
    return sUnknown_203B250->unk7;
}

void sub_801D1E0(void)
{
    if (sUnknown_203B250 != NULL) {
        sUnknown_203B254 = sUnknown_203B250->menuAction;
        MemoryFree(sUnknown_203B250);
        sUnknown_203B250 = NULL;
    }
}

static void sub_801D208(u32 newState)
{
    sUnknown_203B250->state = newState;
    sub_801D220();
    sub_801D3A8();
}

static void sub_801D220(void)
{
    s32 i;

    RestoreSavedWindows(&sUnknown_203B250->windows);

    switch (sUnknown_203B250->state) {
        case 0:
            sub_801D4C0();

            if (sUnknown_203B250->pokeStruct != NULL) {
                for (i = 0; i < 4; i++)
                    sUnknown_203B250->windows.id[i] = sUnknown_80DBE7C;

                sUnknown_203B250->windows.id[0] = sUnknown_80DBE98;
                sub_8012CAC(&sUnknown_203B250->windows.id[0], sUnknown_203B250->unk68);
                sUnknown_203B250->windows.id[0].width = 9;
            }
            else {
                for (i = 0; i < MAX_WINDOWS; i++)
                    sUnknown_203B250->windows.id[i] = sUnknown_80DBEB0[i];

                sub_8012CAC(&sUnknown_203B250->windows.id[0], sUnknown_203B250->unk68);
                sUnknown_203B250->windows.id[0].width = 8;
            }
            break;
        case 1:
            sub_801D4C0();

            if (sUnknown_203B250->pokeStruct != NULL) {
                for (i = 0; i < MAX_WINDOWS; i++)
                    sUnknown_203B250->windows.id[i] = sUnknown_80DBE7C;

                sUnknown_203B250->windows.id[0] = sUnknown_80DBE98;
                sub_8012CAC(&sUnknown_203B250->windows.id[0], sUnknown_203B250->unk68);
                sUnknown_203B250->windows.id[0].width = 9;
            }
            else {
                for (i = 0; i < MAX_WINDOWS; i++)
                    sUnknown_203B250->windows.id[i] = sUnknown_80DBEB0[i];

                sub_8012CAC(&sUnknown_203B250->windows.id[0], sUnknown_203B250->unk68);
                sUnknown_203B250->windows.id[0].width = 8;
            }
            break;
        default:
            for (i = 0; i < 4; i++)
                sUnknown_203B250->windows.id[i] = sUnknown_80DBE7C;
            break;
    }

    ResetUnusedInputStruct();
    ShowWindows(&sUnknown_203B250->windows, TRUE, TRUE);
}

static void sub_801D3A8(void)
{
    switch (sUnknown_203B250->state) {
        case 0:
        case 1:
            if (sUnknown_203B250->pokeStruct != NULL) {
                PrintColoredPokeNameToBuffer(gFormatBuffer_Monsters[0], sUnknown_203B250->pokeStruct, 7);
                sUnknown_203B250->unk18.unk0 = gFormatBuffer_Monsters[0];
                sub_8012D60(&sUnknown_203B250->unk18, sUnknown_203B250->unk68, 0, sUnknown_203B250->unkA8, sUnknown_203B250->menuAction, 0);
            }
            else {
                sub_801D894();
                sub_8012D60(&sUnknown_203B250->unk18, sUnknown_203B250->unk68, 0, sUnknown_203B250->unkA8, sUnknown_203B250->menuAction, 0);
            }
            break;
        case 3:
            sub_80227B8(sUnknown_203B250->pokeStruct);
            break;
        case 4:
            sub_8027074();
            break;
        case 5:
            CreatePartyListMenu(sUnknown_203B250->pokeStruct);
            break;
        case 6:
            CreateFriendListMenu(0);
            break;
        case 7:
            CreateFriendListMenu(1);
            break;
        case 8:
            sub_8024458(sUnknown_203B250->index, 2);
            break;
        case 9:
            CreateIQSkillMenu(sUnknown_203B250->index);
            break;
        case 10:
            CreateWigglytuffShopFriendAreaMenu(sUnknown_203B250->currFriendAreaLocation, TRUE, 2);
            break;
        case 11:
            InitializeJobListMenu(0);
            break;
        case 12:
            sub_801DCC4();
            break;
        case CHANGE_SEED_STATE_INPUT:
            MemoryFill8(sChangeSeedBuffer, 0, CHANGE_SEED_BUFFER_SIZE);
            NamingScreen_Init(NAMING_SCREEN_NUMERIC, sChangeSeedBuffer);
            break;
        case CHANGE_SEED_STATE_SAVING:
            PrepareSavePakWrite(MONSTER_NONE);
            break;
    }
}

static void sub_801D4C0(void)
{
    s32 index;
    s32 loopMax;

    loopMax = 0;
    MemoryFill16(sUnknown_203B250->unkA8, 0, sizeof(sUnknown_203B250->unkA8));

    if (sUnknown_203B250->currFriendAreaLocation == 0) {
        if (sub_8096E2C()) {
            sUnknown_203B250->unk68[loopMax].menuAction = 2;
            sUnknown_203B250->unk68[loopMax].text = sItems;
            if (GetNumberOfFilledInventorySlots() == 0)
                sUnknown_203B250->unkA8[loopMax] = 1;

            loopMax++;
            sUnknown_203B250->unk68[loopMax].text = sTeam;
            sUnknown_203B250->unk68[loopMax].menuAction = 4;
            loopMax++;
        }

        sUnknown_203B250->unk68[loopMax].text = sOthers;
        sUnknown_203B250->unk68[loopMax].menuAction = 11;
        loopMax++;

        sUnknown_203B250->unk68[loopMax].text = sChangeSeed;
        sUnknown_203B250->unk68[loopMax].menuAction = CHANGE_SEED_MENU_ACTION;
        loopMax++;
    }
    else {
        strcpy(gFormatBuffer_Monsters[0], sTripleQuestionMark);

        if (sub_8096E2C()) {
            sUnknown_203B250->unk68[loopMax].text = gCommonFriend[0];
            sUnknown_203B250->unk68[loopMax].menuAction = 5;
            if (sub_8024108(4))
                sUnknown_203B250->unkA8[loopMax] = 1;

            loopMax++;
            sUnknown_203B250->unk68[loopMax].menuAction = 2;
            sUnknown_203B250->unk68[loopMax].text = sItems;
            if (GetNumberOfFilledInventorySlots() == 0)
                sUnknown_203B250->unkA8[loopMax] = 1;

            loopMax++;
        }

        sUnknown_203B250->unk68[loopMax].text = gCommonExit[0];
        sUnknown_203B250->unk68[loopMax].menuAction = 10;

        loopMax++;
    }

    sUnknown_203B250->unk68[loopMax].text = NULL;
    sUnknown_203B250->unk68[loopMax].menuAction = 1;

    for (index = 0; index < loopMax; index++) {
        if (sUnknown_203B250->unkA8[index] == 0) {
            if (sUnknown_203B250->unk68[index].menuAction == sUnknown_203B250->menuAction)
                return;
        }
    }

    for (index = 0; index < loopMax; index++) {
        if (sUnknown_203B250->unkA8[index] == 0) {
            sUnknown_203B250->menuAction = sUnknown_203B250->unk68[index].menuAction;
            break;
        }
    }
}

static void sub_801D680(void)
{
    s32 menuAction;

    menuAction = 0;
    MGBA_Warnf("[code_801D014] sub_801D680: entering, currFriendAreaLocation=%d", sUnknown_203B250->currFriendAreaLocation);

    if (!sub_8012FD8(&sUnknown_203B250->unk18)) {
        sub_8013114(&sUnknown_203B250->unk18, &menuAction);
        if (menuAction != 1)
            sUnknown_203B250->menuAction = menuAction;
    }

    MGBA_Warnf("[code_801D014] sub_801D680: post-input menuAction=%d state=%d", menuAction, sUnknown_203B250->state);
    switch (menuAction) {
        case 2:
            sub_801D208(3);
            break;
        case 4:
            sub_801D208(6);
            break;
        case 3:
            sub_801D208(4);
            break;
        case 5:
            sub_801D208(7);
            break;
        case 6:
            sub_801D208(8);
            break;
        case 7:
            sub_801D208(9);
            break;
        case 8:
            sub_801D208(10);
            break;
        case 9:
            sub_801D208(11);
            break;
        case 10:
            sUnknown_203B250->unk9 = 1;
            sub_801D208(2);
            break;
        case 11:
            sub_801D208(12);
            break;
        case CHANGE_SEED_MENU_ACTION:
            sub_801D208(CHANGE_SEED_STATE_INPUT);
            break;
        case 1:
            sub_801D208(2);
            break;
    }
}

static void sub_801D760(void)
{
    switch (sub_8022860()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            sub_8022908();
            sub_801D208(1);
            break;
    }
}

static void sub_801D77C(void)
{
    switch (sub_80270A4()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            sub_8027168();
            sub_801D208(1);
            break;
    }
}

static void sub_801D798(void)
{
    switch (sub_8025F68()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            if (sub_802604C()) {
                sUnknown_203B250->unk7 = sUnknown_203B250->currFriendAreaLocation;
                MGBA_Warnf("[code_801D014] sub_801D798: returning with unk7=%d", sUnknown_203B250->unk7);
            }

            CleanPartyListMenu();
            sub_801D208(2);
            break;
    }
}

static void sub_801D7CC(void)
{
    u32 result;

    MGBA_Warnf("[code_801D014] sub_801D7CC: calling sub_8025354");
    result = sub_8025354();
    MGBA_Warnf("[code_801D014] sub_801D7CC: sub_8025354 returned %d", result);

    switch (result) {
        case 0:
        case 1:
        default:
            MGBA_Warnf("[code_801D014] sub_801D7CC: case 0/1/default, breaking");
            break;
        case 2:
        case 3:
            MGBA_Warnf("[code_801D014] sub_801D7CC: case 2/3, cleaning up");
            sUnknown_203B250->unk7 = sub_802540C();
            CleanFriendListMenu();

            if (sUnknown_203B250->unk7 != 0)
                sub_801D208(2);
            else
                sub_801D208(1);
            break;
    }
    MGBA_Warnf("[code_801D014] sub_801D7CC: returning");
}

static void sub_801D808(void)
{
    switch (sub_80244E4()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            sub_802453C();
            sub_801D208(1);
            break;
    }
}

static void sub_801D824(void)
{
    switch (sub_801BF48()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            CleanIQSkillMenu();
            sub_801D208(1);
            break;
    }
}

static void sub_801D840(void)
{
    switch (HandleWigglytuffShopFriendAreaMenuInput()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            CleanWigglytuffShopFriendAreaInfoMenu();
            sub_801D208(1);
            break;
    }
}

static void sub_801D85C(void)
{
    switch (sub_802C898()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            sub_802C8F4();
            sub_801D208(1);
            break;
    }
}

static void sub_801D878(void)
{
    switch (sub_801DCE8()) {
        case 0:
        case 1:
        default:
            break;
        case 2:
        case 3:
            sub_801DD50();
            sub_801D208(1);
            break;
    }
}

// Parse the numeric-keypad string into a signed 32-bit seed.
// Mirrors ParseSeedString() from the personality quiz: optional leading '-',
// decimal digits only, range-checked to int32, and -1 (the "no seed" sentinel)
// is rejected.
static bool8 ParseChangeSeed(const u8 *text, s32 *seedOut)
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
        if (!negative && value > CHANGE_SEED_INT32_MAX)
            return FALSE;
        if (negative && value > (s64)CHANGE_SEED_INT32_MAX + 1)
            return FALSE;
        ptr++;
    }

    if (digitCount > 10)
        return FALSE;

    if (negative)
        value = -value;

    if (value < CHANGE_SEED_INT32_MIN || value > CHANGE_SEED_INT32_MAX)
        return FALSE;

    if (value == -1)
        return FALSE;

    *seedOut = (s32)value;
    return TRUE;
}

// Apply the new seed both to the live global and to the persisted team info so
// it survives the save and the DungeonSeedOverrides recovery path.
static void CommitChangeSeed(s32 seed)
{
    TeamBasicInfo info;

    sub_8011C40(seed);
    ReadTeamBasicInfo(&info);
    info.customSeed = seed;
    WriteTeamBasicInfo(&info);
}

// Drive the numeric keypad: 0 keep waiting, 2 cancel (back to the top menu),
// 3 confirm. On a valid confirm, commit the seed and move to the save state; on
// an invalid value the keypad stays open so the player can correct it.
static void HandleChangeSeedInput(void)
{
    s32 seed;

    switch (NamingScreen_HandleInput()) {
        case 0:
            break;
        case 2:
            NamingScreen_Free();
            ResetUnusedInputStruct();
            ShowWindows(NULL, TRUE, TRUE);
            sub_801D208(1);
            break;
        case 3:
            if (ParseChangeSeed(sChangeSeedBuffer, &seed)) {
                NamingScreen_Free();
                ResetUnusedInputStruct();
                ShowWindows(NULL, TRUE, TRUE);
                CommitChangeSeed(seed);
                sub_801D208(CHANGE_SEED_STATE_SAVING);
            }
            break;
    }
}

// Drive the save UI to completion, then soft reset to the title screen so the
// run resumes from Continue with the new seed applied.
static void HandleChangeSeedSaving(void)
{
    if (WriteSavePak())
        return;

    FinishWriteSavePak();
    SoftReset(RESET_ALL);
}

static void sub_801D894(void)
{
    const u8 *location;
    s32 location_length;
    s32 x_coord;
    u8 buffer[100]; // sp +4

    if (sUnknown_203B250->currFriendAreaLocation == FRIEND_AREA_NONE)
        location = GetCurrentGroundPlaceName();
    else
        location = GetFriendAreaName(sUnknown_203B250->currFriendAreaLocation);

    FormatString(location, buffer, buffer + sizeof(buffer), 0);
    location_length = GetStringLineWidth(buffer);
    x_coord = (128 - location_length) / 2; // Centers the location name
    CallPrepareTextbox_8008C54(1);
    sub_80073B8(1);
    PrintStringOnWindow(x_coord, 4, buffer, 1, 0);
    sub_80073E0(1);
    CallPrepareTextbox_8008C54(2);
    sub_80073B8(2);
    LoadTeamRankBadge(2, 8, 6);

    // %s {COLOR CYAN}%d{RESET} Pts.
    sprintfStatic(buffer, sFmtPointsCyan, GetTeamRankString(GetRescueTeamRank()), GetTeamRankPts());
    PrintStringOnWindow(32, 4, buffer, 2, 0);
    sprintfStatic(buffer, sFmtMoneyCyan, gTeamInventoryRef->teamMoney);
    PrintStringOnWindow(32, 18, buffer, 2, 0);
    sub_80073E0(2);
}

static void LoadTeamRankBadge(u32 a0, u32 a1, u32 a2)
{
    OpenedFile *teamBadgeFile;
    s32 paletteIndex;
    u8 rank;
    RGB *colorArray;
    u8 *teamBadgePic;

    teamBadgeFile = OpenFileAndGetFileDataPtr(sTeamRankBadgeFileName, &gTitleMenuFileArchive);
    teamBadgePic = ((struct TeamBadgeData *)(teamBadgeFile->data))->pics;
    colorArray = ((struct TeamBadgeData *)(teamBadgeFile->data))->palette;

    for (paletteIndex = 0; paletteIndex < 16; paletteIndex++) {
        SetBGPaletteBufferColorArray(paletteIndex + 224, colorArray);
        colorArray++;
    }

    rank = GetRescueTeamRank();
    teamBadgePic = &teamBadgePic[rank * 128];
    sub_8007E20(a0, a1, a2, 16, 16, (void *) teamBadgePic, 14);
    CloseFile(teamBadgeFile);
}
