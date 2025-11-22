#include "global.h"
#include "type_selection.h"

#include "constants/input.h"
#include "input.h"
#include "memory.h"
#include "menu_input.h"
#include "mgba_log.h"
#include "music_util.h"
#include "strings.h"
#include "text_1.h"
#include "text_2.h"
#include "text_3.h"

#define TYPE_SELECTION_MAX_OPTIONS 2
#define TYPE_SELECTION_PROMPT_WINDOW_ID 0
#define TYPE_SELECTION_MENU_WINDOW_ID 1
#define TYPE_SELECTION_MENU_ENTRY_HEIGHT 40
#define TYPE_SELECTION_MENU_LINE_LENGTH 36

typedef struct TypeSelectionMenuState
{
    bool8 active;
    MenuWindow window;
    WindowTemplates previousWindows;
    bool8 hasPreviousWindows;
    u8 optionText[TYPE_SELECTION_MAX_OPTIONS][192];
} TypeSelectionMenuState;

static EWRAM_DATA TypeSelectionMenuState sMenuState = {0};

static const u8 sMenuHeaderText[] = _("Which path to take?");
static const u8 sMenuPromptText[] = _("You see a vision of tomorrow's adventures, which vision do you see?");
const u8 gTypeSelectionFallbackText[] = _("That was good work today.\nI should get some rest.");

static const WindowHeader sTypeSelectionWindowHeader = {1, 0, 22, 0};
static const WindowTemplates sTypeSelectionWindowTemplates = {
    .id = {
        [TYPE_SELECTION_PROMPT_WINDOW_ID] = {
            .unk0 = 0,
            .type = WINDOW_TYPE_NORMAL,
            .pos = {1, 14},
            .width = 26,
            .height = 4,
            .unk10 = 4,
            .unk12 = 0,
            .header = NULL,
        },
        [TYPE_SELECTION_MENU_WINDOW_ID] = {
            .unk0 = 0,
            .type = WINDOW_TYPE_WITH_HEADER,
            .pos = {1, 2},
            .width = 26,
            .height = 11,
            .unk10 = 13,
            .unk12 = 0,
            .header = &sTypeSelectionWindowHeader,
        },
        [2] = WIN_TEMPLATE_DUMMY,
        [3] = WIN_TEMPLATE_DUMMY,
    },
};

static void BuildMenuEntries(void);
static void BuildHintLabel(u8 *dst, s32 index, const TypeHintDefinition *hint);
static void AppendWrappedHint(u8 *dst, const u8 *text);
static void InitMenuWindow(void);
static void DestroyMenuWindow(void);
static void DrawMenuWindow(void);
static void DrawPromptWindow(void);
const void *const gTypeSelectionUiLinkAnchor[];

bool8 TypeSelectionMenu_Begin(void)
{
    (void)gTypeSelectionUiLinkAnchor;
    if (!TypeSelection_ShouldPromptPlayer()) {
        MGBA_Warnf("[TypeSelectionUI] Begin aborted: ShouldPromptPlayer=FALSE");
        return FALSE;
    }
    if (!TypeSelection_EnsurePendingHints()) {
        MGBA_Warnf("[TypeSelectionUI] Begin aborted: failed to ensure hints");
        return FALSE;
    }

    BuildMenuEntries();
    InitMenuWindow();
    DrawMenuWindow();
    DrawPromptWindow();
    MGBA_Infof("[TypeSelectionUI] Menu opened. Awaiting choice.");

    sMenuState.active = TRUE;
    PlayMenuSoundEffect(4);
    return TRUE;
}

bool8 TypeSelectionMenu_Update(void)
{
    s32 input;

    if (!sMenuState.active)
        return TRUE;

    input = GetKeyPress(&sMenuState.window.input);
    switch (input) {
        case INPUT_A_BUTTON: {
            s32 selection = GET_CURRENT_MENU_ENTRY(sMenuState.window.input);
            MGBA_Infof("[TypeSelectionUI] Player selected action=%d", selection);
            if (selection < 0 || selection >= TYPE_SELECTION_MAX_OPTIONS) {
                PlayMenuSoundEffect(2);
                break;
            }
            if (!TypeSelection_SelectHint(selection, NULL)) {
                MGBA_Warnf("[TypeSelectionUI] SelectHint failed, keeping menu open");
                BuildMenuEntries();
                DrawMenuWindow();
                PlayMenuSoundEffect(2);
                break;
            }
            DestroyMenuWindow();
            sMenuState.active = FALSE;
            MGBA_Infof("[TypeSelectionUI] Selection processed, exiting menu");
            return TRUE;
        }
        case INPUT_B_BUTTON:
            PlayMenuSoundEffect(2);
            break;
        default:
            if (MenuCursorUpdate(&sMenuState.window.input, TRUE))
                DrawMenuWindow();
            return FALSE;
    }

    if (MenuCursorUpdate(&sMenuState.window.input, TRUE))
        DrawMenuWindow();
    return FALSE;
}

void TypeSelectionMenu_Reset(void)
{
    DestroyMenuWindow();
    MemoryFill8(&sMenuState, 0, sizeof(sMenuState));
}

static void BuildMenuEntries(void)
{
    s32 i;

    for (i = 0; i < TYPE_SELECTION_MAX_OPTIONS; i++) {
        const TypeHintDefinition *hint = TypeSelection_GetPendingHint(i);
        if (hint != NULL) {
            BuildHintLabel(sMenuState.optionText[i], i, hint);
        } else {
            sprintf(sMenuState.optionText[i], _("Hint %d:\nUnavailable"), i + 1);
            MGBA_Warnf("[TypeSelectionUI] Hint %d unavailable", i + 1);
        }
    }
}

