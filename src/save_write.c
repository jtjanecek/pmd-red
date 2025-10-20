#include "global.h"
#include "globaldata.h"
#include "code_8099360.h"
#include "memory.h"
#include "save.h"
#include "event_flag.h" // SetScriptVarValue, SetScriptVarArrayValue
#include "constants/ground_map.h" // MAP_TEAM_BASE_INSIDE
#include "constants/event_flag.h" // SCENARIO_MAIN, START_MODE, etc.
#include "save_write.h"
#include "string_format.h"
#include "menu_input.h"

// size: 0x20
typedef struct SavePakWrite
{
    /* 0x0 */ u32 state;
    s32 unk4;
    /* 0x8 */ u32 saveStatus;
    /* 0xC */ MonPortraitMsg monPortrait;
    /* 0x1C */ u16 pokeID;
} SavePakWrite;

static EWRAM_INIT SavePakWrite *sSavePakWrite = {NULL};

#include "data/save_write.h"

void PrepareSavePakWrite(s16 pokemonID)
{
    OpenedFile *file;
    s32 id_s32;

    id_s32 = pokemonID; // had to cast for asr shift

    sub_80993D8();
    sSavePakWrite = MemoryAlloc(sizeof(SavePakWrite), 5);
    sSavePakWrite->pokeID = id_s32;
    sSavePakWrite->monPortrait.faceFile = NULL;
    sSavePakWrite->monPortrait.faceData = NULL;

    if (pokemonID != MONSTER_NONE) {
        file = GetDialogueSpriteDataPtr(pokemonID);
        sSavePakWrite->monPortrait.faceFile = file;
        sSavePakWrite->monPortrait.faceData = (PortraitGfx *) file->data;
        sSavePakWrite->monPortrait.spriteId = 0;
        sSavePakWrite->monPortrait.flip = FALSE;
        sSavePakWrite->monPortrait.unkE = 0;
        sSavePakWrite->monPortrait.pos.x = 2;
        sSavePakWrite->monPortrait.pos.y = 8;
    }

    if (sSavePakWrite->monPortrait.faceFile != 0) {
        MonPortraitMsg *monPortraitPtr = &sSavePakWrite->monPortrait;
        CreateDialogueBoxAndPortrait(sSavingAdventure, 0, monPortraitPtr, 0x20);
    }
    else
        CreateDialogueBoxAndPortrait(sSavingAdventure, 0, NULL, 0x20);

    sSavePakWrite->state = 3;
}

bool8 WriteSavePak(void)
{
    MonPortraitMsg *monPortraitPtr;
    u32 local_14;
    u32 other_stack;

    monPortraitPtr = NULL;
    if (sSavePakWrite->monPortrait.faceFile != NULL)
        monPortraitPtr = &sSavePakWrite->monPortrait;

    switch (sSavePakWrite->state) {
        case 0:
            sSavePakWrite->state = 7;
            break;
        case 1:
            sSavePakWrite->unk4++;
            if (sSavePakWrite->unk4 > 8) {
                CreateDialogueBoxAndPortrait(sWriteGamePak, 0, 0, 0x20);
                sSavePakWrite->state = 3;
            }
            break;
        case 2:
            break;
        case 3:
            sSavePakWrite->state = 4;
            break;
        case 4:
            local_14 = 0;
            sub_80140DC();
            sSavePakWrite->saveStatus = WriteSavetoPak(&local_14, sub_8011C1C());

            switch (sSavePakWrite->saveStatus) {
                case SAVE_COMPLETED:
                    if (sSavePakWrite->monPortrait.faceFile != NULL)
                        CreateDialogueBoxAndPortrait(sSaveCompleted, 0, monPortraitPtr, 0x101);
                    else
                        CreateDialogueBoxAndPortrait(sSaveCompleted, 0, monPortraitPtr, 0x101);

                    sSavePakWrite->state = 5;
                    break;
                case SAVE_NOT_WRTTEN:
                    CreateDialogueBoxAndPortrait(sSaveNotWritten, 0, 0, 0);
                    sSavePakWrite->state = 6;
                    break;
                default:
                    if (sSavePakWrite->monPortrait.faceFile != NULL)
                        CreateDialogueBoxAndPortrait(sSaveFailed, 0, monPortraitPtr, 0x101);
                    else
                        CreateDialogueBoxAndPortrait(sSaveFailed, 0, monPortraitPtr, 0x101);

                    sSavePakWrite->state = 5;
                    break;
            }
            sub_8014114();
            break;
        case 5:
            if (sub_80144A4(&other_stack) == 0)
                sSavePakWrite->state = 7;
            break;
        case 6:
            break;
        case 7:
            return FALSE;
    }
    return TRUE;
}

u32 GetSavePakStatus(void)
{
    return sSavePakWrite->saveStatus;
}

void FinishWriteSavePak(void)
{
    if (sSavePakWrite != NULL) {
        if (sSavePakWrite->monPortrait.faceFile != NULL)
            CloseFile(sSavePakWrite->monPortrait.faceFile);
        FREE_AND_SET_NULL(sSavePakWrite);
    }
    // When skipping cutscenes, normalize state to postgame after a save.
    // This prevents early "wake" sequences from re-running and ensures
    // the player resumes in Team Base Inside free-roam.
    if (GetSkipCutscenesSetting()) {
        // Clamp to postgame scenario and ground mode
        SetScriptVarValue(0, SCENARIO_MAIN, 19);
        SetScriptVarValue(0, START_MODE, 2); // MODE_GROUND
        // Normalize enter/exit to Team Base Inside
        SetScriptVarValue(0, GROUND_ENTER, MAP_TEAM_BASE_INSIDE);
        SetScriptVarValue(0, GROUND_ENTER_LINK, 0);
        SetScriptVarValue(0, GROUND_GETOUT, MAP_TEAM_BASE_INSIDE);
        // Clear last-enter markers to avoid resume logic misfires
        SetScriptVarValue(0, DUNGEON_ENTER, -1);
        SetScriptVarValue(0, DUNGEON_ENTER_INDEX, -1);
        SetScriptVarValue(0, DUNGEON_RESULT, 0);
        // Clear warp lock if any and set a postgame one-shot guard flag
        SetScriptVarValue(0, WARP_LOCK, 0);
        SetScriptVarArrayValue(0, EVENT_S08E01, 0, 1);
    }
    sub_80993E4();
}
