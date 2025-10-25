#ifndef GUARD_STORY_DEBUG_H
#define GUARD_STORY_DEBUG_H

#include "global.h"
#include "event_flag.h"
#include "script_vars_info.h"
#include "mgba_log.h"
#include "constants/event_flag.h"

// Helper to build a compact 0/1 bitstring for a bit-array script var.
static inline void SD_BuildBitString(char *out, size_t outSize, s16 varId)
{
    const struct ScriptVarInfo *info;
    s32 len;
    s32 i;
    if (outSize == 0)
        return;
    info = &gScriptVarInfo[varId];
    len = info->arrayLen;
    if ((s32)outSize < len + 1)
        len = (s32)outSize - 1;
    for (i = 0; i < len; i++)
        out[i] = GetScriptVarArrayValue(NULL, varId, i) ? '1' : '0';
    out[len] = '\0';
}

// Pretty-print key story/progression state to the emulator log.
//
// Verbosity control (compile-time):
//   STORY_DEBUG_VERBOSITY = 0  -> disabled (no logs)
//   STORY_DEBUG_VERBOSITY = 1  -> summary only (SCEN/MODE lines)
//   STORY_DEBUG_VERBOSITY = 2  -> full detail (includes long bitstrings)
#ifndef STORY_DEBUG_VERBOSITY
#define STORY_DEBUG_VERBOSITY 1
#endif

// 'tag' is a short context like "load", "save", "ground".
static inline void DumpStoryFlags(const char *tag)
{
    s32 mainM, mainS;
    s32 s1M, s1S, s2M, s2S, s3M, s3S, s4M, s4S, s5M, s5S, s6M, s6S, s7M, s7S, s8M, s8S, s9M, s9S;
    s32 start, ge, gel, go, ds, de, dei, dr, warpL;
    char goOrder[65];
    char goJob[65];
    char goConq[65];
    char enterList[100];
    char clearList[100];
    char warpList[20];

#if STORY_DEBUG_VERBOSITY == 0
    (void)tag; // suppress unused warning
    return;
#endif

    // Scenario pairs (main, sub) for main + 1..9
    mainM = GetScriptVarArrayValue(NULL, SCENARIO_MAIN, 0);
    mainS = GetScriptVarArrayValue(NULL, SCENARIO_MAIN, 1);
    s1M = GetScriptVarArrayValue(NULL, SCENARIO_SUB1, 0);
    s1S = GetScriptVarArrayValue(NULL, SCENARIO_SUB1, 1);
    s2M = GetScriptVarArrayValue(NULL, SCENARIO_SUB2, 0);
    s2S = GetScriptVarArrayValue(NULL, SCENARIO_SUB2, 1);
    s3M = GetScriptVarArrayValue(NULL, SCENARIO_SUB3, 0);
    s3S = GetScriptVarArrayValue(NULL, SCENARIO_SUB3, 1);
    s4M = GetScriptVarArrayValue(NULL, SCENARIO_SUB4, 0);
    s4S = GetScriptVarArrayValue(NULL, SCENARIO_SUB4, 1);
    s5M = GetScriptVarArrayValue(NULL, SCENARIO_SUB5, 0);
    s5S = GetScriptVarArrayValue(NULL, SCENARIO_SUB5, 1);
    s6M = GetScriptVarArrayValue(NULL, SCENARIO_SUB6, 0);
    s6S = GetScriptVarArrayValue(NULL, SCENARIO_SUB6, 1);
    s7M = GetScriptVarArrayValue(NULL, SCENARIO_SUB7, 0);
    s7S = GetScriptVarArrayValue(NULL, SCENARIO_SUB7, 1);
    s8M = GetScriptVarArrayValue(NULL, SCENARIO_SUB8, 0);
    s8S = GetScriptVarArrayValue(NULL, SCENARIO_SUB8, 1);
    s9M = GetScriptVarArrayValue(NULL, SCENARIO_SUB9, 0);
    s9S = GetScriptVarArrayValue(NULL, SCENARIO_SUB9, 1);

    start = GetScriptVarValue(NULL, START_MODE);
    ge = GetScriptVarValue(NULL, GROUND_ENTER);
    gel = GetScriptVarValue(NULL, GROUND_ENTER_LINK);
    go = GetScriptVarValue(NULL, GROUND_GETOUT);
    ds = GetScriptVarValue(NULL, DUNGEON_SELECT);
    de = GetScriptVarValue(NULL, DUNGEON_ENTER);
    dei = GetScriptVarValue(NULL, DUNGEON_ENTER_INDEX);
    dr = GetScriptVarValue(NULL, DUNGEON_RESULT);
    warpL = GetScriptVarValue(NULL, WARP_LOCK);

    MGBA_Warnf("[STATE %s] SCEN=[%d,%d] S1=[%d,%d] S2=[%d,%d] S3=[%d,%d] S4=[%d,%d] S5=[%d,%d] S6=[%d,%d] S7=[%d,%d] S8=[%d,%d] S9=[%d,%d]",
               tag ? tag : "", mainM, mainS, s1M, s1S, s2M, s2S, s3M, s3S,
               s4M, s4S, s5M, s5S, s6M, s6S, s7M, s7S, s8M, s8S, s9M, s9S);
    MGBA_Warnf("[STATE %s] MODE=%d GE=%d GEL=%d GO=%d DS=%d DE=%d DEI=%d DR=%d WL=%d",
               tag ? tag : "", start, ge, gel, go, ds, de, dei, dr, warpL);

    if (STORY_DEBUG_VERBOSITY >= 2) {
        SD_BuildBitString(goOrder, (size_t)sizeof(goOrder), RESCUE_SCENARIO_ORDER_LIST);
        SD_BuildBitString(goJob, (size_t)sizeof(goJob), RESCUE_SCENARIO_JOB_LIST);
        SD_BuildBitString(goConq, (size_t)sizeof(goConq), RESCUE_SCENARIO_CONQUEST_LIST);
        SD_BuildBitString(enterList, (size_t)sizeof(enterList), DUNGEON_ENTER_LIST);
        SD_BuildBitString(clearList, (size_t)sizeof(clearList), DUNGEON_CLEAR_LIST);
        SD_BuildBitString(warpList, (size_t)sizeof(warpList), WARP_LIST);

        MGBA_Warnf("[STATE %s] GO-ORDER=%s", tag ? tag : "", goOrder);
        MGBA_Warnf("[STATE %s] GO-JOB  =%s", tag ? tag : "", goJob);
        MGBA_Warnf("[STATE %s] GO-CONQ =%s", tag ? tag : "", goConq);
        MGBA_Warnf("[STATE %s] ENTER   =%s", tag ? tag : "", enterList);
        MGBA_Warnf("[STATE %s] CLEAR   =%s", tag ? tag : "", clearList);
        MGBA_Warnf("[STATE %s] WARP    =%s", tag ? tag : "", warpList);
    }
}

#endif // GUARD_STORY_DEBUG_H
