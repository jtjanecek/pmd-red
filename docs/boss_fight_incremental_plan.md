# Boss Fight System - Incremental Implementation Plan

**Goal:** Build the boss fight system one feature at a time, testing stability at each step.

**Strategy:** Start with minimal functionality and add complexity only after confirming each step works.

---

## Step 1: Arena Generation Only ✅ CURRENTLY IMPLEMENTED - TESTING

**What:** Generate empty boss arena with player spawn, NO entity spawning at all.

**Success Criteria:**
- [ ] Floor loads without freezing ← **TEST THIS FIRST**
- [ ] Empty 15×10 arena appears
- [ ] Player spawns at bottom-center of arena
- [ ] No enemies spawn (neither boss nor normal enemies)
- [ ] Can move around freely without freezes

**Implementation Status:**
- ✅ `GenerateBossArena()` creates arena structure (dungeon_generation.c:6164)
- ✅ Sets `gDungeon->playerSpawn` and `gDungeon->stairsSpawn` (lines 6181-6186)
- ✅ Disables all enemy spawning mechanisms (dungeon_floor_spawns.c:73-76):
  - `enemyDensity = 0`
  - `currFloorMonsterSpawnsCount = 0`
  - `monsterSpawnsPopulated = FALSE`
  - Cleared all spawn table entries
- ✅ Entity spawning DISABLED in run_dungeon.c:405-406
- ✅ Always spawns boss floors on >= 2 for testing

**Code Locations:**
- Arena generation: `src/dungeon_generation.c:6164-6194`
- Spawn disabling: `src/dungeon_floor_spawns.c:65-77`
- Boss config: `src/dungeon_seed_overrides.c:416-448`
- Entity spawn hook: `src/run_dungeon.c:401-411`

**Build Output:**
```
Memory region         Used Size  Region Size  %age Used
           EWRAM:      242604 B       256 KB     92.55%
           IWRAM:       30960 B        32 KB     94.48%
             ROM:         32 MB        32 MB    100.00%
```

**Files Modified:**
- `src/dungeon_generation.c` - Arena generation, hook
- `src/dungeon_floor_spawns.c` - Spawn disabling, EWRAM storage
- `src/run_dungeon.c` - Entity spawn hook point
- `include/dungeon_seed_overrides.h` - BossFightConfig struct
- `src/dungeon_seed_overrides.c` - Boss config generation
- `include/dungeon_generation.h` - Function declarations
- `sym_ewram.txt` - EWRAM linker configuration

---

## Step 2: Single Boss Spawn (No HP/Music Override)

**What:** Spawn a single boss enemy with default stats, no custom HP or music.

**Success Criteria:**
- [ ] Arena loads successfully
- [ ] Single boss enemy appears at top-center
- [ ] Boss has normal HP/behavior (no custom settings)
- [ ] No freezes when entering arena
- [ ] Can attack and defeat boss normally

**Implementation:**
- Add basic `SpawnWildMon()` call in `run_dungeon.c` after player spawn
- No `SetupBossFightHP()` call
- No minion spawning
- No boss registration

**Testing Focus:** Verify `SpawnWildMon()` works at this timing point.

---

## Step 3: Boss HP and Music Override

**What:** Add custom HP and music to boss using `SetupBossFightHP()`.

**Success Criteria:**
- [ ] Boss spawns with custom HP (e.g., 500 HP instead of normal)
- [ ] Boss music plays when floor loads
- [ ] Boss can be defeated normally
- [ ] No freezes or crashes

**Implementation:**
- Call `SetupBossFightHP(bossEntity, config->bossHP, config->bossMusic)` after spawning
- Verify HP is actually set correctly

**Testing Focus:** Confirm HP/music override doesn't cause issues.

---

## Step 4: Add Minion Spawning

**What:** Spawn 1-3 minions around boss.

**Success Criteria:**
- [ ] Boss spawns correctly
- [ ] Minions spawn in positions around boss
- [ ] Multiple entities don't cause conflicts
- [ ] Can defeat minions and boss separately

