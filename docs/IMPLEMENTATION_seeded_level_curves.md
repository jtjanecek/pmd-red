# Seeded Enemy Level Curve Plan

Goals: (1) each species keeps a single level across the floor span it appears in, (2) dungeons ramp up over the run order, and (3) floors ramp up inside a dungeon. All scaling must stay deterministic off the quiz seed + dungeon/floor, and respect the existing seed override hooks.

## Inputs + Anchors
- Seed + dungeon/floor context already built in `DungeonSeedOverrides_GenerateFloorConfig` (seed, `dungeonId`, `floorId`, `DungeonSeedRng`).
- Progression order is encoded in `sSequentialDungeonList` / `GetDungeonNumberForFloorScaling`; difficulty setting is available via `GetGameDifficultySetting()`.
- Spawn bands and per-floor type selections are created in `BuildSpawnRangesForDungeon()` and consumed in `PopulateSpawnTableFromRanges()`.
- Vanilla level references live in `docs/vanilla-dungeon-level-curves.md` if we need sanity bounds.

## Proposed Curve
- **Dungeon progression:** derive a baseline level from `dungeonNumber = GetDungeonNumberForFloorScaling(dungeonId)`. Example target: start at 3–5 for dungeon #1 and add ~2–3 levels per dungeon, capped to ~80–90. Apply a small difficulty offset (Normal 0, Hard +3, Nightmare +6).
- **Floor slope:** convert the floor index to a 0–1 scalar (floorId vs. `floorCount`) and add a per-floor ramp (e.g., +0–8 levels across the whole dungeon). Use a piecewise table (reuse `sFloorBandTable` or a new `sFloorLevelBands[]`) to avoid huge jumps in 99F dungeons.
- **Seed jitter:** add a tiny, deterministic offset (0–2) via `DungeonSeedRng` with a new salt to keep runs with the same seed identical while breaking perfect symmetry between equal-depth dungeons.
- **Per-species lock:** when a spawn range is chosen, compute its level once from the midpoint of that range and store it; every spawn that reuses that range keeps that level.

## Implementation Steps
1) **Level context helper:** add `SeededLevelCurveContext` + `CalcSeededSpawnLevel(seed, dungeonId, floorIndex, floorCount)` in `dungeon_seed_overrides.c` to implement the formula above (progression baseline + floor slope + difficulty + jitter, min/max clamped). Keep it independent of species to avoid bias.  
2) **Cache level per range:** extend `SeededSpawnRange` with `u8 level`. In `BuildSpawnRangesForDungeon()`, after picking `start`/`end`, compute `midFloor = start + (len/2)` and set `range->level = CalcSeededSpawnLevel(..., midFloor, floorCount)`. Store `floorCount` in the cache for reuse.  
3) **Use cached level on spawn fill:** in `PopulateSpawnTableFromRanges()`, replace calls to `RollSpawnLevel` with `range->level` (fall back to `RollSpawnLevel` only if cache is invalid). Remove per-entry variance so the level stays fixed for that species’ band.  
4) **Boss/minion sanity:** keep boss level selection separate (their config already sets moves/stats). If minions should mirror the floor curve, feed them through the same helper using the boss floor index.  
5) **Logging + validation:** add MGBA logs when building ranges to print `(species, start-end, level)`, and a one-time summary per dungeon showing min/max levels after the curve. Validate by running two seeds and comparing `execution.log` to confirm determinism and ramps.  
6) **Tuning hooks:** expose a small table for progression slope per difficulty (e.g., `sDifficultyLevelStep[NUM_DIFFICULTY_SETTINGS]`) and a clamp for late-game caps so designers can tweak without touching code paths.

## Rollout Notes
- Changes live entirely in the seeded override path; vanilla tables stay untouched. Fixed rooms and special-case dungeons can bypass the helper if needed by setting `range->level` explicitly.
- After implementation, rebuild (`make`) and spot-check a few dungeons across the progression list to ensure the per-range lock is respected (no level drift within a band, clear increases by dungeon and by floor).
