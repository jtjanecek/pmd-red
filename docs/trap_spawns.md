# Trap Spawn Controls

Notes on how trap counts and trap composition are chosen per floor, and a plan to thread both knobs through our seeded dungeon overrides.

## Current Behavior
- **Per-floor properties load** – `SetFloorItemMonsterSpawns()` pulls a `FloorProperties` entry plus the trap table for the current floor from `mapparam` (`src/dungeon_floor_spawns.c:247-275`). `GeneralizeMazeDungeonLoc()` means maze variants share the same indices.
- **How many traps spawn** – During floor generation the engine scans eligible room tiles (no corridors/shops/items/junctions/unbreakables) and rolls `DungeonRandRange(trapDensity/2, trapDensity)` capped at 56, then marks that many tiles with `SPAWN_FLAG_TRAP` (`src/dungeon_generation.c:4420-4468`). Setting `trapDensity` to 0 disables normal trap placement; Monster House tiles still do a 50/50 item-or-trap roll gated by `gDungeon->unk644.unk18` (`src/dungeon_generation.c:4385-4410`).
- **Which trap appears** – When a trap entity is placed, `GetRandomFloorTrap()` draws a number 0–9999 and returns the first trap whose cumulative threshold in `gDungeon->trapSpawnChances[]` meets/exceeds it (`src/dungeon_floor_spawns.c:326-335`). The thresholds are cumulative (monotonic) with 10000 as the terminator; zero entries mean “never spawn.”
- **Data sources** – `mapparam` is assembled from:
  - `data/dungeon/*/main_data.inc` (packed `FloorProperties` per unique floor config) → `trapDensity` lives here.
  - `data/dungeon/traps_found.inc` → table of pointers into `trap_data.inc` with cumulative thresholds per trap set. Each floor’s `floor_id.json` row chooses a `Traps` index that selects one of these tables.
  - The JSON source for trap weights is `data/dungeon/trap_data.json` (weights sum to 10000 before conversion to cumulative thresholds).

## Control Options Today
- **Density** – Edit the `trapDensity` byte in the relevant `main_data.inc` entry for static tweaks. A value of 0 suppresses normal traps; higher numbers roughly increase counts but still roll between 50–100% of the value.
- **Composition** – Swap the `Traps` index in `data/dungeon/<Dungeon>/floor_id.json` to reuse another existing trap palette, or change/add entries in `data/dungeon/trap_data.json` and rebuild. Remember thresholds must stay cumulative and end at 10000 after conversion.

## Seeded Override Plan
Goal: allow per-dungeon/per-floor trap density and trap mix to depend on the personality seed, without mutating the baked data.

1) **Extend the override struct** – Add optional trap fields to `DungeonSeedFloorOverrides` (e.g., `s16 trapDensity` with `-1` meaning “use default” plus `bool hasTrapTable; u16 trapSpawnChances[NUM_TRAPS]` to hold a full cumulative table).
2) **Apply at load time** – In `ApplySeedOverridesToCurrentFloor()` (`src/dungeon_floor_spawns.c`), after we copy in the seed-derived override:
   - If `trapDensity >= 0`, overwrite `gDungeon->floorProperties.trapDensity`.
   - If `hasTrapTable`, memcpy into `gDungeon->trapSpawnChances` so `GetRandomFloorTrap()` uses the seeded mix.
3) **Generate deterministic trap knobs** – In `DungeonSeedOverrides_GenerateFloorConfig()`:
   - Seed an RNG with `DungeonSeedOverrides_GetDungeonRngSeed(seed, dungeonId, floorId ^ kTrapSalt)` so trap rolls stay stable across runs.
   - Start from the baked values (read `gDungeon->floorProperties.trapDensity`/`gDungeon->trapSpawnChances` just after mapparam load) and mutate them: e.g., scale density by difficulty band, clamp to 0–56, and pick from a small library of trap palettes or procedurally build one by weighting hazards vs utility.
   - Provide a helper to convert weight arrays to cumulative thresholds (ending at 10000) to reduce mistakes.
4) **Per-dungeon defaults** – For static, non-seeded edits, prefer adjusting `trap_data.json`/`floor_id.json` so mapparam remains the source of truth; the override path should only kick in when `DungeonSeedOverrides_IsEnabled()` is true.
5) **Validation** – After wiring in overrides, add a lightweight assert/log when entering a floor to confirm the cumulative table ends at 10000 and is monotonic, then `make` to ensure the ROM still builds.

These hooks line up with our existing runtime override entry points in `docs/runtime-dungeon-hooks.md`: all trap changes happen after mapparam load but before `GenerateFloor()` consumes `trapDensity` and `trapSpawnChances`.
