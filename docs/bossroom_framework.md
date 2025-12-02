# Boss Room Framework: Tilesets and Fixed Room Layouts

This document explains how the Skarmory boss fight system works, how we separate **terrain** from **visuals**, and how you can easily switch between different boss room themes.

## Overview

The boss room system has two independent components:

1. **Terrain/Layout** (Fixed Room Pattern): Defines walkable areas, walls, spawn positions
2. **Visual Tileset**: Defines how the room looks (graphics, colors, theme)

These are **completely separate**, allowing us to mix and match any layout with any visual theme.

---

## Current Implementation: Skarmory Boss Room

### How It Works

**Location in code:** `src/dungeon_seed_overrides.c:993`

```c
result->bossFight.roomTileset = 64;  // Skarmory boss fight tileset
result->bossFight.useFixedRoomLayout = TRUE;
result->bossFight.fixedRoomNumber = 1;  // Fixed Room 1 (Skarmory arena)
```

### What Happens During Boss Floor Generation

1. **Tileset is set** (line `src/run_dungeon.c:372`):
   ```c
   gDungeon->tileset = gDungeon->floorProperties.tileset;
   ```
   - For boss floors, this gets overridden to `64` (Skarmory tileset)
   - Tileset 64 = Mt. Steel boss room graphics (metallic/steel theme)

2. **Fixed Room Layout is loaded** (in `GenerateFloor()`):
   - Calls `LoadFixedRoomLayout(1, FALSE)` where `1` = Skarmory's arena pattern
   - This reads compressed data and applies terrain using `PlaceFixedRoomTile()`
   - Creates a 9x17 arena with specific walkable paths
   - Sets player spawn at bottom-center, stairs at top-center

3. **Result**: Skarmory's arena **shape** with Skarmory's boss room **visuals**

---

## Understanding Tilesets

### What is a Tileset?

A **tileset** is a collection of graphics that determine how a dungeon floor looks:
- Floor tile graphics
- Wall tile graphics
- Water/lava appearance
- Overall color palette and theme

The tileset is just a number (ID) that tells the game which graphics to load.

### Boss Room Tilesets

From `data/dungeon/*/main_data.inc` files (byte 2 of floor properties):

| Tileset ID | Decimal | Dungeon | Visual Theme |
|------------|---------|---------|--------------|
| 0x40 | **64** | Mt. Steel boss floor | **Steel/metallic (Skarmory)** |
| 0x41 | **65** | Sinister Woods boss floor | **Dark forest (Team Meanies)** |
| 0x42 | 66 | Mt. Thunder Peak | Electric/lightning |
| 0x43 | 67 | Mt. Blaze Peak | Fire/lava |
| 0x44 | 68 | Frosty Grotto | Ice/snow |
| 0x45 | 69 | Mt. Freeze Peak | Ice cave |
| 0x46 | 70 | Magma Cavern Pit | Deep lava |
| 0x47 | 71 | Sky Tower Summit | Sky/clouds |
| 0x4a | 74 | Stormy Sea | Ocean/water |

**Key insight from data:**
- Mt. Steel normal floors use tileset 39 (0x27)
- Mt. Steel **boss floor** uses tileset **64** (0x40) ← Skarmory room
- Sinister Woods normal floors use tileset 41 (0x29)
- Sinister Woods **boss floor** uses tileset **65** (0x41) ← Team Meanies room

Boss room tilesets (64+) have special properties:
- Walls are automatically made **impassable** (see `src/dungeon_generation.c:6030`)
- Custom graphics and palettes designed for boss encounters

---

## Understanding Fixed Room Layouts

### What is a Fixed Room?

A **fixed room** is a pre-designed terrain pattern that defines:
- Where walls are
- Where floor tiles are
- Where the player spawns
- Where the stairs spawn
- Special terrain (water, lava, etc.)

Fixed rooms are used in the story mode for specific boss arenas.

### Two Types of Fixed Room Systems

#### 1. Original Fixed Rooms (ROM-based)

The game's original fixed rooms are stored as **compressed data in ROM**. These are loaded using `LoadFixedRoomLayout()`.

**Function:** `LoadFixedRoomLayout(roomNumber, spawnEntities)` in `src/dungeon_generation.c:6488`

