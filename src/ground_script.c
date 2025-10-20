#include "global.h"
#include "globaldata.h"
#include "ground_script.h"
#include "mgba_log.h"
#include "constants/dungeon.h"
#include "constants/friend_area.h"
#include "constants/item.h"
#include "constants/monster.h"
#include "constants/move_id.h"
#include "code_8002774.h"
#include "music_util.h"
#include "code_8099360.h"
#include "code_8094F88.h"
#include "code_80958E8.h"
#include "code_80972F4.h"
#include "code_8097670.h"
#include "code_80A26CC.h"
#include "constants/script_dungeon_id.h"
#include "debug.h"
#include "dungeon_info.h"
#include "event_flag.h"
#include "exclusive_pokemon.h"
#include "friend_area.h"
#include "ground_link.h"
#include "ground_lives.h"
#include "ground_main.h"
#include "ground_map.h"
#include "ground_map_1.h"
#include "ground_place.h"
#include "ground_script.h"
#include "ground_sprite.h"
#include "input.h"
#include "items.h"
#include "main_loops.h"
#include "math.h"
#include "music.h"
#include "other_random.h"
#include "random.h"
#include "rescue_team_info.h"
#include "save.h"
#include "string_format.h"
#include "text_util.h"
#include "pokemon.h"
#include "wigglytuff_shop1.h"
#include "wonder_mail.h"
#include "palette_util.h"
#include "pokemon_3.h"
#include "memory.h"
#include "script_item.h"
#include "ground_lives_helper.h"
#include "friend_area_dialogue.h"
#include "structs/str_dungeon_setup.h"
#include "ground_map_conversion_table.h"
#include "unk_ds_only_feature.h"
#include "textbox.h"
#include "data_script.h"
#include "story_debug.h"

// Debug helper to dump core script variables around ground station routing
static void DebugDumpCoreVars(const char *tag)
{
    s32 scen = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
    s32 start = (s16)GetScriptVarValue(NULL, START_MODE);
    s32 ge = (s16)GetScriptVarValue(NULL, GROUND_ENTER);
    s32 gel = (u8)GetScriptVarValue(NULL, GROUND_ENTER_LINK);
    s32 go = (s16)GetScriptVarValue(NULL, GROUND_GETOUT);
    s32 gm = (s16)GetScriptVarValue(NULL, GROUND_MAP);
    s32 gp = (s16)GetScriptVarValue(NULL, GROUND_PLACE);
    s32 ds = (s16)GetScriptVarValue(NULL, DUNGEON_SELECT);
    s32 de = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
    s32 dei = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER_INDEX);
    s32 dr = (u8)GetScriptVarValue(NULL, DUNGEON_RESULT);
    s32 baseK = (s8)GetScriptVarValue(NULL, BASE_KIND);
    s32 baseL = (s8)GetScriptVarValue(NULL, BASE_LEVEL);
    s32 warpL = (s8)GetScriptVarValue(NULL, WARP_LOCK);
    s32 s1 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB1);
    s32 s2 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB2);
    s32 s3 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB3);
    s32 s4 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB4);
    s32 s5 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB5);
    s32 s6 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB6);
    s32 s7 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB7);
    s32 s8 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB8);
    s32 s9 = (u8)GetScriptVarValue(NULL, SCENARIO_SUB9);
    s32 e8_0 = GetScriptVarArrayValue(NULL, EVENT_S08E01, 0);
    s32 e8_1 = GetScriptVarArrayValue(NULL, EVENT_S08E01, 1);
    s32 e8_2 = GetScriptVarArrayValue(NULL, EVENT_S08E01, 2);
    s32 e8_3 = GetScriptVarArrayValue(NULL, EVENT_S08E01, 3);
    MGBA_Warnf("[GS] dump(%s): scen=%d start=%d GE=%d GEL=%d GO=%d GM=%d GP=%d DS=%d DE=%d DEI=%d DR=%d BK=%d BL=%d WL=%d s1=%d s2=%d s3=%d s4=%d s5=%d s6=%d s7=%d s8=%d s9=%d E8=[%d,%d,%d,%d]",
               tag, scen, start, ge, gel, go, gm, gp, ds, de, dei, dr, baseK, baseL, warpL,
               s1, s2, s3, s4, s5, s6, s7, s8, s9, e8_0, e8_1, e8_2, e8_3);
}
// Forward declares for helpers used by linear skip functions
extern void sub_80973A8(s32, u32);          // set GO flag
extern void sub_8096488(void);              // seed initial news

// Linear skip mode helpers (alternative to scene-aware skip flow)
// Enable linear flow when SkipCutscenes is ON. Keep the legacy per-scene flow disabled.
static inline bool8 IsSkipLinearMode(void) { return FALSE; }
static inline bool8 UseOldSkipCutsceneFlow(void) { return FALSE; }

// Main-story linear progression order for SkipCutscenes=ON
static const s16 kSkipLinearOrder[] = {
    SCRIPT_DUNGEON_TINY_WOODS,
    SCRIPT_DUNGEON_THUNDERWAVE_CAVE,
    SCRIPT_DUNGEON_MT_STEEL,
    SCRIPT_DUNGEON_SINISTER_WOODS,
    SCRIPT_DUNGEON_SILENT_CHASM,
    SCRIPT_DUNGEON_MT_THUNDER,
    SCRIPT_DUNGEON_GREAT_CANYON,
    SCRIPT_DUNGEON_LAPIS_CAVE,
    SCRIPT_DUNGEON_MT_BLAZE,
    SCRIPT_DUNGEON_MT_BLAZE_PEAK,
    SCRIPT_DUNGEON_FROSTY_FOREST,
    SCRIPT_DUNGEON_FROSTY_GROTTO,
    SCRIPT_DUNGEON_MT_FREEZE,
    SCRIPT_DUNGEON_MT_FREEZE_PEAK,
    SCRIPT_DUNGEON_MAGMA_CAVERN,
    SCRIPT_DUNGEON_MAGMA_CAVERN_PIT,
    SCRIPT_DUNGEON_SKY_TOWER,
    SCRIPT_DUNGEON_SKY_TOWER_SUMMIT,
};

static s16 SkipLinear_FindNext(s16 cleared)
{
    s32 i;
    for (i = 0; i < (s32)(sizeof(kSkipLinearOrder)/sizeof(kSkipLinearOrder[0])); i++) {
        if (kSkipLinearOrder[i] == cleared) {
            if (i + 1 < (s32)(sizeof(kSkipLinearOrder)/sizeof(kSkipLinearOrder[0])))
                return kSkipLinearOrder[i + 1];
            return -1;
        }
    }
    return -1;
}

static void SkipLinear_EnsureInitialGo(void)
{
    if (!IsSkipLinearMode()) return;
    // If no story dungeon is currently GO, seed Tiny Woods as GO and preselect it.
    if (!sub_8097384(SCRIPT_DUNGEON_TINY_WOODS)) {
        // As a simple heuristic, if nothing is conquered yet, force Tiny Woods GO.
        if (!RescueScenarioConquered(SCRIPT_DUNGEON_TINY_WOODS)) {
            sub_80973A8(SCRIPT_DUNGEON_TINY_WOODS, 1);
            {
                s32 twIndex = sub_80A26B8(SCRIPT_DUNGEON_TINY_WOODS);
                if (twIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, twIndex);
            }
            // Seed base menus/news in case intro scenes were skipped entirely.
            if (sub_8096E2C() == 0 && CountFilledMailboxSlots() == 0) {
                sub_8096488();
                sub_80961B4();
            }
        }
    }
}

static inline bool8 SkipLinear_IsSuccess(s32 res)
{
    return (res == 6 || res == 9 || res == 11 || res == 12);
}

// Returns TRUE if it handled a clear and requested a warp.
static bool8 SkipLinear_HandleClearAndWarp(void)
{
    s32 lastRes;
    s32 lastEnter;
    s32 lastEnterNorm;
    s16 cleared;
    s16 next;
    s32 selIndex;

    if (!IsSkipLinearMode()) return FALSE;

    lastRes = (s16)GetScriptVarValue(NULL, DUNGEON_RESULT);
    lastEnter = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
    lastEnterNorm = lastEnter;
    if (lastEnter == 0x50 || lastEnter == 0x51 || lastEnter == 0x52)
        lastEnterNorm = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER_INDEX);

    if (SkipLinear_IsSuccess(lastRes)) {
        cleared = (s16)lastEnterNorm;
        next = SkipLinear_FindNext(cleared);
        if (next != -1) {
            // Mark cleared and advance GO to next.
            if (!RescueScenarioConquered(cleared))
                sub_8097418(cleared, 1);
            if (sub_8097384(cleared))
                sub_80973A8(cleared, 0);
            if (!sub_8097384(next))
                sub_80973A8(next, 1);
            selIndex = sub_80A26B8(next);
            if (selIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, selIndex);
            MGBA_Warnf("[GS] linear-skip: cleared=%d -> next GO=%d", cleared, next);
            // Return to Team Base INSIDE for a consistent loop.
            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
            return TRUE;
        }
    }
    return FALSE;
}

// Starter items helpers
static void GiveStarterSetIfNeeded(void)
{
    // Avoid duplicates by checking for one representative item.
    if (FindItemInInventory(ITEM_ORAN_BERRY) < 0 && FindItemInInventory(ITEM_REVIVER_SEED) < 0) {
        AddItemIdToInventory(ITEM_ORAN_BERRY, FALSE);
        AddItemIdToInventory(ITEM_PECHA_BERRY, FALSE);
        AddItemIdToInventory(ITEM_RAWST_BERRY, FALSE);
        AddItemIdToInventory(ITEM_REVIVER_SEED, FALSE);
        AddItemIdToInventory(ITEM_PECHA_SCARF, FALSE);
        FillInventoryGaps();
    }
}

void GroundMap_Select(s16);
void GroundMap_SelectDungeon(s32, DungeonLocation*, u32);
void GroundMap_GetStationScript(ScriptInfoSmall *out, s16, s32, s32);
void GroundObject_ExecuteScript(s32, ActionUnkIds *, ScriptInfoSmall *);
void GroundEffect_ExecuteScript(s32, ActionUnkIds *, ScriptInfoSmall *);
void GroundLives_Select(s32, s32 group, s32 sector);
void GroundObject_Select(s32, s32 group, s32 sector);
void GroundEffect_Select(s32, s32 group, s32 sector);
void GroundEvent_Select(s32, s32 group, s32 sector);
void GroundLives_Cancel(s32 group, s32 sector);
void GroundObject_Cancel(s32 group, s32 sector);
void GroundEffect_Cancel(s32 group, s32 sector);
void GroundEvent_Cancel(s32 group, s32 sector);
void GroundLives_CancelBlank_1(void);
void GroundObject_CancelBlank(void);
void GroundEffect_CancelBlank(void);
void GroundWeather_Select(s16);
u32 GroundLives_ExecutePlayerScriptActionLives();
s16 GroundObject_Add(s16 id, GroundObjectData*, s16 group, s8 sector);
s16 GroundEffect_Add(s16 id, GroundEffectData*, s16 group, s8 sector);

// Beware of the declarations without specified arguments, returning u32 or s32, these were quickly hacked in to get the code to compile and link
// The return values are almost certainly NOT correct and will need to be rechecked when moving to header files
char sub_8002984(s32, u8);
bool8 sub_802FCF0(void);


void sub_809733C(s16, u32);
void sub_80973A8(s32, u32);
void sub_80975A8(s16, u8);
u32 sub_809A6E4();
u32 sub_809A6F8();
u32 sub_809A768();
void sub_809AFC8(bool8, s32, s32, const char*);
u32 sub_809B028(const MenuItem *, s32 a1_, s32 a2, s32 a3, s32 a4_, const char *text);
bool8 sub_809B1C0(s32, s32, char[12]);
void sub_809B1D4(u8, s32, s32, const char*);
void sub_809D0BC(void);
void sub_809D124(s32, s32, s32);
void sub_809D158(s32, PixelPos*);
void sub_809D170(s32, s32);
void sub_809D190(s32, PixelPos*, s32);
void sub_809D1A8(s32, s32, s32);
void sub_809D1CC(s32, PixelPos*, s32);
void sub_809D1E4(s32, s32, s32);
void sub_809D208(s32, PixelPos*, s32);
void sub_809D220(s32, s32, s32);
void GroundScriptLockJumpZero(s16);
void sub_80A87AC(s32, s32);
void sub_80A8BD8(s16, s32*);
u32 sub_80A8C2C();
u32 GroundLives_IsStarterMon();
Pokemon *sub_80A8D54(s16);
s16 sub_80A8F9C(s32, PixelPos*);
u32 sub_80A9050();
u32 sub_80A9090();
s16 sub_80AC448(s16, PixelPos*);
s32 sub_80AC49C(s16, PixelPos*);
s16 sub_80AD360(s16, PixelPos*);
s16 sub_80AD3B4(s16, PixelPos*);
void DeleteGroundEvents(void);
void DeleteGroundLives(void);
void DeleteGroundObjects(void);
void DeleteGroundEffects(void);
s32 ExecuteScriptCommand(Action *action);
bool8 sub_8099B94(void);
PixelPos SetVecFromDirectionSpeed(s8, s32);
bool8 sub_8098DCC(u32 speed);

void sub_8099220(void *param_1, s32 param_2);
bool8 sub_809B260(void *dst);
bool8 sub_809B18C(s32 *sp);
bool8 sub_809AFFC(u8 *);
bool8 sub_809D234(void);
s32 sub_80A14E8(Action *, u8, u32, s32);
u8 sub_80990EC(struct DungeonSetupInfo *param_1, s32 param_2);

extern u8 GroundObjectsCancelAll(void);
extern u8 GroundEffectsCancelAll(void);
extern u8 GroundLivesCancelAll(void);
extern u8 IsTextboxOpen_809A750(void);
extern Action *sub_80A882C(s32);
extern Action *GroundObject_GetAction(s32);
extern Action *sub_80AD158(s32);
extern void sub_809AB4C(s32, s32);
extern void sub_809ABB4(s32, s32);
extern void sub_809AC18(s32, s32);
extern s16 sub_80A8BBC(s32 id_);

bool8 GroundLivesNotifyAll(s16);
bool8 GroundObjectsNotifyAll(s16);
bool8 GroundEffectsNotifyAll(s16);

void sub_8098C58(void);
void sub_8098CC8(void);
bool8 sub_80961D8(void);
void ResetMailbox(void);
void sub_80963FC(void);
void sub_8096488(void);
bool8 sub_80964B4(void);
s16 sub_80A8C4C();
bool8 sub_8097640();
u8 sub_80964E4();
s32 sub_80A8E9C();
u8 sub_80A8D20();
bool8 sub_80A87E0();
s16 sub_80A8BFC(s16);
void sub_80A8F50(const u8 *buffer, s32, s32 size);
void sub_80A56A0(s32, s32);
void sub_80A56F0(PixelPos *);
void sub_80A5704(PixelPos *);
void sub_80A86C8(s16, s32);
void sub_80AC1B0(s16, s32);
void sub_80AD0C8(s16, s32);
s32 sub_80A5984();
void sub_80A59A0(s32, s32 *, u16);
extern bool8 sub_80A579C(PixelPos *a0, PixelPos *a1);

// For gScriptLocks, gScriptLockConds, gUnlockBranchLabels
#define SCRIPT_LOCKS_ARR_COUNT 129

static EWRAM_DATA s16 gCurrentMap = 0;
static EWRAM_DATA s16 gUnknown_2039A32 = 0;
static EWRAM_DATA s16 gUnknown_2039A34 = 0;
static EWRAM_DATA u8 gAnyScriptLocked = 0;
// Hard to say why the arrays are larger than SCRIPT_LOCKS_ARR_COUNT. Could be unused EWRAM variables or special case indexes.
static ALIGNED(4) EWRAM_DATA u8 gScriptLocks[SCRIPT_LOCKS_ARR_COUNT + 7] = {0};
static ALIGNED(4) EWRAM_DATA u8 gScriptLockConds[SCRIPT_LOCKS_ARR_COUNT + 7] = {0};
static EWRAM_DATA u32 gUnlockBranchLabels[SCRIPT_LOCKS_ARR_COUNT + 1] = {0};
static EWRAM_DATA MenuItem gChoices[9] = {0};
static EWRAM_DATA char sPokeNameBuffer[POKEMON_NAME_LENGTH + 2] = {0};
static EWRAM_DATA u32 gUnknown_2039DA4 = 0;
static EWRAM_DATA u16 gUnknown_2039DA8 = 0;

static EWRAM_INIT int sNumChoices = 0;
static EWRAM_INIT u8 *gUnknown_203B4B0 = NULL;

static const CallbackData sNullCallbackData = {
    .maybeId = 4,
    .getIndex = NULL,
    .getSize = NULL,
    .getHitboxCenter = NULL,
    .getPosHeightAndUnk = NULL,
    .getDirection = NULL,
    .getFlags = NULL,
    .setHitboxPos = NULL,
    .setPositionBounds = NULL,
    .moveReal = NULL,
    .setPosHeight = NULL,
    .setDirection = NULL,
    .setEventIndex = NULL,
    .livesOnlyNullsub = NULL,
    .func38 = NULL,
    .setFlags = NULL,
    .clearFlags = NULL,
    .func44_livesOnlySpriteRelated = NULL,
    .moveRelative = NULL,
    .func4C_spriteRelatedCheck = NULL,
    .func50_spriteRelated = NULL,
};

static const PixelPos sPixelPosZero = {0, 0};

static const ScriptCommand gUnknown_81164E4[] = {
    {0xF6, 0, 0xC5, 0, 0, "../ground/ground_script.c"},
    {0xEF, 0, 0,    0, 0, NULL},
};

static const ScriptCommand *FindLabel(Action *action, s32 r1);
static const ScriptCommand *ResolveJump(Action *action, s32 r1);
static void sub_80A2500(s32 param_1, ActionUnkIds *param_2);
static void sub_80A252C(s32 param_1, ActionUnkIds *param_2);
static void sub_80A2558(s32 param_1, ActionUnkIds *param_2);
static void sub_80A2584(s16 r0, s16 r1);
static void sub_80A2598(s16 r0, s16 r1);
static u32 sub_80A25AC(u16 param_1);

void sub_809D490(void)
{
    UNUSED void *oldPtr = gUnknown_203B4B0; // Needed to match
    gUnknown_203B4B0 = MemoryAlloc(0x400, 6);
    sub_809D4B0();
}

void sub_809D4B0(void)
{
    s32 i;

    gCurrentMap = -1;
    gUnknown_2039A32 = -1;
    gUnknown_2039A34 = -1;
    gAnyScriptLocked = 0;
    for (i = 0; i < SCRIPT_LOCKS_ARR_COUNT; i++) {
        gScriptLocks[i] = 0;
        gScriptLockConds[i] = 0;
        gUnlockBranchLabels[i] = 0;
    }
}

void sub_809D508(void)
{
    FREE_AND_SET_NULL(gUnknown_203B4B0);
}

static void sub_809D520(ActionUnkIds *a0)
{
    a0->unk0 = -1;
}

static Action *sub_809D52C(ActionUnkIds *a0)
{
    switch (a0->unk0) {
        case 0: return 0;
        case 1: return sub_80A882C(a0->unk2);
        case 2: return GroundObject_GetAction(a0->unk2);
        case 3: return sub_80AD158(a0->unk2);
    }
    return NULL;
}

void InitScriptData(ScriptData *a0)
{
    s32 i;

    a0->state = -1;
    a0->savedState = 0;
    a0->script.ptr2 = 0;
    a0->script.ptr = 0;
    a0->savedScript.ptr2 = 0;
    a0->savedScript.ptr = 0;
    a0->curScriptOp = 0;
    a0->curPtr = 0;
    a0->unk22 = -1;
    a0->unk24 = 0;
    a0->unk26 = 0xFF;
    a0->branchDiscriminant = 0;
    a0->unk2A = 0;
    a0->unk2C = 0;
    a0->unk30 = 0;
    for (i = 0; i < 4; i++) {
        a0->localVars.val[i] = 0;
    }
}

void InitAction(Action *a0)
{
    s32 i;

    a0->callbacks = NULL;
    a0->parentObject = NULL;
    a0->group = -1;
    a0->sector = 0xFF;
    sub_809D520(&a0->unkC);

    for (i = 0; i < 4; i++) {
        a0->predefinedScripts[i] = NULL;
    }

    InitScriptData(&a0->scriptData);
    InitScriptData(&a0->scriptData2);
}

void InitActionWithParams(Action *action, const CallbackData *callbacks, void *parent, s32 group, s32 sector)
{
    s32 group_s32 = (s16) group;
    s32 sector_s32 = (s8) sector;

    InitAction(action);

    action->callbacks = callbacks;
    action->parentObject = parent;
    action->group = group_s32;
    action->sector = sector_s32;
    action->unk8.unk0 = callbacks->maybeId;

    if(callbacks->getIndex)
        action->unk8.unk2 = callbacks->getIndex(parent);
    else
        action->unk8.unk2 = 0;
}

void InitAction2(Action *action)
{
    InitAction(action);
}

UNUSED static s16 sub_809D654(Action *action)
{
    return action->scriptData.savedState;
}

UNUSED static s16 sub_809D65C(Action *action)
{
    if(action->scriptData.savedState != 0)
        return action->scriptData.state;
    else
        return -1;
}

bool8 sub_809D678(Action *action)
{
    return action->scriptData.savedState == 0 ? FALSE : TRUE;
}

bool8 sub_809D684(Action *action, ScriptInfoSmall *scriptInfo)
{
    if(action->scriptData.savedState != 0)
    {
        if(action->scriptData.savedScript.ptr != 0)
        {
            if(action->scriptData.savedScript.ptr2 == scriptInfo->ptr) return 1;
        }
        else
        {
            if(action->scriptData.script.ptr2 == scriptInfo->ptr) return 1;
        }
    }
    if(action->scriptData2.savedState != 0)
    {
        if(action->scriptData2.savedScript.ptr != 0)
        {
            if(action->scriptData2.savedScript.ptr2 == scriptInfo->ptr) return 1;
        }
        else
        {
            if(action->scriptData2.script.ptr2 == scriptInfo->ptr) return 1;
        }
    }
    return 0;
}

void SetPredefinedScript(Action *param_1, s16 index, ScriptCommand *param_3)
{
    param_1->predefinedScripts[index] = param_3;
}

bool8 GetPredefinedScript(Action *param_1, ScriptInfoSmall *script, s16 _index)
{
    const ScriptCommand *scriptPtr;
    s32 index = _index;

    scriptPtr = param_1->predefinedScripts[index];
    script->ptr = scriptPtr;
    script->state = index;
    script->group = param_1->group;
    script->sector = param_1->sector;
    return scriptPtr != NULL;
}

void GetFunctionScript(Action *param_1, ScriptInfoSmall *script, s16 index)
{
    s32 index_s32 = index;
    script->ptr = gFunctionScriptTable[index_s32].script;
    script->state = 2;
    if (param_1 != NULL) {
        script->group = param_1->group;
        script->sector = param_1->sector;
    }
    else {
        script->group = -1;
        script->sector = -1;
    }
}

bool8 ActionResetScriptData(Action *param_1, const DebugLocation *unused)
{
    InitScriptData(&param_1->scriptData);
    InitScriptData(&param_1->scriptData2);
    return TRUE;
}

bool8 ActionResetScriptDataForDeletion(Action *param_1, DebugLocation *unused)
{
    InitScriptData(&param_1->scriptData);
    InitScriptData(&param_1->scriptData2);
    param_1->scriptData.savedState = 4;
    return TRUE;
}

bool8 GroundScript_ExecutePP(Action *action, ActionUnkIds *param_2, ScriptInfoSmall *param_3, const DebugLocation *unused)
{
    if ((param_3 == NULL) || (param_3->ptr == NULL)) {
        return FALSE;
    }
    switch(param_3->state) {
        case 2:
        case 3:
            if (action->scriptData.state == 1) {
                action->scriptData2 = action->scriptData;
                break;
            }
            if (action->scriptData.state == 5) {
                InitScriptData(&action->scriptData2);
            }
            break;
        case 5:
            if (action->scriptData.state != 2) {
                FATAL_ERROR_ARGS("../ground/ground_script.c", 688, "execute script type error B");
            }
            if (action->scriptData2.state != -1) {
                FATAL_ERROR_ARGS("../ground/ground_script.c", 689, "execute script type error C");
            }
            action->scriptData2 = action->scriptData;
            break;
        case 0:
            if (action->scriptData.state != 1) {
                InitScriptData(&action->scriptData2);
            }
            else {
                action->scriptData2 = action->scriptData;
            }
            break;
        case 1:
            InitScriptData(&action->scriptData2);
            break;
        default:
            FATAL_ERROR_ARGS("../ground/ground_script.c", 708, "execute script type error %d", param_3->state);
    }
    InitScriptData(&action->scriptData);
    if (param_2 != NULL) {
        action->unkC = *param_2;
    }
    else {
        sub_809D520(&action->unkC);
    }
    action->scriptData.state = param_3->state;
    action->scriptData.savedState = 3;
    action->scriptData.script.group = param_3->group;
    action->scriptData.script.sector = param_3->sector;
    action->scriptData.script.ptr = param_3->ptr;
    action->scriptData.script.ptr2 = param_3->ptr;
    action->scriptData.savedScript.ptr = 0;
    action->scriptData.savedScript.ptr2 = NULL;

    if (action->callbacks->getDirection != 0) {
        action->callbacks->getDirection(action->parentObject, &action->scriptData.unk26);
    }
    return TRUE;
}

