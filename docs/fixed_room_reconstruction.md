# Fixed Room Reconstruction for Custom Boss Fights

## Problem Summary

We want to use story boss arena layouts (like Skarmory's arena) for custom boss fights, but the fixed room system is deeply tied to story cutscenes and special initialization that causes crashes and incompatibilities.

### Current Issues with Fixed Room System

When using `LoadFixedRoomLayout(roomNumber)`:
- ✗ Forces cutscene system initialization (Skarmory dialogue, etc.)
- ✗ Many rendering/initialization functions crash with fixed room layouts
- ✗ We had to skip 9+ critical functions causing side effects (leader disappears)
- ✗ Screen rendering doesn't work properly
- ✗ Tied to specific story events and dungeon configurations

**Functions we had to skip (problematic):**
1. Cutscene system (sub_80847D4)
2. Fade-in animation
3. Tileset asset loading (LoadDungeonTilesetAssets)
4. Initialization routine (sub_806B168)
5. Weather initialization (sub_807E5AC)
6. Camera/view positioning (sub_803F4A0)
7. Floor cleanup (sub_806AA70)
8. Screen fade (sub_803E708)
9. Screen transition (sub_803E830)

## Proposed Solution: Terrain Reconstruction

**Key Insight:** Fixed rooms bundle three things together:
1. **Terrain/walkable area pattern** (what we want)
2. **Visual tileset** (we want to control this)
3. **Cutscenes and special behavior** (we don't want this)

**Solution:** Extract the terrain pattern from fixed rooms and rebuild it manually using normal dungeon generation, bypassing the fixed room system entirely.

### How It Works

**Example: Skarmory's Arena (Fixed Room #1)**
- Fixed room defines the arena **shape** (9x17 room with specific walkable paths)
- Tileset ID 64 = Skarmory boss arena **visuals**
- If we use normal generation + tileset 64: Visuals work, but walkable areas are random
- **We need:** Same walkable pattern as fixed room, but using normal dungeon generation

### Benefits of This Approach

✓ No fixed room system = no cutscene triggers
✓ Normal rendering pipeline = no crashes
✓ Can use any boss with any arena tileset
✓ Leader entity stays valid
✓ All initialization functions work normally
✓ Clean separation: arena shape vs. visual theme

## Simple Implementation Approach ✓

### Key Insight

We DON'T need to extract compressed data! We can manually define arena patterns using action IDs and apply them with `PlaceFixedRoomTile()`.

### Example: Simple Boss Arena

```c
void CreateSimpleBossArena(u8 tilesetId) {
    s32 x, y;
    Tile *tile;

    // Set the visual tileset (e.g., 64 for Skarmory)
    gDungeon->tileset = tilesetId;

    // Define arena bounds (9x17 like Skarmory's arena)
    const s32 ARENA_X = 5;
    const s32 ARENA_Y = 5;
    const s32 ARENA_WIDTH = 9;
    const s32 ARENA_HEIGHT = 17;

    // Create border walls
    for (y = 0; y < DUNGEON_MAX_SIZE_Y; y++) {
        for (x = 0; x < DUNGEON_MAX_SIZE_X; x++) {
            if (x < ARENA_X || x >= ARENA_X + ARENA_WIDTH ||
                y < ARENA_Y || y >= ARENA_Y + ARENA_HEIGHT) {
                tile = GetTileMut(x, y);
                tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
                SetTerrainWall(tile);
            }
        }
    }

    // Create arena interior
    for (y = ARENA_Y; y < ARENA_Y + ARENA_HEIGHT; y++) {
        for (x = ARENA_X; x < ARENA_X + ARENA_WIDTH; x++) {
            tile = GetTileMut(x, y);

            // Walls around perimeter
            if (x == ARENA_X || x == ARENA_X + ARENA_WIDTH - 1 ||
                y == ARENA_Y || y == ARENA_Y + ARENA_HEIGHT - 1) {
                PlaceFixedRoomTile(tile, 1, x, y, FALSE);  // Wall
            }
            // Player spawn at bottom-center
            else if (x == ARENA_X + ARENA_WIDTH/2 && y == ARENA_Y + ARENA_HEIGHT - 2) {
                PlaceFixedRoomTile(tile, 4, x, y, FALSE);  // Player spawn
            }
            // Stairs at top-center
            else if (x == ARENA_X + ARENA_WIDTH/2 && y == ARENA_Y + 1) {
                PlaceFixedRoomTile(tile, 8, x, y, FALSE);  // Stairs
            }
            // Regular floor
            else {
                PlaceFixedRoomTile(tile, 0, x, y, FALSE);  // Floor
            }
        }
    }

    // Make walls impassable if tileset >= 64
    if (gDungeon->tileset >= 64) {
        for (y = 0; y < DUNGEON_MAX_SIZE_Y; y++) {
            for (x = 0; x < DUNGEON_MAX_SIZE_X; x++) {
                tile = GetTileMut(x, y);
                if (GetTerrainType(tile) == TERRAIN_TYPE_WALL) {
                    tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
                }
            }
        }
    }
}
```

### Advanced: Match Skarmory's Exact Layout

To match Skarmory's arena exactly, we'd define a 9x17 pattern array:

```c
// Skarmory arena pattern (9 wide x 17 tall)
// 0 = floor, 1 = wall, 4 = player spawn, 8 = stairs
static const u8 sSkarmoryArenaPattern[17][9] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1},  // Top wall
    {1, 0, 0, 0, 8, 0, 0, 0, 1},  // Stairs row
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},  // Boss spawn row (around y=8)
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 0, 0, 0, 1},  // Player spawn row
    {1, 1, 1, 1, 1, 1, 1, 1, 1},  // Bottom wall
};

void CreateSkarmoryArena(u8 tilesetId) {
    s32 x, y;
    const s32 ARENA_X = 5;
    const s32 ARENA_Y = 5;

    gDungeon->tileset = tilesetId;  // Usually 64 for Skarmory

    // Apply the pattern
    for (y = 0; y < 17; y++) {
        for (x = 0; x < 9; x++) {
            Tile *tile = GetTileMut(ARENA_X + x, ARENA_Y + y);
            u8 actionId = sSkarmoryArenaPattern[y][x];
            PlaceFixedRoomTile(tile, actionId, ARENA_X + x, ARENA_Y + y, FALSE);
        }
    }

    // Fill borders with walls (same as before)
    // Make walls impassable if tileset >= 64 (same as before)
}
```

## Implementation Plan

### Phase 1: Terrain Pattern Definition (SIMPLIFIED)

**Goal:** Understand how fixed room terrain is stored and how to extract it.

**Files to investigate:**
- `src/dungeon_generation.c` - Contains `LoadFixedRoomLayout()`
- `include/constants/fixed_rooms.h` - Fixed room constants
- Data files containing fixed room layouts

**What we need to find:**
1. How is terrain stored in fixed rooms? (tile type, flags, coordinates)
2. What data structure represents the terrain grid?
3. Where is the fixed room data located? (ROM address, data format)
4. How to read/parse fixed room terrain data

**Key information we already know:**
- Room #1 (Skarmory) dimensions: 9x17
- Player spawn: (9, 14)
- Stairs position: (9, 6)
- Has specific terrain pattern with walls and walkable floor

### Phase 2: Terrain Application (Implementation)

**Goal:** Create a function to manually set terrain tiles to match the extracted pattern.

**Files to modify:**
- `src/dungeon_generation.c` - Add new function for custom terrain patterns
- `src/dungeon_floor_spawns.c` - Integrate terrain application with boss fights

**What we need to implement:**

```c
// New function to apply a pre-defined terrain pattern
void ApplyCustomTerrainPattern(u8 dungeonId, u8 floor) {
    const BossFightConfig *bossFight = DungeonFloorSpawns_GetBossFightConfig();

    if (bossFight == NULL || !bossFight->enabled) {
        return; // Not a boss floor
    }

    // Get the terrain pattern for this boss's arena type
    const TerrainPattern *pattern = GetArenaTerrainPattern(bossFight->arenaType);

    // Apply the pattern to the dungeon tiles
    for (y = 0; y < pattern->height; y++) {
        for (x = 0; x < pattern->width; x++) {
            Tile *tile = GetTileMut(x, y);
            // Set terrain type, flags, etc. based on pattern
            tile->terrainType = pattern->tiles[y][x].terrainType;
            tile->flags = pattern->tiles[y][x].flags;
            // ... set other tile properties
        }
    }

    // Set spawn positions
    gDungeon->playerSpawn = pattern->playerSpawn;
    gDungeon->stairsSpawn = pattern->stairsSpawn;
}
```

**Where to call it:**
In `GenerateFloor()` for boss floors, instead of `LoadFixedRoomLayout()`:
```c
if (bossFight && bossFight->enabled) {
    // Set the visual tileset
    gDungeon->tileset = GetBossTileset(bossFight->species);

    // Apply custom terrain pattern (replaces LoadFixedRoomLayout)
    ApplyCustomTerrainPattern(dungeonId, floor);
}
```

### Phase 3: Arena Pattern Database

**Goal:** Create a database of arena terrain patterns.

**Data structure:**
```c
typedef struct {
    u8 terrainType;  // TERRAIN_NORMAL, TERRAIN_WALL, etc.
    u16 flags;       // Tile flags
} TerrainTile;

typedef struct {
    u8 width;
    u8 height;
    Position playerSpawn;
    Position stairsSpawn;
    TerrainTile tiles[32][56];  // Max dungeon dimensions
} TerrainPattern;
```

**Patterns to extract:**
1. Skarmory's arena (Fixed Room #1) - Mountain/steel themed
2. Zapdos arena (Fixed Room #2) - Electric themed
3. Articuno arena (Fixed Room #3) - Ice themed
4. Moltres arena (Fixed Room #4) - Fire themed
5. Mewtwo arena (Fixed Room #5) - Psychic themed

### Phase 4: Tileset Integration

**Goal:** Map boss types to appropriate visual tilesets.

**Tileset mapping:**
```c
u8 GetBossTileset(u16 species) {
    // Get the boss's primary type
    u8 type = GetPokemonType(species, 0);

    switch (type) {
        case TYPE_STEEL: return 64;  // Skarmory arena visuals
        case TYPE_ELECTRIC: return 65; // Zapdos arena visuals
        case TYPE_ICE: return 66;     // Articuno arena visuals
        case TYPE_FIRE: return 67;    // Moltres arena visuals
        case TYPE_PSYCHIC: return 68; // Mewtwo arena visuals
        // ... map other types to appropriate tilesets
        default: return 64; // Default to Skarmory arena
    }
}
```

## Technical Investigation Results ✓

### 1. Fixed Room Data Format (SOLVED)

**How fixed rooms work:**
- Fixed room data is stored as **compressed action IDs** in ROM
- `LoadFixedRoomLayout()` reads compressed data via `sub_80511F0()`
- Each action ID is passed to `PlaceFixedRoomTile()` which sets terrain
- Room is positioned at offset (5, 5) in the dungeon grid
- Borders are filled with impassable walls

**Key discovery:** We can use `PlaceFixedRoomTile()` directly WITHOUT loading compressed data!

**Code location:**
```c
// src/dungeon_generation_fixed.c:850
bool8 PlaceFixedRoomTile(Tile *tile, u8 fixedRoomActionId, s32 x, s32 y, bool8 spawnTrapOrItem)
```

**Action ID Mapping (from PlaceFixedRoomTile):**
- `0` = Normal floor (walkable)
- `1` = Wall (breakable)
- `2, 13, 14` = Impassable wall
- `4` = Player spawn position (sets gDungeon->playerSpawn)
- `8` = Stairs position (sets gDungeon->stairsSpawn)
- `5, 6` = Secondary terrain (water/lava)
- `16+` = Entity spawns (we skip these with spawnTrapOrItem=FALSE)

### 2. Terrain Tile Structure

**Questions to answer:**
- What is the `Tile` structure definition?
- What terrain types exist? (wall, floor, water, lava, etc.)
- What flags are used? (impassable, room, corridor, etc.)
- How do terrain types map to visual tiles?

**Files to check:**
- `include/dungeon_map.h` - Tile structure definition
- `include/constants/terrain.h` - Terrain type constants

### 3. Normal Floor Generation

**Questions to answer:**
- How does normal `GenerateFloor()` create terrain?
- What functions set individual tile properties?
- How are rooms and corridors created?
- Can we hook into this to override with our pattern?

**Code to understand:**
```c
// In dungeon_generation.c
void GenerateFloor(void) {
    // Understand the generation flow
    // Find where terrain tiles are set
    // Identify where we can inject custom pattern
}
```

## Current Status

### What We've Done
1. ✓ Identified that fixed rooms cause crashes
2. ✓ Attempted to skip problematic functions (caused leader to disappear)
3. ✓ Discovered tileset 64 works for visuals with normal generation
4. ✓ Confirmed the problem is terrain pattern mismatch

### What We're Doing
1. → Creating this implementation plan
2. → Investigating fixed room data format
3. → Understanding terrain tile structures

### What's Next
1. Extract terrain pattern from fixed room #1 (Skarmory)
2. Create manual terrain application function
3. Replace `LoadFixedRoomLayout()` with custom approach
4. Remove all the function skips we added
5. Test with actual boss fight gameplay

## Testing Plan

### Phase 1: Terrain Pattern Test
- Load boss floor with custom terrain pattern
- Verify walkable areas match expected arena shape
- Verify walls and impassable tiles are correct
- Verify spawn positions are correct

### Phase 2: Tileset Visual Test
- Apply tileset 64 (Skarmory visuals)
- Verify visuals render correctly
- Verify no crashes in rendering functions
- Verify screen displays properly

### Phase 3: Boss Spawn Test
- Spawn boss and minions
- Verify entities spawn at correct positions
- Verify positions are walkable
- Verify leader entity stays valid

### Phase 4: Gameplay Test
- Enable actual game loop (remove skip)
- Test boss fight mechanics
- Test floor transitions
- Test victory/defeat scenarios

## Expected Outcomes

### If Successful
✓ Boss floors load with proper arena layouts
✓ Correct visual tilesets for boss types
✓ No crashes or bad memory accesses
✓ Leader entity remains valid
✓ Normal dungeon systems work properly
✓ Can use any arena with any boss

### Potential Issues
- Terrain pattern extraction might be complex
- May need to handle special tile types
- Lighting/fog effects might differ
- May need to adjust spawn algorithms
- Boss AI might need arena awareness

## Alternative Approaches (If Needed)

### Option A: Hybrid Approach
- Use fixed room data but override tileset after loading
- Still avoid cutscene system
- May still have some rendering issues

### Option B: Procedural Arena Generation
- Create algorithm to generate boss arena shapes
- Simple rectangular arenas with pillars
- Easier than extracting fixed room data
- Less authentic to original game

### Option C: Fixed Room Surgery
- Continue skipping problematic functions
- Add special cases to preserve leader entity
- More patches and workarounds
- Harder to maintain

## Notes

- Skarmory's arena is in Mt. Steel, fixed room #1
- Tileset 64 = visual appearance of Mt. Steel boss arena
- Fixed room system designed for story progression, not custom bosses
- Our approach works WITH the normal dungeon system, not against it
- This is more maintainable and extensible than patching fixed rooms

## References

- `src/dungeon_generation.c` - Floor generation and fixed room loading
- `src/dungeon_floor_spawns.c` - Boss fight configuration
- `include/constants/fixed_rooms.h` - Fixed room constants
- `include/dungeon_map.h` - Tile and terrain structures
- Previous implementation: `docs/fixed_room_reuse_for_bossfights.md`
