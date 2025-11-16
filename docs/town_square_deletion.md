# Memory Cleanup Plan

This file tracks everything we have tried so far to lower EWRAM usage and what remains on the roadmap.

## What We Tried

1. **Redirect Pokémon Square map loads** (`src/ground_map.c` & `src/map_script_table.c`).
   - Pointed `MAP_POKEMON_SQUARE` at the base exterior so `GroundBg_Init` no longer allocates the ~80 KiB Square renderer buffers.
2. **Shrank the main heap** (`src/memory.c`).
   - Dropped `HEAP_SIZE` from `0x24000` to `0x1A000`, but this caused boot-time `Bad memory store16` loops because Square scripts still spawned large NPC pools.
3. **Trimmed Ground pools** (`ground_lives/objects/effects/events`).
   - Reduced pool counts, but Square scripts still created more entries than the reduced pool size, leading to out-of-bounds writes and another boot loop.
4. **Restored pool sizes and heap**.
   - Reverted the pool trims and raised `HEAP_SIZE` back to `0x24000` so the ROM boots, even though EWRAM is ~99% used.
5. **Stubbed Pokémon Square scripts and assets** (`src/data/ground/ground_data_t01p01_station.h`, `src/ground_map_conversion_table.c`, `src/world_map_sound.c`).
    - Replaced `gGroundScript_gs1` with an empty header, aliased `MAP_POKEMON_SQUARE` to the base map conversion entry, and pointed world-map BGM requests at the base theme so Square data never loads.
6. **Trimmed active Ground pools** (`ground_lives.c`, `ground_object.c`, `ground_effect.c`, `ground_event.c`).
    - Reduced the live/object/effect/event pool sizes to the handful of actors the base hub still uses and added overflow logs so we can detect if something important fails to spawn.

## Next Steps

1. **Lower `HEAP_SIZE` again** after recording the new peak allocation usage.
   - Log allocations or use MGBA debugging to confirm headroom before finalizing a smaller heap size.