1. Reads compressed terrain data from ROM
2. For each tile, calls `PlaceFixedRoomTile(tile, actionId, x, y, spawnEntities)`
3. Action IDs determine what each tile becomes:
   - `0` = Normal floor (walkable)
   - `1` = Wall (breakable)
   - `2, 13, 14` = Impassable wall
   - `4` = Player spawn position
   - `8` = Stairs position
   - etc.

4. Sets spawn positions in `gDungeon->playerSpawn` and `gDungeon->stairsSpawn`

**Important:** We pass `spawnEntities = FALSE` to skip spawning story NPCs (Diglett, etc.)

**Original Fixed Room IDs:**

From `include/constants/fixed_rooms.h`:

| Fixed Room # | Name | Boss | Layout |
|--------------|------|------|--------|
| 1 | `FIXED_ROOM_MT_STEEL_SKARMORY` | Skarmory | 9x17 rectangular arena |
| 2 | `FIXED_ROOM_SINISTER_WOODS_TEAM_MEANIES` | Team Meanies | Boss arena |
| 3 | Zapdos | Zapdos | Electric arena |
| 4 | Moltres | Moltres | Fire arena |
| 5 | Articuno | Articuno | Ice arena |
| ... | (many more) | ... | ... |

#### 2. Extracted Pattern System (Custom Fixed Rooms)

We've **extracted** the terrain patterns from the original fixed rooms and converted them into C arrays. This gives us several advantages:

**Benefits:**
- ✓ No dependency on compressed ROM data
- ✓ Easier to modify and customize layouts
- ✓ Can visualize patterns with ASCII art
- ✓ Simpler code path (no decompression)
- ✓ Can create new custom arenas easily

**How Extraction Works:**

1. **Extraction Phase** (already done):
   - Load each fixed room using `LoadFixedRoomLayout()`
   - Log every tile's action ID to mGBA console
   - Save logs to `extracted_patterns/raw/roomN.log`
   - Convert logs to C arrays using `convert_to_c.py`

2. **Result**: Clean C arrays in `src/custom_fixed_rooms.c`

**Example - Room 1 (Skarmory Arena):**

From `extracted_patterns/room_visualizations.txt`:
```
Room 1 (17x9)
--------------------------------------------------
+#######+
+#######+
#########
###T!T###
#+++++++#
T+++++++T
~~~~~~~~~
~.......~
....P....

Legend:
  . = Floor (60)      # = Wall (2)        + = Secondary wall (6)
  ~ = Water (10)      P = Player spawn (16)
  ! = Special (17, boss spawn marker)
  T = Trap/Item (68)
```

Converted to C array in `src/custom_fixed_rooms.c:14`:
```c
static const u8 sFixedRoom1_Tiles[] = {
    // Row 0
    6,   2,   2,   2,   2,   2,   2,   2,   6,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 1
    6,   2,   2,   2,   2,   2,   2,   2,   6,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 2
    2,   2,   2,   2,   2,   2,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 3 - Boss spawns here at center (tile 17)
    2,   2,   2,  68,  17,  68,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 4
    2,   6,   6,   6,   6,   6,   6,   6,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 5
    68,   6,   6,   6,   6,   6,   6,   6,  68,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 6
    10,  10,  10,  10,  10,  10,  10,  10,  10,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 7
    10,  60,  60,  60,  60,  60,  60,  60,  10,   0,   0,   0,   0,   0,   0,   0,   0,
    // Row 8 - Player spawns at center (tile 16 at column 4)
    60,  60,  60,  60,  16,  60,  60,  60,  60,   0,   0,   0,   0,   0,   0,   0,   0,
};
```

**Loading Custom Fixed Rooms:**

Function: `LoadCustomFixedRoom(roomId, spawnEntities)` in `src/custom_fixed_rooms.c:136`

This is simpler than the original system:
1. Select room array by ID
2. Loop through array (no decompression needed!)
3. Place tiles using `PlaceCustomTile()`
4. Set spawn positions

**Custom Tile Type Constants** (from `src/custom_fixed_rooms.h:14`):