bool8 ExecutePredefinedScript(Action *param_1, ActionUnkIds *param_2, s16 index, DebugLocation *debug)
{
    ScriptInfoSmall auStack28;

    GetPredefinedScript(param_1,&auStack28,index);
    return GroundScript_ExecutePP(param_1, param_2, &auStack28, debug);
}

u8 GroundScriptCheckLockCondition(Action *param_1, s16 param_2)
{
    s32 param_2_s32;

    param_2_s32 = param_2;

    if ((param_2 == 0) && (IsTextboxOpen_809A750() == 0)) {
        param_1->scriptData.script.ptr = ResolveJump(param_1, 0);
        return 0;
    }
    else {
        param_1->scriptData.unk22 = param_2_s32;
        param_1->scriptData.savedState = 2;
        gAnyScriptLocked = 1;
        return 1;
    }
}

bool8 GroundScript_Cancel(Action *r0)
{
    // NOTE: Will always return TRUE
    return ActionResetScriptDataForDeletion(r0, DEBUG_LOC_PTR("../ground/ground_script.c", 821, "GroundScript_Cancel"));
}

u8 GroundCancelAllEntities(void)
{
    u8 ret;

    ret = GroundLivesCancelAll();
    ret |= GroundObjectsCancelAll();
    ret |= GroundEffectsCancelAll();
    return ret;
}

bool8 GroundScriptNotify(Action *param_1, s16 param_2)
{
    s16 sVar1;
    s16 sVar2;
    bool8 ret;

    s32 param_2_s16 = param_2;

    ret = FALSE;
    sVar1 = param_1->scriptData.unk22;
    if ((sVar1 != -1) && (sVar1 == param_2_s16)) {
        param_1->scriptData.unk22 = -1;
        ret = TRUE;
    }
    sVar2 = param_1->scriptData2.unk22;
    if ((sVar2 != -1) && (sVar2 == param_2_s16)) {
        param_1->scriptData2.unk22 = -1;
        ret = TRUE;
    }
    return ret;
}

void GroundScriptLockJumpZero(s16 index)
{
    s32 index_s16 = index;
    gScriptLocks[index_s16] = 1;
    gUnlockBranchLabels[index_s16] = 0;
    gAnyScriptLocked = 1;
}

void GroundScriptLock(s16 index, s32 r1)
{
    s32 index_s16 = index;
    gScriptLocks[index_s16] = 1;
    gUnlockBranchLabels[index_s16] = r1;
    gAnyScriptLocked = 1;
}

bool8 GroundScriptLockCond(Action *param_1, s16 index, s32 param_3)
{
    s32 index_s32 = index;
    gUnlockBranchLabels[index_s32] = param_3;
    if (index_s32 == 0) {
        if (IsTextboxOpen_809A750() == 0) {
            return FALSE;
        }
        param_1->scriptData.unk22 = index_s32;
    }
    else {
        param_1->scriptData.unk22 = index_s32 | 0x80;
        gScriptLocks[index_s32] = 1;
        gScriptLockConds[index_s32] = 1;
    }
    param_1->scriptData.savedState = 2;
    gAnyScriptLocked = 1;
    return TRUE;
}

