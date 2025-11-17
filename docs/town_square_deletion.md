# Memory Cleanup Plan

This file tracks everything we have tried so far to lower EWRAM usage and what remains on the roadmap.

## What’s Done

1. **Redirect Pokémon Square map loads** (`src/ground_map.c` & `src/map_script_table.c`).
   - Pointed `MAP_POKEMON_SQUARE` at the base exterior so `GroundBg_Init` no longer allocates the ~80 KiB Square renderer buffers.
2. **Shrank the main heap** (`src/memory.c`).
   - Dropped `HEAP_SIZE` from `0x24000` to `0x1A000`, but this caused boot-time `Bad memory store16` loops because Square scripts still spawned large NPC pools.
3. **Trimmed active Ground pools** (`ground_lives/objects/effects/events`).
   - First reduced the pool counts, discovered Square scripts overfilled them, then restored the sizes; later trimmed the pools again once Square actors were gone and added overflow logging for visibility.
4. **Restored pool sizes and heap**.
   - Reverted the drastic pool trims and raised `HEAP_SIZE` back to `0x24000` so the ROM booted reliably while we gathered better usage data.
5. **Stubbed Pokémon Square scripts and assets** (`src/data/ground/ground_data_t01p01_station.h`, `src/ground_map_conversion_table.c`, `src/world_map_sound.c`).
    - Replaced `gGroundScript_gs1` with an empty header, aliased `MAP_POKEMON_SQUARE` to the base conversion entry, and pointed world-map BGM at the base theme so Square assets are never requested.
6. **Reduced `HEAP_SIZE` cautiously** (`src/memory.c`).
    - After stabilizing the hub, dropped the heap to `0x22000` (still 0x2000 bytes leaner than stock) until we can log peak allocations.
7. **Skipped Square-only event chains** (`src/ground_map.c`, `src/ground_event.c`).
    - `GroundMap_ExecuteEvent` and `GroundEvent_Select` now refuse to run the Square-only script IDs (114/117/118). During map transitions we track whether the previous map was on the logo/intro allowlist so Square events stay muted while dungeon warps continue to execute the non-Square handlers (e.g., `EVENT_RESCUE`). `MAP_TITLE_SCREEN` is no longer exempt, so the demo scheduler never spins up once the title loads.
8. **Short-circuited Square/scene stations** (`src/ground_map.c`, `src/save.c`).
    - `GroundMap_ExecuteStation` now always returns for the logo/title scene maps, preventing those station scripts from ever queuing Square cutscenes during dungeon entry. This keeps the Square guard from crashing transitions without touching the normal boot flow.
9. **Hard-wired SkipCutscenes behavior** (`src/personality_test1.c`, `src/save.c`, `src/code_80A26CC.c`).
    - Removed the selectable SkipCutscenes flag entirely: the quiz no longer prompts for it, saves don’t persist it, and dungeon selection/rescue entry paths always use the cutscene-skipping branches. All references now assume the guard is permanently enabled, which simplified the Square filters but did **not** eliminate the Tiny Woods black screen.
10. **Short-circuited the title demo loop entirely** (`src/data/ground/ground_event_data.h`, `src/ground_map.c`, `src/main_loops.c`).
    - `s_script_DEMO_01`/`DEMO_03` now jump straight to the title station (`MAP_TITLE_SCREEN`, group 3) while `DEMO_02`/`DEMO_04` immediately halt, so the Pelipper/Square fly-through never plays. `UpdateSquareEventAllowance()` keeps Square events disabled for the title map so the demo scheduler never relaunches. `GroundMap_ExecuteEvent()` still sees the initial demo trigger, but it just flips the Square demo guard off so subsequent demo calls are ignored. The RunGameMode title loop now boots straight into the Continue/New Game menu after the logos.

## What’s Left

1. **Lower `HEAP_SIZE` again** once we capture the new peak usage numbers.
   - Instrument allocations or use MGBA to ensure enough headroom remains before settling on a smaller heap.
2. **Delete unused Square map files** (`src/map_files_table.c:12-18`, `src/ground_map_files_10.c:5-8`).
   - Removing the `T01P01*.bp?` assets (or hiding them behind a roguelike build flag) ensures `GroundBg_Init` can’t reintroduce the 80 KiB renderer and shrinks the ROM.
3. **Guard any remaining Square hooks** (scripts/pools) that still reference `GROUND_PLACE_SQUARE`.
   - A quick `rg` sweep still finds Square references in various scripts. Each one should either be deleted, redirected to the base hub, or wrapped in the same `ShouldSkipSquareEvent()` guard so Square transitions can’t reintroduce their assets.
