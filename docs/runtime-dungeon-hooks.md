# Runtime Dungeon Hooks

This guide explains how to intercept the existing dungeon systems at runtime so we can randomize dungeon names, enemy spawns, floor counts, and tilesets from the custom seed that players enter during the personality quiz. Each section below points to the concrete functions we need to hook along the call chain and documents what state is available when they run.

## 1. Accessing the Personality Quiz Seed

- **Where the seed is stored:** `TeamBasicInfo.customSeed` inside `PersonalityTestTracker` (`include/personality_test1.h:23`). When the quiz ends, `sub_8011C40()` copies the chosen seed into `gUnknown_202DE28` (`src/save.c:73`).
- **Global accessor:** `sub_8011C34()` returns the last persisted seed (`src/save.c:73`). This value is also written into saves and restored on load (`src/save.c:371`, `src/save.c:482`).
- **Recommended helper:** Wrap `sub_8011C34()` in a small inline such as `s32 GetRuntimePersonalitySeed(void)` so dungeon code doesn’t need to include `save.h`.
- **RNG entry point:** During dungeon setup, `RunDungeon_Async` seeds the dungeon RNG with `gDungeon->unk644.unk3C` (`src/run_dungeon.c:259`). We can replace the call to `YetAnotherRandom24()` with a deterministic value derived from `GetRuntimePersonalitySeed()` plus the dungeon/floor index to make per-floor randomization deterministic.

## 2. Hooking Dungeon Names

1. **Data source:** `gDungeonNames[NUM_DUNGEONS]` in `src/strings.c`.
2. **Accessors:** Every UI path calls `GetDungeonName1()` / `GetDungeonName2()` (`src/dungeon_info.c:2386`) before drawing banners, menus, or logs. `ShowDungeonNameBanner_Async()` (`src/dungeon_name_banner.c:47`) only touches the accessor, so overriding it is sufficient.
3. **Hook plan:**
   - Replace `GetDungeonName1/2` with wrappers that:
     1. Look up the player’s seed via `GetRuntimePersonalitySeed()`.
     2. Hash `(seed, dungeonId)` into a deterministic pseudo-random index.
     3. Return a pointer to either the vanilla entry or a seed-generated buffer (e.g., from a per-run cache).
   - Because callers expect static `u8*`, cache the generated strings in a static array indexed by dungeon ID to avoid lifetime issues.
   - `PrintDungeonLocationtoBuffer()` and `CopyDungeonName1toBuffer()` (`src/dungeon_info.c:2498`) automatically pick up the new names, so no additional hooks are required.

**Key functions to intercept**

| Target | Why hook it |
| --- | --- |
| `GetDungeonName1/2` (`src/dungeon_info.c:2386`) | Single choke point for every rendered dungeon name. |
| `ShowDungeonNameBanner_Async` (`src/dungeon_name_banner.c:47`) | Optional logging/debug hook for verifying overrides mid-dungeon. |

## 3. Hooking Enemy Spawn Tables

Enemy spawns are driven by the `mapparam` resource and cached per floor. We need to intervene right after the game loads that data, but before it copies anything into the live spawn arrays.

1. **Load path:** `SetFloorItemMonsterSpawns()` (`src/dungeon_floor_spawns.c:30`) runs once per floor before generation. It hydrates:
   - `gDungeon->floorProperties` for layout/tileset.
   - `gDungeon->fileMonsterSpawns[]` from the `monsterSpawns` table.
   - `gDungeon->trapSpawnChances[]` and `gDungeon->itemSpawns[]`.
2. **Runtime array:** `SetCurrentMonsterSpawns()` (`src/dungeon_floor_spawns.c:162`) copies `fileMonsterSpawns` into `gDungeon->monsterSpawns`, and `GetRandomFloorMonsterId()` (`src/dungeon_floor_spawns.c:185`) picks entries during `SpawnWildMonsOnFloor()` (`src/dungeon_mon_spawn.c:440`).
3. **Hook plan:**
   - Add `void RandomizeFloorSpawns(void)` and call it at the end of `SetFloorItemMonsterSpawns()` (after `fileMonsterSpawns` is filled, before `CloseFile()`).
   - Inside the hook, iterate over `gDungeon->fileMonsterSpawns` and rewrite each `SpawnPokemonData` entry based on `(seed, dungeonId, floor)`. Utilities like `ExtractSpeciesIndex()` and `SetSpeciesToExtract()` are already available in `dungeon_floor_spawns.c`.
   - Recompute `randNum[]` weights when you shuffle species so `GetRandomFloorMonsterId()` keeps working without modification.
   - If you need different behavior for scripted floors (boss fights, cutscene dungeons), gate the hook by inspecting `gDungeon->floorProperties.fixedRoomNumber` (`src/run_dungeon.c:312`) or `gDungeon->unk644.canRecruit`.

**Key functions to intercept**

