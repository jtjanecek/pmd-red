# Fixed Room Reuse for Custom Boss Fights - Technical Analysis & Implementation Plan

## Executive Summary

This document explains how dungeon visuals and walkable terrain work in PMD Red, how fixed rooms are structured, and provides implementation strategies for reusing fixed room layouts in custom boss fights with different tilesets.

**Key Discovery:** Visuals and walkable terrain are **completely independent systems** in PMD Red. Fixed rooms can have their walkable layout extracted and combined with any tileset.

## Table of Contents

1. [Core Systems Overview](#core-systems-overview)
2. [Tileset System (Visuals)](#tileset-system-visuals)
3. [Terrain System (Walkable Areas)](#terrain-system-walkable-areas)
4. [Fixed Room System](#fixed-room-system)
5. [Current Custom Boss Arena Implementation](#current-custom-boss-arena-implementation)
6. [Implementation Strategies](#implementation-strategies)
7. [Recommended Approach](#recommended-approach)
8. [Technical References](#technical-references)

---

## Core Systems Overview

### The Two Independent Systems

PMD Red dungeons use **two completely separate systems** for rendering and gameplay:

| System | Purpose | Storage Location | Controls |
|--------|---------|------------------|----------|
| **Tileset (Visual)** | Graphics/appearance | `gDungeon->tileset` (global) | Which sprites/tiles are rendered |
| **Terrain (Walkable)** | Collision/pathfinding | `Tile.terrainFlags` (per-tile) | Which tiles are walkable/walls |

**Critical Insight:** These systems don't talk to each other. You can have:
- Water tileset visuals with normal floor walkability
- Rocky arena visuals with fixed room walkable layout
- Any visual tileset with any walkable terrain configuration

### Data Flow Diagram

```
Floor Generation
    │
    ├─> Set Tileset ID (gDungeon->tileset = 19)  ──> Controls rendering/visuals
    │
    └─> Generate Tiles (for each tile in dungeon grid)
            │
            └─> Set terrainFlags (NORMAL/WALL/SECONDARY) ──> Controls walkability
```

**Key Takeaway:** Changing `gDungeon->tileset` changes visuals **without affecting** which tiles are walkable.

---

## Tileset System (Visuals)

### What is a Tileset?

A tileset is a collection of graphics assets (sprites, tiles, palettes) that define how the dungeon **looks**.

**Location:** `gDungeon->tileset` (8-bit value)

**Examples:**
- `19 (0x13)` - Rocky arena (used in current custom boss arenas)
- `33 (0x21)` - Unknown arena type (mentioned in boss fight docs)
- `64+` - Special tilesets with unbreakable walls

### How Tilesets Are Applied

```c
// src/dungeon_generation.c - Tileset is set ONCE per floor
void GenerateFloor(void) {
    // Tileset comes from floor properties or seed overrides
    gDungeon->tileset = floorProps->tileset;  // Applied globally

    // Rest of generation continues...
}
```

**Important:** Tileset is **global per floor**. All tiles use the same tileset for rendering.

### Tileset Data Location

- **CSV Reference:** `docs/tileset_types.csv` (tileset IDs and types)
- **Applied in:** Rendering code (sprite selection based on tileset + terrain type)

---

## Terrain System (Walkable Areas)

### Tile Structure

Each tile in the 56x32 dungeon grid has a `Tile` struct:

```c
// include/structs/map.h:65
typedef struct Tile {
    u16 terrainFlags;  // Defines walkability and terrain type
    union SpawnOrVisibilityFlags spawnOrVisibilityFlags;
    u8 unk8;
    u8 room;  // Room ID this tile belongs to
    // ... more fields
} Tile;
```

### Terrain Flags (terrainFlags)

The `terrainFlags` field uses **bit flags** to define tile properties:

```c
// include/structs/map.h:14-32
enum TerrainType {
    // Main terrain types (mutually exclusive for base type)
    TERRAIN_TYPE_WALL            = 0,      // Neither NORMAL nor SECONDARY set
    TERRAIN_TYPE_NORMAL          = 1 << 0, // 0x1 - Regular walkable floor
    TERRAIN_TYPE_SECONDARY       = 1 << 1, // 0x2 - Water or lava

    // Additional flags (can be combined)
    TERRAIN_TYPE_CORNER_CUTTABLE = 1 << 2, // 0x4
    TERRAIN_TYPE_NATURAL_JUNCTION = 1 << 3, // 0x8 - Natural corridor junction
    TERRAIN_TYPE_IMPASSABLE_WALL = 1 << 4, // 0x10 - Cannot walk through
    TERRAIN_TYPE_SHOP            = 1 << 5, // 0x20
    TERRAIN_TYPE_IN_MONSTER_HOUSE = 1 << 6, // 0x40
    TERRAIN_TYPE_UNBREAKABLE     = 1 << 8, // 0x100 - Cannot be broken
    TERRAIN_TYPE_STAIRS          = 1 << 9, // 0x200
    // ... more flags
};
```

### Setting Terrain Types

```c
// include/structs/map.h:104-112
static inline void SetTerrainNormal(Tile *tile) {
    tile->terrainFlags &= ~(TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
    tile->terrainFlags |= TERRAIN_TYPE_NORMAL;
}

static inline void SetTerrainWall(Tile *tile) {
    tile->terrainFlags &= ~(TERRAIN_TYPE_NORMAL | TERRAIN_TYPE_SECONDARY);
    // WALL = neither NORMAL nor SECONDARY set
}
```

### Walkability Determination

```c
// Pseudocode for movement checks
bool CanWalkOnTile(Tile *tile) {
    if (tile->terrainFlags & TERRAIN_TYPE_IMPASSABLE_WALL)
        return false;  // Hard wall
    if (GetTerrainType(tile) == TERRAIN_TYPE_WALL)
        return false;  // Regular wall
    if (tile->terrainFlags & TERRAIN_TYPE_NORMAL)
        return true;   // Walkable floor
    // ... more checks for water, lava, etc.
}
```

**Key Point:** The `terrainFlags` value **alone** determines walkability. Tileset is irrelevant.

---

## Fixed Room System

### What Are Fixed Rooms?

Fixed rooms are **pre-authored dungeon layouts** stored in `data/dungeon/fixedmap.inc`. They define both walkable layout and entity placements for special floors (boss fights, legendary encounters, etc.).

### Fixed Room Components

Each fixed room contains:

1. **Dimensions** (width × height)
2. **Tile Map Data** - Compressed byte array defining each tile
3. **Action IDs** - Byte values (0-219) that map to tile/entity types

### Fixed Room IDs

```c
// include/constants/fixed_rooms.h
enum FixedRoomID {
    FIXED_ROOM_MT_STEEL_SKARMORY = 1,
    FIXED_ROOM_SINISTER_WOODS_TEAM_MEANIES,
    FIXED_ROOM_MT_THUNDER_PEAK_ZAPDOS,
    // ...
    FIXED_ROOM_BURIED_RELIC_REGIROCK = 17,  // Example boss room
    FIXED_ROOM_BURIED_RELIC_REGICE,
    FIXED_ROOM_BURIED_RELIC_REGISTEEL,
    // ...
};
```

### Fixed Room Data Format

**Example from `data/dungeon/fixedmap.inc`:**

```asm
gUnknown_849E933:
.byte 0x09, 0x11, 0x00  ; Width=9, Height=17, flags=0
.byte 0x60, 0x26, 0x61, 0x26, 0x60, 0x2b  ; Compressed tile data
.byte 0x0e, 0x44, 0x0e, 0x11              ; More tile data (action IDs)
; ...
```

**Format:**
- First 3 bytes: `[width, height, flags]`
- Remaining bytes: Run-length encoded tile data
  - Format: `0xNM` where `N` = count-1, `M` = repeat count
  - Value `0x0e, 0x3c` = 1 tile of type 0x3c, repeat 14 times

### Action ID Mapping

Action IDs define what each tile becomes:

```c
// src/dungeon_generation_fixed.c:856 - PlaceFixedRoomTile()
switch (fixedRoomActionId) {
    case 0:  // Normal floor
        SetTerrainNormal(tile);
        tile->room = 0;
        break;

    case 1:  // Wall (breakable)
        SetTerrainWall(tile);
        tile->terrainFlags &= ~TERRAIN_TYPE_UNBREAKABLE;
        tile->room = CORRIDOR_ROOM;
        break;

    case 4:  // Player spawn
        SetTerrainNormal(tile);
        gDungeon->playerSpawn.x = x;
        gDungeon->playerSpawn.y = y;
        tile->room = 0;
        break;

    case 8:  // Stairs
        SetTerrainNormal(tile);
        tile->spawnOrVisibilityFlags.spawn |= SPAWN_FLAG_STAIRS;
        gDungeon->stairsSpawn.x = x;
        gDungeon->stairsSpawn.y = y;
        break;

    case 16-219:  // Entity spawn (monsters/items)
        // Looks up sFixedRoomEntities table
        const struct FixedRoomEntitiesInfo *entity = &sFixedRoomEntities[actionId - 16];
        if (entity->speciesId != 0)
            SpawnSpeciesAtPos(entity->speciesId, x, y, ...);
        if (entity->itemId != 0)
            SpawnItemAtPos(entity->itemId, x, y, ...);
        break;
}
```

### Fixed Room Entity Table

Boss entities are defined in `sFixedRoomEntities`:

```c
// src/dungeon_generation_fixed.c:99-847
static const struct FixedRoomEntitiesInfo sFixedRoomEntities[204] = {
    // Index 28 = Action ID 44 (16+28)
    [28] = {
        .speciesId = MONSTER_REGIROCK,
        .monsterBehavior = BEHAVIOR_REGIROCK,
        .trapId = NUM_TRAPS,
        .roomId = 0,
    },
    // ...
};
```

### How Fixed Rooms Are Loaded

**Full-Floor Fixed Rooms** (boss arenas like Regirock):

```c
// src/dungeon_generation.c:5970 - sub_8051288()
static void sub_8051288(s32 fixedRoomNumber) {
    // Get room dimensions from fixedmap data
    s32 fixedRoomSizeX = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->x;
    s32 fixedRoomSizeY = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->y;

    // Load compressed tile data
    gUnknown_202F1DC = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->unk3;

    // Place each tile using action IDs
    for (y = 5; y < fixedRoomSizeY + 5; y++) {
        for (x = 5; x < fixedRoomSizeX + 5; x++) {
            u8 actionId = sub_80511F0();  // Read next byte from compressed data
            PlaceFixedRoomTile(GetTileMut(x, y), actionId, x, y, TRUE);
        }
    }

    // Fill borders with impassable walls
    for (y = 0; y < DUNGEON_MAX_SIZE_Y; y++) {
        for (x = 0; x < DUNGEON_MAX_SIZE_X; x++) {
            if (x <= 4 || x >= fixedRoomSizeX + 5 || y <= 4 || y >= fixedRoomSizeY + 5) {
                Tile *tile = GetTileMut(x, y);
                tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
                SetTerrainWall(tile);
            }
        }
    }
}
```

### Fixed Room Processing Flow

```
GenerateFloor()
    │
    ├─> Check if gDungeon->fixedRoomNumber != 0
    │
    └─> ProcessFixedRoom(fixedRoomNumber, floorProps)
            │
            ├─> If fixedRoomNumber < FIRST_NON_FLOORWIDE_FIXED_ROOM (< 57)
            │       └─> sub_8051288() - Load full-floor fixed room
            │               └─> Returns TRUE (generation complete)
            │
            └─> Else: sub_804C790() - Generate normal floor with embedded fixed room
                    └─> Returns FALSE (continue with normal generation)
```

### Key Fixed Room Properties

**Stored in `struct FixedRoomsData`:**
```c
struct FixedRoomsData {
    u8 x;      // Width
    u8 y;      // Height
    u8 unk2;   // Flags
    u8 unk3[0]; // Compressed tile data pointer
};
```

**Accessed via:**
```c
struct FixedRoomsData **fixedRoomData = (struct FixedRoomsData **)gDungeon->unk13568->data;
s32 width = fixedRoomData[roomNumber]->x;
s32 height = fixedRoomData[roomNumber]->y;
u8 *tileData = fixedRoomData[roomNumber]->unk3;
```

---

## Current Custom Boss Arena Implementation

### Overview

Current implementation (as of November 2024) generates **simple rectangular arenas** programmatically without using fixed room data.

**Location:** `src/dungeon_generation.c:6295` - `GenerateBossArena()`

### Arena Parameters

```c
#define ARENA_WIDTH 9
#define ARENA_HEIGHT 10
#define ARENA_START_X 10
#define ARENA_START_Y 10
```

Creates a 9×10 arena at position (10, 10) on the dungeon grid.

### Arena Generation Logic

```c
void GenerateBossArena(BossFightConfig *config) {
    // Step 1: Reset floor (clear all tiles)
    ResetFloor();

    // Step 2: Create arena layout
    for (y = ARENA_START_Y; y < ARENA_START_Y + ARENA_HEIGHT; y++) {
        for (x = ARENA_START_X; x < ARENA_START_X + ARENA_WIDTH; x++) {
            tile = GetTileMut(x, y);

            // Perimeter = walls
            if (x == ARENA_START_X || x == ARENA_START_X + ARENA_WIDTH - 1 ||
                y == ARENA_START_Y || y == ARENA_START_Y + ARENA_HEIGHT - 1) {
                SetTerrainType(tile, TERRAIN_TYPE_WALL);
                tile->room = 0;
            }
            // Interior = walkable floor
            else {
                SetTerrainType(tile, TERRAIN_TYPE_NORMAL);
                tile->room = 0;
            }
        }
    }

    // Step 3: Set spawn positions
    gDungeon->playerSpawn.x = centerX;
    gDungeon->playerSpawn.y = playerY;  // Bottom of arena
    gDungeon->stairsSpawn.x = centerX;
    gDungeon->stairsSpawn.y = ARENA_START_Y + 1;  // Top of arena

    // Step 4: Disable normal enemy spawning
    gDungeon->unk644.enemyDensity = 0;
}
```

### Visual Tileset

Arena uses **tileset override** from `BossFightConfig`:

```c
// src/dungeon_seed_overrides.c - Boss configuration
typedef struct BossFightConfig {
    bool8 enabled;
    s16 bossSpecies;
    s16 bossHP;
    u16 bossMusic;
    u16 dropItem;
    u8 monsterBehavior;
    u8 minionCount;
    s16 minionSpecies[4];
    u8 roomTileset;  // ← Controls visual appearance
} BossFightConfig;

// Applied in dungeon_floor_spawns.c:
if (overrides.bossFight.enabled) {
    gDungeon->floorProperties.tileset = overrides.bossFight.roomTileset;
}
```

### Limitations of Current Approach

1. **Simple Geometry**: Only rectangular rooms with wall perimeter
2. **No Pre-Authored Layouts**: Cannot reuse existing fixed room designs
3. **Limited Variety**: All arenas look identical (just a box)
4. **Manual Entity Placement**: Boss/minion positions are hardcoded

---

## Implementation Strategies

### Strategy 1: Extract Fixed Room Walkable Layout (RECOMMENDED)

**Concept:** Load a fixed room's walkable terrain using `PlaceFixedRoomTile()`, but override the tileset to use custom visuals.

#### Advantages
- ✅ Reuses existing fixed room layouts (complex geometry, tested designs)
- ✅ Complete control over visuals (any tileset)
- ✅ No scenario/cutscene triggers (we skip entity spawning)
- ✅ Simple implementation (just call existing functions)
- ✅ Can mix-and-match: different fixed room layouts with different tilesets

#### Implementation Steps

**Step 1: Identify Fixed Rooms to Reuse**

Choose fixed rooms with good boss fight geometry:

```c
// Good candidates (simple boss arenas):
FIXED_ROOM_BURIED_RELIC_REGIROCK = 17    // 9×17 arena
FIXED_ROOM_BURIED_RELIC_REGICE = 18      // Similar layout
FIXED_ROOM_BURIED_RELIC_REGISTEEL = 19   // Similar layout
FIXED_ROOM_MT_THUNDER_PEAK_ZAPDOS = 3    // Larger arena
FIXED_ROOM_MT_BLAZE_PEAK_MOLTRES = 4     // Fire arena
FIXED_ROOM_FROSTY_GROTTO_ARTICUNO = 5    // Ice arena
```

**Step 2: Create Fixed Room Loader Function**

```c
// New function in dungeon_generation.c
void LoadFixedRoomLayout(s32 fixedRoomNumber, bool8 spawnEntities) {
    s32 x, y;
    s32 fixedRoomSizeX = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->x;
    s32 fixedRoomSizeY = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->y;

    // Load compressed tile data
    gUnknown_202F1DC = ((struct FixedRoomsData **)(gDungeon->unk13568->data))[fixedRoomNumber]->unk3;
    gUnknown_202F1E1 = 0;

    // Place tiles (walkable areas)
    for (y = 5; y < fixedRoomSizeY + 5; y++) {
        for (x = 5; x < fixedRoomSizeX + 5; x++) {
            u8 actionId = sub_80511F0();

            // Place tile, but DON'T spawn entities if spawnEntities == FALSE
            PlaceFixedRoomTile(GetTileMut(x, y), actionId, x, y, spawnEntities);
        }
    }

    // Create wall borders
    for (y = 0; y < DUNGEON_MAX_SIZE_Y; y++) {
        for (x = 0; x < DUNGEON_MAX_SIZE_X; x++) {
            if (x <= 4 || x >= fixedRoomSizeX + 5 || y <= 4 || y >= fixedRoomSizeY + 5) {
                Tile *tile = GetTileMut(x, y);
                tile->terrainFlags |= TERRAIN_TYPE_IMPASSABLE_WALL;
                SetTerrainWall(tile);
            }
        }
    }
}
```

**Step 3: Integrate with Boss Fight Generation**

```c
void GenerateBossArena(BossFightConfig *config) {
    if (!config->enabled)
        return;

    ResetFloor();

    // OPTION A: Use fixed room layout
    if (config->useFixedRoomLayout) {
        LoadFixedRoomLayout(config->fixedRoomNumber, FALSE);  // Don't spawn entities

        // Tileset is ALREADY set in dungeon_floor_spawns.c:
        // gDungeon->floorProperties.tileset = config->roomTileset;

        // Player/stairs spawns are set by PlaceFixedRoomTile (action IDs 4 and 8)
    }
    // OPTION B: Use simple rectangular arena (current implementation)
    else {
        // ... existing rectangular arena code ...
    }

    // Disable enemy spawning
    gDungeon->unk644.enemyDensity = 0;
}
```

**Step 4: Extend BossFightConfig**

```c
// include/dungeon_seed_overrides.h
typedef struct BossFightConfig {
    bool8 enabled;
    s16 bossSpecies;
    s16 bossHP;
    // ...
    u8 roomTileset;           // Visual tileset (unchanged)

    // NEW: Fixed room integration
    bool8 useFixedRoomLayout;  // TRUE = use fixed room, FALSE = simple rectangle
    u8 fixedRoomNumber;        // Which fixed room to load (e.g., 17 = Regirock)
} BossFightConfig;
```

**Step 5: Procedural Generation Integration**

```c
// src/dungeon_seed_overrides.c - PopulateBossFightConfig()
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result,
                                     DungeonSeedRng *rng,
                                     s32 dungeonId,
                                     s32 floorId) {
    // ... existing boss generation ...

    // NEW: Randomly select fixed room layout (or use simple rectangle)
    if (DungeonSeedRng_NextRange(rng, 0, 100) < 70) {  // 70% chance to use fixed room
        result->bossFight.useFixedRoomLayout = TRUE;

        // Pick random Regi arena (17, 18, or 19)
        s32 arenaChoice = DungeonSeedRng_NextRange(rng, 0, 3);
        result->bossFight.fixedRoomNumber = FIXED_ROOM_BURIED_RELIC_REGIROCK + arenaChoice;
    } else {
        result->bossFight.useFixedRoomLayout = FALSE;  // Simple rectangle
    }

    // Tileset is still randomized independently
    result->bossFight.roomTileset = DungeonSeedRng_NextRange(rng, 0, 2) == 0 ? 19 : 33;
}
```

#### Result

- **Fixed room walkable layout** (e.g., Regirock's 9×17 arena shape)
- **Custom tileset visuals** (e.g., tileset 19 rocky arena instead of Regirock's original tileset)
- **No scenario triggers** (no Regirock entity, no cutscenes)
- **Custom boss spawned separately** via `SpawnBossFightEntities()`

---

### Strategy 2: Use Fixed Room Fully (Visuals + Layout)

**Concept:** Load the fixed room completely (layout + visuals) but bypass scenario system to avoid cutscenes.

#### Advantages
- ✅ Authentic boss arena appearance (matches original game)
- ✅ Reuses all fixed room data (layout, visuals, geometry)

#### Disadvantages
- ❌ Tileset is coupled to fixed room (can't easily change visuals)
- ❌ May trigger scenario checks (need to carefully bypass)
- ❌ Less flexible (stuck with original visual style)

#### Implementation

```c
void GenerateBossArena(BossFightConfig *config) {
    if (!config->enabled)
        return;

    ResetFloor();

    // Load fixed room with original tileset
    gDungeon->fixedRoomNumber = config->fixedRoomNumber;

    // Set tileset to match fixed room's expected tileset
    // (Need to determine which tileset each fixed room expects)
    gDungeon->tileset = GetFixedRoomTileset(config->fixedRoomNumber);

    // Load layout
    LoadFixedRoomLayout(config->fixedRoomNumber, FALSE);  // Don't spawn entities

    // Bypass scenario system by NOT setting scenario ID
    // (Scenario triggers are in dungeon_cutscene.c, keyed by fixed room number)
}
```

#### Determining Fixed Room Tilesets

Need to analyze each fixed room to determine expected tileset:

```bash
# Extract fixedmap data and analyze
./tools/dungeonjson/dungeonjson data/dungeon/fixedmap.inc > fixedmap_analysis.txt
```

**Known Examples:**
- Regirock (17): Likely uses rocky/cave tileset
- Mt. Thunder Zapdos (3): Thunder cave tileset
- Mt. Blaze Moltres (4): Fire/lava tileset

---

### Strategy 3: Hybrid Approach

**Concept:** Mix procedural generation with fixed room elements.

#### Examples

1. **Fixed Room Shape + Procedural Entities**
   - Load Regirock's arena layout
   - Spawn custom boss/minions instead of Regirock

2. **Procedural Shape + Fixed Room Features**
   - Generate simple rectangle
   - Add fixed room "decorations" (water pools, lava, etc.)

3. **Multiple Fixed Rooms Per Seed**
   - Floor 3: Use Regirock arena + tileset 19
   - Floor 7: Use Zapdos arena + tileset 33
   - Floor 12: Use Articuno arena + tileset 19

---

## Recommended Approach

### Best Solution: Strategy 1 (Extract Fixed Room Layout)

**Reasons:**

1. **Maximum Flexibility**
   - Any fixed room layout can be paired with any tileset
   - Regirock layout with fire tileset? No problem.
   - Articuno layout with rocky tileset? Easy.

2. **No Scenario Conflicts**
   - By setting `spawnEntities = FALSE` in `PlaceFixedRoomTile()`, we skip:
     - Boss entity spawning (we spawn custom boss instead)
     - Item spawning (we handle loot separately)
   - No risk of triggering cutscenes or scenario handlers

3. **Reuses Tested Code**
   - `PlaceFixedRoomTile()` is battle-tested
   - Fixed room data is already in ROM
   - No need to reverse-engineer complex formats

4. **Procedural Variation**
   - Seed can pick different fixed room layouts per floor
   - Different tileset per floor
   - Endless combinations with no additional ROM data

5. **Gradual Migration**
   - Can keep simple rectangle arenas as fallback
   - Add fixed room layouts incrementally
   - Easy A/B testing

### Implementation Checklist

- [ ] **Phase 1: Foundation**
  - [ ] Create `LoadFixedRoomLayout(roomNumber, spawnEntities)` function
  - [ ] Add `useFixedRoomLayout` and `fixedRoomNumber` to `BossFightConfig`
  - [ ] Test loading Regirock layout with default tileset

- [ ] **Phase 2: Tileset Override**
  - [ ] Verify tileset override works with fixed room layouts
  - [ ] Test Regirock layout + tileset 19
  - [ ] Test Regirock layout + tileset 33

- [ ] **Phase 3: Entity Integration**
  - [ ] Verify player spawn position (set by action ID 4)
  - [ ] Verify stairs spawn position (set by action ID 8)
  - [ ] Spawn custom boss/minions at calculated positions

- [ ] **Phase 4: Procedural Selection**
  - [ ] Add fixed room selection to `PopulateBossFightConfig()`
  - [ ] Implement 70/30 split (70% fixed room, 30% simple rectangle)
  - [ ] Test determinism (same seed = same layout + tileset)

- [ ] **Phase 5: Expansion**
  - [ ] Identify 5-10 good boss arena fixed rooms
  - [ ] Create mapping: fixed room ID → recommended tilesets
  - [ ] Add weighted random selection

- [ ] **Phase 6: Polish**
  - [ ] Handle edge cases (invalid room numbers)
  - [ ] Add MGBA debug logging
  - [ ] Document which fixed rooms work best

---

## Technical References

### Key Files

**Fixed Room System:**
- `data/dungeon/fixedmap.inc` - Compressed fixed room data
- `include/constants/fixed_rooms.h` - Fixed room IDs
- `src/dungeon_generation_fixed.c` - Fixed room loading logic
- `include/dungeon_generation_fixed.h` - Fixed room API

**Dungeon Generation:**
- `src/dungeon_generation.c` - Main generation logic
  - `GenerateFloor()` - Entry point (line ~160)
  - `ProcessFixedRoom()` - Fixed room dispatcher (line 1093)
  - `sub_8051288()` - Full-floor fixed room loader (line 5970)
  - `GenerateBossArena()` - Custom arena generation (line 6295)

**Tile System:**
- `include/structs/map.h` - Tile struct and terrain enums
  - `struct Tile` (line 65)
  - `enum TerrainType` (line 12)

**Boss Fight Configuration:**
- `src/dungeon_seed_overrides.c` - Boss generation logic
  - `PopulateBossFightConfig()` - Boss randomization
- `include/dungeon_seed_overrides.h` - BossFightConfig struct
- `src/dungeon_floor_spawns.c` - Boss config application

### Important Functions

```c
// Fixed Room Loading
bool8 PlaceFixedRoomTile(Tile *tile, u8 actionId, s32 x, s32 y, bool8 spawnTrapOrItem);
static void sub_8051288(s32 fixedRoomNumber);  // Full-floor loader
static u8 sub_80511F0(void);  // Read next byte from compressed data

// Terrain Manipulation
void SetTerrainNormal(Tile *tile);
void SetTerrainWall(Tile *tile);
void SetTerrainSecondary(Tile *tile);
void SetTerrainType(Tile *tile, u32 terrainFlags);

// Boss Arena Generation
void GenerateBossArena(BossFightConfig *config);
void SpawnBossFightEntities(BossFightConfig *config);

// Floor Generation
void GenerateFloor(void);
static bool8 ProcessFixedRoom(s32 fixedRoomNumber, FloorProperties *floorProps);
```

### Useful Constants

```c
// Dungeon Grid
#define DUNGEON_MAX_SIZE_X 56
#define DUNGEON_MAX_SIZE_Y 32

// Fixed Room Categories
#define FIRST_NON_FLOORWIDE_FIXED_ROOM 57
#define LAST_FLOORWIDE_FIXED_ROOM 56

// Boss Arena Fixed Rooms (Good Candidates)
#define FIXED_ROOM_BURIED_RELIC_REGIROCK 17
#define FIXED_ROOM_BURIED_RELIC_REGICE 18
#define FIXED_ROOM_BURIED_RELIC_REGISTEEL 19
#define FIXED_ROOM_MT_THUNDER_PEAK_ZAPDOS 3
#define FIXED_ROOM_MT_BLAZE_PEAK_MOLTRES 4
#define FIXED_ROOM_FROSTY_GROTTO_ARTICUNO 5
```

---

## Example: Regirock Layout with Fire Tileset

### Code Example

```c
// In dungeon_seed_overrides.c
static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result,
                                     DungeonSeedRng *rng,
                                     s32 dungeonId,
                                     s32 floorId) {
    // ... determine boss fight enabled ...

    // Use Regirock's arena layout
    result->bossFight.useFixedRoomLayout = TRUE;
    result->bossFight.fixedRoomNumber = FIXED_ROOM_BURIED_RELIC_REGIROCK;

    // But use fire/lava tileset for visuals
    result->bossFight.roomTileset = 33;  // Or whatever tileset ID is fire

    // Custom boss (not Regirock)
    result->bossFight.bossSpecies = MONSTER_MOLTRES;
    result->bossFight.bossHP = 500;

    // ... rest of config ...
}
```

### Result

- **Layout:** Regirock's 9×17 arena with specific walkable geometry
- **Visuals:** Fire/lava tileset graphics
- **Boss:** Moltres spawned instead of Regirock
- **No Cutscenes:** Scenario system bypassed

### Technical Flow

```
GenerateFloor()
    │
    ├─> Check boss fight config: config->enabled == TRUE
    │
    └─> GenerateBossArena(config)
            │
            ├─> ResetFloor()
            │
            ├─> config->useFixedRoomLayout == TRUE
            │       └─> LoadFixedRoomLayout(17, FALSE)  // Regirock layout, no entities
            │               ├─> Read fixedmap data for room 17
            │               ├─> For each tile: PlaceFixedRoomTile(tile, actionId, x, y, FALSE)
            │               │       ├─> Action 0: SetTerrainNormal() → walkable floor
            │               │       ├─> Action 1: SetTerrainWall() → wall
            │               │       ├─> Action 4: Set player spawn position
            │               │       ├─> Action 8: Set stairs spawn position
            │               │       └─> Action 16+: SKIP (spawnEntities == FALSE)
            │               └─> Fill borders with impassable walls
            │
            ├─> Tileset already set to 33 in dungeon_floor_spawns.c
            │       (before GenerateFloor() was called)
            │
            └─> gDungeon->unk644.enemyDensity = 0  // Disable auto-spawn

    (Back in GenerateFloor)
    └─> SpawnBossFightEntities(config)
            ├─> Spawn Moltres at calculated position
            └─> Spawn minions around Moltres
```

---

## Conclusion

**Visuals and walkable terrain are completely independent.** You can extract any fixed room's walkable layout and combine it with any tileset for limitless variety in boss arenas.

**Recommended implementation:**
1. Use `LoadFixedRoomLayout(roomNumber, FALSE)` to extract walkable layout
2. Override tileset via `BossFightConfig.roomTileset`
3. Spawn custom bosses separately via `SpawnBossFightEntities()`
4. Procedurally vary both layout choice and tileset per seed

This gives you the best of both worlds:
- **Rich, pre-designed arena layouts** (from fixed rooms)
- **Visual variety** (from tileset mixing)
- **Custom bosses** (your procedural system)
- **No cutscene conflicts** (bypass scenario system)