s16 HandleAction(Action *action, DebugLocation *debug)
{
    ScriptCommand cmd;

    if (action->scriptData.savedState) {
        bool8 loopContinue = TRUE;
        while (loopContinue) {
            switch (action->scriptData.savedState) {
                case 2: {
                    switch (action->scriptData.curScriptOp) {
                        // handled cases (all hex)
                        // 3, 4, 5, 6, 7, 22..28, 2c, 30..38, 39, 3a, 3b, 3c, 3d, 3e, 3f,
                        // 58..5b, 5d, 5e, 60, 61..67, 68, 69..6f, 70, 71..76, 77..7c, 7d..82, 83..88,
                        // 89, 8a, 8b..90, 91..95, 98, 99, 9b..a3, cf, d2..d8, da, db, dc, dd, de, df,
                        // e0, e1, e2, e3, e5
                        // other cases up to 0xf1 immediately break (which breaks again and loops for ExScrCmd)
                        case 0x58 ... 0x5b: case 0x5d: case 0x5e: case 0x60:
                        case 0x8b ... 0x90: case 0x98: case 0x99:
                        case 0xdb: case 0xdc: {
                            if (action->scriptData.unk2A > 0) {
                                action->scriptData.unk2A--;
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0xdd: {
                            if (action->callbacks->func4C_spriteRelatedCheck(action->parentObject)) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0xde: {
                            if (action->callbacks->func50_spriteRelated(action->parentObject)) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x22 ... 0x28:
                        case 0xdf: {
                            if (sub_8099B94()) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0xe0: {
                            cmd = *action->scriptData.curPtr;
                            if (IsEqualtoBGTrack(cmd.argShort)) {
                                if (action->scriptData.unk2C++ < 10800) {
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.savedState = 3;
                                }
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0xe1: case 0xe2: {
                            cmd = *action->scriptData.curPtr;
                            if (IsSoundPlaying(cmd.argShort)) {
                                if (action->scriptData.unk2C++ < 3600) {
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.savedState = 3;
                                }
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x61 ... 0x67: {
                            PixelPos pos, pos2;
                            action->callbacks->getHitboxCenter(action->parentObject, &pos);
                            pos2.x = action->scriptData.pos2.x - pos.x;
                            pos2.y = action->scriptData.pos2.y - pos.y;
                            sub_800290C(&pos2, action->scriptData.unk30);
                            if (pos2.x == 0 && pos2.y == 0) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            if (action->callbacks->moveRelative(action->parentObject, &pos2) & 9) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            action->callbacks->setEventIndex(action->parentObject, 0x1000);

                            if (action->scriptData.unk2A >= 0) {
                                if (action->scriptData.unk2A > 0) {
                                    action->scriptData.unk2A--;
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.savedState = 3;
                                }
                            }
                            else {
                                loopContinue = FALSE;
                            }
                            break;
                        }
                        case 0x69 ... 0x6f: {
                            PixelPos pos, pos2;
                            s32 res;
                            s32 dir;
                            UNUSED s32 dirBefore;
                            s8 dirS8;
                            action->callbacks->getHitboxCenter(action->parentObject, &pos);
                            pos2.x = action->scriptData.pos2.x - pos.x;
                            pos2.y = action->scriptData.pos2.y - pos.y;
                            sub_800290C(&pos2, action->scriptData.unk30);
                            if (pos2.x == 0 && pos2.y == 0) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            res = action->callbacks->moveRelative(action->parentObject, &pos2);
                            dir = (s8) VecDirection8Radial(&pos2);
                            dirBefore = action->scriptData.unk26;
                            dirS8 = dir;
                            action->scriptData.unk26 = dirS8;
                            action->callbacks->setDirection(action->parentObject, dir);
                            if (res & 9) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            action->callbacks->setEventIndex(action->parentObject, 0x1000);

                            if (action->scriptData.unk2A >= 0) {
                                if (action->scriptData.unk2A > 0) {
                                    action->scriptData.unk2A--;
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.savedState = 3;
                                }
                            }
                            else {
                                loopContinue = FALSE;
                            }
                            break;
                        }
                        case 0x71 ... 0x76:
                        case 0x7d ... 0x82: {
                            if (action->scriptData.unk2A > 0) {
                                PixelPos pos;
                                sub_8002934(&pos, &action->scriptData.pos1, &action->scriptData.pos2, action->scriptData.unk2A, action->scriptData.unk2C);
                                action->callbacks->moveReal(action->parentObject, &pos);
                                action->callbacks->setEventIndex(action->parentObject, 0x1000);
                                action->scriptData.unk2A--;
                                action->scriptData.unk2C++;
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x77 ... 0x7c:
                        case 0x83 ... 0x88: {
                            if (action->scriptData.unk2A > 0) {
                                PixelPos pos1, pos2, pos3;
                                s32 dir;
                                UNUSED s32 dirBefore;
                                s8 dirS8;

                                action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                                sub_8002934(&pos2, &action->scriptData.pos1, &action->scriptData.pos2, action->scriptData.unk2A, action->scriptData.unk2C);
                                pos3.x = pos2.x - pos1.x;
                                pos3.y = pos2.y - pos1.y;
                                dir = (s8) VecDirection8Radial(&pos3);
                                dirBefore = action->scriptData.unk26;
                                dirS8 = dir;
                                action->scriptData.unk26 = dirS8;
                                action->callbacks->setDirection(action->parentObject, dir);
                                action->callbacks->moveReal(action->parentObject, &pos2);
                                action->callbacks->setEventIndex(action->parentObject, 0x1000);
                                action->scriptData.unk2A--;
                                action->scriptData.unk2C++;
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x68: case 0x70: {
                            s32 height, unk;
                            s32 delta;
                            action->callbacks->getPosHeightAndUnk(action->parentObject, &height, &unk);
                            delta = action->scriptData.unk48 - height;
                            if (delta == 0) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            // oh wow, cmn instruction
                            height += (delta < -action->scriptData.unk30) ? -action->scriptData.unk30 :
                                      (delta > action->scriptData.unk30) ? action->scriptData.unk30 : delta;
                            action->callbacks->setPosHeight(action->parentObject, height);
                            loopContinue = FALSE;
                            break;
                        }
                        case 0x89: case 0x8a: {
                            PixelPos pos;
                            s8 dir;
                            action->callbacks->getDirection(action->parentObject, &dir);
                            pos = SetVecFromDirectionSpeed(dir, action->scriptData.unk30);

                            if (action->callbacks->moveRelative(action->parentObject, &pos) & 9) {
                                action->scriptData.savedState = 3;
                                break;
                            }
                            action->callbacks->setEventIndex(action->parentObject, 0x1000);

                            if (action->scriptData.unk2A > 0) {
                                action->scriptData.unk2A--;
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x91 ... 0x95: {
                            if (action->scriptData.unk2A > 0) {
                                action->scriptData.unk2A--;
                                loopContinue = FALSE;
                            }
                            else {
                                PixelPos pos1, pos2, pos3, pos4;
                                UNUSED s32 tmp1;
                                s32 tmp2;
                                s8 dir;
                                bool8 flag;
                                cmd = *action->scriptData.curPtr;
                                tmp1 = -1;
                                tmp2 = -1;
                                flag = FALSE;
                                action->callbacks->getDirection(action->parentObject, &dir);
                                // arg1h synthetic
                                switch (cmd.op) {
                                    case 0x91: case 0x92: {
                                        tmp2 = (s8) action->scriptData.unk4D;
                                        break;
                                    }
                                    case 0x93: {
                                        s16 res;
                                        res = sub_80A7AE8((s16)cmd.arg1);
                                        if (res >= 0) {
                                            flag = TRUE;
                                            sub_80A8FD8(res, &pos1);
                                            sub_80A8F9C(res, &pos2);
                                        }
                                        break;
                                    }
                                    case 0x94: {
                                        s32 res;
                                        res = (s16)sub_80A7AE8((s16)cmd.arg1);
                                        if (res >= 0) {
                                            flag = TRUE;
                                            sub_80A8FD8(res, &pos1);
                                            pos2 = sPixelPosZero;
                                        }
                                        break;
                                    }
                                    case 0x95: {
                                        flag = TRUE;
                                        action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                                        action->callbacks->getSize(action->parentObject, &pos2);
                                        GroundLink_GetPos((s16)cmd.arg1, &pos1);
                                        break;
                                    }
                                }
                                if (flag) {
                                    action->callbacks->getHitboxCenter(action->parentObject, &pos3);
                                    action->callbacks->getSize(action->parentObject, &pos4);
                                    tmp2 = SizedDeltaDirection8(&pos3, &pos4, &pos1, &pos2);
                                    if (tmp2 == -1) {
                                        tmp2 = SizedDeltaDirection4(&pos3, &sPixelPosZero, &pos1, &sPixelPosZero);
                                    }
                                }
                                if (tmp2 == -1 || tmp2 == dir) {
                                    action->scriptData.savedState = 3;
                                    break;
                                }

                                ASM_MATCH_TRICK(dir);
                                action->scriptData.unk26 = sub_8002A70(dir, tmp2, (u8)cmd.argShort);
                                action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                                action->scriptData.unk2A = cmd.argByte;
                            }
                            break;
                        }
                        case 0x9b ... 0xa3: {
                            if (sub_809D234()) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0xe3: case 0xe5: {
                            if (action->scriptData.unk22 != -1) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.script.ptr = ResolveJump(action, gUnlockBranchLabels[action->scriptData.branchDiscriminant]);
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        case 0x2c: case 0x30 ... 0x38: {
                            if (action->scriptData.unk22 != -1) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                                sub_80A87AC(0, 0);
                            }
                            break;
                        }
                        case 0x39: {
                            if (action->scriptData.unk22 != -1) {
                                loopContinue = FALSE;
                            }
                            else if (sub_8099B94()) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                                sub_80A87AC(0, 0);
                            }
                            break;
                        }
                        case 0x3b: {
                            s32 tmp;
                            cmd = *action->scriptData.curPtr;
                            tmp = sub_80A14E8(action, cmd.argByte, cmd.argShort, cmd.arg1);
                            if (tmp < 0) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.savedState = 3;
                                action->scriptData.script.ptr = ResolveJump(action, tmp);
                            }
                            break;
                        }
                        case 0x3c: {
                            s32 val;
                            if (!sub_809B260(&val)) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.script.ptr = ResolveJump(action, val);
                                action->scriptData.savedState = 3;
                                sub_80A87AC(0, 0);
                            }
                            break;
                        }
                        case 0x03: {
                            s32 val;
                            if (!sub_809B260(&val)) {
                                loopContinue = FALSE;
                                break;
                            }
                            if (val >= 0) {
                                cmd = *action->scriptData.curPtr;
                                GroundMainGroundRequest((s16)FriendAreaIdToMapId((u8)val), 0, cmd.argShort);
                            }
                            action->scriptData.script.ptr = ResolveJump(action, val);
                            action->scriptData.savedState = 3;
                            sub_80A87AC(0, 0);
                            break;
                        }
                        case 0x04: {
                            if (action->scriptData.branchDiscriminant == 0) {
                                s32 val;
                                if (!sub_809B260(&val)) {
                                    loopContinue = FALSE;
                                    break;
                                }
                                if (val >= 0) {
                                    SetScriptVarValue(NULL, 18, sub_80A26B8(val));
                                    action->scriptData.branchDiscriminant = 1;
                                } else {
                                    action->scriptData.branchDiscriminant = -1;
                                }
                                sub_80A87AC(0, 0);
                                break;
                            }
                            if (action->scriptData.branchDiscriminant == 1) {
                                s32 dungeonSelect;
                                s32 dungeonEnterElt;
                                cmd = *action->scriptData.curPtr;
                                dungeonSelect = (s16) GetScriptVarValue(NULL, 18);
                                dungeonEnterElt = GetScriptVarArrayValue(NULL, 48, (u16) dungeonSelect);
                                if (dungeonSelect != 19 || dungeonEnterElt != 0) {
                                    sub_8098DCC(cmd.argShort);
                                    action->scriptData.script.ptr = ResolveJump(action, 0);
                                    sub_80A87AC(0, 0);
                                    action->scriptData.savedState = 3;
                                    break;
                                }
                                SetScriptVarValue(NULL, 19, 19);
                            }
                            // fallthrough
                        }
                        case 0x06: {
                            if (action->scriptData.branchDiscriminant == 0) {
                                s32 val;
                                s16 disc;
                                if (!sub_809B260(&val)) {
                                    loopContinue = FALSE;
                                    break;
                                }
                                if (val >= 0) {
                                    SetScriptVarValue(NULL, 19, action->scriptData.curScriptOp == 4 ? sub_80A26B8(val) : (s16)val);
                                    disc = 1;
                                } else {
                                    disc = -1;
                                }
                                action->scriptData.branchDiscriminant = disc;
                                sub_80A87AC(0, 0);
                                break;
                            }
                            // fallthrough
                        }
                        case 0x07: {
                            if (action->scriptData.branchDiscriminant == 1) {
                                s32 dungeonEnter;
                                u32 res;
                                struct DungeonSetupInfo unkStruct;
                                dungeonEnter = (s16)GetScriptVarValue(NULL, 19);
                                if (sub_80990EC(&unkStruct, dungeonEnter)) {
                                    s32 val;
                                    sub_8099220(&unkStruct, dungeonEnter);
                                    val = sub_80023E4(6);
                                    res = BufferDungeonRequirementsText(unkStruct.sub0.unk0.id, 0, gUnknown_203B4B0, val, FALSE);
                                    gUnknown_2039DA4 = res;
                                    switch (res) {
                                        case 2: {
                                            action->scriptData.branchDiscriminant = 2;
                                            sub_809B1C0(9, 1, gUnknown_203B4B0);
                                            if (GroundScriptCheckLockCondition(action, 1)) {
                                                sub_80A87AC(0, 11);
                                            }
                                            break;
                                        }
                                        case 0: {
                                            action->scriptData.branchDiscriminant = 3;
                                            break;
                                        }
                                        case 1: {
                                            action->scriptData.branchDiscriminant = 4;
                                            sub_809B1C0(10, 1, gUnknown_203B4B0);
                                            if (GroundScriptCheckLockCondition(action, 1)) {
                                                sub_80A87AC(0, 11);
                                            }
                                            break;
                                        }
                                        default: {
                                            action->scriptData.branchDiscriminant = -1;
                                            break;
                                        }
                                    }
                                    break;
                                }
                                else {
                                    action->scriptData.branchDiscriminant = 3;
                                    break;
                                }
                            }
                        case 0x05:
                            if (action->scriptData.branchDiscriminant == 1) {
                                unkStruct_203B480 *ret;
                                s32 val;
                                u32 res;
                                ret = GetMailatIndex(GetScriptVarValue(NULL, 20));
                                val = sub_80023E4(6);
                                res = BufferDungeonRequirementsText(ret->dungeonSeed.location.id, 0, gUnknown_203B4B0, val, TRUE);
                                gUnknown_2039DA4 = res;
                                switch (res) {
                                    case 2: {
                                        action->scriptData.branchDiscriminant = 2;
                                        sub_809B1C0(9, 1, gUnknown_203B4B0);
                                        if (GroundScriptCheckLockCondition(action, 1)) {
                                            sub_80A87AC(0, 11);
                                        }
                                        break;
                                    }
                                    case 0: {
                                        action->scriptData.branchDiscriminant = 3;
                                        break;
                                    }
                                    case 1: {
                                        action->scriptData.branchDiscriminant = 4;
                                        sub_809B1C0(10, 1, gUnknown_203B4B0);
                                        if (GroundScriptCheckLockCondition(action, 1)) {
                                            sub_80A87AC(0, 11);
                                        }
                                        break;
                                    }
                                    default: {
                                        action->scriptData.branchDiscriminant = -1;
                                        break;
                                    }
                                }
                                break;
                            }
                            if (action->scriptData.branchDiscriminant == 2) {
                                u32 val;
                                if (!sub_809B260(&val)) {
                                    loopContinue = FALSE;
                                    break;
                                }

                                if (val != 0) {
                                    action->scriptData.branchDiscriminant = 3;
                                    sub_80A87AC(0, 0);
                                    ScriptClearTextbox();
                                    break;
                                }
                                else {
                                    action->scriptData.branchDiscriminant = -1;
                                    sub_80A87AC(0, 0);
                                    ScriptClearTextbox();
                                    break;
                                }
                            }
                            if (action->scriptData.branchDiscriminant == 3) {
                                cmd = *action->scriptData.curPtr;
                                action->scriptData.savedState = 3;
                                sub_80999FC(cmd.argShort);
                                GroundMap_ExecuteEvent(0x70, 0);
                                if (action->unk8.unk0 == 0) continue;
                                action->scriptData.script.ptr = ResolveJump(action, 1);
                                break;
                            }
                            if (action->scriptData.branchDiscriminant == 4) {
                                u32 val;
                                if (!sub_809B260(&val)) {
                                    loopContinue = FALSE;
                                    break;
                                }
                                action->scriptData.branchDiscriminant = -1;
                                sub_80A87AC(0, 0);
                                ScriptClearTextbox();
                                break;
                            }

                            sub_8099220(0, 0);
                            SetScriptVarValue(NULL, 19, -1);
                            action->scriptData.script.ptr = ResolveJump(action, -1);
                            action->scriptData.savedState = 3;
                            break;
                        }
                        case 0x3d: {
                            s32 val;
                            cmd = *action->scriptData.curPtr;
                            if (!sub_809B260(&val)) {
                                loopContinue = FALSE;
                                break;
                            }
                            if (val == 1) {
                                s32 id = (s16)cmd.arg1;
                                if (id != -1) {
                                    Pokemon *mon = sub_80A8D54(id);
                                    s32 i;
                                    for (i = 0; i < POKEMON_NAME_LENGTH; i++) {
                                        mon->name[i] = sPokeNameBuffer[i];
                                    }
                                }
                            }
                            action->scriptData.script.ptr = ResolveJump(action, val);
                            action->scriptData.savedState = 3;
                            sub_80A87AC(0, 0);
                            break;
                        }
                        case 0x3e: {
                            s32 val;
                            if (!sub_809B260(&val)) {
                                loopContinue = FALSE;
                                break;
                            }
                            if (val == 1) {
                                SetRescueTeamName(sPokeNameBuffer);
                            }
                            action->scriptData.script.ptr = ResolveJump(action, val);
                            action->scriptData.savedState = 3;
                            sub_80A87AC(0, 0);
                            break;
                        }
                        case 0x3f: {
                            s32 val;
                            const u8 *ptr;
                            cmd = *action->scriptData.curPtr;
                            ptr = cmd.argPtr;
                            if (sub_809B260(&val)) {
                                if (val == 1) {
                                    s32 i;
                                    u8 name[16];
                                    for (i = 0; i < 16; i++) {
                                        name[i] = '\0';
                                    }

                                    CopyStringtoBuffer(sPokeNameBuffer, name);
                                    for (i = 0; i < 16; i++) {
                                        if (name[i] != ptr[i]) {
                                            val = 2;
                                            break;
                                        }
                                        if (name[i] == '\0')
                                            break;
                                    }
                                }
                                action->scriptData.script.ptr = ResolveJump(action, val);
                                action->scriptData.savedState = 3;
                                sub_80A87AC(0, 0);
                            }
                            else {
                                loopContinue = FALSE;
                            }
                            break;
                        }
                        case 0x3a: {
                            if (action->scriptData.unk2C == 0) {
                                if (action->scriptData.unk22 != -1) {
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.unk2C++;
                                }
                            }
                            else {
                                bool8 c;
                                if (sub_809AFFC(&c)) {
                                    if (c) {
                                        cmd = *action->scriptData.curPtr;
                                        action->scriptData.script.ptr = FindLabel(action, cmd.argShort);
                                    }
                                    action->scriptData.savedState = 3;
                                    sub_80A87AC(0, 0);
                                }
                                else {
                                    loopContinue = FALSE;
                                }
                            }
                            break;
                        }
                        case 0xcf: {
                            cmd = *action->scriptData.curPtr;
                            if (action->scriptData.unk2C == 0) {
                                bool8 flag = FALSE;
                                if (action->scriptData.branchDiscriminant < 0) {
                                    if (action->scriptData.script.ptr->op != 0xd1) {
                                        action->scriptData.savedState = 3;
                                        break;
                                    }
                                }
                                else {
                                    while (action->scriptData.script.ptr->op == 0xd0) {
                                        if (action->scriptData.script.ptr->argShort == action->scriptData.branchDiscriminant) {
                                            flag = TRUE;
                                            break;
                                        }
                                        action->scriptData.script.ptr++;
                                    }
                                    if (!flag) {
                                        while (action->scriptData.script.ptr->op == 0xd1) {
                                            action->scriptData.script.ptr++;
                                        }
                                        action->scriptData.savedState = 3;
                                        break;
                                    }
                                }
                                if (ScriptPrintText(cmd.argByte, (s16) cmd.arg1, action->scriptData.script.ptr->argPtr)) {
                                    sub_80A87AC(0,10);
                                    if (GroundScriptCheckLockCondition(action, 0)) {
                                        action->scriptData.unk2C = 1;
                                    }
                                }
                                action->scriptData.script.ptr++;
                            }
                            else {
                                if (action->scriptData.unk22 != -1) {
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.unk2C = 0;
                                    sub_80A87AC(0, 0);
                                }
                            }
                            break;
                        }
                        case 0xd2 ... 0xd8: {
                            s32 tmp;
                            if (action->scriptData.unk2C == 0) {
                                if (action->scriptData.unk22 != -1) {
                                    loopContinue = FALSE;
                                }
                                else {
                                    action->scriptData.unk2C++;
                                }
                            }
                            else if (sub_809B18C(&tmp)) {
                                if (tmp > 0) {
                                    cmd = *(action->scriptData.curPtr + action->scriptData.branchDiscriminant + tmp);
                                    action->scriptData.script.ptr = FindLabel(action, cmd.argShort);
                                }
                                action->scriptData.savedState = 3;
                                sub_80A87AC(0, 0);
                            }
                            else {
                                loopContinue = FALSE;
                            }
                            break;
                        }
                        case 0xda: {
                            if (action->scriptData.unk22 != -1) {
                                loopContinue = FALSE;
                            }
                            else {
                                action->scriptData.script.ptr = ResolveJump(action, gUnlockBranchLabels[1]);
                                action->scriptData.savedState = 3;
                            }
                            break;
                        }
                        default:
                        case 0x08 ... 0x21: case 0x29 ... 0x2b: case 0x2d ... 0x2f: case 0x40 ... 0x57: case 0x5c: case 0x5f:
                        case 0x96: case 0x97: case 0x9a: case 0xa4 ... 0xce: case 0xd0: case 0xd1: case 0xd9: case 0xe4: case 0xe6 ... 0xf0: {
                            loopContinue = FALSE;
                            break;
                        }
                    }
                    break;
                }
                case 3: {
                    s32 state = ExecuteScriptCommand(action);
                    action->scriptData.savedState = state;
                    switch ((s16)state) {
                        case 0: {
                            if (action->scriptData2.savedState) {
                                action->scriptData = action->scriptData2;
                                if (action->callbacks->setDirection && action->scriptData.unk26 != -1) {
                                    u32 tmp;
                                    action->callbacks->getFlags(action->parentObject, &tmp);
                                    if (tmp & 0x400) {
                                        action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                                    }
                                }
                                if (action->callbacks->setEventIndex) {
                                    action->callbacks->setEventIndex(action->parentObject, action->scriptData.unk24);
                                }
                                InitScriptData(&action->scriptData2);
                            }
                            else {
                                InitScriptData(&action->scriptData);
                            }
                            return 3;
                        }
                        case 3: return 3;
                        case 4: return 4;
                        case 1: return 1;
                        case 2: default: {
                            action->scriptData.unk2C = 0;
                            break;
                        }
                    }
                    break;
                }
                case 0: case 1: case 4: {
                    return action->scriptData.savedState;
                }
            }
        }
        return 3;
    }
    else {
        return 0;
    }
}

// overlay_0000.bin::021497FC
// Return values:
// This function returns what's likely an enum, which controls the state of the script engine state machine, and possibly provides information to code calling the engine.
// The enum is shared at least with HandleAction.
// This value is saved into the state field of the Action when returned from this function.
// - Value 0 indicates a RET on script engine level, HandleAction copies action->scriptData2 onto action->scriptData, and reinitializes scriptData2.
//     If there is no active scriptData2 it clears and reinits scriptData.
//     Ultimately returns code 3 to the script engine caller.
//     Caveat: If the scripting engine is in state 0, HandleAction will immediately return 0 without performing any work.
// - Value 1 is a terminal state (script success? error?), no further scripting progress will happen. This code is always returned to the caller from now on.
// - Value 2 gives control back to the HandleAction function (entry point into the scripting engine state machine)
//     This is the only return value that does not return to the script engine caller
// - Value 3 returns to the caller, but will give control back to ExecuteScriptCommand when reentering the script ("script not finished")
// - Value 4 is some kind of fatal error state, no further scripting progress will happen. This code is always returned to the caller from now on.
s32 ExecuteScriptCommand(Action *action)
{
    ScriptCommand curCmd;
    ScriptData *scriptData = &action->scriptData;

    while (1) {
        scriptData->curPtr = scriptData->script.ptr;
        curCmd = *scriptData->script.ptr++;
        switch (scriptData->curScriptOp = curCmd.op) {
            case 0x01: {
                u32 arg = (s16)curCmd.arg1;
                u32 argCopy = arg;
                u32 byte = (u8)curCmd.argByte;
                if (ScriptLoggingEnabled(TRUE)) {
                    Log(1, "    ground select %3d[%s] %3d", arg, gGroundMapConversionTable[arg].text, byte);
                }
                GroundMainGroundRequest(argCopy, byte, curCmd.argShort);
                break;
            }
            case 0x02: {
                s32 dungeonId = (s16)curCmd.arg1;
                if (dungeonId == -1) dungeonId = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
                if (ScriptLoggingEnabled(TRUE)) {
                    Log(1, "    dungeon select %3d", dungeonId);
                }
                if (dungeonId != -1) {
                    GroundMainRescueRequest(dungeonId, curCmd.argShort);
                    action->scriptData.script.ptr = ResolveJump(action, 1);
                } else {
                    action->scriptData.script.ptr = ResolveJump(action, 0);
                }
                break;
            }
            case 0x03: {
                if (sub_8021700(curCmd.arg1)) {
                    action->scriptData.script.ptr = ResolveJump(action, -1);
                } else {
                    sub_8098D80(curCmd.argShort);
                    action->scriptData.script.ptr = ResolveJump(action, 0);
                }
                break;
            }
            case 0x04: {
                if (curCmd.arg1 == -1) {
                    if (!(u8)sub_802FCF0() && (u8)sub_809B1C0(12,0,0)) {
                        sub_80A87AC(0, 11);
                        action->scriptData.branchDiscriminant = 0;
                    } else {
                        action->scriptData.branchDiscriminant = -1;
                    }
                    return 2; // do action
                } else {
                    SetScriptVarValue(NULL, DUNGEON_SELECT, sub_80A26B8((s16)curCmd.arg1));
                    action->scriptData.branchDiscriminant = 1;
                    return 2; // do action
                }
                break;
            }
            case 0x06: {
                if (curCmd.arg1 == -1) {
                    if ((s8)sub_809B1C0(36, 0, 0)) {
                        sub_80A87AC(0, 11);
                        action->scriptData.branchDiscriminant = 0;
                        return 2;
                    } else {
                        action->scriptData.branchDiscriminant = -1;
                        return 2;
                    }
                } else {
                    SetScriptVarValue(NULL, DUNGEON_ENTER, curCmd.arg1);
                    action->scriptData.branchDiscriminant = 1;
                    return 2;
                }
            }
            case 0x05: {
                action->scriptData.branchDiscriminant = 1;
                return 2;
            }
            case 0x07: {
                s32 tmp = (s16)curCmd.arg1;
                if (tmp == -1) {
                    tmp = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
                } else {
                    SetScriptVarValue(NULL, DUNGEON_ENTER, tmp);
                }
                if (ScriptLoggingEnabled(TRUE)) {
                    Log(1, "    dungeon enter check %3d", tmp);
                }
                if (tmp != -1) {
                    action->scriptData.branchDiscriminant = 1;
                    return 2;
                } else {
                    action->scriptData.branchDiscriminant = -1;
                    return 2;
                }
            }
            case 0x08: case 0x09: {
                if (curCmd.op == 0x08) {
                    gCurrentMap = curCmd.arg1;
                    gUnknown_2039A32 = GetAdjustedGroundMap((s16)curCmd.arg1);
                    gUnknown_2039A34 = gUnknown_2039A32;
                    if (ScriptLoggingEnabled(TRUE)) {
                        Log(1,"    map select %3d %3d[%s]",gCurrentMap,gUnknown_2039A32,
                            gGroundMapConversionTable[gCurrentMap].text);
                    }
                } else {
                    gUnknown_2039A32 = gCurrentMap = curCmd.arg1;
                    gUnknown_2039A34 = curCmd.arg1;
                    if (ScriptLoggingEnabled(TRUE)) {
                        Log(1,"    ground select %3d %3d[%s]",gCurrentMap,gUnknown_2039A32,
                            gGroundMapConversionTable[gCurrentMap].text);
                    }
                }
                SetScriptVarValue(NULL,GROUND_MAP,gCurrentMap);
                SetScriptVarValue(NULL,GROUND_PLACE,gGroundMapConversionTable[gCurrentMap].groundPlaceId);
                GroundSprite_Reset(gUnknown_2039A32);
                sub_809D0BC();
                DeleteGroundEvents();
                DeleteGroundLives();
                DeleteGroundObjects();
                DeleteGroundEffects();
                sub_809C770(gCurrentMap, gGroundMapConversionTable[gCurrentMap].groundPlaceId);
                GroundMap_Select(gUnknown_2039A32);
                GroundLink_Select(gUnknown_2039A32);
                GroundLives_Select(gUnknown_2039A32,0,0);
                GroundObject_Select(gUnknown_2039A32,0,0);
                break;
            }
            case 0x0a: {
                const DungeonInfo *tmp;
                DungeonLocation loc;
                tmp = GetDungeonInfo_80A2608((s16)curCmd.arg1);
                gUnknown_2039A34 = gUnknown_2039A32 = gCurrentMap = (s16)curCmd.arg2;
                if (ScriptLoggingEnabled(TRUE)) {
                    Log(1, "    dungeon select %3d %3d[%s]", gCurrentMap,gUnknown_2039A32,
                        gGroundMapConversionTable[gCurrentMap].text);
                }
                GroundSprite_Reset(gUnknown_2039A32);
                sub_809D0BC();
                DeleteGroundEvents();
                DeleteGroundLives();
                DeleteGroundObjects();
                DeleteGroundEffects();
                loc.id = tmp->dungeonIndex;
                loc.floor = curCmd.argShort;
                GroundMap_SelectDungeon(gUnknown_2039A32, &loc, curCmd.argByte);
                GroundLink_Select(gUnknown_2039A32);
                GroundLives_Select(gUnknown_2039A32,0,0);
                GroundObject_Select(gUnknown_2039A32,0,0);
                break;
            }
            case 0x0b: {
                GroundWeather_Select((s16)curCmd.arg1);
                break;
            }
            case 0x0c ... 0x15: {
                s32 group;
                s32 sector;
                {
                    s32 tmp = curCmd.argShort < 0 ? scriptData->script.group : curCmd.argShort;
                    group = tmp;
                }
                {
                    s32 tmp = (s8)curCmd.argByte < 0 ? scriptData->script.sector : (s8)curCmd.argByte;
                    sector = tmp;
                }
                switch (curCmd.op) {
                    case 0x0c: {
                        GroundLives_Select(gUnknown_2039A32, group, sector);
                        GroundObject_Select(gUnknown_2039A32, group, sector);
                        GroundEffect_Select(gUnknown_2039A32, group, sector);
                        GroundEvent_Select(gUnknown_2039A32, group, sector);
                        break;
                    }
                    case 0x0d: {
                        GroundLives_Select(gUnknown_2039A32, group, sector);
                        break;
                    }
                    case 0x0e: {
                        GroundObject_Select(gUnknown_2039A32, group, sector);
                        break;
                    }
                    case 0x0f: {
                        GroundEffect_Select(gUnknown_2039A32, group, sector);
                        break;
                    }
                    case 0x10: {
                        GroundEvent_Select(gUnknown_2039A32, group, sector);
                        break;
                    }
                    case 0x11: {
                        GroundLives_Cancel(group, sector);
                        GroundObject_Cancel(group, sector);
                        GroundEffect_Cancel(group, sector);
                        GroundEvent_Cancel(group, sector);
                        break;
                    }
                    case 0x12: {
                        GroundLives_Cancel(group, sector);
                        break;
                    }
                    case 0x13: {
                        GroundObject_Cancel(group, sector);
                        break;
                    }
                    case 0x14: {
                        GroundEffect_Cancel(group, sector);
                        break;
                    }
                    case 0x15: {
                        GroundEvent_Cancel(group, sector);
                        break;
                    }
                }
                break;
            }
            case 0x16: {
                GroundLives_CancelBlank_1();
                break;
            }
            case 0x17: {
                GroundObject_CancelBlank();
                break;
            }
            case 0x18: {
                GroundEffect_CancelBlank();
                break;
            }
            case 0x19: {
                s8 unk[4];
                GroundObjectData *obj;
                PixelPos pos;
                s16 res;
                s32 group;
                s32 sector;
                action->callbacks->getDirection(action->parentObject, unk);
                obj = ({ GroundObjectData obj = {
                    .unk1 = *unk,
                    .width = 1,
                    .height = 1,
                    .pos = {},
                    .kind = curCmd.arg2,
                    .scripts = { [3] = gFunctionScriptTable[curCmd.arg1].script },
                    };
                    group = curCmd.argShort < 0 ? scriptData->script.group : curCmd.argShort;
                    sector = (s8)curCmd.argByte < 0 ? scriptData->script.sector : (s8)curCmd.argByte;
                    &obj;
                });
                res = GroundObject_Add(-1, obj, group, sector);
                if (res >= 0) {
                    action->callbacks->getHitboxCenter(action->parentObject, &pos);
                    sub_80AC49C(res, &pos);
                }
                break;
            }
            case 0x1a: {
                s8 unk;
                GroundEffectData *eff;
                PixelPos pos;
                s16 res;
                s32 group;
                s32 sector;
                action->callbacks->getDirection(action->parentObject, &unk);
                eff = ({ GroundEffectData eff = {
                    .unk1 = unk,
                    .width = 1,
                    .height = 1,
                    .pos = {},
                    .kind = curCmd.arg2,
                    .script = gFunctionScriptTable[curCmd.arg1].script,
                    };
                    group = curCmd.argShort < 0 ? scriptData->script.group : curCmd.argShort;
                    sector = (s8)curCmd.argByte < 0 ? scriptData->script.sector : (s8)curCmd.argByte;
                    &eff;
                });
                res = GroundEffect_Add(-1, eff, group, sector);
                if (res >= 0) {
                    action->callbacks->getHitboxCenter(action->parentObject, &pos);
                    sub_80AD3B4(res, &pos);
                }
                break;
            }
            case 0x1c: {
                // EXECUTE_FUNCTION* (always allow; targeted skips handled elsewhere)
                GroundMap_ExecuteEvent(curCmd.argShort, 1);
                break;
            }
            case 0x1b: {
                // EXECUTE_FUNCTION (always allow; targeted skips handled elsewhere)
                GroundMap_ExecuteEvent(curCmd.argShort, 0);
                break;
            }
            case 0x1d: case 0x1e: {
                s32 map;
                s32 group, sector;
                bool8 res;
                {
                    s32 tmp = curCmd.arg1 != -1 ? (s16)curCmd.arg1 : gCurrentMap;
                    map = tmp;
                }
                {
                    s32 tmp = curCmd.argShort < 0 ? scriptData->script.group : curCmd.argShort;
                    group = tmp;
                }
                {
                    s32 tmp = (s8)curCmd.argByte < 0 ? scriptData->script.sector : (s8)curCmd.argByte;
                    sector = tmp;
                }
                map = GetAdjustedGroundMap(map);
                DebugDumpCoreVars("pre-station");
                DumpStoryFlags("pre-station");

                // SkipCutscenes no longer alters station execution.
                if (0 && GetSkipCutscenesSetting()) {
                    s32 placeNow = gGroundMapConversionTable[map].groundPlaceId;
                    // Sky Tower: skip the long entry cutscene (g1 s0) and jump to the
                    // streamlined entry that immediately hands off to the dungeon (g2 s0).
                    if (placeNow == GROUND_PLACE_SKY_TOWER && group == 1 && sector == 0) {
                        MGBA_Warnf("[GS] skip: Sky Tower entry g1 s0 -> redirect to g2 s0 (skip intro)");
                        group = 2;
                        sector = 0;
                    }
                    // In Team Base Inside, suppress various morning/wake/control stations.
                    // Known stations from logs to suppress: g41 (wake), g42 (dream), g45 (postgame morning),
                    // and 6/8/11 control stations that run on postgame wake without visible cutscenes.
                    if (placeNow == GROUND_PLACE_TEAM_BASE_INSIDE) {
                        // Post-save guard: after saving, ensure first entry to TB Inside executes
                        // free-roam (g16 s0) even if a wake/intro station was scheduled.
                        if (GetScriptVarArrayValue(NULL, EVENT_S08E01, 0) == 1) {
                            if (!(group == 16 && sector == 0)) {
                                MGBA_Warnf("[GS] skip: post-save TB Inside g%d s%d -> force g16 s0 free-roam", group, sector);
                                group = 16;
                                sector = 0;
                            } else {
                                // Clear guard once we actually run free-roam
                                SetScriptVarArrayValue(NULL, EVENT_S08E01, 0, 0);
                            }
                        }
                        if (group == 45 && sector == 0) {
                            // Redirect postgame morning to free-roam (g16 s0) and preselect Lives/Objects
                            // so the map is populated immediately, avoiding black screens and loops.
                        MGBA_Warnf("[GS] skip: TB Inside g45 s0 -> redirect to g16 s1 (free-roam)");
                        // Normalize scenario to post‑Rayquaza baseline so g16 free‑roam stations populate
                        // correctly (some TB INSIDE setups expect scen≈0x12, not later postgame values).
                        if ((s16)GetScriptVarValue(NULL, SCENARIO_MAIN) > 0x12) {
                            SetScriptVarValue(NULL, SCENARIO_MAIN, 0x12);
                        }
                        SetScriptVarValue(NULL, GROUND_GETOUT, map);
                            // Align current map globals for correct script file/entity resolution.
                            gUnknown_2039A32 = map;
                            gUnknown_2039A34 = map;
                            SetScriptVarValue(NULL, GROUND_MAP, map);
                            SetScriptVarValue(NULL, GROUND_PLACE, gGroundMapConversionTable[map].groundPlaceId);
                            group = 16;
                            sector = 1;
                            // Ensure the ground map resources are active for this map before entity selection.
                            GroundMap_Select(map);
                            GroundLives_Select(map, group, sector);
                            GroundObject_Select(map, group, sector);
                            GroundEffect_Select(map, group, sector);
                            GroundEvent_Select(map, group, sector);
                            // Also ensure door/bed events from sector 0 are active for leaving/sleeping.
                            GroundEvent_Select(map, group, 0);
                            // Do not execute inline; allow the normal exec path below to run.
                            // This avoids script engine stalls after SAVE.
                        } else if ((group == 17 || group == 18 || group == 19) && sector == 0) {
                            // Early-morning variants inside the base. Redirect to free‑roam and run it.
                            MGBA_Warnf("[GS] skip: TB Inside g%d s0 -> redirect to g16 s0 (free-roam)", group);
                            // Normalize to true postgame after a save when skipping cutscenes
                            // so story wake sequences don’t advance SCENARIO_MAIN back to early scenes.
                            if ((s16)GetScriptVarValue(NULL, SCENARIO_MAIN) < 19)
                                SetScriptVarValue(NULL, SCENARIO_MAIN, 19);
                            // Stabilize start mode and base enter/exit to inside
                            SetScriptVarValue(NULL, START_MODE, 2); // MODE_GROUND
                            SetScriptVarValue(NULL, GROUND_ENTER, MAP_TEAM_BASE_INSIDE);
                            SetScriptVarValue(NULL, GROUND_ENTER_LINK, 0);
                            // Set a one-shot postgame guard used by some resume paths
                            SetScriptVarArrayValue(NULL, EVENT_S08E01, 0, 1);
                            SetScriptVarValue(NULL, GROUND_GETOUT, map);
                            gUnknown_2039A32 = map;
                            gUnknown_2039A34 = map;
                            SetScriptVarValue(NULL, GROUND_MAP, map);
                            SetScriptVarValue(NULL, GROUND_PLACE, gGroundMapConversionTable[map].groundPlaceId);
                            group = 16;
                            sector = 0;
                            // Schedule a clean warp back inside; free‑roam station selection
                            // will be handled by the normal ground init, avoiding post‑save stalls.
                            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                            break;
                        } else if (group == 41 || group == 42 || group == 6 || group == 8 || group == 11) {
                            MGBA_Warnf("[GS] skip: TB Inside g%d s%d -> warp inside free-roam", group, sector);
                            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                            break;
                        }
                    } else if (placeNow == GROUND_PLACE_TEAM_BASE) {
                        // Post-save guard: first time stepping outside after a save, enforce
                        // free-roam in Square instead of triggering outside base mini-scenes.
                        if (GetScriptVarArrayValue(NULL, EVENT_S08E01, 0) == 1) {
                            if (!(group == 7 && sector == 0)) {
                                MGBA_Warnf("[GS] skip: post-save outside TB group=%d s=%d -> warp to Square g7 s0", group, sector);
                                SetScriptVarArrayValue(NULL, EVENT_S08E01, 0, 0);
                                GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                                break;
                            }
                            // Clear the guard when we actually execute the free-roam station
                            SetScriptVarArrayValue(NULL, EVENT_S08E01, 0, 0);
                        }
                        // Team Base (outside) first-day station — suppress when skipping cutscenes
                        if (group == 18 && sector == 0) {
                            MGBA_Warnf("[GS] skip: TB Outside g18 s0 -> warp inside free-roam");
                            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                            break;
                        }
                    }
                    // Do not globally warp from Tiny Woods entry here; routing is handled upstream.
                }

                // In full skip/postgame mode, clamp scenario forward so early-story
                // stations cannot reschedule prologue and first-mission flows.
                if (IsSkipLinearMode()) {
                    s32 scenClamp = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    if (scenClamp < 0x12)
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 0x12);
                }

                // Linear-skip: ensure initial GO and handle any dungeon clear globally.
                if (IsSkipLinearMode()) {
                    // Only seed initial GO in true early-game; do not in postgame skip mode.
                    s32 scenNowX = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    if (scenNowX < 0x12)
                        SkipLinear_EnsureInitialGo();
                    // If the engine tries to start the Tiny Woods entry cutscene, reroute to Team Base INSIDE.
                    if (map == MAP_TINY_WOODS_ENTRY && group == 1 && sector == 0) {
                        // First arrival after quiz wants to start TW entry; in skip mode,
                        // bypass it and move directly to Team Base INSIDE. Do not set any GO
                        // here to avoid re-triggering story guidance. Also lift scenario past
                        // prologue to prevent early base scenes from scheduling mandatory saves.
                        MGBA_Warnf("[GS] linear-skip: bypass TW entry -> Team Base INSIDE (no GO)");
                        // Lift scenario to postgame baseline so no early-base scenes trigger
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 0x12);
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        break;
                    }

                    // If entering Team Base INSIDE at the default enter station (g0 s0)
                    // during postgame skip, jump directly to free‑roam (g16 s1).
                    if (gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE_INSIDE
                        && group == 0 && sector == 0 && (s16)GetScriptVarValue(NULL, SCENARIO_MAIN) >= 0x12) {
                        MGBA_Warnf("[GS] linear-skip: reroute TB INSIDE g0 s0 -> g16 s0 (postgame)");
                        // Ensure getout points to the current inside map to avoid stale returns.
                        SetScriptVarValue(NULL, GROUND_GETOUT, map);
                        // Align current map globals so SELECT_ENTITIES resolves against the right script file.
                        gUnknown_2039A32 = map;
                        gUnknown_2039A34 = map;
                        SetScriptVarValue(NULL, GROUND_MAP, map);
                        SetScriptVarValue(NULL, GROUND_PLACE, gGroundMapConversionTable[map].groundPlaceId);
                        group = 16;
                        sector = 0;
                        // Ensure the ground map resources are active for this map before entity selection.
                        GroundMap_Select(map);
                        // Proactively select Lives/Objects/Effects/Events for the free‑roam sector
                        // to avoid blank screen when arriving via redirect.
                        GroundLives_Select(map, group, sector);
                        GroundObject_Select(map, group, sector);
                        GroundEffect_Select(map, group, sector);
                        GroundEvent_Select(map, group, sector);
                        // Load door/bed events from sector 0 as well so the player can leave/sleep.
                        GroundEvent_Select(map, group, 0);
                        // Execute the free‑roam station immediately to drive wake/init.
                        GroundMap_ExecuteStation(map, group, sector, 0);
                        break;
                    }

                    // Suppress Team Base INSIDE “Dream Eater” (Team Meanies/Gengar) cutscene
                    // which is driven by the station group 42 in b01p02a (Team Base INSIDE).
                    // When SkipCutscenes is ON, short-circuit this station and resume free roam
                    // to avoid replaying the same “light is coming” sequence at the base.
                    if ((gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE_INSIDE
                         || gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE)
                        && group == 42 && sector == 0) {
                        MGBA_Warnf("[GS] linear-skip: suppress Team Base g42 (Dream Eater) -> base free-roam");
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        break;
                    }

                    // Also skip the Team Base INSIDE wake-up ("The next morning...") station
                    // (group 41, sector 0) when skipping cutscenes; route straight to free‑roam.
                    if (gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE_INSIDE
                        && group == 41 && sector == 0) {
                        MGBA_Warnf("[GS] linear-skip: suppress Team Base g41 (wake-up) -> base free-roam");
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        break;
                    }
                    // Postgame morning chatter at Team Base INSIDE (group 45 sector 0) -> inline redirect to free‑roam
                    if (gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE_INSIDE
                        && group == 45 && sector == 0) {
                        MGBA_Warnf("[GS] linear-skip: suppress Team Base g45 (postgame morning) -> redirect to g16 s1");
                        if ((s16)GetScriptVarValue(NULL, SCENARIO_MAIN) > 0x12) {
                            SetScriptVarValue(NULL, SCENARIO_MAIN, 0x12);
                        }
                        SetScriptVarValue(NULL, GROUND_GETOUT, map);
                        // Align current map globals so SELECT_ENTITIES resolves against the right script file.
                        gUnknown_2039A32 = map;
                        gUnknown_2039A34 = map;
                        SetScriptVarValue(NULL, GROUND_MAP, map);
                        SetScriptVarValue(NULL, GROUND_PLACE, gGroundMapConversionTable[map].groundPlaceId);
                        group = 16;
                        sector = 1;
                        // Ensure the ground map resources are active for this map.
                        GroundMap_Select(map);
                        GroundLives_Select(map, group, sector);
                        GroundObject_Select(map, group, sector);
                        GroundEffect_Select(map, group, sector);
                        GroundEvent_Select(map, group, sector);
                        // Do not execute inline; allow the normal exec path below to run.
                    }
                    if (SkipLinear_HandleClearAndWarp())
                        break;

                    // Starter kit + first letter cutscenes at Team Base (outside) during scene 3
                    // Show up as groups 18/19/20. In linear-skip, fast-forward once as if done,
                    // seed news and starter items, set TWC GO, and immediately switch this station
                    // execution to Team Base Inside free‑roam (g16 s0) without scheduling a warp.
                    {
                        s32 scenNow = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                        s32 place = gGroundMapConversionTable[map].groundPlaceId;
                        if (scenNow == 3 && place == GROUND_PLACE_TEAM_BASE &&
                            ( (group == 18 && sector == 0) || group == 19 || group == 20) &&
                            GetScriptVarArrayValue(NULL, EVENT_S08E01, 2) == 0) {
                            // Guard so we only do this once
                            SetScriptVarArrayValue(NULL, EVENT_S08E01, 2, 1);
                            sub_8096488(); // seed news
                            sub_80961B4();
                            GiveStarterSetIfNeeded();
                            if (!RescueScenarioConquered(SCRIPT_DUNGEON_TINY_WOODS))
                                sub_8097418(SCRIPT_DUNGEON_TINY_WOODS, 1);
                            if (!sub_8097384(SCRIPT_DUNGEON_THUNDERWAVE_CAVE))
                                sub_80973A8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                            {
                                s32 twcIndex = sub_80A26B8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE);
                                if (twcIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, twcIndex);
                            }
                            // Fast-forward scenario beyond early base scenes
                            SetScriptVarValue(NULL, SCENARIO_MAIN, 5);
                            // Request a proper warp to Team Base Inside free‑roam to avoid
                            // lingering station state issues that can occur after saving.
                            MGBA_Warnf("[GS] linear-skip: warp TB starter kit/letter -> TB inside free-roam (set TWC select)");
                            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                            break;
                        }
                    }

                    // Note: We previously redirected TB INSIDE group 17 to group 16 (free‑roam)
                    // unconditionally. That caused occasional hangs after saving. The targeted
                    // g41/g42 skips above are sufficient; avoid rewriting group here.

                    // Skip Thunderwave Cave entrance cutscene: jump straight into the
                    // dungeon when the engine attempts to enter the TWC entry map.
                    if (map == MAP_THUNDERWAVE_CAVE_ENTRY) {
                        MGBA_Warnf("[GS] linear-skip: redirect TWC entry -> direct dungeon");
                        // Ensure TWC is visible as GO in UI if needed, then enter dungeon.
                        if (!sub_8097384(SCRIPT_DUNGEON_THUNDERWAVE_CAVE))
                            sub_80973A8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                        GroundMainRescueRequest(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 30);
                        break;
                    }
                }

                // No unconditional overrides when skip-cutscene logic is disabled.

                // Verbose tracing to identify stations that still show mini-cutscenes when skipping.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting()) {
                    s32 place = gGroundMapConversionTable[map].groundPlaceId;
                    s32 scenDbg = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    bool8 msGo = sub_8097384(SCRIPT_DUNGEON_MT_STEEL);
                    bool8 swGo = sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS);
                    bool8 scGo = sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM);
                    bool8 swOrd = sub_8097318(SCRIPT_DUNGEON_SINISTER_WOODS);
                    bool8 scOrd = sub_8097318(SCRIPT_DUNGEON_SILENT_CHASM);
                    s32 sel = GetScriptVarValue(NULL, DUNGEON_SELECT);
                    MGBA_Warnf("[GS] enter map=%d place=%d grp=%d sec=%d scen=%d | MS=%d SW=%d SC=%d | ordSW=%d ordSC=%d sel=%d",
                               map, place, group, sector, scenDbg, msGo, swGo, scGo, swOrd, scOrd, sel);
                }

                // Do not force a specific Team Base INSIDE station here; rely on
                // GroundMap_ExecuteEnter to choose a safe entry (g0 s0), and on the
                // cutscene suppressors above to bypass unwanted sequences.

                // In Square-sleeping pre‑LC, any attempt to route to Team Base should
                // be overridden to Lapis Cave entrance. This avoids bouncing into
                // base/save flows when confirming "All set!".
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting()) {
                    s32 scenNow = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    bool8 postGC = RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON);
                    bool8 preLC = !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE);
                    s32 place = gGroundMapConversionTable[map].groundPlaceId;
                    if ((scenNow == 11 || scenNow == 14) && postGC && preLC &&
                        (place == GROUND_PLACE_TEAM_BASE || place == GROUND_PLACE_TEAM_BASE_INSIDE)) {
                        map = MAP_LAPIS_CAVE_ENTRY;
                        group = 4;
                        sector = 0;
                        MGBA_Warnf("[GS] override TB -> Lapis entry (g4 s0) during Square sleeping pre-LC");
                    }
                }

                // At Lapis Cave entrance in skip mode, default to the entrance event
                // station (g4 s0) ONLY on initial arrival (group 0/sector 0) so we
                // don't loop when stations hop internally to g3.* for the partner
                // selection ("Which way should we go?"). Use MAP_LOCAL[0] as a
                // one‑shot guard within this map.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_LAPIS_CAVE_ENTRY && group == 0 && sector == 0) {
                    s32 scenNow = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    if ((scenNow == 11 || scenNow == 14)
                        && RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON)
                        && !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE)) {
                        s32 lapisEntryGuard = GetScriptVarArrayValue(NULL, MAP_LOCAL, 0);
                        if (lapisEntryGuard == 0) {
                            SetScriptVarArrayValue(NULL, MAP_LOCAL, 0, 1);
                            group = 4;
                            sector = 0;
                            MGBA_Warnf("[GS] reroute Lapis entry -> entrance event station (g4 s0)");
                        }
                    }
                }

                // Skip the Tiny Woods initial "You're finally awake!" cutscene when enabled.
                // This station is gs178 group 1 sector 0 (MAP_TINY_WOODS_ENTRY).
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_TINY_WOODS_ENTRY && group == 1 && sector == 0) {
                    // Directly request entering Tiny Woods like the script's NEXT_DUNGEON would.
                    // Use a fade speed of 30 to match the original script.
                    GiveStarterSetIfNeeded();
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 2);
                    GroundMainRescueRequest(SCRIPT_DUNGEON_TINY_WOODS, 30);
                    break;
                }
                // Skip Tiny Woods end-room station (success scene inside the dungeon end map).
                // Jump directly to the next story step as if the scene completed.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_TINY_WOODS_END && group == 1 && sector == 0) {
                    // Advance scenario and return to Team Base with a short fade.
                    GiveStarterSetIfNeeded();
                    // Mark Tiny Woods scenario as completed so it no longer shows as a GO story mission.
                    sub_8097418(SCRIPT_DUNGEON_TINY_WOODS, 1);
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 3);
                    GroundMainGroundRequest(MAP_TEAM_BASE, 0, 30);
                    break;
                }
                // Skip the post-Tiny Woods cutscene at Tiny Woods entry
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_TINY_WOODS_ENTRY && group == 3 && sector == 0) {
                    // Normally gives Toolbox + Badge + Pokémon News and then sets next dungeon.
                    // Seed the initial Pokémon News so the base menu unlocks (Items/Team/Job List).
                    sub_8096488();        // Put Pokémon News (floor 0) in mailbox
                    sub_80961B4();        // Allow news generation pipeline to start
                    // Unlock Thunderwave Cave in the Dungeons list like the script would.
                    sub_80973A8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 3);
                    GroundMainRescueRequest(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 30);
                    break;
                }

                // Skip the next-morning "...Hunh?! Oh, no!" cutscene at Team Base (group 18 sector 0)
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting()
                    && gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE
                    && group == 18 && sector == 0) {
                    // Ensure initial news exists to unlock menus if we skipped earlier scenes.
                    sub_8096488();
                    sub_80961B4();
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 3);
                    GroundMainRescueRequest(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 30);
                    break;
                }
                // Skip early Team Base arrival cutscene: "Well, this is the place..."
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting()
                    && gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE
                    && group == 17 && sector == 0) {
                    // Nothing to set here; EVENT_M01E01A already sets SCENARIO_MAIN after this.
                    break;
                }

                // Safety: when entering Team Base during early game with skip enabled,
                // ensure the appropriate story dungeons are unlocked in the Dungeons list.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE) {
                    s32 scen = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    s32 lastRes = (s16)GetScriptVarValue(NULL, DUNGEON_RESULT);
                    s32 lastEnter = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
                    s32 lastEnterNorm = lastEnter;
                    if (lastEnter == 0x50 || lastEnter == 0x51 || lastEnter == 0x52) {
                        lastEnterNorm = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER_INDEX);
                    }
                    // Trace last dungeon result to tighten skip flow around returns
                    MGBA_Warnf("[GS] TB resume: D_RESULT=%d D_ENTER=%d scen=%d", lastRes, lastEnter, scen);
                    // Before Thunderwave Cave (scene 3): ensure TWC is marked as current story mission.
                    if (scen == 3 && !sub_8097384(SCRIPT_DUNGEON_THUNDERWAVE_CAVE)) {
                        sub_80973A8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                        // Tiny Woods should not remain as a story mission anymore.
                        sub_8097418(SCRIPT_DUNGEON_TINY_WOODS, 1);
                    }
                    // After Thunderwave Cave (scene 4): ensure Mt. Steel is the current story mission.
                    if (scen == 4 && !sub_8097384(SCRIPT_DUNGEON_MT_STEEL)) {
                        sub_80973A8(SCRIPT_DUNGEON_MT_STEEL, 1);
                        sub_8097418(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                    }
                    // After Mt. Steel (scene 5): ensure Sinister Woods is the current story mission.
                    if (scen == 5 && !sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                        sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        sub_8097418(SCRIPT_DUNGEON_MT_STEEL, 1);
                    }

                    // Enforce: while SW not conquered (scene 5, pre-SW clear), SC must NOT be GO.
                    if (scen == 5 && !RescueScenarioConquered(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                        if (sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                            sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                            MGBA_Warnf("[GS] enforce TB: clear Silent Chasm GO while SW incomplete");
                        }
                        // Reassert SW GO and explicitly set selection to SW dungeon index
                        sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        {
                            s32 swIndex = sub_80A26B8(SCRIPT_DUNGEON_SINISTER_WOODS);
                            if (swIndex != -1) {
                                SetScriptVarValue(NULL, DUNGEON_SELECT, swIndex);
                                MGBA_Warnf("[GS] enforce TB: set Sinister Woods GO + select=%d", swIndex);
                            }
                        }
                    }

                    // Detect Silent Chasm clear return and promote Mt. Thunder immediately
                    // Guard on lastEnter (normalized) to ensure we actually returned from Silent Chasm
                    if ((lastRes == 6 || lastRes == 9 || lastRes == 11 || lastRes == 12)
                        && lastEnterNorm == SCRIPT_DUNGEON_SILENT_CHASM
                        && sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_SILENT_CHASM))
                            sub_8097418(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                        sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                        sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 1);
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 7);
                        MGBA_Warnf("[GS] TB detect SC clear: set MT GO + scen=7");
                    }

                    // After SW clear (scene 6): ensure SC is the current story mission
                    if (scen == 6) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_SILENT_CHASM)) {
                            // Clear any lingering SW GO and select SC
                            if (sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                                sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 0);
                                MGBA_Warnf("[GS] enforce TB: clear SW GO after SW clear (scene 6)");
                            }
                            sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                            {
                                s32 scIndex = sub_80A26B8(SCRIPT_DUNGEON_SILENT_CHASM);
                                if (scIndex != -1) {
                                    SetScriptVarValue(NULL, DUNGEON_SELECT, scIndex);
                                    MGBA_Warnf("[GS] enforce TB: set Silent Chasm GO + select=%d", scIndex);
                                }
                            }
                        }
                    }

                    // After SC clear (scene 7): ensure Mt. Thunder becomes the current story mission
                    if (scen == 7) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER)) {
                            // Clear any lingering SC GO and select Mt. Thunder
                            if (sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                                sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                                MGBA_Warnf("[GS] enforce TB: clear SC GO after SC clear (scene 7)");
                            }
                            sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 1);
                            {
                                s32 mtIndex = sub_80A26B8(SCRIPT_DUNGEON_MT_THUNDER);
                                if (mtIndex != -1) {
                                    SetScriptVarValue(NULL, DUNGEON_SELECT, mtIndex);
                                    MGBA_Warnf("[GS] enforce TB: set Mt. Thunder GO + select=%d", mtIndex);
                                }
                            }
                        }
                    }

                    // After Mt. Thunder clear: promote Great Canyon as GO
                    if (RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER) &&
                        !RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON)) {
                        // Clear any lingering Mt. Thunder GO and set Great Canyon
                        if (sub_8097384(SCRIPT_DUNGEON_MT_THUNDER)) {
                            sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 0);
                            MGBA_Warnf("[GS] enforce TB: clear MT GO post-MT clear");
                        }
                        sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                        {
                            s32 gcIndex = sub_80A26B8(SCRIPT_DUNGEON_GREAT_CANYON);
                            if (gcIndex != -1) {
                                SetScriptVarValue(NULL, DUNGEON_SELECT, gcIndex);
                                MGBA_Warnf("[GS] enforce TB: set Great Canyon GO + select=%d", gcIndex);
                            }
                        }
                    }

                    // After Great Canyon clear: fast‑forward to Square sleeping (scene 14)
                    // without setting LC GO; partner talk will trigger the dungeon.
                    if (RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON) &&
                        !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE)) {
                        if (sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON)) {
                            sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 0);
                            MGBA_Warnf("[GS] enforce TB: clear GC GO post-GC clear");
                        }
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 14);
                        MGBA_Warnf("[GS] enforce TB: scen=14 (Square sleeping), await partner talk");
                    }

                    // Detect Great Canyon clear return and jump to Square sleeping (scene 14)
                    // Only trigger if the last entered dungeon was Great Canyon.
                    // Accept common success-like result codes (6, 9, 11, 12).
                    if ((lastRes == 6 || lastRes == 9 || lastRes == 11 || lastRes == 12)
                        && lastEnterNorm == SCRIPT_DUNGEON_GREAT_CANYON) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON))
                            sub_8097418(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                        if (sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON))
                            sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 0);
                    ScenarioCalc(SCENARIO_MAIN, 11, 2);
                    MGBA_Warnf("[GS] TB detect GC clear: scen=11.2 -> Square sleeping (partner talk)");
                    GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                    break;
                }

                // Detect Lapis Cave clear return and jump straight to Mt. Blaze entrance
                // Only trigger if the last entered dungeon was Lapis Cave.
                if ((lastRes == 6 || lastRes == 9 || lastRes == 11 || lastRes == 12)
                    && lastEnterNorm == SCRIPT_DUNGEON_LAPIS_CAVE) {
                    if (!RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE))
                        sub_8097418(SCRIPT_DUNGEON_LAPIS_CAVE, 1);
                    if (sub_8097384(SCRIPT_DUNGEON_LAPIS_CAVE))
                        sub_80973A8(SCRIPT_DUNGEON_LAPIS_CAVE, 0);
                    if (!sub_8097384(SCRIPT_DUNGEON_MT_BLAZE))
                        sub_80973A8(SCRIPT_DUNGEON_MT_BLAZE, 1);
                    {
                        s32 mbIndex = sub_80A26B8(SCRIPT_DUNGEON_MT_BLAZE);
                        if (mbIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, mbIndex);
                    }
                    // Advance scenario into the Mt. Blaze arc (use 12.* window) and warp to entrance.
                    ScenarioCalc(SCENARIO_MAIN, 12, 2);
                    MGBA_Warnf("[GS] TB detect LC clear: scen=12.2 -> Mt. Blaze entrance (set MB GO)");
                    GroundMainGroundRequest(MAP_MT_BLAZE_ENTRY, 0, 30);
                    break;
                }

                // Detect Mt. Blaze clear return and jump straight to Frosty Forest entrance
                if ((lastRes == 6 || lastRes == 9 || lastRes == 11 || lastRes == 12)
                    && (lastEnterNorm == SCRIPT_DUNGEON_MT_BLAZE || lastEnterNorm == SCRIPT_DUNGEON_MT_BLAZE_PEAK)) {
                    if (!RescueScenarioConquered(SCRIPT_DUNGEON_MT_BLAZE))
                        sub_8097418(SCRIPT_DUNGEON_MT_BLAZE, 1);
                    if (sub_8097384(SCRIPT_DUNGEON_MT_BLAZE))
                        sub_80973A8(SCRIPT_DUNGEON_MT_BLAZE, 0);
                    if (!sub_8097384(SCRIPT_DUNGEON_FROSTY_FOREST))
                        sub_80973A8(SCRIPT_DUNGEON_FROSTY_FOREST, 1);
                    {
                        s32 ffIndex = sub_80A26B8(SCRIPT_DUNGEON_FROSTY_FOREST);
                        if (ffIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, ffIndex);
                    }
                    ScenarioCalc(SCENARIO_MAIN, 13, 2);
                    MGBA_Warnf("[GS] TB detect MB clear: scen=13.2 -> Frosty Forest entrance (set FF GO)");
                    GroundMainGroundRequest(MAP_FROSTY_FOREST_ENTRY, 0, 30);
                    break;
                }

                    // Strong skip: Starter‑kit and first‑letter flow at Team Base (scene 3)
                    // When SkipCutscenes is ON, pretend the mailbox “Rescue Team Starter Set”
                    // and first letter have already been handled. Seed initial news and a small
                    // starter item set, mark Tiny Woods as conquered and set Thunderwave Cave as GO.
                    if (scen == 3 &&
                        gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_TEAM_BASE &&
                        ((group == 18 && sector == 0) || (group == 19) || (group == 20))) {
                        // Seed base menus/news and initial items.
                        sub_8096488();        // Put Pokémon News in mailbox
                        sub_80961B4();        // Enable news generation pipeline
                        GiveStarterSetIfNeeded();
                        // Ensure post‑Tiny Woods state and prepare TWC.
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_TINY_WOODS))
                            sub_8097418(SCRIPT_DUNGEON_TINY_WOODS, 1);
                        if (!sub_8097384(SCRIPT_DUNGEON_THUNDERWAVE_CAVE))
                            sub_80973A8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                        {
                            s32 twcIndex = sub_80A26B8(SCRIPT_DUNGEON_THUNDERWAVE_CAVE);
                            if (twcIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, twcIndex);
                        }
                        // Fast‑forward scenario to after the letter acceptance (3.6)
                        ScenarioCalc(SCENARIO_MAIN, 3, 6);
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        MGBA_Warnf("[GS] strong-skip TB starter kit/letter -> inside free-roam (set TWC GO)");
                        break;
                    }

                    // Strong skip for the "Good morning... rescue mission" mini-scene (scene 4)
                    // and for the Meanies + Caterpie mini-scene (scene 5). Limit to the
                    // specific Team Base outside stations that trigger these.
                    if (scen == 4 &&
                        ( (group == 18 && sector == 0) ||
                          (group == 17 && sector == 0) ||
                          (group == 26 && sector == 0) ) ) {
                        if (!sub_8097384(SCRIPT_DUNGEON_MT_STEEL))
                            sub_80973A8(SCRIPT_DUNGEON_MT_STEEL, 1);
                        sub_8097418(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                        // Reload Team Base INSIDE (wake-up scene position) into its
                        // free-roam station so the partner is present, then skip this station.
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        MGBA_Warnf("[GS] strong-skip TB outside grp=%d sec=%d -> inside free-roam", group, sector);
                        break;
                    }

                    if (scen == 5 && (group == 31 && sector == 0)) {
                        if (!sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS))
                            sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0); // prevent early GO on Silent Chasm
                        sub_8097418(SCRIPT_DUNGEON_MT_STEEL, 1);
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        MGBA_Warnf("[GS] strong-skip TB outside post-MS grp=%d sec=%d -> inside free-roam (clear SC GO)", group, sector);
                        break;
                    }

                    // Strong skip: after Silent Chasm clear (scene 6) we already
                    // skip the outside thank‑you. After Mt. Thunder (scene 8), skip the
                    // outside partner "Let's go to Great Canyon" prompt and jump straight
                    // to inside free‑roam with GC set as GO.
                    // One-shot skip outside partner prompt after Mt. Thunder.
                    // Use EVENT_S08E01[1] as a guard so this only triggers once,
                    // avoiding bounce-backs when leaving base later. Index [0] is
                    // managed by dungeon return; index [1] is reserved for our skip.
                    if (scen == 8 && GetScriptVarArrayValue(NULL, EVENT_S08E01, 1) == 0 &&
                        ( (group == 31 && sector == 0) ||
                          (group == 30 && sector == 0) ||
                          (group == 26 && sector == 0) ||
                          (group == 43 && sector == 0) ) ) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER))
                            sub_8097418(SCRIPT_DUNGEON_MT_THUNDER, 1);
                        if (!sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON))
                            sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                        {
                            s32 gcIndex = sub_80A26B8(SCRIPT_DUNGEON_GREAT_CANYON);
                            if (gcIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, gcIndex);
                        }
                        SetScriptVarArrayValue(NULL, EVENT_S08E01, 1, 1);
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        MGBA_Warnf("[GS] strong-skip TB outside post-MT grp=%d sec=%d -> inside free-roam (skip partner prompt)", group, sector);
                        break;
                    }

                    // Strong skip: after Sinister Woods clear (scene 6), skip the
                    // outside Team Base "Caterpie/Metapod thank-you" scene and go
                    // straight to free-roam inside the base. Limit to common TB
                    // outside groups observed to trigger mini-scenes.
                    if (scen == 6 &&
                        ( (group == 31 && sector == 0) ||
                          (group == 26 && sector == 0) ) ) {
                        // Keep SC as current story mission; ensure SW/MS are marked cleared.
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_SINISTER_WOODS))
                            sub_8097418(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                        sub_8097418(SCRIPT_DUNGEON_MT_STEEL, 1);
                        GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                        MGBA_Warnf("[GS] strong-skip TB outside post-SW grp=%d sec=%d -> inside free-roam (skip thank-you)", group, sector);
                        break;
                    }
                }

                // Skip Pokémon Square "Team Meanies + Caterpie" scene after Mt. Steel.
                // When arriving at Pokémon Square with scen==5 (post–Mt. Steel), just
                // keep Sinister Woods as GO and enter Square free-roam.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting()
                    && gGroundMapConversionTable[map].groundPlaceId == GROUND_PLACE_POKEMON_SQUARE) {
                    s32 scen = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    s32 lastRes2 = (s16)GetScriptVarValue(NULL, DUNGEON_RESULT);
                    s32 lastRes = (s16)GetScriptVarValue(NULL, DUNGEON_RESULT);
                    s32 lastEnter = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
                    s32 lastEnterNorm = lastEnter;
                    if (lastEnter == 0x50 || lastEnter == 0x51 || lastEnter == 0x52) {
                        lastEnterNorm = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER_INDEX);
                    }
                    MGBA_Warnf("[GS] SQ resume: D_RESULT=%d D_ENTER=%d scen=%d", lastRes, lastEnter, scen);
                    if (scen == 5) {
                        if (!sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS))
                            sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0); // ensure SC not GO until SW complete
                        sub_8097418(SCRIPT_DUNGEON_MT_STEEL, 1);
                        MGBA_Warnf("[GS] strong-skip Square grp=%d sec=%d -> Square free-roam (clear SC GO)", group, sector);
                        GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                        break;
                    }

                    // One-shot skip for Square outside station that leads into the
                    // post-Mt. Thunder partner prompt. Trigger once and jump to TB inside.
                    if (scen == 8 && GetScriptVarArrayValue(NULL, EVENT_S08E01, 1) == 0) {
                        if (group == 30 && sector == 0) {
                            if (!RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER))
                                sub_8097418(SCRIPT_DUNGEON_MT_THUNDER, 1);
                            if (!sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON))
                                sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                            {
                                s32 gcIndex = sub_80A26B8(SCRIPT_DUNGEON_GREAT_CANYON);
                                if (gcIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, gcIndex);
                            }
                            SetScriptVarArrayValue(NULL, EVENT_S08E01, 1, 1);
                            MGBA_Warnf("[GS] strong-skip Square post-MT grp=%d sec=%d -> TB inside free-roam", group, sector);
                            GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                            break;
                        }
                    }

                    // Detect Great Canyon clear return and jump to Square sleeping (scene 14)
                    // Only trigger if the last entered dungeon was Great Canyon.
                    if ((lastRes == 6 || lastRes == 9 || lastRes == 11 || lastRes == 12)
                        && lastEnterNorm == SCRIPT_DUNGEON_GREAT_CANYON) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON))
                            sub_8097418(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                        if (sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON))
                            sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 0);
                    ScenarioCalc(SCENARIO_MAIN, 11, 2);
                    MGBA_Warnf("[GS] SQ detect GC clear: scen=11.2 -> Square sleeping (partner talk)");
                    GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                    break;
                }

                    // Detect Lapis Cave clear return and jump straight to Mt. Blaze entrance
                    if ((lastRes2 == 6 || lastRes2 == 9 || lastRes2 == 11 || lastRes2 == 12)
                        && lastEnterNorm == SCRIPT_DUNGEON_LAPIS_CAVE) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE))
                            sub_8097418(SCRIPT_DUNGEON_LAPIS_CAVE, 1);
                        if (sub_8097384(SCRIPT_DUNGEON_LAPIS_CAVE))
                            sub_80973A8(SCRIPT_DUNGEON_LAPIS_CAVE, 0);
                        if (!sub_8097384(SCRIPT_DUNGEON_MT_BLAZE))
                            sub_80973A8(SCRIPT_DUNGEON_MT_BLAZE, 1);
                        {
                            s32 mbIndex = sub_80A26B8(SCRIPT_DUNGEON_MT_BLAZE);
                            if (mbIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, mbIndex);
                        }
                        ScenarioCalc(SCENARIO_MAIN, 12, 2);
                        MGBA_Warnf("[GS] SQ detect LC clear: scen=12.2 -> Mt. Blaze entrance (set MB GO)");
                        GroundMainGroundRequest(MAP_MT_BLAZE_ENTRY, 0, 30);
                        break;
                    }
                    // Detect Silent Chasm clear return and promote Mt. Thunder immediately
                    // Guard on lastEnter (normalized) to ensure we actually returned from Silent Chasm
                    if ((lastRes2 == 6 || lastRes2 == 9 || lastRes2 == 11 || lastRes2 == 12)
                        && lastEnterNorm == SCRIPT_DUNGEON_SILENT_CHASM
                        && sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_SILENT_CHASM))
                            sub_8097418(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                        sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                        sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 1);
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 7);
                        MGBA_Warnf("[GS] SQ detect SC clear: set MT GO + scen=7");
                    }

                    // Enforce while roaming Square: ensure only SW is GO until SW is completed
                    if (scen == 5 && !RescueScenarioConquered(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                        if (sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                            sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                            MGBA_Warnf("[GS] enforce SQ: clear Silent Chasm GO while SW incomplete");
                        }
                        sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                        {
                            s32 swIndex = sub_80A26B8(SCRIPT_DUNGEON_SINISTER_WOODS);
                            if (swIndex != -1) {
                                SetScriptVarValue(NULL, DUNGEON_SELECT, swIndex);
                                MGBA_Warnf("[GS] enforce SQ: set Sinister Woods GO + select=%d", swIndex);
                            }
                        }
                    }

                    // After SW clear (scene 6): ensure SC becomes the active GO and SW is cleared
                    if (scen == 6) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_SILENT_CHASM)) {
                            if (sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                                sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 0);
                                MGBA_Warnf("[GS] enforce SQ: clear SW GO after SW clear (scene 6)");
                            }
                            sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                            {
                                s32 scIndex = sub_80A26B8(SCRIPT_DUNGEON_SILENT_CHASM);
                                if (scIndex != -1) {
                                    SetScriptVarValue(NULL, DUNGEON_SELECT, scIndex);
                                    MGBA_Warnf("[GS] enforce SQ: set Silent Chasm GO + select=%d", scIndex);
                                }
                            }
                        }
                    }

                    // After SC clear (scene 7): ensure Mt. Thunder becomes the active GO and SC is cleared
                    if (scen == 7) {
                        if (!RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER)) {
                            if (sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM)) {
                                sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                                MGBA_Warnf("[GS] enforce SQ: clear SC GO after SC clear (scene 7)");
                            }
                            sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 1);
                            {
                                s32 mtIndex = sub_80A26B8(SCRIPT_DUNGEON_MT_THUNDER);
                                if (mtIndex != -1) {
                                    SetScriptVarValue(NULL, DUNGEON_SELECT, mtIndex);
                                    MGBA_Warnf("[GS] enforce SQ: set Mt. Thunder GO + select=%d", mtIndex);
                                }
                            }
                        }
                    }

                    // After Mt. Thunder clear: promote Great Canyon as GO
                    if (RescueScenarioConquered(SCRIPT_DUNGEON_MT_THUNDER) &&
                        !RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON)) {
                        if (sub_8097384(SCRIPT_DUNGEON_MT_THUNDER)) {
                            sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 0);
                            MGBA_Warnf("[GS] enforce SQ: clear MT GO post-MT clear");
                        }
                        sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                        {
                            s32 gcIndex = sub_80A26B8(SCRIPT_DUNGEON_GREAT_CANYON);
                            if (gcIndex != -1) {
                                SetScriptVarValue(NULL, DUNGEON_SELECT, gcIndex);
                                MGBA_Warnf("[GS] enforce SQ: set Great Canyon GO + select=%d", gcIndex);
                            }
                        }
                    }

                    // After Great Canyon clear: fast‑forward to Square sleeping (scene 14)
                    // without setting LC GO; partner talk will trigger the dungeon.
                    if (RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON) &&
                        !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE)) {
                        if (sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON)) {
                            sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 0);
                            MGBA_Warnf("[GS] enforce SQ: clear GC GO post-GC clear");
                        }
                        SetScriptVarValue(NULL, SCENARIO_MAIN, 14);
                        MGBA_Warnf("[GS] enforce SQ: scen=14 (Square sleeping), await partner talk");
                    }
                }

                // Skip the Thunderwave Cave end-room cutscene ("Oh, there they are!")
                // This station is gs181 group 1 sector 0 (MAP_THUNDERWAVE_CAVE_END).
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_THUNDERWAVE_CAVE_END && group == 1 && sector == 0) {
                    // Ensure base menu unlocks if the initial news was never seeded.
                    if (sub_8096E2C() == 0 && CountFilledMailboxSlots() == 0) {
                        sub_8096488();
                        sub_80961B4();
                    }
                    // Mark Thunderwave Cave as completed and unlock Mt. Steel for the next mission.
                    sub_8097418(SCRIPT_DUNGEON_THUNDERWAVE_CAVE, 1);
                    sub_80973A8(SCRIPT_DUNGEON_MT_STEEL, 1);
                    // Advance scenario to immediately after TWC completion and return to Team Base.
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 4);
                    MGBA_Warnf("[GS] skip TWC end -> Team Base outside free-roam");
                    GroundMainGroundRequest(MAP_TEAM_BASE, 0, 30);
                    break;
                }

                // Skip the Mt. Steel end-room cutscene and jump straight to Sinister Woods unlock.
                // NOTE: Stations/scene flow: at scen==5 (post–Mt. Steel), the story GO should
                // be Sinister Woods. The Dungeons list expects SW to render under display id 3,
                // while the GO flag is stored under script id 4. UI code accounts for this by
                // remapping the display id when SW is GO; here we ensure the flags are set so
                // the UI can reflect the intended state immediately upon returning to base.
                // This station is gs183 group 1 sector 0 (MAP_MT_STEEL_END).
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_MT_STEEL_END && group == 1 && sector == 0) {
                    // Mark Mt. Steel completed and set Sinister Woods as the next story dungeon.
                    sub_8097418(SCRIPT_DUNGEON_MT_STEEL, 1);
                    sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                    // Ensure Silent Chasm is not prematurely marked as a story GO before SW is cleared.
                    sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 0);
                    // Pre-select Sinister Woods on the World Map so the player sees the correct GO.
                    {
                        s32 swIndex = sub_80A26B8(SCRIPT_DUNGEON_SINISTER_WOODS);
                        if (swIndex != -1) {
                            SetScriptVarValue(NULL, DUNGEON_SELECT, swIndex);
                        }
                    }
                    // Move scenario forward to the post–Mt. Steel stage (major scene 5) and return to base.
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 5);
                    MGBA_Warnf("[GS] skip MS end -> Team Base (set SW GO, clear SC GO, select SW)");
                    GroundMainGroundRequest(MAP_TEAM_BASE, 0, 30);
                    break;
                }

                // Skip the Sinister Woods end-room cutscene (Metapod + Caterpie thank-you).
                // Jump straight to setting Silent Chasm as next story mission.
                // This station is gs185 group 1 sector 0 (MAP_SINISTER_WOODS_END).
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_SINISTER_WOODS_END && group == 1 && sector == 0) {
                    // Mark Sinister Woods completed and set Silent Chasm as the next story dungeon.
                    sub_8097418(SCRIPT_DUNGEON_SINISTER_WOODS, 1);
                    sub_80973A8(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                    // Advance scenario to the post–Sinister Woods stage (major scene 6) and return to base.
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 6);
                    MGBA_Warnf("[GS] skip SW end -> Team Base (set SC GO)");
                    GroundMainGroundRequest(MAP_TEAM_BASE, 0, 30);
                    break;
                }

                // Skip the Silent Chasm end-room cutscene and jump straight to Mt. Thunder unlock.
                // Station: gs187 group 1 sector 0 (MAP_SILENT_CHASM_END)
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_SILENT_CHASM_END && group == 1 && sector == 0) {
                    // Mark Silent Chasm completed and set Mt. Thunder as the next story dungeon.
                    sub_8097418(SCRIPT_DUNGEON_SILENT_CHASM, 1);
                    sub_80973A8(SCRIPT_DUNGEON_MT_THUNDER, 1);
                    // Advance scenario to post–Silent Chasm (major scene 7) and return to base.
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 7);
                    MGBA_Warnf("[GS] skip SC end -> Team Base (set MT GO)");
                    GroundMainGroundRequest(MAP_TEAM_BASE, 0, 30);
                    break;
                }

                // Skip the Mt. Thunder end-room cutscene and unlock Great Canyon immediately.
                // Station: gs190 group 1 sector 0 (MAP_MT_THUNDER_END)
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_MT_THUNDER_END && group == 1 && sector == 0) {
                    // Mark Mt. Thunder completed and set Great Canyon as the next story dungeon.
                    sub_8097418(SCRIPT_DUNGEON_MT_THUNDER, 1);
                    sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                    // Advance scenario to post–Mt. Thunder (major scene 8) and return to base.
                    SetScriptVarValue(NULL, SCENARIO_MAIN, 8);
                    MGBA_Warnf("[GS] skip MT end -> Team Base INSIDE (set GC GO)");
                    GroundMainGroundRequest(MAP_TEAM_BASE_INSIDE, 0, 30);
                    break;
                }

                // Skip the Great Canyon end-room cutscene (Hill of the Ancients) and
                // jump straight to the "Square sleeping" free-roam (partner talk triggers LC).
                // Station: Hill of the Ancients (MAP_HILL_OF_THE_ANCIENTS), group 1 sector 0
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_HILL_OF_THE_ANCIENTS && group == 1 && sector == 0) {
                    // Mark Great Canyon conquered and fast-forward the scenario.
                    sub_8097418(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                    // Fast‑forward to the post–Great Canyon stage (major scene 14) and
                    // arrive at Pokémon Square free‑roam (sleeping town). Do NOT set LC GO here;
                    // the partner talk in Square will trigger the dungeon entry.
                    ScenarioCalc(SCENARIO_MAIN, 11, 2);
                    MGBA_Warnf("[GS] skip GC end -> scen=11.2 -> Square sleeping (partner talk)");
                    GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                    break;
                }

                // Skip the Mt. Blaze → Frosty interlude: when arriving at the
                // Frosty Forest END station in skip mode, immediately promote
                // Frosty Grotto and jump to the Frosty Forest MID (save point).
                // This mirrors how we handle Lapis→Mt. Blaze.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && map == MAP_FROSTY_FOREST_END && group == 1 && sector == 0) {
                    // Mark Frosty Forest completed and set Frosty Grotto as next story dungeon.
                    if (!RescueScenarioConquered(SCRIPT_DUNGEON_FROSTY_FOREST))
                        sub_8097418(SCRIPT_DUNGEON_FROSTY_FOREST, 1);
                    if (!sub_8097384(SCRIPT_DUNGEON_FROSTY_GROTTO))
                        sub_80973A8(SCRIPT_DUNGEON_FROSTY_GROTTO, 1);
                    {
                        s32 fgIndex = sub_80A26B8(SCRIPT_DUNGEON_FROSTY_GROTTO);
                        if (fgIndex != -1) SetScriptVarValue(NULL, DUNGEON_SELECT, fgIndex);
                    }
                    ScenarioCalc(SCENARIO_MAIN, 13, 2);
                    MGBA_Warnf("[GS] skip FF end -> Frosty Forest MID (set FG GO)");
                    GroundMainGroundRequest(MAP_FROSTY_FOREST_MID, 0, 30);
                    break;
                }

                res = curCmd.op == 0x1e;
                // SkipCutscenes no longer alters station set-mode.
                if (0 && GetSkipCutscenesSetting()) {
                    s32 placeExec = gGroundMapConversionTable[map].groundPlaceId;
                    if (placeExec == GROUND_PLACE_TEAM_BASE_INSIDE && group == 16 && sector == 0) {
                        res = 0;
                    }
                }
                MGBA_Warnf("[GS] exec station map=%d group=%d sector=%d set=%d place=%d", map, group, sector, res, gGroundMapConversionTable[map].groundPlaceId);
                DebugDumpCoreVars("pre-exec");
                DumpStoryFlags("pre-exec");
                GroundMap_ExecuteStation(map, group, sector, res);
                DebugDumpCoreVars("post-exec");
                DumpStoryFlags("post-exec");
                if (gUnknown_2039A34 != map) {
                    gUnknown_2039A34 = map;
                    GroundCancelAllEntities();
                    if (action->unk8.unk0 != 0)
                        return 4; // Fatal?
                }
                break;
            }
            case 0x1f: {
                s32 a = (s16)GetScriptVarValue(NULL, DUNGEON_ENTER);
                const DungeonInfo *ret1 = GetDungeonInfo_80A2608(a);
                s32 thing = GetScriptVarArrayValue(NULL, DUNGEON_ENTER_LIST, (u16) a) == 0 ? ret1->unk6 : ret1->unk8;
                // SkipCutscenes no longer reroutes partner readiness to Lapis Cave.
                if (0 && GetSkipCutscenesSetting()) {
                    s32 scenNow = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                    bool8 postGC = RescueScenarioConquered(SCRIPT_DUNGEON_GREAT_CANYON);
                    bool8 preLC = !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE);
                    // Accept both scen==14 and scen==11.* (fast-forward) as Square sleeping states
                    if ((scenNow == 14 || scenNow == 11) && postGC && preLC) {
                        s32 curMap = gUnknown_2039A34;
                        s32 curPlace = gGroundMapConversionTable[curMap].groundPlaceId;
                        // Avoid redirect loops when already at Lapis Cave entrance/exit
                        if (curPlace == GROUND_PLACE_LAPIS_CAVE || curPlace == GROUND_PLACE_LAPIS_CAVE_EXIT) {
                            MGBA_Warnf("[GS] suppress redirect at Lapis place=%d (avoid loop)", curPlace);
                        } else {
                            MGBA_Warnf("[GS] redirect: partner 'ready' -> Lapis Cave entrance cutscene");
                            GroundMainGroundRequest(MAP_LAPIS_CAVE_ENTRY, 0, 30);
                            break;
                        }
                    }
                }
                // fakematch: this is almost certainly a range check of the form 0x37 <= a && a < 0x48
                // but that loses the s32 -> u16 cast. Inlines, macros, or other schenanigans are likely involved
                if (!((u16)(a - 0x37) < 0x11) && (s16)sub_80A2750(a) == 1) {
                    if (thing == -1) {
                        if (ScriptLoggingEnabled(TRUE)) {
                            Log(1, "    dungeon rescue select %3d", a);
                        }
                        GroundMainRescueRequest(a, -1);
                    } else {
                        GroundMap_ExecuteEvent(thing, 0);
                        break;
                    }
                } else {
                    GroundMainRescueRequest(a, -1);
                }
                break;
            }
            case 0x20: {
                switch (action->unkC.unk0) {
                    case 0:
                        GroundMap_ExecuteEvent(curCmd.argShort, 0);
                        break;
                    case 1: {
                        ScriptInfoSmall info1;
                        GetFunctionScript(action, &info1, curCmd.argShort);
                        GroundLives_ExecuteScript(action->unkC.unk2, &action->unk8, &info1);
                        break;
                    }
                    case 2: {
                        ScriptInfoSmall info2;
                        GetFunctionScript(action, &info2, curCmd.argShort);
                        GroundObject_ExecuteScript(action->unkC.unk2, &action->unk8, &info2);
                        break;
                    }
                    case 3: {
                        ScriptInfoSmall info3;
                        GetFunctionScript(action, &info3, curCmd.argShort);
                        GroundEffect_ExecuteScript(action->unkC.unk2, &action->unk8, &info3);
                        break;
                    }
                }
                break;
            }
            case 0x21: {
                s32 ret;
                s32 unk;
                PixelPos pos1;
                PixelPos pos2;
                PixelPos pos3;
                PixelPos pos4;
                s32 tmp;
                ret = (s16)sub_80A7AE8((s16)curCmd.arg1);
                if (ret >= 0) {
                    sub_80A8BD8(ret, &unk);
                    if (unk & 0x200) {
                        action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                        action->callbacks->getSize(action->parentObject, &pos2);
                        sub_80A8FD8(ret, &pos3);
                        sub_80A8F9C(ret, &pos4);
                        if ((tmp = SizedDeltaDirection8(&pos3, &pos4, &pos1, &pos2)) != -1 ||
                            (tmp = SizedDeltaDirection4(&pos1, &sPixelPosZero, &pos3, &sPixelPosZero)) != -1) {
                            sub_80A9090(ret, tmp);
                        }
                    }
                    GroundLives_ExecutePlayerScriptActionLives(action->unk8.unk2, ret);
                    return 3;
                }
                break;
            }
            case 0x22: {
                sub_80999E8(curCmd.argShort);
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x23: {
                sub_80999FC(curCmd.argShort);
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x24: {
                sub_8099A10(curCmd.argShort, curCmd.arg1, curCmd.arg2);
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x25: {
                sub_8099A34(curCmd.argShort);
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x26: {
                sub_8099A48(curCmd.argShort);
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x27: case 0x28: {
                RGB_Array color = { curCmd.arg2 >> 16, curCmd.arg2 >> 8, curCmd.arg2, 0 };
                switch (curCmd.op) {
                    case 0x27:
                        sub_8099A5C(curCmd.argShort, curCmd.arg1, color);
                        break;
                    case 0x28:
                        sub_8099AFC(curCmd.argShort, curCmd.arg1, color);
                        break;
                }
                if (curCmd.argByte) return 2;
                break;
            }
            case 0x29: {
                sub_809A6E4((u16)curCmd.argShort);
                break;
            }
            case 0x2a: {
                sub_809A6F8((u16)curCmd.argShort);
                break;
            }
            case 0x2b: {
                SetAutoPressTextboxMidEndMsgFrames(curCmd.arg1, curCmd.arg2);
                break;
            }
            case 0x2c: {
                if (!(s8)sub_809A768()) break;
                sub_80A87AC(0, 10);
                if (GroundScriptCheckLockCondition(action, 0)) return 2;
                break;
            }
            case 0x30: {
                ScriptClearTextbox();
                break;
            }
            case 0x31: {
                ScriptClearTextbox2();
                break;
            }
            case 0x2d: {
                switch ((u8)curCmd.argByte) {
                    case 0: {
                        ResetTextboxPortrait(curCmd.argShort);
                        break;
                    }
                    case 1: {
                        sub_80A2500(curCmd.argShort, &action->unk8);
                        break;
                    }
                    case 2: {
                        sub_80A2500(curCmd.argShort, &action->unkC);
                        break;
                    }
                    case 3: {
                        ActionUnkIds unk;
                        unk.unk2 = sub_80A7AE8((s16)curCmd.arg1);
                        unk.unk0 = 1;
                        sub_80A2500(curCmd.argShort, &unk);
                        break;
                    }
                    case 4: {
                        sub_80A252C(curCmd.argShort, &action->unk8);
                        break;
                    }
                    case 5: {
                        sub_80A252C(curCmd.argShort, &action->unkC);
                        break;
                    }
                    case 6: {
                        ActionUnkIds unk;
                        s16 res = sub_80A7AE8((s16)curCmd.arg1);
                        unk.unk2 = res;
                        if (unk.unk2 >= 0) {
                            unk.unk0 = 1;
                            sub_80A252C(curCmd.argShort, (void*)&unk);
                        } else {
                            sub_80A2584(curCmd.argShort, (s16)curCmd.arg1);
                        }
                        break;
                    }
                    case 7: {
                        sub_80A2558(curCmd.argShort, &action->unk8);
                        break;
                    }
                    case 8: {
                        sub_80A2558(curCmd.argShort, &action->unkC);
                        break;
                    }
                    case 9: {
                        ActionUnkIds unk;
                        s16 res = sub_80A7AE8((s16)curCmd.arg1);
                        unk.unk2 = res;
                        if (unk.unk2 >= 0) {
                            unk.unk0 = 1;
                            sub_80A2558(curCmd.argShort, (void*)&unk);
                        } else {
                            sub_80A2598(curCmd.argShort, (s16)curCmd.arg1);
                        }
                        break;
                    }
                    case 10: {
                        u8 a = ScriptDungeonIdToDungeonId((s16)curCmd.arg1);
                        s32 o = 0;
                        switch ((s16)curCmd.arg1) {
                            case 0: o = 1; break;
                            case 2: o = -1; break;
                        }
                        gFormatArgs[curCmd.argShort] = GetDungeonFloorCount(a) + o;
                        break;
                    }
                }
                break;
            }
            case 0x2e: {
                if (0 && GetSkipCutscenesSetting()) break;
                ScriptSetPortraitInfo(curCmd.argShort, (s8)curCmd.arg1, (u8)curCmd.argByte);
                break;
            }
            case 0x2f: {
                PixelPos pos;
                if (0 && GetSkipCutscenesSetting()) break;
                pos.x = curCmd.arg1;
                pos.y = curCmd.arg2;
                ScriptSetPortraitPosDelta(curCmd.argShort, &pos);
                break;
            }
            case 0x32 ... 0x38: {
                s8 ret = 0;
                if (0 && GetSkipCutscenesSetting()) {
                    // Suppress all dialogue/text when skipping cutscenes
                    break;
                }
                switch (scriptData->curScriptOp) {
                    case 0x32: ret = ScriptPrintText(SCRIPT_TEXT_TYPE_INSTANT, curCmd.argShort, curCmd.argPtr); break;
                    case 0x33: ret = ScriptPrintText(SCRIPT_TEXT_TYPE_PLAYER, curCmd.argShort, curCmd.argPtr); break;
                    case 0x34: ret = ScriptPrintText(SCRIPT_TEXT_TYPE_NPC, curCmd.argShort, curCmd.argPtr); break;
                    case 0x35: ret = ScriptPrintText(SCRIPT_TEXT_TYPE_LETTER, curCmd.argShort, curCmd.argPtr); break;
                    case 0x36: ret = ScriptPrintText(SCRIPT_TEXT_TYPE_4, curCmd.argShort, curCmd.argPtr); break;
                    case 0x37: ret = ScriptPrintTextOnBg(curCmd.argPtr); break;
                    case 0x38: ret = ScriptPrintTextOnBg2(curCmd.argPtr); break;
                }
                if (ret) {
                    sub_80A87AC(0, 10);
                    if (GroundScriptCheckLockCondition(action, 0)) return 2;
                }
                break;
            }
            case 0x39: {
                if (0 && GetSkipCutscenesSetting()) break;
                if ((s8)ScriptPrintTextOnBgAuto(curCmd.argShort, curCmd.argPtr) && curCmd.argShort >= 0) {
                    sub_80A87AC(0, 10);
                    if (GroundScriptCheckLockCondition(action, 0)) return 2;
                }
                break;
            }
            case 0x3a: {
                sub_809AFC8((u8)curCmd.argByte > 0, curCmd.arg1, (s16)curCmd.arg2, curCmd.argPtr);
                if (GroundScriptCheckLockCondition(action, 1)) {
                    sub_80A87AC(0, 11);
                    return 2;
                }
                break;
            }
            case 0x3c: {
                sub_809B1D4(curCmd.argByte, curCmd.arg1, curCmd.arg2, curCmd.argPtr);
                sub_80A87AC(0, 11);
                return 2;
            }
            case 0x3b: {
                scriptData->unk2A = 0;
                return 2;
            }
            case 0x3d: {
                int i;
                if ((s16)curCmd.arg1 != -1) {
                    Pokemon *mon = sub_80A8D54(curCmd.arg1);
                    if (mon != NULL) {
                        for (i = 0; i < POKEMON_NAME_LENGTH; i++) {
                            sPokeNameBuffer[i] = mon->name[i];
                        }
                        sPokeNameBuffer[POKEMON_NAME_LENGTH] = 0;
                        sub_809B1C0(4, 0, sPokeNameBuffer);
                        sub_80A87AC(0, 11);
                        return 2;
                    }
                    break;
                } else {
                    sub_809B1C0(4, 1, sPokeNameBuffer);
                    sub_80A87AC(0, 11);
                    return 2;
                }
            }
            case 0x3e: {
                sub_80920B8(sPokeNameBuffer);
                sPokeNameBuffer[10] = '\0';
                sub_809B1C0(5, 0, sPokeNameBuffer);
                sub_80A87AC(0, 11);
                return 2;
            }
            case 0x3f: {
                int i;
                for (i = 0; i < ARRAY_COUNT_INT(sPokeNameBuffer); i++) {
                    sPokeNameBuffer[i] = '\0';
                }
                sub_809B1C0(6, (u8)curCmd.argByte, sPokeNameBuffer);
                sub_80A87AC(0, 11);
                return 2;
            }
            case 0x42: {
                StopAllMusic_1();
                break;
            }
            case 0x43: {
                FadeOutAllMusic(curCmd.argShort < 0 ? 30 : curCmd.argShort);
                break;
            }
            case 0x44: {
                u16 id = curCmd.argByte == 0 ? sub_80A25AC((u16)curCmd.arg1) : curCmd.arg1;
                if (id != 999) {
                    StartNewBGM_(id);
                } else {
                    StopBGMusic();
                }
                break;
            }
            case 0x45: {
                u16 id = curCmd.argByte == 0 ? sub_80A25AC((u16)curCmd.arg1) : curCmd.arg1;
                if (id != 999) {
                    FadeInNewBGM_(id, curCmd.argShort);
                } else {
                    StopBGMusic();
                }
                break;
            }
            case 0x46: {
                u16 id = curCmd.argByte == 0 ? sub_80A25AC((u16)curCmd.arg1) : curCmd.arg1;
                if (id != 999) {
                    QueueBGM_((u16)id);
                }
                break;
            }
            case 0x47: {
                StopBGMusic();
                break;
            }
            case 0x48: {
                FadeOutBGM_(curCmd.argShort < 0 ? 30 : (u16)curCmd.argShort);
                break;
            }
            case 0x49: case 0x4c: {
                PlaySoundWithVolume((u16)curCmd.arg1, 256);
                break;
            }
            case 0x4a: case 0x4d: {
                StopSound((u16)curCmd.arg1);
                break;
            }
            case 0x4b: case 0x4e: {
                FadeOutSound((u16)curCmd.arg1, curCmd.argShort < 0 ? 30 : (u16)curCmd.argShort);
                break;
            }
            case 0x4f: {
                if (action->scriptData2.state == 1) {
                    InitScriptData(&action->scriptData2);
                }
                action->callbacks->setHitboxPos(action->parentObject, 0);
                break;
            }
            case 0x50: {
                Action *ptr;
                PixelPos pos;
                s8 c;
                {
                    Action *tmp = sub_809D52C(&action->unkC);
                    ptr = tmp;
                }
                if (ptr) {
                    ptr->callbacks->getHitboxCenter(action->parentObject, &pos);
                    ptr->callbacks->getDirection(action->parentObject, &c);
                    action->callbacks->moveReal(action->parentObject, &pos);
                    action->scriptData.unk26 = c;
                    action->callbacks->setDirection(action->parentObject, c);
                }
                break;
            }
            case 0x51: {
                PixelPos posIn;
                PixelPos posOut1;
                PixelPos posOut2;
                action->callbacks->getHitboxCenter(action->parentObject, &posIn);
                GroundLink_GetArea(curCmd.argShort, &posOut1, &posOut2, &posIn);
                action->callbacks->setPositionBounds(action->parentObject, &posOut1, &posOut2);
                break;
            }
            case 0x52: {
                action->callbacks->setFlags(action->parentObject, curCmd.arg1);
                break;
            }
            case 0x53: {
                if (curCmd.arg1 & 0x400 && action->scriptData2.state == 1) {
                    InitScriptData(&action->scriptData2);
                }
                action->callbacks->clearFlags(action->parentObject, curCmd.arg1);
                break;
            }
            case 0x54: {
                action->callbacks->getDirection(action->parentObject, &action->scriptData.unk26);
                if (curCmd.argShort) {
                    action->scriptData.unk24 = curCmd.argShort;
                }
                action->callbacks->setEventIndex(action->parentObject, (u16)curCmd.argShort);
                break;
            }
            case 0x55: {
                action->callbacks->livesOnlyNullsub(action->parentObject, (u16)curCmd.argShort);
                break;
            }
            case 0x56: {
                action->callbacks->func38(action->parentObject, (s16)curCmd.arg1, curCmd.argShort);
                break;
            }
            case 0x57: {
                action->callbacks->func44_livesOnlySpriteRelated(action->parentObject, curCmd.argShort);
                break;
            }
            case 0x58: {
                PixelPos unk;
                unk.x = curCmd.arg1 << 8;
                unk.y = curCmd.arg2 << 8;
                action->callbacks->moveReal(action->parentObject, &unk);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x59: {
                PixelPos unk;
                unk.x = curCmd.argShort << 8;
                unk.y = curCmd.arg1 << 8;
                action->callbacks->moveRelative(action->parentObject, &unk);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x5a: {
                u32 unk[2];
                unk[0] = OtherRandInt(curCmd.argShort) << 8;
                unk[1] = OtherRandInt(curCmd.arg1) << 8;
                action->callbacks->moveRelative(action->parentObject,
                                                (PixelPos*)unk);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x5b: {
                PixelPos unk;
                action->callbacks->getHitboxCenter(action->parentObject, &unk);
                GroundLink_GetPos((s16)curCmd.arg1, &unk);
                action->callbacks->moveReal(action->parentObject, &unk);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x5c: {
                PixelPos pos, pos1, pos2;
                action->callbacks->getHitboxCenter(action->parentObject, &pos);
                GroundLink_GetArea((s16)curCmd.arg1, &pos1, &pos2, &pos);
                pos.x = pos1.x + OtherRandInt(pos2.x - pos1.x);
                pos.y = pos1.y + OtherRandInt(pos2.y - pos1.y);
                action->callbacks->moveReal(action->parentObject, &pos);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x5d: {
                PixelPos unk;
                s16 res = sub_80A7AE8((s16)curCmd.arg1);
                if (res >= 0) {
                    sub_80A8FD8(res, &unk);
                    action->callbacks->moveReal(action->parentObject, &unk);
                    scriptData->unk2A = (u8)curCmd.argByte;
                    return 2;
                }
                break;
            }
            case 0x5e: {
                PixelPos pos;
                s32 height;
                s32 dir;
                pos.x = GetScriptVarArrayValue(NULL, POSITION_X, (u16)curCmd.arg1);
                pos.y = GetScriptVarArrayValue(NULL, POSITION_Y, (u16)curCmd.arg1);
                height = GetScriptVarArrayValue(NULL, POSITION_HEIGHT, (u16)curCmd.arg1);
                dir = (s8)GetScriptVarArrayValue(NULL, POSITION_DIRECTION, (u16)curCmd.arg1);
                action->callbacks->moveReal(action->parentObject, &pos);
                action->callbacks->setPosHeight(action->parentObject, height);
                action->scriptData.unk26 = dir;
                action->callbacks->setDirection(action->parentObject, dir);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x5f: {
                PixelPos pos;
                u32 height;
                u32 wat;
                s8 dir;
                action->callbacks->getHitboxCenter(action->parentObject, &pos);
                action->callbacks->getPosHeightAndUnk(action->parentObject, &height, &wat);
                action->callbacks->getDirection(action->parentObject, &dir);
                SetScriptVarArrayValue(NULL, POSITION_X, (u16)curCmd.arg1, pos.x);
                SetScriptVarArrayValue(NULL, POSITION_Y, (u16)curCmd.arg1, pos.y);
                SetScriptVarArrayValue(NULL, POSITION_HEIGHT, (u16)curCmd.arg1, height);
                SetScriptVarArrayValue(NULL, POSITION_DIRECTION, (u16)curCmd.arg1, dir);
                break;
            }
            case 0x60: {
                action->callbacks->setPosHeight(action->parentObject, curCmd.arg1 << 8);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x61: case 0x69: {
                scriptData->pos2.x = curCmd.arg1 << 8;
                scriptData->pos2.y = curCmd.arg2 << 8;
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = -1;
                return 2;
            }
            case 0x62: case 0x6a: {
                PixelPos pos;
                action->callbacks->getHitboxCenter(action->parentObject, &pos);
                scriptData->pos2.x = pos.x + (curCmd.arg1 << 8);
                scriptData->pos2.y = pos.y + (curCmd.arg2 << 8);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = -1;
                return 2;
            }
            case 0x63: case 0x6b: {
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos2);
                GroundLink_GetPos((s16)curCmd.arg1, &scriptData->pos2);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = -1;
                return 2;
            }
            case 0x64: case 0x6c: {
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos2);
                GroundLink_GetPos((s16)curCmd.arg1, &scriptData->pos2);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x65: case 0x6d: {
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos2);
                GroundLink_GetPos((s16)curCmd.arg1, &scriptData->pos2);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = OtherRandInt((u8)curCmd.argByte);
                return 2;
            }
            case 0x66: case 0x67: case 0x6e: case 0x6f: {
                s16 ret = sub_80A7AE8((s16)curCmd.arg1);
                if (ret >= 0) {
                    sub_80A8FD8(ret, &scriptData->pos2);
                    scriptData->unk30 = curCmd.argShort;
                    scriptData->unk2A = (u8)curCmd.argByte;
                    return 2;
                }
                break;
            }
            case 0x71: case 0x77: case 0x7d: case 0x83: {
#define HYPOT FP24_8_Hypot((s24_8){scriptData->pos2.x - scriptData->pos1.x}, (s24_8){scriptData->pos2.y - scriptData->pos1.y}).raw / curCmd.argShort
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                scriptData->pos2.x = curCmd.arg1 << 8;
                scriptData->pos2.y = curCmd.arg2 << 8;
                if (curCmd.op == 0x7d || curCmd.op == 0x83) {
                    scriptData->unk2A = HYPOT;
                    if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                } else {
                    scriptData->unk2A = curCmd.argShort;
                }
                return 2;
            }
            case 0x72: case 0x78: case 0x7e: case 0x84: {
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                scriptData->pos2.x = scriptData->pos1.x + (curCmd.arg1 << 8);
                scriptData->pos2.y = scriptData->pos1.y + (curCmd.arg2 << 8);
                if (curCmd.op == 0x7e || curCmd.op == 0x84) {
                    scriptData->unk2A = HYPOT;
                    if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                } else {
                    scriptData->unk2A = curCmd.argShort;
                }
                return 2;
            }
            case 0x73: case 0x79: case 0x7f: case 0x85: {
                // BUG: (or two): these lines use the wrong script command arguments to calculate the position offset
                // making the target position nonsense. But even if they were correct,
                // the way the cap is calculated would make the random offset biased off-center.
                // This doesn't affect the released version because these script commands are never used.
#ifndef BUGFIX
                s32 cap1 = curCmd.arg1 * 2 - 1;
                s32 cap2 = curCmd.arg2 * 2 - 1;

                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                scriptData->pos2.x = scriptData->pos1.x + ((OtherRandInt(cap1) - curCmd.argShort) << 8);
                scriptData->pos2.y = scriptData->pos1.y + ((OtherRandInt(cap2) - curCmd.arg1) << 8);
#else
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                scriptData->pos2.x = scriptData->pos1.x + ((OtherRandInt(curCmd.arg1 * 2 + 1) - curCmd.arg1) << 8);
                scriptData->pos2.y = scriptData->pos1.y + ((OtherRandInt(curCmd.arg2 * 2 + 1) - curCmd.arg2) << 8);
#endif
                if (curCmd.op == 0x7f || curCmd.op == 0x85) {
                    scriptData->unk2A = HYPOT;
                    if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                } else {
                    scriptData->unk2A = curCmd.argShort;
                }
                return 2;
            }
            case 0x74: case 0x7a: case 0x80: case 0x86: {
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                scriptData->pos2 = scriptData->pos1;
                GroundLink_GetPos((s16)curCmd.arg1, &scriptData->pos2);
                if (curCmd.op == 0x80 || curCmd.op == 0x86) {
                    scriptData->unk2A = HYPOT;
                    if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                } else {
                    scriptData->unk2A = curCmd.argShort;
                }
                return 2;
            }
            case 0x75: case 0x7b: case 0x81: case 0x87: {
                s32 cap = curCmd.arg1 * 2 - 1;
                action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos2);
                GroundLink_GetPos((s16)curCmd.arg2, &scriptData->pos2);
                scriptData->pos2.x = scriptData->pos2.x + ((OtherRandInt(cap) - curCmd.argShort) << 8);
                scriptData->pos2.y = scriptData->pos2.y + ((OtherRandInt(cap) - curCmd.argShort) << 8);
                if (curCmd.op == 0x81 || curCmd.op == 0x87) {
                    scriptData->unk2A = HYPOT;
                    if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                } else {
                    scriptData->unk2A = curCmd.argShort;
                }
                return 2;
            }
            case 0x76: case 0x7c: case 0x82: case 0x88: {
                s16 ret = sub_80A7AE8((s16)curCmd.arg1);
                if (ret >= 0) {
                    action->callbacks->getHitboxCenter(action->parentObject, &scriptData->pos1);
                    sub_80A8FD8(ret, &scriptData->pos2);
                    if (curCmd.op == 0x82 || curCmd.op == 0x88) {
                        scriptData->unk2A = HYPOT;
                        if (scriptData->unk2A <= 0) scriptData->unk2A = 1;
                    } else {
                        scriptData->unk2A = curCmd.argShort;
                    }
                    return 2;
                }
                break;
            }
            case 0x68: case 0x70: {
                scriptData->unk48 = curCmd.arg1 << 8;
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = -1;
                return 2;
            }
            case 0x89: {
                action->scriptData.unk26 = curCmd.arg1;
                action->callbacks->setDirection(action->parentObject, (s8) curCmd.arg1);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x8a: {
                s8 dir;
                action->callbacks->getDirection(action->parentObject, &dir);
                action->scriptData.unk26 = sub_8002984(dir, (s8)curCmd.arg1);
                action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                scriptData->unk30 = curCmd.argShort;
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x8b: {
                action->scriptData.unk26 = curCmd.argShort;
                action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x8c: {
                int ret = (s16)sub_80A7AE8((s16)curCmd.arg1);
                s8 dir;
                if (ret >= 0) {
                    sub_80A9050(ret, &dir);
                    action->scriptData.unk26 = sub_8002984(dir, curCmd.argShort);
                    action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                }
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x8d: {
                s8 dir;
                action->callbacks->getDirection(action->parentObject, &dir);
                action->scriptData.unk26 = sub_8002984(dir, curCmd.argShort);
                action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x8e: case 0x8f: case 0x90: {
                bool8 flag = FALSE;
                s8 dir;
                PixelPos pos1, pos2, pos3, pos4;
                switch (curCmd.op) {
                    case 0x8e: {
                        s32 val = (s16)sub_80A7AE8((s16)curCmd.arg1);
                        if (val >= 0) {
                            flag = TRUE;
                            sub_80A8FD8(val, &pos1);
                            sub_80A8F9C(val, &pos2);
                        }
                        break;
                    }
                    case 0x8f: {
                        s32 val = (s16)sub_80A7AE8((s16)curCmd.arg1);
                        if (val >= 0) {
                            flag = TRUE;
                            sub_80A8FD8(val, &pos1);
                            pos2 = sPixelPosZero;
                        }
                        break;
                    }
                    case 0x90: {
                        flag = TRUE;
                        action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                        action->callbacks->getSize(action->parentObject, &pos2);
                        GroundLink_GetPos((s16)curCmd.arg1, &pos1);
                        break;
                    }
                }
                if (flag) {
                    s8 ret;
                    int tmp;
                    action->callbacks->getHitboxCenter(action->parentObject, &pos3);
                    action->callbacks->getSize(action->parentObject, &pos4);
                    ret = SizedDeltaDirection8(&pos3, &pos4, &pos1, &pos2); // wtf
                    *&dir = ret;

                    tmp = -1;
                    if (dir == tmp) {
                        dir = SizedDeltaDirection4(&pos3, &sPixelPosZero, &pos1, &sPixelPosZero);
                    }
                    if (dir == tmp) {
                        action->callbacks->getDirection(action->parentObject, &dir);
                    }
                    action->scriptData.unk26 = sub_8002984(dir, (s8)curCmd.argShort);
                    action->callbacks->setDirection(action->parentObject, action->scriptData.unk26);
                }
                scriptData->unk2A = (u8)curCmd.argByte;
                return 2;
            }
            case 0x93: case 0x94: case 0x95: {
                scriptData->unk2A = 0;
                return 2;
            }
            case 0x91: {
                action->scriptData.unk4D = (s8)curCmd.arg1;
                scriptData->unk2A = 0;
                return 2;
            }
            case 0x92: {
                s8 unk;
                action->callbacks->getDirection(action->parentObject, &unk);
                action->scriptData.unk4D = sub_8002984(unk, (s8)curCmd.arg1);
                scriptData->unk2A = 0;
                return 2;
            }
            case 0x97: {
                sub_809D124(curCmd.argShort, curCmd.arg1, curCmd.arg2);
                break;
            }
            case 0x98: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                switch(action->unk8.unk0) {
                    case 1:
                        sub_809D170(1, id);
                        break;
                    case 2:
                        sub_809D170(2, id);
                        break;
                    case 3:
                        sub_809D170(3, id);
                        break;
                }
                break;
            }
            case 0x99: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                PixelPos unk;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_80A8FD8(id, &unk);
                        sub_809D158(0, &unk);
                        break;
                    case 2:
                        sub_80AC448(id, &unk);
                        sub_809D158(0, &unk);
                        break;
                    case 3:
                        sub_80AD360(id, &unk);
                        sub_809D158(0, &unk);
                        break;
                }
                break;
            }
            case 0x9a: {
                sub_809D170(1, 0);
                break;
            }
            case 0x9b: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                if (id < 0) break;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_809D1A8(1, id, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_809D1A8(2, id, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_809D1A8(3, id, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0x9c: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                PixelPos unk;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_80A8FD8(id, &unk);
                        sub_809D190(0, &unk, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_80AC448(id, &unk);
                        sub_809D190(0, &unk, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_80AD360(id, &unk);
                        sub_809D190(0, &unk, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0x9d: {
                sub_809D1A8(1, 0, curCmd.argShort);
                return 2;
            }
            case 0x9e: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                if (id < 0) break;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_809D1E4(1, id, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_809D1E4(2, id, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_809D1E4(3, id, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0x9f: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                PixelPos unk;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_80A8FD8(id, &unk);
                        sub_809D1CC(0, &unk, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_80AC448(id, &unk);
                        sub_809D1CC(0, &unk, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_80AD360(id, &unk);
                        sub_809D1CC(0, &unk, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0xa0: {
                sub_809D1E4(1, 0, curCmd.argShort);
                return 2;
            }
            case 0xa1: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                if (id < 0) break;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_809D220(1, id, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_809D220(2, id, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_809D220(3, id, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0xa2: {
                s32 id = action->callbacks->getIndex(action->parentObject);
                PixelPos unk;
                switch(action->unk8.unk0) {
                    case 1:
                        sub_80A8FD8(id, &unk);
                        sub_809D208(0, &unk, curCmd.argShort);
                        return 2;
                    case 2:
                        sub_80AC448(id, &unk);
                        sub_809D208(0, &unk, curCmd.argShort);
                        return 2;
                    case 3:
                        sub_80AD360(id, &unk);
                        sub_809D208(0, &unk, curCmd.argShort);
                        return 2;
                }
                break;
            }
            case 0xa3: {
                sub_809D220(1, 0, curCmd.argShort);
                return 2;
            }
            case 0xa4: {
                ResetScriptVarArray(scriptData->localVars.buf, curCmd.argShort);
                break;
            }
            case 0xa5: {
                ClearScriptVarArray(scriptData->localVars.buf, curCmd.argShort);
                break;
            }
            case 0xa6: {
                UpdateScriptVarWithImmediate(scriptData->localVars.buf, curCmd.argShort, curCmd.arg1, curCmd.argByte);
                break;
            }
            case 0xa7: {
                UpdateScriptVarWithVar(scriptData->localVars.buf, curCmd.argShort, (s16)curCmd.arg1, curCmd.argByte);
                break;
            }
            case 0xa8: {
                SetScriptVarArrayValue(scriptData->localVars.buf, curCmd.argShort, (u16)curCmd.arg1, curCmd.arg2);
                break;
            }
            case 0xa9: {
                ScenarioCalc(curCmd.argShort, curCmd.arg1, curCmd.arg2);
                break;
            }
            case 0xaa: {
                s32 a, b;
                GetScriptVarScenario(curCmd.argShort, &a, &b);
                ScenarioCalc(curCmd.argShort, a, b+1);
                break;
            }
            case 0xab: {
                SetScriptVarValue(NULL, DUNGEON_ENTER, curCmd.arg1);
                SetScriptVarValue(NULL, DUNGEON_RESULT, curCmd.argShort);
                break;
            }
            case 0xac: {
                SetScriptVarValue(NULL, PLAYER_KIND, curCmd.argShort);
                break;
            }
            case 0xad: {
                sub_80026E8(curCmd.argShort, (u8)curCmd.argByte > 0);
                break;
            }
            case 0xae: {
                sub_809733C(curCmd.argShort, (u8)curCmd.argByte > 0);
                break;
            }
            case 0xaf: {
                s16 scriptDungeonId = curCmd.argShort;
                bool8 setGo = ((u8)curCmd.argByte) > 0;
                s32 scen = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                MGBA_Warnf("[GS] script GO op id=%d set=%d scen=%d", scriptDungeonId, setGo, scen);
                if (0 && GetSkipCutscenesSetting()) {
                    if (scriptDungeonId == SCRIPT_DUNGEON_TINY_WOODS) {
                        MGBA_Warnf("[GS] skip: block script GO for Tiny Woods at scen=%d", scen);
                        break;
                    }
                }
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && scriptDungeonId == SCRIPT_DUNGEON_SILENT_CHASM) {
                    if (scen == 5 && !RescueScenarioConquered(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                        MGBA_Warnf("[GS] block script GO for Silent Chasm (scene 5; SW incomplete)");
                        break;
                    }
                    // Also block any attempts to re‑set SC GO once we've advanced beyond it.
                    if (scen >= 7) {
                        MGBA_Warnf("[GS] block script GO for Silent Chasm (scene %d; post-SC)", scen);
                        break;
                    }
                }
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && (scriptDungeonId == SCRIPT_DUNGEON_MT_STEEL || scriptDungeonId == SCRIPT_DUNGEON_3)) {
                    // Never allow re-setting GO on Mt. Steel once we've advanced past it.
                    if (scen >= 5) {
                        MGBA_Warnf("[GS] block script GO for Mt. Steel (id=%d, scene %d)", scriptDungeonId, scen);
                        break;
                    }
                }
                sub_80973A8(scriptDungeonId, setGo);
                // Post-state: log flags for visibility
                {
                    bool8 msGo = sub_8097384(SCRIPT_DUNGEON_MT_STEEL);
                    bool8 swGo = sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS);
                    bool8 scGo = sub_8097384(SCRIPT_DUNGEON_SILENT_CHASM);
                    s32 sel = GetScriptVarValue(NULL, DUNGEON_SELECT);
                    MGBA_Warnf("[GS] state after GO op: MS_GO=%d SW_GO=%d SC_GO=%d sel=%d", msGo, swGo, scGo, sel);
                }
                break;
            }
            case 0xb0: {
                sub_8097418(curCmd.argShort, (u8)curCmd.argByte > 0);
                // Alias fix: some scripts mark an alias id as conquered (e.g., 3),
                // but the active GO for the stage lives on the true script id.
                // When skipping cutscenes, proactively clear the SW GO flag if its
                // alias gets conquered so the UI does not retain a stale GO badge.
                if (UseOldSkipCutsceneFlow() && GetSkipCutscenesSetting() && (u8)curCmd.argByte > 0) {
                    if (curCmd.argShort == SCRIPT_DUNGEON_3) {
                        if (sub_8097384(SCRIPT_DUNGEON_SINISTER_WOODS)) {
                            sub_80973A8(SCRIPT_DUNGEON_SINISTER_WOODS, 0);
                            MGBA_Warnf("[GS] post-SW conquer via alias (id=3): clear SW GO");
                        }
                    }
                    // After Great Canyon is conquered, skip the Hill of the Ancients scene
                    // and jump to Square sleeping (scene 14) with Lapis Cave set as GO.
                    if (curCmd.argShort == SCRIPT_DUNGEON_GREAT_CANYON) {
                        // Only apply the GC -> Square sleeping fast‑forward while still in
                        // the post‑MT arc (before entering Lapis/Mt. Blaze arcs). Prevents
                        // late GC flags from bouncing mid‑Frosty flows.
                        s32 scenNow2 = (s16)GetScriptVarValue(NULL, SCENARIO_MAIN);
                        if (scenNow2 <= 11 && !RescueScenarioConquered(SCRIPT_DUNGEON_LAPIS_CAVE)) {
                            if (sub_8097384(SCRIPT_DUNGEON_GREAT_CANYON)) {
                                sub_80973A8(SCRIPT_DUNGEON_GREAT_CANYON, 0);
                            }
                            sub_8097418(SCRIPT_DUNGEON_GREAT_CANYON, 1);
                            SetScriptVarValue(NULL, SCENARIO_MAIN, 14);
                            MGBA_Warnf("[GS] post-GC conquer: scen=14 -> Square sleeping (await partner talk)");
                            GroundMainGroundRequest(MAP_POKEMON_SQUARE, 0, 30);
                            break;
                        }
                    }
                }
                break;
            }
            case 0xb1: {
                sub_80975A8(curCmd.argShort, (u8)curCmd.argByte > 0);
                break;
            }
            case 0xb2: {
                SetAdventureAchievement(curCmd.argShort);
                break;
            }
            case 0xb3: {
                if (JudgeVarWithImmediate(NULL, curCmd.argShort, curCmd.arg1, JUDGE_EQ)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xb4: {
                if (JudgeVarWithImmediate(scriptData->localVars.buf, (s16)curCmd.arg1, curCmd.arg2, (u8)curCmd.argByte)) {
                    scriptData->script.ptr = FindLabel(action, curCmd.argShort);
                }
                break;
            }
            case 0xb5: {
                if (JudgeVarWithVar(scriptData->localVars.buf, (s16)curCmd.arg1, (s16)curCmd.arg2, (u8)curCmd.argByte)) {
                    scriptData->script.ptr = FindLabel(action, curCmd.argShort);
                }
                break;
            }
            case 0xb6: {
                if (GetScriptVarArrayValue(scriptData->localVars.buf, (s16)curCmd.arg1, (u16)curCmd.arg2)) {
                    scriptData->script.ptr = FindLabel(action, curCmd.argShort);
                }
                break;
            }
            case 0xb7: {
                if (FlagJudge(GetScriptVarArraySum(scriptData->localVars.buf, (s16)curCmd.arg1), curCmd.arg2, (u8)curCmd.argByte)) {
                    scriptData->script.ptr = FindLabel(action, curCmd.argShort);
                }
                break;
            }
            case 0xb8: {
                if (ScriptVarScenarioBefore(curCmd.argShort, curCmd.arg1, curCmd.arg2)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xb9: {
                if (ScriptVarScenarioEqual(curCmd.argShort, curCmd.arg1, curCmd.arg2)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xba: {
                if (ScriptVarScenarioAfter(curCmd.argShort, curCmd.arg1, curCmd.arg2)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xbb: {
                if (sub_80023E4(curCmd.argShort)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xbc: {
                if (sub_8098100(curCmd.argShort)) {
                    scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xbd: {
                if (sub_80026CC(curCmd.arg1)) {
                        scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xbf: {
                if (HasItemInInventory(curCmd.argShort) > 0) {
                        scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                }
                break;
            }
            case 0xbe: {
                if (action->unk8.unk0 == 1) {
                    if ((s8)GroundLives_IsStarterMon(action->unk8.unk2)) {
                        scriptData->script.ptr = FindLabel(action, (u8)curCmd.argByte);
                    }
                }
                break;
            }
            case 0x41: {
                s32 val;
                val = FindItemInInventory(curCmd.argShort);
                if ( val >= 0) ShiftItemsDownFrom(val);
                break;
            }
            case 0xc0 ... 0xcb: {
                s32 val;
                PixelPos pos, pos2, pos3;
                switch (curCmd.op) {
                    case 0xc0: {
                        val = GetScriptVarValue(scriptData->localVars.buf, curCmd.argShort);
                        break;
                    }
                    case 0xc1: {
                        val = FlagCalc(
                            GetScriptVarValue(scriptData->localVars.buf, curCmd.argShort),
                            curCmd.arg1,
                            (u8)curCmd.argByte);
                        break;
                    }
                    case 0xc2: {
                        val = FlagCalc(
                            GetScriptVarValue(scriptData->localVars.buf, curCmd.argShort),
                            GetScriptVarValue(scriptData->localVars.buf, (s16)curCmd.arg1),
                            (u8)curCmd.argByte);
                        break;
                    }
                    case 0xc3: {
                        val = OtherRandInt(curCmd.argShort);
                        break;
                    }
                    case 0xc4: {
                        val = GetScriptVarArrayValue(NULL, curCmd.argShort, 0);
                        break;
                    }
                    case 0xc5: {
                        val = GetScriptVarArrayValue(NULL, curCmd.argShort, 1);
                        break;
                    }
                    case 0xc6: {
                        val = (s16)sub_80A8C2C((s16)curCmd.arg1);
                        break;
                    }
                    case 0xc7: {
                        s8 dir;
                        action->callbacks->getDirection(action->parentObject, &dir);
                        val = dir;
                        break;
                    }
                    case 0xca: {
                        action->callbacks->getHitboxCenter(action->parentObject, &pos);
                        pos2 = pos;
                        GroundLink_GetPos((s16)curCmd.arg1, &pos2);
                        pos3.x = pos2.x - pos.x;
                        pos3.y = pos2.y - pos.y;
                        val = (s8) VecDirection8Radial(&pos3);
                        break;
                    }
                    case 0xc8: {
                        s16 tmp = (s16)sub_80A7AE8((s16)curCmd.arg1);
                        if (tmp >= 0) {
                            PixelPos pos1, pos2, pos3, pos4;
                            action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                            action->callbacks->getSize(action->parentObject, &pos2);
                            sub_80A8FD8(tmp, &pos3);
                            sub_80A8F9C(tmp, &pos4);
                            val = SizedDeltaDirection8(&pos1, &pos2, &pos3, &pos4);
                            if (val == -1) {
                                val = SizedDeltaDirection4(&pos1, &sPixelPosZero, &pos3, &sPixelPosZero);
                            }
                        } else {
                            val = -1;
                        }
                        break;
                    }
                    case 0xc9: {
                        s16 tmp = (s16)sub_80A7AE8((s16)curCmd.arg1);
                        if (tmp >= 0) {
                            PixelPos pos1, pos2, pos3;
                            action->callbacks->getHitboxCenter(action->parentObject, &pos1);
                            action->callbacks->getSize(action->parentObject, &pos2);
                            sub_80A8FD8(tmp, &pos3);
                            val = SizedDeltaDirection8(&pos1, &pos2, &pos3, &sPixelPosZero);
                            if (val == -1) {
                                val = SizedDeltaDirection4(&pos1, &sPixelPosZero, &pos3, &sPixelPosZero);
                            }
                        } else {
                            val = -1;
                        }
                        break;
                    }
                    case 0xcb: {
                        val = CheckScriptItemSpace(curCmd.argShort);
                        break;
                    }
                    default: {
                        // The locdata says this is part of an inlined function... :/
                        FATAL_ERROR_ARGS2("../ground/ground_script.c", 4222, "_AnalyzeProcess", "switch type error %d", curCmd.op);
                    }
                }
                scriptData->script.ptr = ResolveJump(action, val);
                break;
            }
            case 0xcf: {
                scriptData->branchDiscriminant = GetScriptVarValue(scriptData->localVars.buf, curCmd.argShort);
                while (scriptData->script.ptr->op == 0xd0) {
                    if (scriptData->script.ptr->argShort == scriptData->branchDiscriminant)
                        return 2;
                    scriptData->script.ptr++;
                }
                if (scriptData->script.ptr->op == 0xd1) {
                    scriptData->branchDiscriminant = -1;
                    return 2;
                }
                break;
            }
            case 0xd2 ... 0xd8: {
                // DS: Assert(TRUE, "Script command call error SWITCH MENY") [sic]
                const char *out = curCmd.argPtr;
                sNumChoices = 0;
                scriptData->branchDiscriminant = 0;
                switch(curCmd.op) {
                    case 0xd6: case 0xd7: case 0xd8: {
                        s32 disc = GetScriptVarValue(scriptData->localVars.buf, (s16)curCmd.arg2);
                        for (; scriptData->script.ptr->op == 0xd0; scriptData->script.ptr++, scriptData->branchDiscriminant++) {
                            if (scriptData->script.ptr->argShort == disc)
                                out = scriptData->script.ptr->argPtr;
                        }
                        for (; scriptData->script.ptr->op == 0xd1; scriptData->script.ptr++, scriptData->branchDiscriminant++) {
                            if (!out) out = scriptData->script.ptr->argPtr;
                        }
                    }
                }
                if (!out) out = "";
                for (; scriptData->script.ptr->op == 0xd9; scriptData->script.ptr++) {
                    gChoices[sNumChoices].text = scriptData->script.ptr->argPtr;
                    gChoices[sNumChoices].menuAction = sNumChoices + 1;
                    sNumChoices++;
                }
                if (sNumChoices <= 0) break;
                gChoices[sNumChoices].text = NULL;
                gChoices[sNumChoices].menuAction = curCmd.argShort;
                switch (curCmd.op) {
                    case 0xd2: case 0xd3: case 0xd6: {
                        sub_809B028(gChoices, (u8)curCmd.argByte > 0, -1, 0, (s16)curCmd.arg1, out);
                        break;
                    }
                    case 0xd4: case 0xd7: {
                        sub_809B028(gChoices, (u8)curCmd.argByte > 0, -1, 1, (s16)curCmd.arg1, out);
                        break;
                    }
                    case 0xd5: case 0xd8: {
                        sub_809B028(gChoices, (u8)curCmd.argByte > 0, -1, 2, (s16)curCmd.arg1, out);
                        break;
                    }
                }
                if (GroundScriptCheckLockCondition(action, 1)) {
                    sub_80A87AC(0, 11);
                    return 2;
                }
                break;
            }
            case 0xda: {
                if (GroundScriptCheckLockCondition(action, 1)) {
                    return 2;
                }
                break;
            }
            case 0xdb: {
                scriptData->unk2A = curCmd.argShort;
                return 2;
            }
            case 0xdc: {
                scriptData->unk2A = curCmd.argShort + OtherRandInt(curCmd.arg1 - curCmd.argShort);
                return 2;
            }
            case 0xdd ... 0xe2: {
                return 2; // do action
            }
            case 0xe3: {
                scriptData->branchDiscriminant = curCmd.argShort;
                if (GroundScriptCheckLockCondition(action, curCmd.argShort)) {
                    return 2;
                }
                break;
            }
            case 0xe4: {
                GroundScriptLockJumpZero(curCmd.argShort);
                break;
            }
            case 0xe5: {
                scriptData->branchDiscriminant = curCmd.argShort;
                if (GroundScriptLockCond(action, curCmd.argShort, curCmd.argByte)) {
                    return 2;
                }
                break;
            }
            case 0xe6: {
                scriptData->savedScript = scriptData->script;
            } //fallthrough
            case 0xe7: {
                scriptData->script.ptr = FindLabel(action, curCmd.argShort);
                break;
            }
            case 0xe8: {
                scriptData->savedScript = scriptData->script;
            } // fallthrough
            case 0xe9: {
                scriptData->script.ptr2 = scriptData->script.ptr = gFunctionScriptTable[curCmd.argShort].script;
                break;
            }
            case 0xea: {
                scriptData->savedScript = scriptData->script;
            } // fallthrough
            case 0xeb: {
                ScriptInfoSmall info;
                u32 group, sector;
                u32 tmp = gUnknown_2039A34;
                {
                    s32 tmp = curCmd.argShort < 0 ? scriptData->script.group : curCmd.argShort;
                    group = tmp;
                }
                {
                    s32 tmp = (s8)curCmd.argByte < 0 ? scriptData->script.sector : (s8)curCmd.argByte;
                    sector = tmp;
                }
                GroundMap_GetStationScript(&info, tmp, group, sector);
                scriptData->script.ptr = info.ptr;
                scriptData->script.ptr2 = info.ptr;
                scriptData->script.group = group;
                scriptData->script.sector = sector;
                break;
            }
            case 0xec: {
                gUnknown_2039A34 = GetAdjustedGroundMap((s16)GetScriptVarValue(scriptData->localVars.buf, curCmd.argShort));
                GroundCancelAllEntities();
                GroundMap_ExecuteEnter(gUnknown_2039A34);
                break;
            }
            case 0xed: {
                // DS: if (scriptData->savedScript.ptr == NULL) Assert(FALSE, "Script flash stack error");
                scriptData->savedScript.ptr = NULL;
                scriptData->savedScript.ptr2 = NULL;
                break;
            }
            case 0xee: {
                if (scriptData->savedScript.ptr == NULL) {
                    // DS: Assert(FALSE, "Script return stack error");
                    return 0;
                }
                scriptData->script = scriptData->savedScript;
                scriptData->savedScript.ptr = NULL;
                scriptData->savedScript.ptr2 = NULL;
                break;
            }
            case 0xef:
                // DS: if (scriptData->savedScript.ptr != NULL) Assert(FALSE, "Script end stack error");
                return 0;
            case 0xf0:
                return 1;
            case 0xf1:
                return 4; // fatal error?
            case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: {
                // Debug, not in release ROM
                break;
            }
        }
    }
}

UNUSED static u32 sub_80A1440(s32 r0, s32 r1, s32 r2)
{
   return sub_80A14E8(NULL, r0, r1, r2);
}

UNUSED static bool8 GroundScript_ExecuteTrigger(s16 r0)
{
    s32 ret;
    ScriptInfoSmall scriptInfo;
    Action action;
    const ScriptRef *ptr;

    ptr = &gFunctionScriptTable[r0];

    if(ptr->type != 0xB)
        return FALSE;
    InitActionWithParams(&action, &sNullCallbackData, NULL, 0, 0);
    GetFunctionScript(NULL, &scriptInfo, r0);
    GroundScript_ExecutePP(&action, NULL, &scriptInfo, DEBUG_LOC_PTR("../ground/ground_script.c", 4553, "GroundScript_ExecuteTrigger"));

    action.scriptData.savedScript = action.scriptData.script;
    action.scriptData.savedScript.ptr = gUnknown_81164E4;
    action.scriptData.savedScript.ptr2 = gUnknown_81164E4;
    ret = HandleAction(&action, DEBUG_LOC_PTR("../ground/ground_script.c", 4558, "GroundScript_ExecuteTrigger"));
    InitAction2(&action);
    if(ret == 0)
        return TRUE;
    else
        return FALSE;
}

s32 sub_80A14E8(Action *action, u8 idx, u32 r2, s32 r3)
{
    switch(idx)
    {
        case 1:
            sub_8098C58();
            return 0;
        case 2:
            if(sub_8011C34() == -1)
                sub_8011C40(Rand32Bit());
            sub_8011C28(1);
            sub_8001064();
            sub_809965C();
            return 0;
        case 3:
            sub_8098CC8();
            return 0;
        case 4:
            return sub_80961D8() == 0 ? 0 : 1;
        case 5:
            ResetMailbox();
            return 0;
        case 6:
            sub_80963FC();
            return 0;
        case 7:
            sub_8096488();
            return 0;
        case 8:
            return sub_80964B4() == 0 ? 0 : 1;
        case 9:
            {
                u8 var;
                if (sub_80023E4(8) == 0) {
                    var = sub_808D4B0();
                }
                else {
                    var =  sub_808D500();
                }
                nullsub_104();
                return var != 0;
            }
            break;
        case 0xA:
            {
                if ((action->unkC).unk0 == 1)
                {
                    u8 text[0x100];
                    DungeonLocation dungLocation;
                    s32 ret = sub_80A8C4C(action->unkC.unk2, &dungLocation);
                    if (ret != 0)
                    {
                        s32 dialogueId;
                        if (dungLocation.id == DUNGEON_HOWLING_FOREST_2 && ret == 0x104) {
                            return 2;
                        }

                        dialogueId = GetFriendAreaDialogueId(action->unkC.unk2);
                        InlineStrcpy(text, gFriendAreaDialogue[dialogueId]);
                        if (ScriptPrintText(0, 1, text) != 0)
                            return 1;
                    }
                }
            }
            return 0;
        case 0xB:
            return (CountJobsinDungeon(ScriptDungeonIdToDungeonId(GetScriptVarValue(0, DUNGEON_ENTER_INDEX))) > 0);
        case 0xC:
            {
                u8 sp_104;
                if(sub_8099394(&sp_104) != 0)
                {
                    unkStruct_203B480 *p = gUnknown_203B480;
                    p += sp_104;
                    if(p->rescuesAllowed > 0)
                        return 1;
                }
            }
            return 0;
        case 0xD:
            return sub_8097640() ? 1 : 0;
            break;
        case 0xE:
            {
                s32 index;
                for(index = 0x11; index < NUM_DUNGEON_MAZE; index++)
                {
                    if (IsMazeCompleted((s16) index)) {
                        if (!GetScriptVarArrayValue(0, TRAINING_PRESENT_LIST, (u16) index)) {
                            SetScriptVarArrayValue(0, TRAINING_PRESENT_LIST, (u16) index, 1);
                            return index - 0x10;
                        }
                    }
                }
                return 0;
            }
        case 0xF:
            return sub_80964E4() == 0 ? 0 : 1;
        case 0x10:
            if(action->unk8.unk0 == 1)
                if(action->unk8.unk2 == 0)
                    if(action->unkC.unk0 == 1)
                    {
                        if(sub_80A87E0(action->unk8.unk2, sub_80A8E9C(action->unkC.unk2)) != 0)
                            return 1;
                    }
            return 0;
        case 0x11:
            return sub_80A8D20() == 0 ? 0 : 1;
        case 0x12:
            {
                s32 held = gRealInputs.held;
                if((held & (R_BUTTON | L_BUTTON)))
                    return 1;
                else
                    return 0;
            }

        case 0x13:
            {
                Pokemon *ptr;

                ptr = sub_80A8D54(r2);
                if(ptr)
                    return PokemonFlag2(ptr);
            }
            return 0;
        case 0x14:
            if(action->unk8.unk0 == 1)  {
                if(GetCanMoveFlag(sub_80A8BFC(action->unk8.unk2)))
                    return 1;
            }
            return 0;
        case 0x15:
            {
                s32 r4;
                PixelPos sp_318;
                PixelPos sp_320;
                s32 r5 = (s16) sub_80A7AE8(r2);
                if(r5 >= 0) {
                    r4 = (r3 << 8);
                    action->callbacks->getHitboxCenter(action->parentObject, &sp_318);
                    sub_80A8FD8(r5, &sp_320);

                    if(sp_318.x - r4 <= sp_320.x
                        && sp_318.x + r4 >= sp_320.x
                        && sp_318.y - r4 <= sp_320.y
                        && sp_318.y + r4 >= sp_320.y)
                    {
                        return 1;
                    }
                }
            }
            return 0;
        case 0x16:
            {
                s32 index;
                Pokemon *ptr; ptr = sub_80A8D54(1);
                if(ptr)
                {
                    for(index = 0; index < POKEMON_NAME_LENGTH; index++)
                    {
                        ptr->name[index] = 0;
                    }
                    return 1;
                }
            }
            return 0;
        case 0x17:
            UnlockFriendArea(WILD_PLAINS);
            UnlockFriendArea(MIST_RISE_FOREST);
            return 0;
        case 0x18:
            sub_80A8F50(sPokeNameBuffer, 0x3C, POKEMON_NAME_LENGTH);
            return 0;
        case 0x19:
            UnlockFriendArea(GetFriendArea(MONSTER_MAGNEMITE));
            {
                struct StoryMonData magnemiteData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_MAGNEMITE,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_POKEMON_SQUARE_2, .floor = 0},
                    .moveID = {MOVE_METAL_SOUND, MOVE_TACKLE, MOVE_THUNDERSHOCK, MOVE_NOTHING},
                    .pokeHP = 38,
                    .level = 6,
                    .IQ = 1,
                    .offenseAtk = {20, 18},
                    .offenseDef = {20, 18},
                    .currExp = 4560,
                };
                Pokemon magnemiteMon;
                Pokemon *recruitPtr;
                s32 index;

                ConvertStoryMonToPokemon(&magnemiteMon, &magnemiteData);
                recruitPtr = TryAddPokemonToRecruited(&magnemiteMon);
                if (recruitPtr == NULL)
                    return 1;
                for (index = 0; index < POKEMON_NAME_LENGTH; index++) {
                    recruitPtr->name[index] = sPokeNameBuffer[index];
                }
                StrncpyCustom(gFormatBuffer_Names[r2], sPokeNameBuffer, POKEMON_NAME_LENGTH);
                IncrementAdventureNumJoined();
                return 0;
            }
            break;
        case 0x1A:
            sub_80A8F50(sPokeNameBuffer, 0x53, POKEMON_NAME_LENGTH);
            return 0;
        case 0x1B:
             UnlockFriendArea(GetFriendArea(MONSTER_ABSOL));
             {
                Pokemon *recruitPtr;
                struct StoryMonData absolData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_ABSOL,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_FROSTY_GROTTO_2, .floor = 0},
                    .moveID = {MOVE_SCRATCH, MOVE_LEER, MOVE_TAUNT, MOVE_QUICK_ATTACK},
                    .pokeHP = 80,
                    .level = 20,
                    .IQ = 1,
                    .offenseAtk = {33, 32},
                    .offenseDef = {31, 32},
                    .currExp = 43000,
                };
                Pokemon absolMon;

                ConvertStoryMonToPokemon(&absolMon, &absolData);
                recruitPtr = TryAddPokemonToRecruited(&absolMon);
                if (recruitPtr == NULL)
                    return 1;
                IncrementAdventureNumJoined();
                recruitPtr->flags |= POKEMON_FLAG_ON_TEAM;
                return 0;
             }
        case 0x1C:
            {
                Pokemon *pokemon = GetRecruitedMonBySpecies(MONSTER_ABSOL, 0);
                if (pokemon == NULL)
                    return 1;
                pokemon->flags |= POKEMON_FLAG_ON_TEAM;
                return 0;
            }
        case 0x1D:
            {
                if( sub_808D278(MONSTER_SMEARGLE) == 0)
                    return 1;
                else
                    return 0;
            }
        case 0x1E:
             sub_80A8F50(sPokeNameBuffer, 0x7C, POKEMON_NAME_LENGTH);
             return 0;

        case 0x1F:
            {
                Pokemon *recruitPtr;
                s32 index;
                struct StoryMonData smeargleData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_SMEARGLE,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_HOWLING_FOREST_2, .floor = 0},
                    .moveID = {MOVE_SKETCH, MOVE_NOTHING, MOVE_NOTHING, MOVE_NOTHING},
                    .pokeHP = 47,
                    .level = 5,
                    .IQ = 1,
                    .offenseAtk = {16, 20},
                    .offenseDef = {20, 16},
                    .currExp = 1600,
                };
                Pokemon smeargleMon;

                ConvertStoryMonToPokemon(&smeargleMon, &smeargleData);
                recruitPtr = TryAddPokemonToRecruited(&smeargleMon);
                if (!recruitPtr)
                    return 1;
                for (index = 0; index < POKEMON_NAME_LENGTH; index++) {
                    recruitPtr->name[index] = sPokeNameBuffer[index];
                }
                StrncpyCustom(gFormatBuffer_Names[r2], sPokeNameBuffer, POKEMON_NAME_LENGTH);
                IncrementAdventureNumJoined();
                return 0;
            }
            break;
        case 0x20:
            sub_80026E8(0x9E, 0x1);
            if (GetRecruitedMonBySpecies(MONSTER_ZAPDOS, 0) == NULL) {
                static const DungeonLocation zapdosLoc = {.id = DUNGEON_MT_THUNDER_PEAK, .floor = 99};
                if (TryAddLevel1PokemonToRecruited(MONSTER_ZAPDOS, NULL, ITEM_NOTHING, &zapdosLoc, NULL))
                    IncrementAdventureNumJoined();
            }
            if (GetRecruitedMonBySpecies(MONSTER_MOLTRES, 0) == NULL) {
                static const DungeonLocation moltresLoc = {.id = DUNGEON_MT_BLAZE_PEAK, .floor = 99};
                if (TryAddLevel1PokemonToRecruited(MONSTER_MOLTRES, NULL, ITEM_NOTHING, &moltresLoc, NULL))
                    IncrementAdventureNumJoined();
            }
            if (GetRecruitedMonBySpecies(MONSTER_ARTICUNO, 0) == NULL) {
                static const DungeonLocation articunoLoc = {.id = DUNGEON_FROSTY_GROTTO, .floor = 99};
                if (TryAddLevel1PokemonToRecruited(MONSTER_ARTICUNO, NULL, ITEM_NOTHING, &articunoLoc, NULL))
                    IncrementAdventureNumJoined();
            }
            return 0;
        case 0x21:
            if (HasRecruitedMon(MONSTER_ARTICUNO) && HasRecruitedMon(MONSTER_ZAPDOS) && HasRecruitedMon(MONSTER_MOLTRES))
                return 1;
            return 0;
        case 0x22:
            {
                Pokemon *pokemon = GetPlayerPokemonStruct();

                if (pokemon->speciesNum != MONSTER_ARTICUNO && pokemon->speciesNum != MONSTER_ZAPDOS && pokemon->speciesNum != MONSTER_MOLTRES)
                    return 1;
            }
            return 0;
         case 0x23:
            {
                Pokemon *pokemon;
                pokemon = GetPlayerPokemonStruct();
                if (pokemon != NULL && pokemon->speciesNum == MONSTER_HO_OH)
                    return 2;
                else if (sub_8098134(MONSTER_HO_OH) != 0)
                    return 1;
            }
            return 0;
        case 0x24:
            sub_80A8F50(sPokeNameBuffer, 0x79, POKEMON_NAME_LENGTH);
            return 0;

        case 0x25:
            if (!GetFriendAreaStatus(GetFriendArea(MONSTER_LATIOS)))
                UnlockFriendArea(GetFriendArea(MONSTER_LATIOS));
            {
                Pokemon *recruitPtr;
                s32 index;
                struct StoryMonData latiosData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_LATIOS,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_POKEMON_SQUARE, .floor = 0},
                    .moveID = {MOVE_PSYWAVE, MOVE_MEMENTO, MOVE_HELPING_HAND, MOVE_SAFEGUARD},
                    .pokeHP = 125,
                    .level = 30,
                    .IQ = 1,
                    .offenseAtk = {60, 59},
                    .offenseDef = {42, 44},
                    .currExp = 273400,
                };
                Pokemon latiosMon;

                ConvertStoryMonToPokemon(&latiosMon, &latiosData);
                recruitPtr = TryAddPokemonToRecruited(&latiosMon);
                if (recruitPtr == NULL)
                    return 1;
                for (index = 0; index < POKEMON_NAME_LENGTH; index++) {
                    recruitPtr->name[index] = sPokeNameBuffer[index];
                }
                IncrementAdventureNumJoined();
                return 0;
            }

            break;
        case 0x26:
            sub_80A8F50(sPokeNameBuffer, 0x7A, POKEMON_NAME_LENGTH);
            return 0;
        case 0x27:
            {
                Pokemon *recruitPtr;
                s32 index;
                struct StoryMonData latiasData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_LATIAS,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_POKEMON_SQUARE, .floor = 0},
                    .moveID = {MOVE_PSYWAVE, MOVE_WISH, MOVE_HELPING_HAND, MOVE_SAFEGUARD},
                    .pokeHP = 120,
                    .level = 28,
                    .IQ = 1,
                    .offenseAtk = {58, 57},
                    .offenseDef = {40, 43},
                    .currExp = 245400,
                };
                Pokemon latiasMon;

                ConvertStoryMonToPokemon(&latiasMon, &latiasData);
                recruitPtr = TryAddPokemonToRecruited(&latiasMon);
                if (recruitPtr == NULL)
                    return 1;
                for (index = 0; index < POKEMON_NAME_LENGTH; index++) {
                    recruitPtr->name[index] = sPokeNameBuffer[index];
                }
                IncrementAdventureNumJoined();
                return 0;
            }
            break;

        case 0x28:
            {
                static const Item item = {.flags = 0, .quantity = 0, .id = ITEM_WISH_STONE};
                if (GetNumberOfFilledInventorySlots() >= INVENTORY_SIZE) {
                    if (IsNotMoneyOrUsedTMItem(item.id) && gTeamInventoryRef->teamStorage[item.id] < 999)
                        gTeamInventoryRef->teamStorage[item.id] += 1;

                }
                else {
                    AddItemIdToInventory(item.id, FALSE);
                    FillInventoryGaps();
                }
                return 0;
            }
        case 0x29:
            {
                s32 index = (s16)(RandInt(0x1A2) + 1);
                s32 r4 = RandInt(0x20);
                s32 r7 = 0x1A4;
                s32 r6 = 0;

                while (1)
                {
                    s16 indexS16;
                    index = (s16)(index + 1);
                    if (index >= 0x1a4) {
                        index = 1;
                    }
                    if (--r7 < 0) {
                        if (r6 == 0) {
                            SetScriptVarValue(NULL, NEW_FRIEND_KIND, 0);
                            return 0;
                        }
                        if (r4 >= r6) {
                            r4 = RandInt(r6);
                            r7 = 0x1A4;
                        }
                    }
                    // S16 memes AGAIN...
                    indexS16 = index;
                    index = indexS16;
                    if (indexS16 == MONSTER_ARTICUNO || indexS16 == MONSTER_ZAPDOS || indexS16 == MONSTER_MOLTRES
                        || indexS16 == MONSTER_GROUDON
                        || indexS16 == MONSTER_RAYQUAZA
                        || indexS16 == MONSTER_RAYQUAZA_CUTSCENE
                        || indexS16 == MONSTER_KYOGRE
                        || indexS16 == MONSTER_LUGIA
                        || indexS16 == MONSTER_CASTFORM_SNOWY
                        || indexS16 == MONSTER_CASTFORM_SUNNY
                        || indexS16 == MONSTER_CASTFORM_RAINY
                        || indexS16 == MONSTER_DEOXYS_NORMAL
                        || indexS16 == MONSTER_DEOXYS_ATTACK
                        || indexS16 == MONSTER_DEOXYS_DEFENSE
                        || indexS16 == MONSTER_DEOXYS_SPEED
                        || indexS16 == MONSTER_MEWTWO
                        || indexS16 == MONSTER_JIRACHI
                        || indexS16 == MONSTER_MEW
                        || indexS16 == MONSTER_LATIAS
                        || indexS16 == MONSTER_LATIOS
                        || indexS16 == MONSTER_ENTEI
                        || indexS16 == MONSTER_RAIKOU
                        || indexS16 == MONSTER_SUICUNE
                        || indexS16 == MONSTER_HO_OH
                        || indexS16 == MONSTER_REGIROCK
                        || indexS16 == MONSTER_REGICE
                        || indexS16 == MONSTER_REGISTEEL
                        || indexS16 == MONSTER_CELEBI
                        )
                        continue;
                    if(!IsExclusivePokemonUnlocked(index)) continue;
                    if(GetFriendAreaStatus(GetFriendArea(index))) {
                        if(HasRecruitedMon(index)) continue;
                        if(sub_808D278(index) == 0) continue;
                    }

                    r6++;
                    r4--;
                    if (r4 < 0)
                        break;
                }

                SetScriptVarValue(NULL, NEW_FRIEND_KIND, index);
                WriteFriendAreaName(gFormatBuffer_FriendArea, GetFriendArea(index), FALSE);
                if (GetFriendAreaStatus(GetFriendArea(index)) != 0)
                    return 1;
                else
                    return 2;
            }
        case 0x2A:
            {
                s32 id = (s16) GetScriptVarValue(0, NEW_FRIEND_KIND);
                WriteFriendAreaName(gFormatBuffer_FriendArea,(GetFriendArea(id)), FALSE);
                if (id == 0)
                    return 0;
                else if(GetFriendAreaStatus(GetFriendArea(id)) != 0)
                   return 1;
                else
                   return 2;
            }
            break;
        case 0x2B:
            sub_80A8F50(sPokeNameBuffer, 0x20, POKEMON_NAME_LENGTH);
            return 0;
        case 0x2C:
            {
                Pokemon *recruitPtr;
                s32 index;
                if (r2 != 0) {
                    static const DungeonLocation dungLoc = {.id = DUNGEON_RESCUE_TEAM_BASE_2, .floor = 0};
                    s32 id = (s16) GetScriptVarValue(0, NEW_FRIEND_KIND);
                    s32 id_ = id;
                    WriteFriendAreaName(gFormatBuffer_FriendArea,(GetFriendArea((s16)id)), FALSE);
                    if (id == 0)
                        return 0;

                    if (!GetFriendAreaStatus(GetFriendArea(id)))
                        UnlockFriendArea(GetFriendArea(id));

                    recruitPtr = TryAddLevel1PokemonToRecruited(id_, NULL ,ITEM_NOTHING, &dungLoc, MOVE_NOTHING);
                    if (recruitPtr == NULL)
                        return 0;

                    for (index = 0; index < POKEMON_NAME_LENGTH; index++)
                        recruitPtr->name[index] = sPokeNameBuffer[index];
                    IncrementAdventureNumJoined();
                    return 1;
                }
                else {
                    SetScriptVarValue(NULL, NEW_FRIEND_KIND, 0);
                    return 0;
                }
            }

        case 0x2D:
            if(GetPtsToNextRank() > 0)
                return 1;
            else
                return 0;
        case 0x2E:
            {
                s32 rankBefore = GetRescueTeamRank();
                s32 points = GetPtsToNextRank();
                if (points > 0) {
                    s32 rankAfter;
                    AddToTeamRankPts(points);
                    rankAfter = GetRescueTeamRank();
                    InlineStrcpy(gFormatBuffer_Items[0], GetTeamRankString(rankBefore));
                    InlineStrcpy(gFormatBuffer_Items[1], GetTeamRankString(rankAfter));
                    if (ScriptPrintText(0, -1, _("{CENTER_ALIGN}The rescue rank went up from\n{CENTER_ALIGN}{MOVE_ITEM_0} to {MOVE_ITEM_1}!")) != 0)
                        return 1;
                }
                else {
                    return 0;
                }
            }
        // breakthrough
        case 0x2F:
            AddToTeamMoney(10000);
            return 0;
        case 0x30:
            if (sub_808D278(MONSTER_GARDEVOIR) == 0)
                return 1;
            else
                return 0;
        case 0x31:
            sub_80A8F50(sPokeNameBuffer, 0x52, POKEMON_NAME_LENGTH);
            return 0;

        case 0x32:
            {
                struct StoryMonData gardevoirData = {
                    .name = sPokeNameBuffer,
                    .speciesNum = MONSTER_GARDEVOIR,
                    .itemID = ITEM_NOTHING,
                    .dungeonLocation = {.id = DUNGEON_RESCUE_TEAM_BASE_2, .floor = 0},
                    .moveID = {MOVE_CONFUSION, MOVE_DOUBLE_TEAM, MOVE_TELEPORT, MOVE_GROWL},
                    .pokeHP = 53,
                    .level = 5,
                    .IQ = 1,
                    .offenseAtk = {18, 18},
                    .offenseDef = {11, 10},
                    .currExp = 2800,
                };
                Pokemon gardevoirMon;

                ConvertStoryMonToPokemon(&gardevoirMon, &gardevoirData);
                if (TryAddPokemonToRecruited(&gardevoirMon) == 0) {
                    return 1;
                }
                else {
                    IncrementAdventureNumJoined();
                    return 0;
                }
            }
        case 0x33:
            if (ScriptVarScenarioAfter(SCENARIO_MAIN, 0x12, -1)
               && GetScriptVarValue(0, GROUND_GETOUT) != 4
               && GetScriptVarArrayValue(0, EVENT_GONBE, 0) <= 0)
            {
                if (OtherRandInt(0x100) == 0) {
                    SetScriptVarArrayValue(0, EVENT_GONBE, 0, 4);
                    return 1;
                }
                else {
                    SetScriptVarArrayValue(0, EVENT_GONBE, 0, 1);
                }
            }
            return 0;
        case 0x34:
            {
                s32 i;
                static const Item appleItem = {.flags = 0, .quantity = 0, .id = ITEM_APPLE};
                for (i = 0; i < 3; i++) {
                    if (GetNumberOfFilledInventorySlots() >= INVENTORY_SIZE) {
                        if (IsNotMoneyOrUsedTMItem(appleItem.id) && gTeamInventoryRef->teamStorage[appleItem.id] < 999)
                            gTeamInventoryRef->teamStorage[appleItem.id] += 1;

                    }
                    else {
                        AddItemIdToInventory(appleItem.id, 0);
                        FillInventoryGaps();
                    }
                }
            }
            return 0;
        case 0x35:
            {
                Pokemon *pokemon = GetPlayerPokemonStruct();
                if(pokemon != NULL && pokemon->speciesNum == MONSTER_CHANSEY)
                    return 2;
                else
                    if(sub_8098134(MONSTER_CHANSEY) != 0)
                        return 1;
                    else
                        return 0;
            }
            break;

        case 0x38:
            sub_80A56A0(0, 1);
            return 0;
        case 0x36: {
            PixelPos sp_328;
            sp_328.x = r2;
            sp_328.y = r3;
            sub_80A56F0(&sp_328);
            return 0;
        }
        case 0x37: {
            PixelPos sp_330;
            sp_330.x = r2;
            sp_330.y = r3;
            sub_80A5704(&sp_330);
            return 0;
        }
        case 0x39:
            sub_809C6CC(r2);
            return 0;
        case 0x3A:
            sub_809C6EC();
            return 0;
        case 0x3B:
            sub_809C760();
            return 0;
        case 0x3C:
            {
                s32 index;
                index = 0;
                for(index = 0; index < 0x18; index = (s16)(index + 1))
                {
                    sub_80A86C8(index, 0x400000);
                }
                for (index = 0; index < 0x10; index = (s16)(index + 1))
                {
                    sub_80AC1B0(index, 0x400000);
                }
                for (index = 0; index < 0x10; index = (s16)(index + 1))
                {
                    sub_80AD0C8(index, 0x400000);
                }
            }
            return 0;
        case 0x3D: {
            s32 sp_338[2];
            sp_338[0] = r2;
            sp_338[1] = r3;
            sub_80A59A0(0, sp_338, sub_80A5984(1, sp_338));
            return 0;
        }
        case 0x3E:
            {
                s32 ret;
                Action *r7;
                GroundEffectData sp_308;
                PixelPos sp_340;
                PixelPos sp_348;
                PixelPos sp_350;
                PixelPos sp_358;

                sp_308.kind = 1;
                sp_308.unk1 = 0;
                sp_308.width = 1;
                sp_308.height = 1;
                sp_308.pos = (CompactPos) {0};
                sp_308.script = gFunctionScriptTable[406].script; // MOVE_DEBUG_CAMERA
                ret = GroundEffect_Add(-1, &sp_308, r2, r3);
                if(ret < 0) break;
                r7 = sub_80AD158(ret);
                sub_80A579C(&sp_340, &sp_348);
                sp_340.y += 0xC00;
                sp_348.y += 0xC00;
                action->callbacks->getHitboxCenter(action->parentObject, &sp_350);
                if(sp_350.x < sp_340.x)
                    sp_350.x = sp_340.x;
                else if (sp_350.x >= sp_348.x)
                    sp_350.x = sp_348.x - 1;
                if(sp_350.y < sp_340.y)
                    sp_350.y = sp_340.y;
                else if (sp_350.y >= sp_348.y)
                    sp_350.y = sp_348.y - 1;

                sp_340.x -= 1024;
                sp_340.y -= 1024;
                sp_348.x += 1024;
                sp_348.y += 1024;

                r7->callbacks->setPositionBounds(r7->parentObject, &sp_340, &sp_348);
                r7->callbacks->moveReal(r7->parentObject, &sp_350);
                sp_358 = (PixelPos) {0};
                r7->callbacks->moveRelative(r7->parentObject, &sp_358);
                return 1;
            }
            break;

        case 0x3F:
            {
                s32 index;
                s32 held = gRealInputs.held;
                s32 pressed = gRealInputs.pressed;
                if(!(pressed & (START_BUTTON | SELECT_BUTTON)))
                {
                    PixelPos sp_368;
                    s32 dir = DpadToDirection(held);
                    if((s8)dir != -1)
                    {
                        s32 to;
                        sp_368 = SetVecFromDirectionSpeed(dir,0x100);

                        to = 2;
                        if(held & B_BUTTON) {
                            to = 4;
                        }

                        for(index = 0; index < to; index++)
                        {
                            if(action->callbacks->moveRelative(action->parentObject, &sp_368) != 0)
                            {
                                PixelPos pixelPos = {0, sp_368.y};
                                if(action->callbacks->moveRelative(action->parentObject, &pixelPos) != 0)
                                {
                                    PixelPos pixelPos = {sp_368.x, 0};
                                    action->callbacks->moveRelative(action->parentObject, &pixelPos);
                                }
                            }
                        }
                    }
                    return -1;
                }
            }
            break;

        case 0x40:
            sub_80993C0(r2 == 0 ? 0 : 1);
            return 0;
        case 0x42:
            sub_8011C28(1);
            GroundMainGameEndRequest(r2);
            FadeOutAllMusic(r2);
            return 0;
        case 0x41:
            GroundMainGameEndRequest(r2);
            FadeOutAllMusic(r2);
            return 0;
        case 0x43:
            gUnknown_2039DA8 = GetCurrentBGSong();
            if(gUnknown_2039DA8 != STOP_BGM)
                return 1;
            return 0;
        case 0x44:
            if (gUnknown_2039DA8 != STOP_BGM)
            {
                StartNewBGM_(gUnknown_2039DA8);
                gUnknown_2039DA8 = STOP_BGM;
                return 1;
            }
            return 0;
        case 0x45:
            if (gUnknown_2039DA8 != STOP_BGM)
            {
                FadeInNewBGM_(gUnknown_2039DA8, r2);
                gUnknown_2039DA8 = STOP_BGM;
                return 1;
            }
            return 0;
        case 0x46:
            if (gUnknown_2039DA8 != STOP_BGM)
            {
                QueueBGM_(gUnknown_2039DA8);
                gUnknown_2039DA8 = STOP_BGM;
                return 1;
            }
            return 0;
        case 0x47:
            sub_80997F4(r2, r3);
            return 0;
        case 0x48:
            sub_80997F4(0x1E, r2);
            return 0;
        case 0x49:
            GroundSprite_ExtendPaletteAdd(0, r2);
            return 0;
        case 0x4A:
            GroundSprite_ExtendPaletteDelete(0);
            return 0;
    }

    return 0;
}

void GroundScript_Unlock(void)
{
    s32 index;
    bool8 cond;

    if(gAnyScriptLocked == 0) return;

    gAnyScriptLocked = 0;
    index = 0;
    for (index = 0; index < SCRIPT_LOCKS_ARR_COUNT; index++) {
        if(gScriptLocks[index] != 0) {
            Log(1, "GroundScript unlock %3d", index);
            cond  = GroundMapNotifyAll(index);
            cond |= GroundLivesNotifyAll(index);
            cond |= GroundObjectsNotifyAll(index);
            cond |= GroundEffectsNotifyAll(index);

            if(gScriptLockConds[index] != 0) {
               if (cond) {
                    GroundMapNotifyAll(index | 0x80);
                    GroundLivesNotifyAll(index | 0x80);
                    GroundObjectsNotifyAll(index | 0x80);
                    GroundEffectsNotifyAll(index | 0x80);
                    gScriptLocks[index] = gScriptLockConds[index] = 0;
               }
            } else {
               gScriptLocks[index] = 0;
            }
        }
    }
}

static const ScriptCommand *FindLabel(Action *action, s32 r1)
{
    ScriptCommand script;
    const ScriptCommand *scriptPtr2;
    const ScriptCommand *scriptPtr;

    scriptPtr2 = action->scriptData.script.ptr2;
    scriptPtr = scriptPtr2 + 1;

    while(1) {
        script = *scriptPtr;
        scriptPtr++;

        if (script.op == 0xF4 && r1 == script.argShort) break;

        {
            UNUSED static const u8 maybeFuncName[] = "_SearchScriptLabel";
            UNUSED static const DebugLocation unusedDebugLoc = {"../ground/ground_script.c", 5822, maybeFuncName};
            UNUSED static const u8 scrLabelError[] = "Script label search error %d";
        }
        // DS: Assert(script.op != 0, "script search label error %d", label)
        // DS: Assert(script.op != 0xF6, "script search label error %d", label)
    }
    return scriptPtr;
}

static const ScriptCommand *ResolveJump(Action *action, s32 r1)
{
    ScriptCommand script;
    const ScriptCommand *scriptPtr;
    s32 temp;

    scriptPtr = action->scriptData.script.ptr;

    while(1) {
        script = *scriptPtr;

        if (script.op == 0xCC)
        {
            if(FlagJudge(r1, script.arg1, JUDGE_EQ))
                return FindLabel(action, script.argShort);
        }
        else if (script.op == 0xCD)
        {
            if(FlagJudge(r1, script.arg1, script.argByte))
                return FindLabel(action, script.argShort);
        }
        else if (script.op == 0xCE)
        {
            temp = GetScriptVarValue(action->scriptData.localVars.buf, (s16)script.arg1);
            if(FlagJudge(r1, temp, script.argByte))
                return FindLabel(action, script.argShort);
        }
        else
        {
            return scriptPtr;
        }
        scriptPtr++;
    }
}

static void sub_80A2500(s32 param_1, ActionUnkIds *param_2)
{
    if (param_2->unk0 == 1) {
        sub_809AB4C((s16) param_1, sub_80A8BBC(param_2->unk2));
    }
}

static void sub_80A252C(s32 param_1, ActionUnkIds *param_2)
{
    if (param_2->unk0 == 1) {
        sub_809ABB4((s16) param_1, sub_80A8BBC(param_2->unk2));
    }
}

static void sub_80A2558(s32 param_1, ActionUnkIds *param_2)
{
    if (param_2->unk0 == 1) {
        sub_809AC18((s16) param_1, sub_80A8BBC(param_2->unk2));
    }
}

static void sub_80A2584(s16 r0, s16 r1)
{
    s32 iVar2 = r1;
    s32 iVar1 = r0;
    sub_809ABB4(iVar1, iVar2);
}

static void sub_80A2598(s16 r0, s16 r1)
{
    s32 iVar2 = r1;
    s32 iVar1 = r0;
    sub_809AC18(iVar1, iVar2);
}

static u32 sub_80A25AC(u16 param_1)
{
    if (sub_8098F88())
        return param_1;
    if (param_1 == 50)
        return 50;
    if (!sub_80023E4(12))
        return 999;
    if (sub_80023E4(13))
        return 19;
    if (param_1 != 1)
        return param_1;
    GetScriptVarValue(NULL, BASE_LEVEL); // wut???
    return 1;
}