| Constant | Value | Meaning |
|----------|-------|---------|
| `CUSTOM_TILE_UNUSED` | 0 | Empty space (skip) |
| `CUSTOM_TILE_WALL` | 2 | Impassable wall |
| `CUSTOM_TILE_STAIRS_DOWN` | 4 | Stairs position |
| `CUSTOM_TILE_SECONDARY_WALL` | 6 | Secondary wall type |
| `CUSTOM_TILE_WATER` | 10 | Water/lava terrain |
| `CUSTOM_TILE_PLAYER_SPAWN` | 16 | Player spawn position |
| `CUSTOM_TILE_SPECIAL` | 17 | Boss spawn marker |
| `CUSTOM_TILE_STAIRS_UP` | 18 | Stairs up |
| `CUSTOM_TILE_FLOOR` | 60 | Normal walkable floor |
| `CUSTOM_TILE_TRAP_ITEM` | 68 | Trap/item spawn |

---

## Separation of Terrain and Visuals

This is the **key insight** that makes the system flexible:

```
Fixed Room Layout (terrain)    +    Tileset (visuals)    =    Final Result
      ↓                                      ↓                      ↓
   9x17 arena                        Steel graphics          Skarmory boss room
   Player at bottom                  Metallic walls
   Stairs at top                     Mountain theme
```

### The Code Flow

#### Using Original Fixed Rooms (ROM-based):

```c
// 1. Set the VISUAL tileset
gDungeon->tileset = 64;  // Skarmory visuals

// 2. Load the TERRAIN pattern from ROM (completely separate!)
LoadFixedRoomLayout(1, FALSE);  // Skarmory arena shape (compressed data)

// Result: Skarmory's arena with Skarmory's visuals
```

#### Using Extracted Custom Fixed Rooms (Recommended):

```c
// 1. Set the VISUAL tileset
gDungeon->tileset = 64;  // Skarmory visuals

// 2. Load the TERRAIN pattern from C array (completely separate!)
LoadCustomFixedRoom(1, FALSE);  // Skarmory arena shape (extracted pattern)

// Result: Same arena, same visuals, simpler code!
```

**Current implementation** in `src/dungeon_generation.c:6630`:
```c
if (config->useFixedRoomLayout) {
    LoadCustomFixedRoom(config->fixedRoomNumber, FALSE);
}
```

We're currently using the **extracted pattern system** because it's simpler and easier to modify.

### Why This Matters

You can **mix and match**! For example:

```c
// Skarmory arena with SINISTER WOODS visuals
gDungeon->tileset = 65;  // Sinister Woods visuals
LoadFixedRoomLayout(1, FALSE);  // Skarmory arena shape

// Sinister Woods arena with SKARMORY visuals
gDungeon->tileset = 64;  // Skarmory visuals
LoadFixedRoomLayout(2, FALSE);  // Team Meanies arena shape

// Fire arena with ICE visuals
gDungeon->tileset = 68;  // Ice visuals
LoadFixedRoomLayout(4, FALSE);  // Moltres fire arena shape
```

---

## How to Switch Boss Rooms Based on Type

### Goal

Make Bug-type bosses use Sinister Woods visuals, Steel-type bosses use Skarmory visuals, etc.

### Implementation Strategy

**Step 1:** Create a function to map Pokemon types to tilesets

```c
// Add to src/dungeon_seed_overrides.c or a new file

u8 GetBossRoomTileset(s16 bossSpecies) {
    u8 primaryType = GetPokemonType(bossSpecies, 0);  // Get first type

    switch (primaryType) {
        case TYPE_STEEL:
            return 64;  // Skarmory boss room (steel/metallic)

        case TYPE_BUG:
        case TYPE_POISON:
        case TYPE_GHOST:
        case TYPE_DARK:
            return 65;  // Sinister Woods boss room (dark forest)

        case TYPE_ELECTRIC:
            return 66;  // Mt. Thunder Peak (electric)

        case TYPE_FIRE:
            return 67;  // Mt. Blaze Peak (fire/lava)

        case TYPE_ICE:
            return 68;  // Frosty Grotto (ice/snow)

        case TYPE_WATER:
            return 74;  // Stormy Sea (water)

        case TYPE_ROCK:
        case TYPE_GROUND:
            return 70;  // Magma Cavern Pit (deep cave)

        case TYPE_FLYING:
            return 71;  // Sky Tower Summit (sky/clouds)

        case TYPE_PSYCHIC:
            // Could use a special tileset when available
            return 64;  // Default to Skarmory for now

        default:
            return 64;  // Default: Skarmory boss room
    }
}
```

