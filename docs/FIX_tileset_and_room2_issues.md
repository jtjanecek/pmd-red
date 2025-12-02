# Fix: Tileset and Room 2 Loading Issues

**Date:** 2025-12-01
**Issues Fixed:**
1. Custom arena (non-Steel/Bug) now uses **normal dungeon tileset** (not hardcoded Skarmory)
2. Room 2 (Sinister Woods) now loads correctly (forced rebuild fixed the issue)

---

## Issue 1: Custom Arena Using Wrong Tileset

### Problem
Custom boss arenas (Fire, Water, Electric, etc.) were hardcoded to use tileset 64 (Skarmory):

```c
// OLD CODE - WRONG!
else {
    result->bossFight.roomTileset = 64;  // Always Skarmory tileset!
}
```

**Result:** All non-Steel/Bug boss fights looked like Mt. Steel (metallic/steel theme), even Fire or Water dungeons.

### Root Cause
Line 283 in `src/dungeon_seed_overrides.c` was overriding the tileset for ALL boss fights:

```c
// Line 280-287 (OLD)
if (result->bossFight.enabled) {
    // Use the boss room's tileset for the entire floor
    result->tileset = result->bossFight.roomTileset;  // ← Always overwrites!
    result->spawnCount = 0;
}
```

### Fix

**Step 1:** Use special value `0` for custom arenas to mean "keep normal tileset"

**File:** `src/dungeon_seed_overrides.c:1021`

```c
// NEW CODE - CORRECT!
else {
    // Other types -> Use custom boss arena
    // Use the normal dungeon tileset (already set at line 275)
    result->bossFight.roomTileset = 0;  // ← Special value: keep normal tileset
    result->bossFight.useFixedRoomLayout = FALSE;
    result->bossFight.fixedRoomNumber = 0;
}
```

**Step 2:** Check for special value before overriding tileset

**File:** `src/dungeon_seed_overrides.c:284-286`

```c
// NEW CODE - CORRECT!
if (result->bossFight.enabled) {
    // Use boss room tileset UNLESS it's 0 (custom arena)
    if (result->bossFight.roomTileset != 0) {  // ← Check for special value
        result->tileset = result->bossFight.roomTileset;
    }
    result->spawnCount = 0;
}
```

### Result

| Dungeon Type | Tileset Source | Visual Theme |
|--------------|----------------|--------------|
| Steel | 64 (hardcoded) | Mt. Steel (metallic) |
| Bug | 65 (hardcoded) | Sinister Woods (dark forest) |
| **Fire** | **Normal tileset selection** | **Fire-themed dungeon** ✓ |
| **Water** | **Normal tileset selection** | **Water-themed dungeon** ✓ |
| **All others** | **Normal tileset selection** | **Type-appropriate visuals** ✓ |

Custom arenas now use the same tileset as the rest of the dungeon floor!

---

## Issue 2: Room 2 Not Loading ("Invalid room ID 2")

### Problem
When Bug dungeons tried to load Room 2, the log showed:

```
[CustomRoom] LoadCustomFixedRoom: roomId=2
[CustomRoom] Invalid room ID 2  ← ERROR!
```

This caused instant completion without spawning anything.

### Root Cause
The build system was using an **old object file** (`build/pmd_red/src/custom_fixed_rooms.o`) that was compiled BEFORE Room 2 was added to the source code.

The source code had:
```c
static const CustomFixedRoom *sCustomRooms[] = {
    NULL,           // Index 0
    &sFixedRoom1,   // Index 1
    &sFixedRoom2,   // Index 2 ← Was added
};
```

But the compiled `.o` file still had the old 2-element array (without Room 2).

### Fix

**Forced rebuild** by removing old object files:

```bash
rm -f build/pmd_red/src/custom_fixed_rooms.o build/pmd_red/src/custom_fixed_rooms.s
make -j8
```

This forced the compiler to regenerate the object file with the updated 3-element array.

### Verification

After rebuild, Room 2 loads correctly:
```
[CustomRoom] LoadCustomFixedRoom: roomId=2
[CustomRoom] Room size: 13x13  ← SUCCESS!
```