**Implementation:**
- Enable minion spawning loop in `SpawnBossFightEntities()`
- Start with 1 minion, then test 2-3

**Testing Focus:** Multiple entity spawning stability.

---

## Step 5: Boss Defeat Detection

**What:** Register boss entity and detect when defeated.

**Success Criteria:**
- [ ] Boss is tracked as "the boss"
- [ ] `DungeonSeedOverrides_IsCustomBoss()` returns TRUE for boss
- [ ] Faint dispatcher recognizes boss defeat
- [ ] No crashes when boss faints

**Implementation:**
- Implement `DungeonSeedOverrides_RegisterBossEntity()`
- Implement `DungeonSeedOverrides_IsCustomBoss()`
- Hook into `sub_8084E00()` faint dispatcher (already exists)

**Testing Focus:** Boss tracking without stairs/loot spawning yet.

---

## Step 6: Stairs Spawning on Boss Defeat

**What:** Spawn stairs when boss is defeated.

**Success Criteria:**
- [ ] Stairs appear at stored position after boss defeat
- [ ] Can walk to stairs and advance to next floor
- [ ] Stairs don't appear before boss is defeated

**Implementation:**
- Implement `DungeonSeedOverrides_HandleBossFaint()`
- Spawn stairs at stored position using appropriate spawn function
- Test without loot first

**Testing Focus:** Stairs spawning mechanics.

---

## Step 7: Loot Drops on Boss Defeat

**What:** Drop items when boss is defeated.

**Success Criteria:**
- [ ] Item drops at boss position when defeated
- [ ] Can pick up item
- [ ] Item doesn't drop if inventory full (appropriate behavior)

**Implementation:**
- Add item spawning to `DungeonSeedOverrides_HandleBossFaint()`
- Use proper item spawn function
- Handle edge cases (full inventory, etc.)

**Testing Focus:** Item spawning and pickup.

---

## Step 8: Restore Normal Boss Frequency

**What:** Change from "always boss on floor >= 2" to normal 20% chance.

**Success Criteria:**
- [ ] Boss floors are randomized (20% chance)
- [ ] Same seed generates same boss floors
- [ ] Normal floors work correctly
- [ ] Boss floors still work correctly

**Implementation:**
- Restore original probability check in `PopulateBossFightConfig()`
- Test multiple floor transitions

**Testing Focus:** Randomization and determinism.

---

## Step 9: Arena Tileset Variation

**What:** Apply custom tileset to boss arenas (tileset 19 or 33).

**Success Criteria:**
- [ ] Boss arenas use procedurally selected tileset
- [ ] Tileset renders correctly
- [ ] No visual glitches

**Implementation:**
- Apply `config->roomTileset` during arena generation
- May need tileset loading code

**Testing Focus:** Visual correctness, no rendering issues.

---

## Step 10: Polish and Edge Cases

**What:** Handle edge cases and polish the system.

**Success Criteria:**
- [ ] No crashes under any conditions
- [ ] Works with all dungeon types
- [ ] Works with save/load
- [ ] Performance is acceptable

**Implementation:**
- Add error checking everywhere
- Handle edge cases (no valid spawn positions, etc.)
- Optimize if needed
- Add logging/debugging support if needed

**Testing Focus:** Robustness and edge cases.

---

## Current Status

**Last Updated:** November 20, 2025
**Current Step:** Step 1 - Arena Generation Only
**Next Step:** Test Step 1, then proceed to Step 2

## Rollback Commands

If a step breaks, rollback to previous working state:

```bash
# Disable entity spawning completely
# Comment out SpawnBossFightEntities() call in run_dungeon.c

# Disable boss check entirely
# Comment out boss fight check in dungeon_generation.c

# Restore normal enemy spawning
# Remove spawn disabling code in dungeon_floor_spawns.c
```

## Testing Checklist Per Step

For each step, verify:
1. ✅ ROM builds without errors
2. ✅ ROM boots successfully
3. ✅ Can reach floor 1 normally
4. ✅ Floor 2+ loads without freezing
5. ✅ New feature works as expected
6. ✅ No regressions from previous steps

Only proceed to next step if ALL checklist items pass!
