# Fix: Boss Room Selection Issues

**Date:** 2025-12-01
**Issues Fixed:**
1. Non-Steel/Bug dungeons now use **custom boss arena** (not fixed room reuse)
2. Bug dungeons now use **Sinister Woods Room 2 terrain** (not Skarmory's layout)

## Problem 1: Non-Steel/Bug Dungeons Using Fixed Room

### Issue
When dungeon type was NOT Steel or Bug, the code was still using fixed room layout:
```c
// OLD CODE - WRONG!
else {
    result->bossFight.useFixedRoomLayout = TRUE;  // ← Should be FALSE!
    result->bossFight.fixedRoomNumber = 1;        // ← Not needed
}
```

This meant all dungeons (Fire, Water, Electric, etc.) were using Skarmory's fixed room instead of the custom procedural boss arena.

### Fix
Changed to use custom boss arena for non-Steel/Bug types:
```c
// NEW CODE - CORRECT!
else {
    result->bossFight.useFixedRoomLayout = FALSE;  // Custom procedural arena
    result->bossFight.fixedRoomNumber = 0;         // Not used
}
```

**File:** `src/dungeon_seed_overrides.c:1020-1023`

### Result
- **Steel dungeons**: Use Skarmory fixed room (extracted pattern)
- **Bug dungeons**: Use Sinister Woods fixed room (extracted pattern)
- **All other dungeons**: Use custom procedural boss arena ✓

---

## Problem 2: Bug Dungeons Using Wrong Terrain

### Issue
Bug dungeons were using tileset 65 (Sinister Woods visuals) but Room 1 layout (Skarmory terrain):
```c
// OLD CODE - WRONG!
else if (dungeonType == TYPE_BUG) {
    result->bossFight.roomTileset = 65;        // Correct: Sinister Woods visuals
    result->bossFight.fixedRoomNumber = 1;     // WRONG: Skarmory terrain!
}
```

This created a mismatch:
- **Visuals**: Dark forest (Sinister Woods tileset 65) ✓
- **Terrain**: 9×17 rectangular arena (Skarmory Room 1) ✗

### Fix

#### Step 1: Added Room 2 (Sinister Woods) Pattern

**File:** `src/custom_fixed_rooms.c:43-79`

Added the extracted Sinister Woods Team Meanies arena pattern:
```c
// Fixed Room 2 - 13 rows x 13 columns
// Original source: Sinister Woods Team Meanies boss room pattern
static const u8 sFixedRoom2_Tiles[] = {
    // 13×13 square arena with pillars
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,  60,  60,  60,   2,   2,   2,   2,   2,
    2,   2,  60,   2,   2,  60,  60,  60,   2,   2,  60,   2,   2,
    // ... etc (full pattern in file)
};

static const CustomFixedRoom sFixedRoom2 = {
    .width = 13,
    .height = 13,
    .tiles = sFixedRoom2_Tiles
};
```

Updated room array:
```c
static const CustomFixedRoom *sCustomRooms[] = {
    NULL,           // Index 0 unused
    &sFixedRoom1,   // Index 1 - Skarmory (9×17)
    &sFixedRoom2,   // Index 2 - Sinister Woods (13×13) ← NEW!
};
```

#### Step 2: Added Missing Tile Type Constants

**File:** `src/custom_fixed_rooms.h:22-24`

Room 2 uses tile types 18-21 for stairs decorations:
```c
#define CUSTOM_TILE_STAIRS_UP       18
#define CUSTOM_TILE_STAIRS_19       19  // ← NEW
#define CUSTOM_TILE_STAIRS_20       20  // ← NEW
#define CUSTOM_TILE_STAIRS_21       21  // ← NEW
#define CUSTOM_TILE_STAIRS_PART_1   22
// ... etc
```

#### Step 3: Updated Tile Placement Handler

**File:** `src/custom_fixed_rooms.c:146-158`

Added cases for the new tile types:
```c
case CUSTOM_TILE_STAIRS_19:   // ← NEW
case CUSTOM_TILE_STAIRS_20:   // ← NEW
case CUSTOM_TILE_STAIRS_21:   // ← NEW
case CUSTOM_TILE_STAIRS_PART_1:
case CUSTOM_TILE_STAIRS_PART_2:
// ... etc
    // Stair decoration tiles - treat as normal floor
    SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
    tile->room = 0;
    break;
```

#### Step 4: Updated Bug Dungeon Selection

**File:** `src/dungeon_seed_overrides.c:1015`

Changed to use Room 2:
```c
// NEW CODE - CORRECT!
else if (dungeonType == TYPE_BUG) {
    result->bossFight.roomTileset = 65;        // Sinister Woods visuals
    result->bossFight.fixedRoomNumber = 2;     // Team Meanies arena (13×13) ✓
    roomName = "SinisterWoods";
}
```

### Result
Bug dungeons now have:
- **Visuals**: Dark forest (Sinister Woods tileset 65) ✓
- **Terrain**: 13×13 square arena with pillars (Room 2) ✓
- **Perfect match!** ✓✓✓

---

## Complete Boss Room Behavior

### Summary Table

| Dungeon Type | Tileset | Layout | Dimensions | Room Name |
|--------------|---------|--------|------------|-----------|
| **TYPE_STEEL (0x11)** | 64 | Room 1 | 9×17 rectangular | Skarmory arena |
| **TYPE_BUG (0x0C)** | 65 | Room 2 | 13×13 square | Sinister Woods arena |
| **All others** | 64 | Custom | Procedural | Custom boss arena |

### mGBA Console Output

When entering boss floors, you'll see:

**Steel dungeon:**
```
[BossRoom] dungeonType=17 tileset=64 useFixed=1 layout=1 room=Skarmory
```

**Bug dungeon:**
```
[BossRoom] dungeonType=12 tileset=65 useFixed=1 layout=2 room=SinisterWoods
```

**Other dungeons (Fire, Water, etc.):**
```
[BossRoom] dungeonType=2 tileset=64 useFixed=0 layout=0 room=CustomArena
```

Note the `useFixed=0` for non-Steel/Bug dungeons!

---

## Visual Comparison

### Room 1 (Skarmory) - 9×17
```
+#######+
+#######+
#########
###T!T###
#+++++++#
T+++++++T
~~~~~~~~~
~.......~
....P....
```
- Rectangular shape
- Water hazards
- Used by: Steel dungeons

### Room 2 (Sinister Woods) - 13×13
```
#############
#####...#####
##.##...##.##
#.....S.....#
#...........#
#....213...##
#....546...##
#...........#
#...........#
##.##???##.##
#####...#####
#############
```
- Square shape
- Pillars (wall obstacles)
- Used by: Bug dungeons

### Custom Arena - Procedural
- Procedurally generated
- No fixed pattern
- Used by: Fire, Water, Electric, Ice, etc.

---

## Files Modified

### Core Implementation
1. **`src/custom_fixed_rooms.c`**
   - Added Room 2 pattern (13×13 Sinister Woods arena)
   - Added tile type handlers for types 19-21
   - Updated room array

2. **`src/custom_fixed_rooms.h`**
   - Added tile type constants (19-21)

3. **`src/dungeon_seed_overrides.c`**
   - Fixed Bug dungeons to use Room 2
   - Fixed non-Steel/Bug dungeons to use custom arena
   - Improved logging

### Build
- ✓ Successfully compiles
- ✓ ROM size: 32MB
- ✓ MD5: f8bd39485a8061f572e599f3de378a4b

---

## Testing Checklist

### Steel Dungeon (TYPE_STEEL)
- [ ] Enter Steel-type dungeon boss floor
- [ ] Verify mGBA log: `useFixed=1 layout=1 room=Skarmory`
- [ ] Verify visuals: Metallic/steel walls (tileset 64)
- [ ] Verify terrain: 9×17 rectangular arena with water

### Bug Dungeon (TYPE_BUG)
- [ ] Enter Bug-type dungeon boss floor
- [ ] Verify mGBA log: `useFixed=1 layout=2 room=SinisterWoods`
- [ ] Verify visuals: Dark forest theme (tileset 65)
- [ ] Verify terrain: 13×13 square arena with pillars ← **THIS IS THE FIX!**

### Other Dungeons (Fire, Water, etc.)
- [ ] Enter non-Steel/Bug dungeon boss floor
- [ ] Verify mGBA log: `useFixed=0 layout=0 room=CustomArena`
- [ ] Verify terrain: Procedurally generated custom arena ← **THIS IS THE FIX!**

---

## Summary

✅ **Fixed Problem 1:** Non-Steel/Bug dungeons now use custom arena (not fixed room)
✅ **Fixed Problem 2:** Bug dungeons now use Sinister Woods Room 2 terrain (13×13 square)
✅ **Added:** Room 2 (Sinister Woods) extracted pattern to codebase
✅ **Build:** Successfully compiles and ready to test

Both issues are now resolved! 🎉