const void *const gTypeSelectionUiLinkAnchor[] = {
    (const void *)TypeSelectionMenu_Begin,
    (const void *)TypeSelectionMenu_Update,
    (const void *)TypeSelectionMenu_Reset,
    gTypeSelectionFallbackText,
};

static void BuildHintLabel(u8 *dst, s32 index, const TypeHintDefinition *hint)
{
    AppendWrappedHint(dst, hint->message);

    MGBA_Infof("[TypeSelectionUI] Hint %d = \"%s\" (%s/%s)",
               index + 1,
               hint->message,
               gUnformattedTypeStrings[hint->type1],
               gUnformattedTypeStrings[hint->type2]);
}

static void AppendWrappedHint(u8 *dst, const u8 *text)
{
    s32 lineLen = 0;
    bool8 firstWord = TRUE;

    while (*text != '\0') {
        const u8 *wordStart;
        s32 wordLen = 0;

        while (*text == ' ')
            text++;
        if (*text == '\0')
            break;

        wordStart = text;
        while (text[wordLen] != '\0' && text[wordLen] != ' ')
            wordLen++;

        if (!firstWord) {
            if (lineLen + 1 + wordLen > TYPE_SELECTION_MENU_LINE_LENGTH) {
                *dst++ = '\n';
                lineLen = 0;
            } else {
                *dst++ = ' ';
                lineLen++;
            }
        }

        while (wordLen-- > 0)
            *dst++ = *text++;

        lineLen += text - wordStart;
        firstWord = FALSE;
    }

    *dst = '\0';
}

static void InitMenuWindow(void)
{
    RestoreSavedWindows(&sMenuState.previousWindows);
    RestoreSavedWindows(&sMenuState.window.windows);
    MemoryCopy8(&sMenuState.window.windows.id[TYPE_SELECTION_PROMPT_WINDOW_ID],
                &sTypeSelectionWindowTemplates.id[TYPE_SELECTION_PROMPT_WINDOW_ID],
                sizeof(WindowTemplate));
    MemoryCopy8(&sMenuState.window.windows.id[TYPE_SELECTION_MENU_WINDOW_ID],
                &sTypeSelectionWindowTemplates.id[TYPE_SELECTION_MENU_WINDOW_ID],
                sizeof(WindowTemplate));
    sMenuState.window.menuWinId = TYPE_SELECTION_MENU_WINDOW_ID;
    sMenuState.window.menuWindow = &sMenuState.window.windows.id[TYPE_SELECTION_MENU_WINDOW_ID];
    sMenuState.hasPreviousWindows = TRUE;
    ResetUnusedInputStruct();
    ShowWindows(&sMenuState.window.windows, TRUE, TRUE);
    CreateMenuOnWindow(&sMenuState.window.input, TYPE_SELECTION_MAX_OPTIONS, TYPE_SELECTION_MAX_OPTIONS, TYPE_SELECTION_MENU_WINDOW_ID);
    sMenuState.window.input.entryHeight = TYPE_SELECTION_MENU_ENTRY_HEIGHT << 8;
    sMenuState.window.input.currPageEntries = TYPE_SELECTION_MAX_OPTIONS;
    sMenuState.window.input.entriesPerPage = TYPE_SELECTION_MAX_OPTIONS;
    sMenuState.window.input.totalEntriesCount = TYPE_SELECTION_MAX_OPTIONS;
    sMenuState.window.input.menuIndex = 0;
    sMenuState.window.input.currPage = 0;
    sMenuState.window.input.unk24 = 0;
    MenuUpdatePagesData(&sMenuState.window.input);
}

static void DestroyMenuWindow(void)
{
    if (sMenuState.window.menuWindow == NULL)
        return;

    ResetUnusedInputStruct();
    if (sMenuState.hasPreviousWindows) {
        ShowWindows(&sMenuState.previousWindows, TRUE, TRUE);
        sMenuState.hasPreviousWindows = FALSE;
    } else {
        ShowWindows(&sTypeSelectionWindowTemplates, TRUE, TRUE);
    }
    sMenuState.window.menuWindow = NULL;
}

static void DrawMenuWindow(void)
{
    s32 i;

    if (sMenuState.window.menuWindow == NULL)
        return;

    CallPrepareTextbox_8008C54(sMenuState.window.menuWinId);
    sub_80073B8(sMenuState.window.menuWinId);
    PrintStringOnWindow(16, 0, sMenuHeaderText, sMenuState.window.menuWinId, 0);
    for (i = 0; i < TYPE_SELECTION_MAX_OPTIONS; i++) {
        s32 y = GetMenuEntryYCoord(&sMenuState.window.input, i);
        PrintStringOnWindow(8, y, sMenuState.optionText[i], sMenuState.window.menuWinId, 0);
    }
    sub_80073E0(sMenuState.window.menuWinId);
}

static void DrawPromptWindow(void)
{
    CallPrepareTextbox_8008C54(TYPE_SELECTION_PROMPT_WINDOW_ID);
    sub_80073B8(TYPE_SELECTION_PROMPT_WINDOW_ID);
    PrintStringOnWindow(8, 4, sMenuPromptText, TYPE_SELECTION_PROMPT_WINDOW_ID, 0);
    sub_80073E0(TYPE_SELECTION_PROMPT_WINDOW_ID);
}
