# Boss Fight Override System - Analysis & Game Plan

## Executive Summary

This document outlines a plan to implement **procedurally-generated boss fights** for the dungeon randomization system. Boss fights are generated **at runtime based on the dungeon seed** - not from static configuration files. The seed deterministically controls which floors have bosses, which species spawn, minion configuration, and loot drops.

**Requirements:**

1. **Boss + minions** - Spawn boss and optional minions at specific positions
2. **Custom tileset** - Use smaller arena tilesets (e.g., 19, 33) for boss rooms
3. **Boss loot** - Drop items when boss is defeated (Regirock-style)
4. **Warp stairs** - Spawn stairs at end of room after boss defeat
5. **Seed-based randomization** - Everything procedurally generated from seed
6. **Dynamic per-floor** - Boss fights can occur on any floor based on seed logic

## Critical Requirement: Procedural Generation

**ALL boss fight configuration must be generated in-memory from the seed.**

The override system does NOT use static JSON files. Instead:
- Input: `seed`, `dungeonId`, `floorId`
- Output: Procedurally-generated boss fight configuration
- Same seed = same boss fights every time (deterministic)
- Different seed = different bosses, floors, minions

Example: Seed `12345` might generate:
- Tiny Woods 2F: Mankey boss + 2 Primeape minions, drops Oran Berry
- Tiny Woods 5F: Electabuzz boss + 3 Pikachu minions, drops Thunder Stone
- Thunderwave Cave 3F: Golem boss + 2 Graveler minions, drops Rock Gem

Seed `67890` generates completely different bosses/floors.

## Current State Analysis

### Existing Boss Fight System

The game has a well-established "Regi-style" boss fight pattern (documented in `docs/custom_bossfights.md`):

**Key Components:**
- **Fixed Rooms** - Boss fights use fixed room layouts (e.g., FIXED_ROOM_BURIED_RELIC_REGIROCK = 17)
- **Scenario System** - Mapping from fixed room ID → scenario ID → pre-fight/post-fight handlers
- **Boss Spawn** - Entities placed via `sFixedRoomEntities` table in `src/dungeon_generation_fixed.c:99`
- **Stairs Spawning** - Function `sub_808B1CC()` spawns stairs at anchor position after boss defeat (`src/dungeon_cutscene_regis.c:299`)
- **Loot Drops** - Same function drops items if team doesn't have them

**Flow:**
1. Floor loads → Fixed room number checked → Scenario ID selected
2. `DisplayPreFightDialogue()` called → Sets boss HP/music via `SetupBossFightHP()`
3. Boss defeated → Faint dispatcher `sub_8084E00()` → Scenario-specific handler
4. Handler calls `sub_808B1CC(itemId)` → Spawns stairs at anchor tile + drops loot

### Current Override System

Located in `src/dungeon_seed_overrides.c` and `include/dungeon_seed_overrides.h`:

**How It Works Now:**

```c
// Main entry point - called when loading floor
void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId,
                                               DungeonSeedFloorOverrides *result)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0);

    // Procedurally generate tileset from seed
    result->tileset = SelectTileset(floorId);

    // Procedurally generate monster spawns from seed
    PopulateSpawnTable(result, &rng, dungeonId, floorId);
}
```

**Procedural RNG System:**
- `DungeonSeedRng` - Custom RNG seeded with (seed + dungeonId + floorId)
- `DungeonSeedRng_Next()` - Get next random number
- `DungeonSeedRng_NextRange(min, max)` - Get random number in range
- Deterministic: same inputs = same outputs every time

**Existing Structures:**
```c
typedef struct DungeonSeedFloorOverrides {
    u8 tileset;                                          // Procedurally selected tileset
    s32 spawnCount;                                      // Number of monster spawns
    SpawnPokemonData spawns[MONSTER_SPAWNS_ARR_COUNT];   // Procedurally generated spawns
} DungeonSeedFloorOverrides;
```

**Previous Boss Fight Attempt:**

There was a previous attempt at boss fights using:
```c
typedef struct CustomBossFight {
    u8 dungeonId;
    s32 floorId;
    s16 bossSpecies;
    s16 bossHP;
    u16 bossMusic;
    u16 dropItem;
    u8 monsterBehavior;
    bool8 isActive;
} CustomBossFight;
```

Functions: `DungeonSeedOverrides_SetupCustomBossFight()`, `DungeonSeedOverrides_SpawnCustomBoss()`, etc.

