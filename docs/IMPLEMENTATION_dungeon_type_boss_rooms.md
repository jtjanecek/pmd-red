# Implementation: Dungeon-Type-Based Boss Rooms

**Date:** 2025-12-01
**Feature:** Boss rooms now change based on the **dungeon type**, not the boss Pokemon's type

## What Was Implemented

Boss fights now use different boss room visuals and layouts depending on the **dungeon's type** (from TypeSelection system), not the boss Pokemon's type.

### Current Behavior

| Dungeon Type | Boss Room Tileset | Boss Room Layout | Visual Theme |
|--------------|-------------------|------------------|--------------|
| **TYPE_STEEL** | 64 | Room 1 (Skarmory) | Mt. Steel - Metallic/steel theme |
| **TYPE_BUG** | 65 | Room 1 (Skarmory)* | Sinister Woods - Dark forest theme |
| **All others** | 64 | Room 1 (Skarmory) | Default custom arena |

\* *Note: Currently uses Room 1 layout. Will use Room 2 (Team Meanies) once it's added to `custom_fixed_rooms.c`*

## Code Changes

### File Modified: `src/dungeon_seed_overrides.c`

**Location:** Lines 992-1030

**What changed:**

Replaced hardcoded boss room selection:
```c
// OLD CODE (lines 993-1000):
result->bossFight.roomTileset = 64;  // Always Skarmory
result->bossFight.useFixedRoomLayout = TRUE;
result->bossFight.fixedRoomNumber = 1;  // Always Room 1
```

With dungeon-type-based selection:
```c
// NEW CODE (lines 992-1030):
// Select boss room based on dungeon type (not boss type!)
{
    u8 dungeonType = TYPE_NONE;
    const char *roomName = "unknown";

    // Get the current dungeon's type
    if (TypeSelection_HasActiveType())
        dungeonType = TypeSelection_GetActiveType();
    else if (TypeSelection_HasCommittedType())
        dungeonType = TypeSelection_GetCommittedType();

    // Choose tileset and layout based on dungeon type
    if (dungeonType == TYPE_STEEL) {
        // Steel dungeon -> Use Skarmory's boss room
        result->bossFight.roomTileset = 64;
        result->bossFight.useFixedRoomLayout = TRUE;
        result->bossFight.fixedRoomNumber = 1;
        roomName = "Skarmory";
    }
    else if (dungeonType == TYPE_BUG) {
        // Bug dungeon -> Use Sinister Woods boss room
        result->bossFight.roomTileset = 65;
        result->bossFight.useFixedRoomLayout = TRUE;
        result->bossFight.fixedRoomNumber = 1;  // TODO: Change to 2 when available
        roomName = "SinisterWoods";
    }
    else {
        // Other types -> Use custom boss arena (our default)
        result->bossFight.roomTileset = 64;
        result->bossFight.useFixedRoomLayout = TRUE;
        result->bossFight.fixedRoomNumber = 1;
        roomName = "Custom";
    }

    MGBA_Warnf("[BossRoom] dungeonType=%d tileset=%d layout=%d room=%s",
               dungeonType, result->bossFight.roomTileset,
               result->bossFight.fixedRoomNumber, roomName);
}
```

## How It Works

### Dungeon Type Detection

The code uses the **TypeSelection** system to determine the current dungeon's type:

1. Check `TypeSelection_HasActiveType()` - Is there an active dungeon type?
2. If yes, use `TypeSelection_GetActiveType()` to get the type
3. Otherwise, check `TypeSelection_HasCommittedType()` and use `TypeSelection_GetCommittedType()`
4. If neither, defaults to `TYPE_NONE`

### Room Selection Logic

Based on the dungeon type:

- **TYPE_STEEL (0x11)**: Uses tileset 64 (Mt. Steel boss room - metallic/steel visuals)
- **TYPE_BUG (0x0C)**: Uses tileset 65 (Sinister Woods boss room - dark forest visuals)
- **All others**: Uses tileset 64 (default custom arena)