**Step 2:** Update boss fight generation to use type-based tileset

In `src/dungeon_seed_overrides.c:993`, replace the hardcoded tileset:

```c
// OLD CODE:
result->bossFight.roomTileset = 64;  // Always Skarmory

// NEW CODE:
result->bossFight.roomTileset = GetBossRoomTileset(result->bossFight.bossSpecies);
```

**Step 3:** (Optional) Also switch fixed room layout by type

```c
u8 GetBossRoomLayout(s16 bossSpecies) {
    u8 primaryType = GetPokemonType(bossSpecies, 0);

    switch (primaryType) {
        case TYPE_BUG:
            return 2;  // Team Meanies arena (Fixed Room 2)

        case TYPE_ELECTRIC:
            return 3;  // Zapdos arena

        case TYPE_FIRE:
            return 4;  // Moltres arena

        case TYPE_ICE:
            return 5;  // Articuno arena

        default:
            return 1;  // Default: Skarmory arena
    }
}

// Then in boss generation:
result->bossFight.fixedRoomNumber = GetBossRoomLayout(result->bossFight.bossSpecies);
```

---

## Practical Examples

### Example 1: Bug-type Boss (Heracross)

**Before (current code):**
```c
// Heracross boss fight
result->bossFight.bossSpecies = MONSTER_HERACROSS;  // Bug/Fighting
result->bossFight.roomTileset = 64;  // Skarmory (steel theme) ← Doesn't match!
result->bossFight.fixedRoomNumber = 1;  // Skarmory arena
```

**After (with type-based selection):**
```c
// Heracross boss fight
result->bossFight.bossSpecies = MONSTER_HERACROSS;  // Bug/Fighting
result->bossFight.roomTileset = GetBossRoomTileset(MONSTER_HERACROSS);  // Returns 65 (Sinister Woods)
result->bossFight.fixedRoomNumber = GetBossRoomLayout(MONSTER_HERACROSS);  // Returns 2 (Team Meanies arena)
```

**Result:** Heracross fights in a dark forest arena that matches its Bug typing!

### Example 2: Fire-type Boss (Magmar)

```c
// Magmar boss fight
result->bossFight.bossSpecies = MONSTER_MAGMAR;  // Fire
result->bossFight.roomTileset = GetBossRoomTileset(MONSTER_MAGMAR);  // Returns 67 (Mt. Blaze Peak)
result->bossFight.fixedRoomNumber = GetBossRoomLayout(MONSTER_MAGMAR);  // Returns 4 (Moltres arena)
```

**Result:** Magmar fights in a fiery lava arena!

### Example 3: Electric-type Boss (Raikou)

```c
// Raikou boss fight
result->bossFight.bossSpecies = MONSTER_RAIKOU;  // Electric
result->bossFight.roomTileset = GetBossRoomTileset(MONSTER_RAIKOU);  // Returns 66 (Mt. Thunder Peak)
result->bossFight.fixedRoomNumber = GetBossRoomLayout(MONSTER_RAIKOU);  // Returns 3 (Zapdos arena)
```

**Result:** Raikou fights in an electric-themed arena with lightning effects!

---

## Advanced: Dual-Type Handling

For Pokemon with two types, you can prioritize which type determines the arena:

```c
u8 GetBossRoomTileset(s16 bossSpecies) {
    u8 type1 = GetPokemonType(bossSpecies, 0);  // Primary type
    u8 type2 = GetPokemonType(bossSpecies, 1);  // Secondary type

    // Prioritize certain types over others
    // Example: STEEL takes priority over FLYING
    if (type1 == TYPE_STEEL || type2 == TYPE_STEEL)
        return 64;  // Skarmory (steel)

    if (type1 == TYPE_BUG || type2 == TYPE_BUG)
        return 65;  // Sinister Woods (bug)

    // Continue with primary type
    switch (type1) {
        case TYPE_ELECTRIC: return 66;
        case TYPE_FIRE: return 67;
        case TYPE_ICE: return 68;
        // ... etc
        default: return 64;
    }
}
```