**Problem:** This approach tried to spawn bosses in randomly-generated dungeons, which caused memory access errors because:
1. No fixed room layout = unreliable tile positions
2. Stairs spawning doesn't work correctly without anchor tiles
3. No proper integration with dungeon generation
4. Boss was spawned AFTER floor generation, not INSTEAD of it

## Problem Analysis

### Memory Errors Root Cause

Testing Tiny Woods 4F with fixed room 17 (Regirock) caused memory errors at addresses like `0x2224A898`:

**Issue:** Fixed rooms rely on:
1. Pre-authored tilemap data in `data/dungeon/fixedmap.inc`
2. Entity spawn tables (`sFixedRoomEntities`)
3. Anchor tiles (action ID 69) for stairs position
4. Proper scenario ID mapping for pre-fight/post-fight hooks

Simply setting `fixedRoomNumber = 17` in floor properties doesn't work because:
- The dungeon seed override system was clearing `fixedRoomNumber` in `src/dungeon_floor_spawns.c:50`
- Even when preserved, the fixed room data expects specific dungeon context
- Tileset mismatches between what fixed room expects vs what override provides

### Tileset Information

Need to understand which tilesets create good boss arenas:
- **Tileset 19 (0x13)** - Rocky arena (used in testing)
- **Tileset 33 (0x21)** - Need to identify from dungeon data
- These should be smaller, arena-style rooms suitable for boss battles

## Proposed Solution

### Option 1: Scenario-Free Boss System (RECOMMENDED)

Create a lightweight boss fight system that works WITHOUT the scenario infrastructure. This avoids the complexity of scenario IDs, fixed room mappings, and cutscene triggers.

**Architecture:**

1. **Extend DungeonSeedFloorOverrides** to include boss fight configuration:
```c
typedef struct BossFightConfig {
    bool8 enabled;                    // Is there a boss fight on this floor?
    s16 bossSpecies;                  // Boss monster species
    s16 bossHP;                       // Boss HP
    u16 bossMusic;                    // Music during fight
    u16 dropItem;                     // Item to drop (ITEM_NOTHING for none)
    u8 monsterBehavior;               // Boss behavior ID
    u8 minionCount;                   // Number of minions to spawn
    s16 minionSpecies[4];             // Minion species (up to 4)
    u8 roomTileset;                   // Override tileset for boss room
} BossFightConfig;

typedef struct DungeonSeedFloorOverrides {
    u8 tileset;
    s32 spawnCount;
    SpawnPokemonData spawns[MONSTER_SPAWNS_ARR_COUNT];
    BossFightConfig bossFight;        // NEW: Boss fight configuration
} DungeonSeedFloorOverrides;
```

2. **Generate simple fixed room layouts programmatically:**
   - Instead of using pre-authored fixedmap data, generate small arena rooms in code
   - Create rectangular arenas (e.g., 15x10 tiles) with walls and walkable floor
   - Place player at bottom-center
   - Place boss at top-center (or specified offset)
   - Place minions around boss
   - Mark stairs spawn position (back of room)

3. **Boss spawning during dungeon generation:**
   - Hook into `GenerateFloor()` in `src/dungeon_generation.c`
   - When boss fight is configured, skip normal dungeon generation
   - Generate simple arena room instead
   - Spawn boss and minions directly (not via scenario system)
   - Set boss HP/music immediately

4. **Boss defeat handling:**
   - Add hook in faint dispatcher `sub_8084E00()` (`src/dungeon_cutscene.c:541`)
   - Check if defeated entity is the boss (by tracking Entity pointer)
   - Spawn stairs at pre-marked position
   - Drop loot item
   - NO scenario ID required

**Advantages:**
- ✅ No scenario ID conflicts with existing game content
- ✅ Works independently of fixedmap data
- ✅ Simple to configure through override JSON
- ✅ Complete control over room layout
- ✅ No memory errors from missing tilemap data

**Disadvantages:**
- ❌ Need to implement room generation code
- ❌ Can't reuse existing boss room art (but we don't need to)
- ❌ Need to manually track boss entity for defeat detection

### Option 2: Reuse Existing Fixed Rooms (NOT RECOMMENDED)

Try to make existing fixed rooms work with overrides.

**Problems:**
- Fixed rooms are hardcoded to specific dungeons/floors
- Scenario system is complex and tightly coupled
- Risk of breaking existing game content
- Memory errors from tilemap/tileset mismatches
- Can't easily change which floor has which boss

**Verdict:** Too fragile, not worth the effort.

## Implementation Plan (Option 1)

### Phase 1: Extend Override Configuration

**File:** `include/dungeon_seed_overrides.h`

