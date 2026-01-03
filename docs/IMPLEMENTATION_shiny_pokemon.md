# Shiny Pokemon Implementation Plan

Goal: every monster has a 1% chance to spawn with an alternate palette, and that shiny state persists for recruits, evolutions, and ground/friend area visuals.

## Current Temporary Settings
- Shiny chance is forced to 100%.
- Shiny palettes use the assigned palette row directly (no cache). Slots 13-15 are not used for shinies.

## Data + Flags
- **Shiny flag:** add `POKEMON_FLAG_SHINY` (unused bit, e.g. `1 << 2`) in `include/pokemon.h` and treat it as persistent state for both `Pokemon` and `DungeonMon` (`include/structs/str_pokemon.h` already shares the flags field).
- **Palette mapping:** `rogue_files/shiny_palette.csv` lists `id,name,shiny_palette_id` for every monster. `rogue_files/shiny_processing/00_gen_shiny_palette_table.py` generates `src/data/shiny_palette_table.c` with `gMonsterShinyPalette[MONSTER_MAX]` and `GetMonsterShinyPalette()`.
- **Index remap:** `rogue_files/shiny_palette.csv` also includes `index0_remap` ... `index15_remap` for optional 4bpp index remapping. Empty/`null` keeps the original index. Remaps are applied simultaneously at runtime for shiny sprites via `GetMonsterShinyIndexRemap()`.
- **Palette assets:** add a second palette bank (e.g., `graphics/ax/pal_shiny/*.pmdpal` + `gAxMonsPaletteShiny` in `src/monster_sbin_palet.c`) and a new entry in `src/monster_files_table.c` (e.g., `"palet_shiny"`). This mirrors the existing 13 base palettes.

## Spawn + Persistence
- **Dungeon spawn chance:** in `src/dungeon_mon_spawn.c` (near `InitEntityFromSpawnInfo`), set `entInfo->isShiny` (new bool field) using `DungeonRandInt(100) == 0` for wild spawns. Keep it deterministic by using dungeon RNG.
- **Team members in dungeon:** when converting recruited mons (`PokemonToDungeonMon` in `src/pokemon.c` + `SetDungeonMonsFromTeam` in `src/dungeon_misc.c`), set `entInfo->isShiny` from the `Pokemon` flag so teammates keep their shiny state.
- **Recruiting from dungeon:** extend `struct unkStruct_8069D4C` in `include/dungeon_misc.h` with `bool8 isShiny`, populate it in `sub_8069D4C` (`src/dungeon_misc.c`), and carry it into `dungeonMon->flags` in `src/dungeon_mon_recruit.c`.
- **Evolution:** preserve the shiny flag in `sub_808F798` (`src/pokemon_evolution.c`). It currently clears `pokemon->flags`; capture the shiny bit before clearing and reapply it to `pokeStruct.flags` before `TryAddPokemonToRecruited`.
- **Serialization:** store shiny flags in the slack area of the recruited-Pokemon save block (`src/pokemon_3.c`), leaving the existing bit count unchanged. On load, restore `POKEMON_FLAG_SHINY` from the saved bitset; if no header is present (old saves), roll for leader/partner once using the same shiny percent.

## Rendering + Palette Selection
- **Palette chooser helper:** add `GetPokemonOverworldPaletteEx(species, isShiny)` in `src/pokemon.c` (or a new `src/pokemon_shiny.c`) that returns the palette index and whether to use the shiny palette bank. It should:
  - prefer `gMonsterShinyPalette[species]` when `isShiny` is true,
  - fall back to `sMonsterParameters[species].overworldPalette` otherwise.
- **Dungeon render hook:** in `src/dungeon_mon_sprite_render.c`, swap the palette lookup to use `entInfo->isShiny`.
- **Ground sprites:** in `src/ground_lives.c`, when a `GroundLive` represents a `Pokemon`, record `isShiny` (new field on `GroundLive` or a flag bit). Use that in `sub_80A7428` (`src/ground_sprite_monster.c`) to pick shiny palettes.
- **Friend Areas map:** update `FriendAreasMap_InitGfx` in `src/friend_areas_map_util.c` to use the player `Pokemon` shiny flag.

## Palette Slot Strategy (Hardware Constraints)
- The OBJ palette has 16 slots; base monster palettes already use 13 (0-12). Slots 13-15 are used by other sprite effects, so shinies must stay within 0-12.

## Content Workflow
- Start by editing `rogue_files/shiny_palette.csv` and rebuilding so `src/data/shiny_palette_table.c` updates. The current default is `0` for every species.
- If unique shiny colors per species are required later, expand `gMonsterShinyPalette` and `palet_shiny` to include more variants, then update the cache size (still bounded by OBJ palette limits).

## QA Notes
- Add a temporary debug toggle (e.g., L+R+SELECT) to force shiny for all spawns for palette validation; remove or gate before release.
- Verify: dungeon wild spawns, recruit flow, evolution in `src/luminous_cave.c`, friend area ground NPCs, and team member visuals in dungeon.

## Palette Preview Helper
- `python3 rogue_files/shiny_processing/00_gen_shiny_palette_table.py` runs the full pipeline: generate the shiny palette table, render idle frames, and build palette grids. Use `--mode table|frames|grid` plus the `--frames-*`/`--grid-*` flags to run only a subset.
- `python3 rogue_files/shiny_processing/02_shiny_palette_webapp.py` launches the local palette picker webapp for experimenting with overrides.