**Example:** Skarmory (Steel/Flying) would use the steel arena because TYPE_STEEL is checked first.

---

## Testing Different Boss Rooms

### Quick Test: Force a Specific Tileset

Temporarily hardcode a tileset to see how it looks:

```c
// In src/dungeon_seed_overrides.c:993
result->bossFight.roomTileset = 65;  // Force Sinister Woods
result->bossFight.fixedRoomNumber = 1;  // Keep Skarmory arena shape
```

Compile and enter a boss floor. You'll see:
- Skarmory's 9x17 arena **layout**
- Sinister Woods dark forest **visuals**

### Test Different Combinations

| Tileset | Layout | Result |
|---------|--------|--------|
| 64 (Steel) | 1 (Skarmory) | ✓ Current default |
| 65 (Forest) | 1 (Skarmory) | Dark forest with rectangular arena |
| 64 (Steel) | 2 (Team Meanies) | Steel theme with Team Meanies layout |
| 67 (Fire) | 1 (Skarmory) | Fire/lava theme with rectangular arena |
| 68 (Ice) | 5 (Articuno) | Ice theme with Articuno's arena |

---

## Implementation Checklist

To add type-based boss room selection:

- [ ] Create `GetBossRoomTileset()` function
- [ ] (Optional) Create `GetBossRoomLayout()` function
- [ ] Update `src/dungeon_seed_overrides.c:993` to call the function
- [ ] Compile and test with different boss types
- [ ] Verify visuals match boss type themes
- [ ] (Optional) Add dual-type priority logic

**Files to modify:**
1. `src/dungeon_seed_overrides.c` - Add helper functions and update line 993
2. (Optional) `include/dungeon_seed_overrides.h` - Add function declarations if making them public

---

## Technical Details

### Why Tileset >= 64 is Special

In `src/dungeon_generation.c:6030` and `6593`:

```c
if (gDungeon->tileset >= 64) {
    // Make walls impassable
    tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
}
```

Boss room tilesets (64+) automatically have **impassable walls**, preventing the player from escaping the arena. This is critical for boss fights!

### Fixed Room Offset

Fixed rooms are placed at offset **(5, 5)** in the dungeon grid:

```c
// From LoadFixedRoomLayout
for (y = 5; y < fixedRoomSizeY + 5; y++) {
    for (x = 5; x < fixedRoomSizeX + 5; x++) {
        // Place tiles...
    }
}
```

This ensures the arena is centered on the floor with walls surrounding it.

### Action ID Reference (from PlaceFixedRoomTile)

| Action ID | Meaning | Notes |
|-----------|---------|-------|
| 0 | Normal floor | Walkable |
| 1 | Wall | Breakable with moves |
| 2, 13, 14 | Impassable wall | Cannot be destroyed |
| 4 | Player spawn | Sets `gDungeon->playerSpawn` |
| 8 | Stairs | Sets `gDungeon->stairsSpawn` |
| 5, 6 | Secondary terrain | Water/lava |
| 16+ | Entity spawns | We skip these with `spawnEntities=FALSE` |

---

## Summary

### Key Concepts

1. **Tileset** (visual) and **Fixed Room Layout** (terrain) are **independent**
2. Tileset is just a number that loads graphics (64 = Skarmory, 65 = Sinister Woods, etc.)
3. Fixed Room Layout defines the walkable area pattern (9x17 arena, spawn positions, etc.)
4. You can mix any tileset with any layout for infinite variety
5. Boss room tilesets (64+) have special properties (impassable walls)

### Current System

- All bosses use tileset **64** (Skarmory steel theme)
- All bosses use fixed room **1** (Skarmory 9x17 arena)
- This is hardcoded in `src/dungeon_seed_overrides.c:993`

### Easy Switch to Sinister Woods

**Single line change:**
```c
// Change line 993 from:
result->bossFight.roomTileset = 64;

// To:
result->bossFight.roomTileset = 65;
```

Now all boss fights use Sinister Woods dark forest visuals! (Same arena shape)

### Type-Based Selection

Add a function to map Pokemon type → tileset, then call it during boss generation. See "How to Switch Boss Rooms Based on Type" section above.

### Testing