Add `BossFightConfig` struct to `DungeonSeedFloorOverrides` as shown above.

**File:** `src/dungeon_seed_overrides.c`

Modify `DungeonSeedOverrides_GenerateFloorConfig()` to procedurally generate boss fights:

```c
// Helper: Get species pool based on floor tier
static const SeedSpeciesPool* GetBossPool(s32 floorId)
{
    if (floorId <= 5) return &sPoolEarlyBosses;    // Weak bosses
    if (floorId <= 15) return &sPoolMidBosses;     // Mid-tier bosses
    return &sPoolLateBosses;                        // Strong bosses
}

static void PopulateBossFightConfig(DungeonSeedFloorOverrides *result,
                                     DungeonSeedRng *rng,
                                     s32 dungeonId,
                                     s32 floorId)
{
    const SeedSpeciesPool *bossPool;
    const SeedSpeciesPool *minionPool;
    s32 bossIndex, i;

    // Procedurally determine if this floor has a boss
    // Example: 20% chance for any floor >= 3
    if (floorId < 2 || DungeonSeedRng_NextRange(rng, 0, 100) > 20) {
        result->bossFight.enabled = FALSE;
        return;
    }

    result->bossFight.enabled = TRUE;

    // Procedurally select boss species from tier-appropriate pool
    bossPool = GetBossPool(floorId);
    bossIndex = DungeonSeedRng_NextRange(rng, 0, bossPool->count);
    result->bossFight.bossSpecies = bossPool->species[bossIndex];

    // Procedurally set HP scaling with floor
    result->bossFight.bossHP = 300 + (floorId * 25);

    // Procedurally select music (vary based on boss type)
    result->bossFight.bossMusic = MUS_BOSS_BATTLE;

    // Procedurally select loot drop
    result->bossFight.dropItem = SelectRandomLoot(rng, floorId);

    // Procedurally determine minion count (0-3)
    result->bossFight.minionCount = DungeonSeedRng_NextRange(rng, 0, 4);

    // Procedurally select minion species (related to boss)
    minionPool = GetMinionPoolForBoss(result->bossFight.bossSpecies);
    for (i = 0; i < result->bossFight.minionCount; i++) {
        s32 minionIdx = DungeonSeedRng_NextRange(rng, 0, minionPool->count);
        result->bossFight.minionSpecies[i] = minionPool->species[minionIdx];
    }

    // Procedurally select arena tileset
    result->bossFight.roomTileset = DungeonSeedRng_NextRange(rng, 0, 2) == 0 ? 19 : 33;

    // Set behavior for boss identification
    result->bossFight.monsterBehavior = BEHAVIOR_BOSS_OVERRIDE;
}

// Modified main function
void DungeonSeedOverrides_GenerateFloorConfig(s32 seed, u8 dungeonId, s32 floorId,
                                               DungeonSeedFloorOverrides *result)
{
    DungeonSeedRng rng = DungeonSeedRng_Init(seed, dungeonId, floorId, 0);

    ClearFloorOverrides(result);

    // NEW: Procedurally generate boss fight configuration
    PopulateBossFightConfig(result, &rng, dungeonId, floorId);

    // If boss fight enabled, use boss tileset; otherwise normal generation
    if (result->bossFight.enabled) {
        result->tileset = result->bossFight.roomTileset;
        result->spawnCount = 0;  // No normal spawns in boss rooms
    } else {
        result->tileset = SelectTileset(floorId);
        PopulateSpawnTable(result, &rng, dungeonId, floorId);
    }
}
```

**Key Points:**
- Boss fights are determined **purely by seed + dungeonId + floorId**
- Same seed = same boss configuration every playthrough
- No static data files required
- All randomization happens in-memory at runtime

### Phase 2: Arena Generation

**File:** `src/dungeon_generation_boss_arena.c` (NEW)

Create new file to handle boss arena generation:

```c
// Generate a simple rectangular boss arena
void GenerateBossArena(BossFightConfig *config)
{
    // 1. Create rectangular room (e.g., 15x10)
    // 2. Fill with walkable floor tiles
    // 3. Add walls around perimeter
    // 4. Set player spawn at bottom-center
    // 5. Mark boss spawn at top-center
    // 6. Mark minion spawns around boss
    // 7. Mark stairs spawn position (back wall)
    // 8. Apply tileset from config->roomTileset
}

// Spawn boss and minions after arena is created
void SpawnBossFightEntities(BossFightConfig *config)
{
    // 1. Spawn boss at marked position
    // 2. Call SetupBossFightHP() to set HP and music
    // 3. Spawn minions at marked positions
    // 4. Track boss entity pointer globally
}
```