| Target | Why hook it |
| --- | --- |
| `SetFloorItemMonsterSpawns` (`src/dungeon_floor_spawns.c:30`) | Guaranteed to run before any spawn data is consumed; ideal place to rewrite `fileMonsterSpawns`. |
| `SetMonsterSpawnsArray` (`src/dungeon_floor_spawns.c:131`) | Secondary hook if we need to change how the runtime array is populated (e.g., inject more than 31 entries). |
| `GetRandomFloorMonsterId` (`src/dungeon_floor_spawns.c:185`) | Optional hook to add seed-based rarity logic without rewriting the table. |

## 4. Hooking Floor Counts and Tilesets

The UI and dungeon generator query floor counts and tilesets in predictable spots:

1. **Floor counts / start floors:** `SetFloorItemMonsterSpawns()` sets `gDungeon->unk1CEC8 = GetDungeonFloorCount(...)` and `gDungeon->startFloorId = GetDungeonStartingFloor(...)` (`src/dungeon_floor_spawns.c:45`). Overriding the two helpers in `src/dungeon_info.c:2518` lets us report dynamic values anywhere else in the game (job board, map UI, wind timers, etc.).
2. **Tileset + layout:** `gDungeon->floorProperties` is pulled from `mapparam` (`src/dungeon_floor_spawns.c:48`). Immediately after, `RunDungeon_Async` copies `tileset`, `bgMusic`, and `fixedRoomNumber` into live state (`src/run_dungeon.c:312`). The tileset then drives every asset load (`LoadDungeonTilesetAssets()`, `src/dungeon_map_access.c:79`).
3. **Hook plan:**
   - Override `GetDungeonFloorCount()` / `GetDungeonStartingFloor()` to consult a seed-driven table. Keep maze handling (`DUNGEON_IS_MAZE`) intact unless you also randomize mazes.
   - After `gDungeon->floorProperties` is assigned, call a new helper such as `ApplyRuntimeFloorOverrides(&gDungeon->floorProperties, seed, dungeonId, floor)`. Update:
     - `tileset`, `layout`, `enemyDensity`, etc. to match the randomized selection.
     - `gDungeon->unk644.enemyDensity` or other cached copies if you change fields they already mirrored.
   - If you change the tileset, also pick a matching `bgMusic`/`weather` combination to avoid mismatched ambience.

**Key functions to intercept**

| Target | Why hook it |
| --- | --- |
| `GetDungeonFloorCount` / `GetDungeonStartingFloor` (`src/dungeon_info.c:2518`) | Single-source helpers for every system that needs floor metadata. |
| `SetFloorItemMonsterSpawns` (`src/dungeon_floor_spawns.c:48`) | First point where `FloorProperties` is available and mutable before any generator code runs. |
| `LoadDungeonTilesetAssets` call site (`src/run_dungeon.c:351`) | Use for debugging to confirm the chosen tileset once overrides are applied. |

## 5. Putting It Together

1. **Seed context:** Build a small struct (e.g., `RuntimeDungeonSeedContext`) that stores `seed`, `dungeonId`, `floor`, and optionally a deterministic RNG. Initialize it inside `SetFloorItemMonsterSpawns()` right after `gDungeon->unk644.dungeonLocation` is finalized.
2. **RNG seeding:** Replace `gDungeon->unk644.unk3C = YetAnotherRandom24()` with `MixSeed(seed, dungeonId, floor)` so `DungeonRandInt()` stays in sync with the randomized content.
3. **Apply overrides:**
   - `RandomizeFloorSpawns(ctx);`
   - `ApplyRuntimeFloorOverrides(&gDungeon->floorProperties, ctx);`
   - `OverrideDungeonNameCache(ctx);` (called from within the custom `GetDungeonName1/2` wrappers).
4. **Testing hooks:** Use the existing banner (`ShowDungeonNameBanner_Async`) and `MGBA_Printf` statements near `RunDungeon_Async` to log the generated values while iterating.

## 6. Quick Reference

| Area | Function(s) | File |
| --- | --- | --- |
| Seed storage | `sub_8011C34`, `sub_8011C40` | `src/save.c` |
| Name lookups | `GetDungeonName1/2`, `PrintDungeonLocationtoBuffer` | `src/dungeon_info.c` |
| Banner render | `ShowDungeonNameBanner_Async` | `src/dungeon_name_banner.c` |
| Spawn data load | `SetFloorItemMonsterSpawns`, `SetMonsterSpawnsArray`, `GetRandomFloorMonsterId` | `src/dungeon_floor_spawns.c` |
| Wild spawn loop | `SpawnWildMonsOnFloor` | `src/dungeon_mon_spawn.c` |
| Floor metadata | `GetDungeonFloorCount`, `GetDungeonStartingFloor` | `src/dungeon_info.c` |
| Tileset assignment | `RunDungeon_Async` (tileset/bg music copy) | `src/run_dungeon.c` |
| RNG seeding | `InitDungeonRNG`, `YetAnotherRandom24` | `src/dungeon_random.c`, `src/run_dungeon.c` |

With these hook points documented, we can build custom classes/helpers that derive per-floor results from the personality quiz seed and confidently inject them at runtime without having to edit the underlying data tables.