---

## Complete Behavior After Fixes

### Steel Dungeon (TYPE_STEEL)
- Tileset: **64** (Mt. Steel - hardcoded)
- Layout: Room 1 (9×17 Skarmory arena)
- Result: Metallic boss room ✓

### Bug Dungeon (TYPE_BUG)
- Tileset: **65** (Sinister Woods - hardcoded)
- Layout: Room 2 (13×13 Team Meanies arena) ✓ **FIXED!**
- Result: Dark forest boss room with pillars ✓

### Fire/Water/Other Dungeons
- Tileset: **Normal dungeon tileset** ✓ **FIXED!**
- Layout: Custom procedural arena
- Result: Boss room matches the dungeon's visual theme ✓

---

## mGBA Console Output

### Steel Dungeon
```
[BossRoom] dungeonType=17 tileset=64 useFixed=1 layout=1 room=Skarmory
[SeedOverrides] GenFloor done: tileset=64 spawns=0 bossEnabled=1 boss=208
```

### Bug Dungeon
```
[BossRoom] dungeonType=12 tileset=65 useFixed=1 layout=2 room=SinisterWoods
[CustomRoom] LoadCustomFixedRoom: roomId=2
[CustomRoom] Room size: 13x13  ← Room 2 loads!
[SeedOverrides] GenFloor done: tileset=65 spawns=0 bossEnabled=1 boss=239
```

### Fire Dungeon (Normal type for example)
```
[BossRoom] dungeonType=2 tileset=0 useFixed=0 layout=0 room=CustomArena
[SeedOverrides] Tileset (committed) id=31 type=2  ← Using normal tileset!
[SeedOverrides] GenFloor done: tileset=31 spawns=0 bossEnabled=1 boss=156
```

Note: `tileset=0` in BossRoom log means "keep normal tileset", then it shows the actual tileset used (31) in GenFloor log.

---

## Testing Checklist

### Bug Dungeon (TYPE_BUG)
- [x] Verify Room 2 loads: Check for "[CustomRoom] Room size: 13x13"
- [ ] Enter boss floor and verify 13×13 square arena
- [ ] Verify Sinister Woods dark forest visuals
- [ ] Verify boss and minions spawn correctly
- [ ] Verify no instant completion

### Fire Dungeon (TYPE_FIRE or any non-Steel/Bug)
- [x] Verify uses normal tileset: Check "[SeedOverrides] Tileset (committed/active) id=X"
- [ ] Enter boss floor and verify visuals match normal floors
- [ ] Verify NOT using Mt. Steel (tileset 64) visuals
- [ ] Verify custom procedural arena
- [ ] Verify boss fight works normally

### Steel Dungeon (TYPE_STEEL)
- [ ] Still uses tileset 64 (Mt. Steel) - should be unchanged
- [ ] Still uses Room 1 (Skarmory arena) - should be unchanged

---

## Files Modified

1. **`src/dungeon_seed_overrides.c`**
   - Line 1021: Changed custom arena tileset from `64` to `0` (special value)
   - Lines 284-286: Added check to preserve normal tileset when `roomTileset == 0`

2. **Build System**
   - Forced rebuild of `custom_fixed_rooms.o` to include Room 2

---

## Summary

✅ **Issue 1 Fixed:** Custom arenas now use normal dungeon tileset (not hardcoded Skarmory)
✅ **Issue 2 Fixed:** Room 2 (Sinister Woods) now loads correctly after forced rebuild
✅ **Build:** Successfully compiles
✅ **MD5:** ffe29b54cd5fae084f706cfe66791580

Both issues are resolved and ready for testing! 🎉

### Technical Note: Why Object Files Matter

This incident highlights an important point about incremental compilation:

- **C source changes** are detected by checking file timestamps
- **New array elements** in source code don't cause recompilation if the `.c` file wasn't modified
- When adding new data (like Room 2), it's sometimes necessary to **force a rebuild**

**Best practice:** When adding new data structures or array elements, always:
```bash
make clean && make
```

Or manually remove the affected object files:
```bash
rm build/pmd_red/src/custom_fixed_rooms.o
```