**Key Functions to Implement:**
- `CreateArenaWalls()` - Place wall tiles around perimeter
- `CreateArenaFloor()` - Fill interior with walkable tiles
- `MarkSpawnPositions()` - Set up player, boss, minion, stairs positions
- `ApplyArenaTileset()` - Load tileset graphics for arena

### Phase 3: Integrate with Dungeon Generation

**File:** `src/dungeon_generation.c`

Modify `GenerateFloor()` to check for boss fights:

```c
void GenerateFloor(void)
{
    // ... existing initialization ...

    // NEW: Check if this floor has a boss fight
    if (gDungeonFloorOverrides.bossFight.enabled) {
        GenerateBossArena(&gDungeonFloorOverrides.bossFight);
        SpawnBossFightEntities(&gDungeonFloorOverrides.bossFight);
        // Skip normal dungeon generation
        return;
    }

    // ... existing floor generation code ...
}
```

**File:** `src/dungeon_floor_spawns.c`

Update `ApplySeedOverridesToCurrentFloor()` to handle boss fights:

```c
static void ApplySeedOverridesToCurrentFloor(void)
{
    DungeonSeedFloorOverrides overrides;

    // ... existing code ...

    DungeonSeedOverrides_GenerateFloorConfig(seed, dungeonId, floor, &overrides);

    // Apply boss fight configuration if enabled
    if (overrides.bossFight.enabled) {
        // Override tileset to boss arena tileset
        gDungeon->floorProperties.tileset = overrides.bossFight.roomTileset;

        // Disable normal spawns
        gDungeon->floorProperties.enemyDensity = 0;

        // Store boss config globally for arena generation
        // (need global variable to pass config to GenerateFloor)
    }

    // ... rest of function ...
}
```

### Phase 4: Boss Defeat Handling

**File:** `src/dungeon_cutscene.c`

Modify faint dispatcher to handle custom boss defeats:

```c
// Add to sub_8084E00() around line 541
void sub_8084E00(Entity *pokemon)
{
    // ... existing scenario-based handlers ...

    // NEW: Check if this is a custom override boss
    if (DungeonSeedOverrides_IsCustomBoss(pokemon)) {
        DungeonSeedOverrides_HandleBossFaint(pokemon);
        return;
    }

    // ... rest of function ...
}
```

**File:** `src/dungeon_seed_overrides.c`

Implement boss defeat handler:

```c
static Entity *sCustomBossEntity = NULL;  // Track boss entity
static DungeonPos sStairsSpawnPos;        // Track stairs position

void DungeonSeedOverrides_HandleBossFaint(Entity *pokemon)
{
    Item item;
    Tile *tile;

    if (pokemon != sCustomBossEntity)
        return;

    // Spawn stairs at marked position
    tile = GetTileMut(sStairsSpawnPos.x, sStairsSpawnPos.y);
    tile->terrainFlags |= TERRAIN_TYPE_STAIRS;

    // Drop loot if configured
    if (gDungeonFloorOverrides.bossFight.dropItem != ITEM_NOTHING) {
        DungeonPos dropPos = sStairsSpawnPos;
        dropPos.y--;  // One tile in front of stairs
        ItemIdToItem(&item, gDungeonFloorOverrides.bossFight.dropItem, 0);
        SpawnItem(&dropPos, &item, 1);
    }

    // Update minimap and visibility
    UpdateTrapsVisibility();
    UpdateMinimap();
}
```

### Phase 5: Testing & Refinement

**Test Cases:**
1. **Tiny Woods 2F** - Spawn weak boss (e.g., Mankey) with 2 minions
2. **Thunderwave Cave 4F** - Spawn mid-tier boss (e.g., Electabuzz)
3. **Verify:**
   - Arena generates correctly with tileset 19
   - Boss spawns at top of arena
   - Minions spawn around boss
   - Player spawns at bottom
   - Boss HP and music set correctly
   - Boss defeat spawns stairs
   - Loot drops correctly
   - No memory errors

## File Changes Summary

### New Files
- `src/dungeon_generation_boss_arena.c` - Boss arena generation logic
- `include/dungeon_generation_boss_arena.h` - Header for arena generation

### Modified Files
- `include/dungeon_seed_overrides.h` - Add BossFightConfig struct
- `src/dungeon_seed_overrides.c` - Boss fight generation and handling
- `src/dungeon_generation.c` - Hook arena generation into floor generation
- `src/dungeon_floor_spawns.c` - Apply boss fight overrides
- `src/dungeon_cutscene.c` - Boss defeat handling