1. Modify tileset/layout numbers in `dungeon_seed_overrides.c`
2. Compile: `make`
3. Enter a boss floor in-game
4. Observe the visual theme and arena layout

---

## Reference Tables

### Pokemon Type Constants

From `include/constants/type.h`:

```c
#define TYPE_NORMAL    0x1
#define TYPE_FIRE      0x2
#define TYPE_WATER     0x3
#define TYPE_GRASS     0x4
#define TYPE_ELECTRIC  0x5
#define TYPE_ICE       0x6
#define TYPE_FIGHTING  0x7
#define TYPE_POISON    0x8
#define TYPE_GROUND    0x9
#define TYPE_FLYING    0xA
#define TYPE_PSYCHIC   0xB
#define TYPE_BUG       0xC
#define TYPE_ROCK      0xD
#define TYPE_GHOST     0xE
#define TYPE_DRAGON    0xF
#define TYPE_DARK      0x10
#define TYPE_STEEL     0x11
```

### Get Pokemon Type Function

From `include/pokemon.h:92`:

```c
u8 GetPokemonType(s32 index, u32 typeIndex);
// typeIndex: 0 = primary type, 1 = secondary type
```

**Example usage:**
```c
u8 primaryType = GetPokemonType(MONSTER_HERACROSS, 0);  // Returns TYPE_BUG
u8 secondaryType = GetPokemonType(MONSTER_HERACROSS, 1);  // Returns TYPE_FIGHTING
```

---

## Creating Custom Arena Layouts

### Using Extracted Patterns

We have 55 extracted boss arena patterns available in `extracted_patterns/`. You can browse them visually:

```bash
cat extracted_patterns/room_visualizations.txt
```

**Available Rooms (examples):**

| Room ID | Dimensions | Description | Extracted From |
|---------|------------|-------------|----------------|
| 1 | 9x17 | Rectangular arena with water | Skarmory (Mt. Steel) |
| 2 | 13x13 | Square arena with pillars | Team Meanies (Sinister Woods) |
| 3 | 9x10 | Compact arena | Zapdos (Mt. Thunder) |
| 4 | 14x17 | Large arena with hazards | Moltres (Mt. Blaze) |
| 5 | 12x12 | Medium square arena | Articuno (Frosty Grotto) |
| ... | ... | ... | ... |

See `extracted_patterns/room_visualizations.txt` for the complete list with ASCII visualizations!

### Adding a New Extracted Pattern to Code

Currently, only Room 1 (Skarmory) is compiled into the code. To add more:

1. **Choose a room** from `extracted_patterns/fixed_rooms.c` or `room_visualizations.txt`

2. **Copy the pattern array** to `src/custom_fixed_rooms.c`:
   ```c
   // Add after sFixedRoom1_Tiles
   static const u8 sFixedRoom2_Tiles[] = {
       // Copy from extracted_patterns/fixed_rooms.c
       2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
       // ... rest of Room 2 data
   };

   static const CustomFixedRoom sFixedRoom2 = {
       .width = 13,
       .height = 13,
       .tiles = sFixedRoom2_Tiles
   };
   ```

3. **Add to room array**:
   ```c
   static const CustomFixedRoom *sCustomRooms[] = {
       NULL,           // Index 0 unused
       &sFixedRoom1,   // Index 1
       &sFixedRoom2,   // Index 2 - NEW!
   };
   ```

4. **Update LoadCustomFixedRoom()** in `src/custom_fixed_rooms.c`:
   ```c
   case 2:
       tiles = sFixedRoom2_Tiles;
       width = 13;
       height = 13;
       break;
   ```

5. **Use it in boss generation**:
   ```c
   result->bossFight.fixedRoomNumber = 2;  // Team Meanies arena
   ```

### Creating Completely New Arenas

You can design your own arena from scratch! Just define a new pattern array:

```c
// Example: Simple 7x7 square arena
static const u8 sCustomArena_7x7[] = {
    // Row 0 - Top wall
    2,   2,   2,   2,   2,   2,   2,
    // Row 1
    2,  60,  60,  60,  60,  60,   2,
    // Row 2
    2,  60,  60,  60,  60,  60,   2,
    // Row 3 - Boss in center
    2,  60,  60,  17,  60,  60,   2,
    // Row 4
    2,  60,  60,  60,  60,  60,   2,
    // Row 5 - Player spawn
    2,  60,  60,  16,  60,  60,   2,
    // Row 6 - Bottom wall + stairs
    2,   2,   2,   4,   2,   2,   2,
};
```