All currently use **Room 1** layout (Skarmory's 9×17 rectangular arena with extracted pattern).

### Logging

Added mGBA debug logging to track room selection:

```
[BossRoom] dungeonType=17 tileset=64 layout=1 room=Skarmory
[BossRoom] dungeonType=12 tileset=65 layout=1 room=SinisterWoods
[BossRoom] dungeonType=2 tileset=64 layout=1 room=Custom
```

This helps verify the correct room is being selected during testing.

## Testing

### How to Test

1. **Load the ROM** in mGBA with console output enabled
2. **Select a dungeon type** using the TypeSelection menu
3. **Enter a boss floor**
4. **Check mGBA output** for the `[BossRoom]` log line
5. **Observe the visuals** in-game

### Expected Results

#### Steel-Type Dungeon
- mGBA log: `[BossRoom] dungeonType=17 tileset=64 layout=1 room=Skarmory`
- Visuals: Metallic/steel walls and floors (Mt. Steel theme)
- Layout: 9×17 rectangular arena

#### Bug-Type Dungeon
- mGBA log: `[BossRoom] dungeonType=12 tileset=65 layout=1 room=SinisterWoods`
- Visuals: Dark forest theme (Sinister Woods)
- Layout: 9×17 rectangular arena (same shape, different visuals!)

#### Other Type Dungeons (Fire, Water, etc.)
- mGBA log: `[BossRoom] dungeonType=X tileset=64 layout=1 room=Custom`
- Visuals: Default (currently Skarmory tileset)
- Layout: 9×17 rectangular arena

## Future Enhancements

### Easy Additions

Add more dungeon types to the selection logic:

```c
else if (dungeonType == TYPE_ELECTRIC) {
    result->bossFight.roomTileset = 66;  // Mt. Thunder Peak
    result->bossFight.fixedRoomNumber = 3;  // Zapdos arena
    roomName = "Electric";
}
else if (dungeonType == TYPE_FIRE) {
    result->bossFight.roomTileset = 67;  // Mt. Blaze Peak
    result->bossFight.fixedRoomNumber = 4;  // Moltres arena
    roomName = "Fire";
}
else if (dungeonType == TYPE_ICE) {
    result->bossFight.roomTileset = 68;  // Frosty Grotto
    result->bossFight.fixedRoomNumber = 5;  // Articuno arena
    roomName = "Ice";
}
```

### TODO: Add Room 2 (Team Meanies Arena)

Currently, Bug-type dungeons use Room 1 layout with tileset 65 visuals. To use the proper Team Meanies arena:

1. **Copy Room 2 pattern** from `extracted_patterns/fixed_rooms.c`
2. **Add to** `src/custom_fixed_rooms.c`:
   ```c
   static const u8 sFixedRoom2_Tiles[] = {
       // Copy from extracted_patterns/fixed_rooms.c
       // Room 2 is 13×13 square arena with pillars
   };

   static const CustomFixedRoom sFixedRoom2 = {
       .width = 13,
       .height = 13,
       .tiles = sFixedRoom2_Tiles
   };
   ```

3. **Update room array**:
   ```c
   static const CustomFixedRoom *sCustomRooms[] = {
       NULL,
       &sFixedRoom1,
       &sFixedRoom2,  // Add this
   };
   ```

4. **Add case to LoadCustomFixedRoom()**:
   ```c
   case 2:
       tiles = sFixedRoom2_Tiles;
       width = 13;
       height = 13;
       break;
   ```

5. **Update Bug-type selection** in `dungeon_seed_overrides.c:1015`:
   ```c
   result->bossFight.fixedRoomNumber = 2;  // Now use Team Meanies layout!
   ```

## Benefits

### Why Dungeon Type Instead of Boss Type?

**Thematic Consistency:**
- The entire dungeon has a consistent theme (Fire, Bug, Steel, etc.)
- Boss room matches the dungeon theme
- More immersive experience

**Example Scenarios:**

| Scenario | Dungeon Type | Boss Species | Boss Room |
|----------|--------------|--------------|-----------|
| Steel dungeon with Aggron boss | TYPE_STEEL | Aggron (Steel) | Skarmory room ✓ |
| Steel dungeon with Metagross boss | TYPE_STEEL | Metagross (Steel) | Skarmory room ✓ |
| Bug dungeon with Heracross boss | TYPE_BUG | Heracross (Bug) | Sinister Woods ✓ |
| Bug dungeon with Scizor boss | TYPE_BUG | Scizor (Bug/Steel) | Sinister Woods ✓ |
| Fire dungeon with Blaziken boss | TYPE_FIRE | Blaziken (Fire) | Custom (for now) |

**If we used boss type instead:**
- Scizor (Bug/Steel) in a Bug dungeon might get Steel room → inconsistent!
- Dungeon theme and boss room theme could mismatch

## Technical Details

### Type Constants Used

From `include/constants/type.h`:

```c
#define TYPE_NONE      0x0
#define TYPE_STEEL     0x11  // Decimal: 17
#define TYPE_BUG       0xC   // Decimal: 12
```

### Tileset IDs

From `docs/boss_dungeon_tileset_ids.md`:

- **64 (0x40)**: Mt. Steel boss floor (Skarmory)
- **65 (0x41)**: Sinister Woods boss floor (Team Meanies)

### Fixed Room IDs

From `src/custom_fixed_rooms.c`:

- **1**: Skarmory arena - 9×17 rectangular with water hazards
- **2**: Team Meanies arena - 13×13 square with pillars (not yet compiled)

## Related Documentation

- `docs/bossroom_framework.md` - Complete boss room framework documentation
- `docs/bossroom_framework_simplified.md` - Quick start guide
- `docs/boss_dungeon_tileset_ids.md` - All tileset IDs and themes
- `extracted_patterns/room_visualizations.txt` - Visual reference for all 55 layouts
- `src/custom_fixed_rooms.c` - Custom fixed room implementation
- `src/dungeon_seed_overrides.c:992-1030` - Implementation code

## Summary

✓ **Implemented**: Dungeon-type-based boss room selection
✓ **Steel dungeons**: Use Mt. Steel boss room (tileset 64)
✓ **Bug dungeons**: Use Sinister Woods boss room (tileset 65)
✓ **Other dungeons**: Use default custom arena (tileset 64)
✓ **Logging**: Added debug output for testing
✓ **Build**: Successfully compiles

**Next steps:** Add more dungeon types (Fire, Ice, Electric) and compile Room 2 for Bug dungeons.