## Alternative Approaches Considered

### A. Mini Fixed Rooms
Generate minimal fixed room data at runtime and inject into fixedmap system.

**Rejected because:**
- Still needs scenario ID mapping
- Complex to generate valid fixedmap format
- Harder to debug

### B. Hybrid Approach
Use fixed rooms for layout, but override spawns and stairs.

**Rejected because:**
- Still have tilemap/tileset mismatch issues
- Can't guarantee rooms exist for all tilesets
- Less flexible

## Next Steps

1. ✅ Document analysis (this file)
2. ⬜ Implement Phase 1 (extend override configuration)
3. ⬜ Implement Phase 2 (arena generation)
4. ⬜ Implement Phase 3 (integrate with dungeon generation)
5. ⬜ Implement Phase 4 (boss defeat handling)
6. ⬜ Test with Tiny Woods and Thunderwave Cave
7. ⬜ Refine and fix any issues

## Procedural Generation Details

### How Boss Randomization Works

```
Input:  seed=12345, dungeonId=TINY_WOODS(0), floorId=2
        ↓
RNG Init: DungeonSeedRng_Init(12345, 0, 2, 0) → state=X
        ↓
Boss Check: NextRange(0,100) = 15 < 20? → YES, boss fight enabled
        ↓
Boss Pool: GetBossPool(2) → sPoolEarlyBosses (Mankey, Geodude, Machop...)
        ↓
Boss Select: NextRange(0, 5) = 2 → bossSpecies = MONSTER_GEODUDE
        ↓
HP Scale: 300 + (2 * 25) = 350 HP
        ↓
Minion Count: NextRange(0, 4) = 2 → 2 minions
        ↓
Minion Pool: GetMinionPoolForBoss(GEODUDE) → (Diglett, Sandshrew)
        ↓
Minion 1: NextRange(0, 2) = 0 → MONSTER_DIGLETT
Minion 2: NextRange(0, 2) = 1 → MONSTER_SANDSHREW
        ↓
Loot: SelectRandomLoot(rng, 2) → ITEM_ORAN_BERRY
        ↓
Tileset: NextRange(0, 2) = 1 → tileset 33
        ↓
Output: BossFightConfig {
    enabled = TRUE,
    bossSpecies = GEODUDE,
    bossHP = 350,
    minionCount = 2,
    minionSpecies = {DIGLETT, SANDSHREW},
    dropItem = ORAN_BERRY,
    roomTileset = 33
}
```

**Same seed = exact same output every time.** This is critical for deterministic randomization.

### Species Pools

Boss and minion species are selected from predefined pools:

```c
// Early game bosses (floors 1-5)
static const s16 sPoolEarlyBosses[] = {
    MONSTER_MANKEY, MONSTER_GEODUDE, MONSTER_MACHOP,
    MONSTER_DROWZEE, MONSTER_NIDORINO
};

// Mid game bosses (floors 6-15)
static const s16 sPoolMidBosses[] = {
    MONSTER_PRIMEAPE, MONSTER_GOLEM, MONSTER_MACHOKE,
    MONSTER_HYPNO, MONSTER_NIDOKING, MONSTER_ELECTABUZZ
};

// Late game bosses (floors 16+)
static const s16 sPoolLateBosses[] = {
    MONSTER_MACHAMP, MONSTER_ALAKAZAM, MONSTER_GENGAR,
    MONSTER_TYRANITAR, MONSTER_SALAMENCE
};
```

Minions are thematically related to their boss (e.g., Geodude → Diglett/Sandshrew).

## Questions & Decisions Needed

1. **Arena size?** Start with 15x10 tiles (configurable per-tileset)
2. **Minion limit?** Support up to 4 minions
3. **Boss frequency?** 20% chance per eligible floor (floors >= 3)
4. **Tileset selection?** Random choice between 19 and 33
5. **Difficulty scaling?** 300 base HP + (floor * 25) = 300-1800 HP range
6. **Boss species pools?** Define early/mid/late game pools with ~5-8 options each

## References

- **Existing boss fight docs:** `docs/custom_bossfights.md`
- **Fixed room constants:** `include/constants/fixed_rooms.h`
- **Fixed room generation:** `src/dungeon_generation_fixed.c`
- **Scenario system:** `src/dungeon_cutscene.c:120`
- **Regi boss handlers:** `src/dungeon_cutscene_regis.c`
- **Override system:** `src/dungeon_seed_overrides.c`
- **Floor generation:** `src/dungeon_generation.c:160`