**Design Guidelines:**
- Use tile value `2` for walls
- Use tile value `60` for walkable floor
- Use tile value `16` for player spawn (required!)
- Use tile value `4` for stairs (required!)
- Use tile value `17` for boss spawn marker (optional but recommended)
- Keep arena size under 20x20 to fit in dungeon grid
- Leave some space around the edges (will be padded with walls)

---

## Available Extracted Patterns Reference

All 55 extracted patterns are available in `extracted_patterns/`:

**Directory structure:**
```
extracted_patterns/
├── raw/                          # Original extraction logs
│   ├── room1.log                 # mGBA console output
│   ├── room2.log
│   └── ... (room3-55)
├── fixed_rooms.c                 # All patterns as C arrays
├── room_visualizations.txt       # ASCII art of all rooms
└── convert_to_c.py              # Conversion script
```

**Using the visualizations:**

```bash
# View all room layouts
cat extracted_patterns/room_visualizations.txt

# Search for specific size
grep "Room.*13x13" extracted_patterns/room_visualizations.txt

# Find rooms with water
grep -A 15 "Room [0-9]" extracted_patterns/room_visualizations.txt | grep "~"
```

**Pattern extraction details:**

The extraction was done by:
1. Adding debug logging to `LoadFixedRoomLayout()` in `src/dungeon_generation.c:6519-6540`
2. Loading each fixed room in-game
3. Capturing mGBA console output to log files
4. Running `convert_to_c.py` to generate C arrays and ASCII visualizations

This means **all original boss arena layouts are preserved** and available for reuse!

---

## Related Documentation

- `docs/fixed_room_reconstruction.md` - Deep dive into fixed room terrain extraction
- `docs/boss_dungeon_tileset_ids.md` - Catalog of all boss tileset IDs
- `docs/fixed_rooms.md` - List of all fixed room IDs
- `src/dungeon_generation.c:6488` - `LoadFixedRoomLayout()` implementation (original system)
- `src/custom_fixed_rooms.c` - Custom fixed room implementation (extracted patterns)
- `src/custom_fixed_rooms.h` - Custom tile type constants
- `src/dungeon_seed_overrides.c:936-1012` - Boss fight generation code
- `extracted_patterns/room_visualizations.txt` - **Visual reference for all 55 extracted arenas**
- `extracted_patterns/fixed_rooms.c` - **Complete C code for all extracted patterns**

---

## Quick Reference: Combining Tilesets and Layouts

**Want a Bug-type boss in a dark forest?**
```c
result->bossFight.roomTileset = 65;        // Sinister Woods tileset (dark forest)
result->bossFight.fixedRoomNumber = 2;     // Team Meanies arena layout
```

**Want an Electric-type boss in the Zapdos arena?**
```c
result->bossFight.roomTileset = 66;        // Mt. Thunder Peak tileset (electric)
result->bossFight.fixedRoomNumber = 3;     // Zapdos arena layout
```

**Want a Fire-type boss in the Moltres arena?**
```c
result->bossFight.roomTileset = 67;        // Mt. Blaze Peak tileset (fire)
result->bossFight.fixedRoomNumber = 4;     // Moltres arena layout
```

**Mix and match for variety:**
```c
// Steel boss in ice-themed arena
result->bossFight.roomTileset = 68;        // Frosty Grotto (ice visuals)
result->bossFight.fixedRoomNumber = 1;     // Skarmory layout (rectangular)

// Electric boss in fire-themed arena
result->bossFight.roomTileset = 67;        // Mt. Blaze (fire visuals)
result->bossFight.fixedRoomNumber = 3;     // Zapdos layout (electric arena shape)
```

The possibilities are endless! **55 layouts × 10+ tilesets = 550+ unique boss room combinations!**

---

## Credits

This system leverages PMD Red's existing fixed room and tileset infrastructure to create flexible, thematically appropriate boss arenas without hardcoding specific boss-to-room mappings.

Special thanks to the pattern extraction system that allows us to reuse all 55 original boss arena layouts with any visual theme!
