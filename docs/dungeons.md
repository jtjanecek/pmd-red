# Dungeons: Names, Indices, and Tilesets

This repo uses a single, global dungeon index to key most dungeon-related tables (names, rules, spawns, etc.). This guide explains how to change a dungeon’s display name, and how dungeon indices and tilesets work under the hood.

## Dungeon Indices

- Global enum: Dungeon IDs are defined once in `include/constants/dungeon.h:1` under `enum DungeonID`. Example entries:
  - `DUNGEON_TINY_WOODS = 0`, `DUNGEON_THUNDERWAVE_CAVE = 1`, … up to `NUM_DUNGEONS`.
  - Some IDs are marked duplicate/special (e.g., `DUNGEON_MT_FREEZE_PEAK_2`, join locations, Pokémon Square). These still consume indices and are part of the global table keyspace.
- Where used: The dungeon index (an `u8`) is used across the codebase to index many arrays/tables:
  - Names: `gDungeonNames[NUM_DUNGEONS]` in `src/strings.c:676`
  - Rules and flags: `gDungeons[]` (type `DungeonDataEntry`) queried via helpers in `src/dungeon_info.c:2386`
  - Floor counts, starting floor, difficulty, etc.: helpers in `src/dungeon_info.c`
  - UI formatting: `PrintYellowDungeonNametoBuffer`, `PrintDungeonLocationtoBuffer` in `src/dungeon_info.c:2498`
- Display vs. internal ID: There is no separate “display ID.” UI uses the same global ID on the current `DungeonLocation.id` to fetch the display name. For debug, a numeric ID is logged on entry by `RunDungeon_Async` via `MGBA_Warnf` at `src/run_dungeon.c:126`.
- Mazes: Maze variants get “generalized” for data lookups (e.g., map parameters) by `GeneralizeMazeDungeonLoc` in `src/dungeon_info.c:2486`. This only rewrites the lookup target for shared data and does not change what name is displayed.
- Caution: Do not reorder or remove IDs in `enum DungeonID` unless you are ready to update every table keyed by dungeon ID. Changing names does not require changing indices.

## Change a Dungeon’s Display Name

Dungeon names are table-driven in one place.

- Table: `gDungeonNames[NUM_DUNGEONS]` at `src/strings.c:676`
- Struct: `DungeonName { const u8 *name1; const u8 *name2; }` in `include/strings.h:15`
- Accessors: `GetDungeonName1/2()` in `src/dungeon_info.c:2386`
- UI usage: `PrintYellowDungeonNametoBuffer`, `CopyDungeonName1toBuffer` in `src/dungeon_info.c:2498`

Steps:
1) Open `src/strings.c:676` and locate the `gDungeonNames` initializer.
2) Find the entry indexed by your dungeon’s constant (e.g., `[DUNGEON_TINY_WOODS] = { _("Tiny Woods"), _("Tiny Woods") },`).
3) Edit the string(s). Typically `name1` and `name2` are the same; if you are unsure, keep them identical.
4) Keep within reasonable length; these names are used in banners and UI strings.
5) Build and run. The new name appears wherever the dungeon name is shown.

Notes:
- The `_()` macro wraps localized strings; you can keep using it.
- Special join-location IDs (e.g., `DUNGEON_JOIN_LOCATION_LEADER`) are also present in this table; you can adjust their display names the same way.

## How Tilesets Are Chosen

Tilesets are selected per-floor via the floor’s map parameters, not from the name table.

- Per-floor properties: The engine loads `FloorProperties` for the current floor from the `mapparam` resource. See `src/dungeon_floor_spawns.c:48` and `include/structs/dungeon_mapparam.h:15`.
  - `gDungeon->floorProperties` is assigned from `mapparam`.
  - `gDungeon->tileset = gDungeon->floorProperties.unk2;` in `src/run_dungeon.c:320`.
- Asset loading: `LoadDungeonTilesetAssets()` uses `gDungeon->tileset` to load the `b%02d*` files in `src/dungeon_map_access.c:79`.
  - Some assets use a remap table `gUnknown_8108EC0[]` (`src/dungeon_info.c:2377`) to select the correct `fon/cel/cex` files for a tileset.
- Water/Lava behavior: The tileset ID indexes `gDungeonWaterType[]` (`src/dungeon_data.c:520`) to control whether the secondary terrain behaves as water or lava; `IsWaterTileset()` adds special cases in `src/dungeon_map_access.c:1212`.

Changing a floor’s tileset:
- Edit the floor’s `FloorProperties` in the `mapparam` data so that `unk2` (tileset) is the desired tileset ID. The engine then assigns `gDungeon->tileset` from that field and loads the corresponding assets.
- If you use a tileset with different water/lava behavior, ensure `gDungeonWaterType` has the right entry for that tileset ID; otherwise terrain rules may be incorrect.

## Finding a Dungeon’s Index

- Look up the constant in `include/constants/dungeon.h:1` (e.g., `DUNGEON_MT_THUNDER_PEAK`). The value assigned in the enum is its index.
- In runtime code, the current dungeon is usually in `DungeonLocation` (`include/structs/str_dungeon_location.h:1`) as `id` and `floor`.
- The entry banner and many UI texts use `DungeonLocation.id` directly to index `gDungeonNames` (see `src/dungeon_info.c:2498`).

## Quick References

- Names table: `src/strings.c:676`
- Name struct: `include/strings.h:15`
- Dungeon IDs: `include/constants/dungeon.h:1`
- Print helpers: `src/dungeon_info.c:2498`
- Debug log of ID on entry: `src/run_dungeon.c:126`
- Floor properties load: `src/dungeon_floor_spawns.c:48`
- Tileset assignment: `src/run_dungeon.c:320`
- Tileset assets loader: `src/dungeon_map_access.c:79`
- Tileset remap for assets: `src/dungeon_info.c:2377`
- Water/lava type by tileset: `src/dungeon_data.c:520`
- Water tileset helper: `src/dungeon_map_access.c:1212`

## Pitfalls and Tips

- Don’t reorder `DungeonID` unless you update every table keyed by dungeon index.
- Changing names is safe: it only affects display text.
- Tileset changes are per-floor via `mapparam`/`FloorProperties`; they are not tied to the dungeon’s display name.
- Maze dungeons share data via `GeneralizeMazeDungeonLoc`; display names still come from the original ID.
