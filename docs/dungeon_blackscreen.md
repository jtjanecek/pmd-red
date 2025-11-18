# Dungeon Black Screen Investigation

## Problem
Black screen when entering Tiny Woods dungeon. Log stops at:
```
[WARN] GBA Debug: GroundMain recue request   0  30
```

Never reaches dungeon initialization code (no "[Dungeon] Setup parameters loaded" logs appear).

## Hypothesis
1. **Town Square remnants**: Code path still references deleted Town Square functions/data
2. **Memory allocation failure**: Something allocates before RunDungeon and fails
3. **Invalid state transition**: Ground mode exit broken by Town Square removal
4. **Missing initialization**: Some required setup step was in Town Square code

## Investigation Plan

### Step 1: Add Extensive Logging to Dungeon Entry Flow
Track the complete call chain from "GroundMain recue request" to RunDungeon:

**Files to instrument:**
- `src/main_loops.c` - Main game loop that calls dungeon
- `src/ground_main.c` - Ground mode that initiates dungeon request
- `src/run_dungeon.c` - Entry point for dungeon (already has some logging)
- Any intermediary functions between ground and dungeon

**Goal:** Identify the exact function/line where execution stops

### Step 2: Check for Town Square References
Search for any remaining Town Square code in the dungeon entry path:

**Actions:**
- Grep for Town Square map IDs (what was the ID?)
- Search for Square-related function calls
- Check if any deleted code is still referenced

### Step 3: Memory Allocation Audit
Track all allocations between ground exit and dungeon entry:

**Actions:**
- Add logging to MemoryAlloc to see what's being allocated
- Check heap state before dungeon entry
- Verify no allocation is failing silently

### Step 4: State Transition Analysis
Verify ground→dungeon transition logic:

**Actions:**
- Check if any required initialization is missing
- Verify game state flags are set correctly
- Ensure all cleanup completes before dungeon starts

## Execution Log

### Step 1: Dungeon Entry Flow Logging ✓

**Test 1 Results:**
- `GroundMainRescueRequest()` **completes successfully**
  - dungeonId=0, r1=30
  - Calls `sub_80A2750(0)` → returns 2
  - Sets `gUnknown_20398A8` state to 6
  - Calls `sub_809C730()` → returns successfully
- Crash happens **AFTER** RescueRequest returns

**Test 2 In Progress:**
- Added logging to ground cleanup sequence (lines 346-368)
- Added logging to state 6 handler (line 409-419)
- Testing to find which cleanup function hangs/crashes

**Key Functions in Cleanup Sequence:**
1. `FreeGroundMapAction()` through `TextboxFree()`
2. `sub_809C618()`
3. `sub_80A658C()` - Sprite cleanup (calls our modified `sub_800DB50`)
4. `sub_809D508()`
5. `sub_80A7754()`
6. State 6 handler - sets dungeon vars, returns 8

### Steps Status
- [x] Step 1: Add logging to dungeon entry flow
- [ ] Step 2: Check Town Square references
- [ ] Step 3: Memory allocation audit
- [ ] Step 4: State transition analysis

## Current Status
EWRAM: 96.57% (253,148 B / 256 KB)
HEAP_SIZE: 0x22000 (139,264 bytes)
Goal: Keep EWRAM low while fixing dungeon entry

**Next Test:** Identify which function in cleanup sequence crashes
