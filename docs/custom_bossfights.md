# Custom Boss Fights (Regi‑Style)

This guide shows how to implement custom boss fights by following the “Regi‑style” pattern used for Regirock/Regice/Registeel in Buried Relic. Regi‑style fights are ideal when you want:

- “Kill the boss, then finish the floor.”
- Optional minions on the same room/floor.
- A stairs spawn (and optional loot drop) when the boss faints, letting the player choose to rush the stairs or finish off remaining enemies.

It reuses existing hooks, is mostly data‑driven, and avoids instantly ending the floor.

## Why Regi‑Style

- Clean KO hook: A central faint dispatcher calls your per‑fight handler on KO (see `sub_8084E00` in src/dungeon_cutscene.c:541). You can handle post‑fight actions without forcing the floor to end.
- Data‑driven exits/loot: One helper (`sub_808B1CC`) spawns stairs at an anchor and optionally drops an item (src/dungeon_cutscene_regis.c:299).
- Natural pacing: The fight ends by spawning stairs; the player can clear adds or ignore them and leave, matching “kill boss then finish.”

Contrast with “normal” boss floors (Skarmory/Zapdos): many of those call `gDungeon->unk2 = 1` upon KO, which immediately ends the floor (see e.g., src/dungeon_cutscene_zapdos.c:102; `IsFloorOver()` at src/dungeon_engine.c:478).

## Flow Overview

- Fixed room anchor: Place a special tile action at your boss room to record an anchor position used for stairs/loot (fixedmap action `69`) (src/dungeon_generation_fixed.c:857).
- Pre‑fight setup: Choose the scenario ID for your fight; pre‑fight code can set boss HP/music via `SetupBossFightHP` (src/dungeon_cutscene.c:1976) and optionally spawn minions.
- Boss KO handler: On boss faint, the dispatcher routes to your handler which calls `sub_808B1CC(itemId)` to mark stairs at the anchor and optionally drop loot (src/dungeon_cutscene_regis.c:299). The floor remains active; the player can take stairs anytime.

## Step‑By‑Step

1) Pick a floor and enable a fixed room
- Set the floor’s `fixedRoomNumber` (aka `FloorProperties.unk12`) to a non‑zero fixed room ID (< 50 for full‑floor fixed) in your dungeon’s `mapparam` tables. See docs/dungeons.md and the per‑dungeon `floor_id.json` (e.g., data/dungeon/UproarForest/floor_id.json:1) and where floor properties are loaded (src/dungeon_floor_spawns.c:48).

2) Author the fixed boss room
- In your `fixedmap` data (data/dungeon/fixedmap.inc), place:
  - Boss spawn tiles using action IDs that map to entries in `sFixedRoomEntities` (see below).
  - Minion spawns (same mechanism as boss) if you want minions to be present from the start.
  - One anchor tile `69` to record the stairs/loot anchor (`gDungeon->unk644.unk40/unk42`) (src/dungeon_generation_fixed.c:857).

- Entity placement is handled by `PlaceFixedRoomTile` for action IDs 16–219 (src/dungeon_generation_fixed.c:932). Those IDs index `sFixedRoomEntities`, which specify `speciesId`, `monsterBehavior`, and optional `.unk4` to record positions into `gDungeon->unkE220[]` for cutscene usage (src/dungeon_generation_fixed.c:946; src/dungeon_cutscene.c:881).

3) Map the floor to a fight scenario ID
- The engine chooses a “fight scenario” (`gDungeon->unk3A0D`) based on the fixed room number via a small table (src/dungeon_cutscene.c:120). Add/repurpose an entry to map your fixed room to a new scenario ID.
- Add your scenario ID to:
  - `DisplayPreFightDialogue()` switch (to run pre‑fight setup) (src/dungeon_cutscene.c:344).
  - The faint dispatcher `sub_8084E00()` switch (to run your KO handler) (src/dungeon_cutscene.c:541).

Tip — scenario gating flags:
- The mapping table (`gUnknown_8107234`) has 6 fields per row: `{unk0, unk1, unk2, unk3, unk4, unk5}`. `unk0` is the fixed room number; the other three are alternative scenario IDs selected by unlock bits (queried via `sub_8098100`). The selector roughly does:
  - If story/refight flag `unk4` is set → use `unk5`.
  - Else if flag `unk2` is set → use `unk3`.
  - Else use `unk1` and, if `unk2 != 0x40`, set that flag.
- For a simple one‑scenario fight, set only `unk1 = <yourScenarioId>` and set both `unk2` and `unk4` to `0x40` to bypass gating.
- Add your pre/KO function prototypes to `include/dungeon_cutscenes.h` so they can be referenced from `src/dungeon_cutscene.c`.

4) Pre‑fight setup (HP/music; optional code‑spawned minions)
- Implement a `PreFightDialogue` function similar to the Regi ones (src/dungeon_cutscene_regis.c:251/263/275). Typical steps:
  - Face boss north if needed (`SetupRegiFacingDirection` pattern).
  - Call `SetupBossFightHP(bossEntity, newHP, MUS_BOSS_BATTLE)` to set HP and music (src/dungeon_cutscene_regis.c:324).
  - If you prefer code‑spawned minions, spawn them here via `SpawnWildMon` and positions saved in `gDungeon->unkE220[]` (src/dungeon_cutscene.c:881), or at fixed coordinates.

5) KO handler (spawn stairs + optional loot)
- Implement a handler like the Regi trio’s `sub_808AE54`/`sub_808AEC8`/`sub_808AF3C` (src/dungeon_cutscene_regis.c:120,153,175):
  - Verify you’re in your scenario and the fainting entity is your boss (by `monsterBehavior` or species).
  - Call `sub_808B1CC(itemId)` to:
    - Mark the tile at the anchor as stairs.
    - Optionally drop `itemId` one tile above the anchor (if the team doesn’t already have it) (src/dungeon_cutscene_regis.c:299).
  - Optionally show a “something fell…” post‑fight dialogue (see Regi post‑fight messages nearby).

- Note: `sub_808B1CC` only sets `gDungeon->unk2 = 1` (ending the floor) if no leader is present (rare fallback). Otherwise the floor stays open until the player takes the stairs.

Optional — delay stairs until all minions are defeated:
- On boss KO, before calling `sub_808B1CC`, scan `gDungeon->wildPokemon` for remaining minions (match by `monsterBehavior` or species). If any remain, return; otherwise, proceed to spawn stairs and loot. See the Mankey pattern in `sub_8089788` (src/dungeon_cutscene_mankey.c:70).

## Minion Spawns

You can mix data‑driven and code‑driven minions:

- Data‑driven (recommended):
  - Add or reuse `sFixedRoomEntities` entries (src/dungeon_generation_fixed.c:932). Set `speciesId` and `monsterBehavior` (e.g., `BEHAVIOR_FIXED_ENEMY`) so AI runs (src/dungeon_ai.c:24). Place the corresponding action IDs into your `fixedmap` layout.
  - If you want specific cutscene placements later, set the `.unk4` field for those entries and place them in `fixedmap`; this writes positions into `gDungeon->unkE220[]` (src/dungeon_generation_fixed.c:946), which can be read in cutscenes (src/dungeon_cutscene.c:881).

- Code‑driven (optional):
  - In your pre‑fight function, build a `MonSpawnInfo` and call `SpawnWildMon(&spawn, TRUE)` for each minion. Use `GetSpawnedMonsterLevel(species)` for an appropriate level (src/dungeon_floor_spawns.c:204).

Minions do not affect the KO handler—only the boss faint triggers the stairs/loot logic—so your “kill boss then finish; clear adds if you like” flow works out of the box.

## Custom Loot Drops

- Pass your item to `sub_808B1CC(itemId)` in the KO handler. It drops the item if the team doesn’t already have it and doesn’t already hold the Music Box (see the Regi checks), then marks stairs at the anchor (src/dungeon_cutscene_regis.c:299).
- For “no loot,” pass `ITEM_NOTHING`.

## Boss/Minion Behaviors

- Boss: Use a dedicated `monsterBehavior` (like the Regi trio do) if you want your KO handler to easily match the fainting entity in `sub_8084E00`. Behaviors are defined in include/structs/dungeon_entity.h:422 and checked via `GetEntInfo(entity)->monsterBehavior`.
- Minions: `BEHAVIOR_FIXED_ENEMY` is a good default (AI runs) (src/dungeon_ai.c:24). You can assign other behaviors if you need special cutscene hooks.

## Finish Conditions

- Regi‑style (recommended): Boss KO spawns stairs (and loot). Floor does not auto‑end; the player can either clear adds or rush the stairs. This matches “kill boss then finish.”
- Mankey‑style (optional addon): If you want to delay stairs until all minions are defeated, copy the scan pattern from `sub_8089788` (src/dungeon_cutscene_mankey.c:70): on boss KO, scan `gDungeon->wildPokemon` for remaining minion behavior/species; if none, then call `sub_808B1CC(itemId)` or set `gDungeon->unk2 = 1` to end immediately.

## Checklist

- Fixed floor configured (mapparam → `fixedRoomNumber`) for your target floor.
- `fixedmap` room created with:
  - One action `69` tile to set the stairs/loot anchor (src/dungeon_generation_fixed.c:857).
  - Boss/minion entity action tiles mapped via `sFixedRoomEntities` (src/dungeon_generation_fixed.c:932).
- Scenario ID mapped to your fixed room in `src/dungeon_cutscene.c:120` and added to:
  - `DisplayPreFightDialogue()` (src/dungeon_cutscene.c:344).
  - `sub_8084E00()` faint dispatcher (src/dungeon_cutscene.c:541).
- Pre‑fight function calls `SetupBossFightHP` for boss music/HP (src/dungeon_cutscene.c:1976).
- KO handler calls `sub_808B1CC(itemId)` to spawn stairs and optional loot (src/dungeon_cutscene_regis.c:299).

## References

- Fixed room generation: src/dungeon_generation_fixed.c:857, src/dungeon_generation_fixed.c:932
- Fight scenario mapping and hooks: src/dungeon_cutscene.c:120, src/dungeon_cutscene.c:344, src/dungeon_cutscene.c:541
- Regi KO/loot helpers: src/dungeon_cutscene_regis.c:120, src/dungeon_cutscene_regis.c:153, src/dungeon_cutscene_regis.c:175, src/dungeon_cutscene_regis.c:299
- Boss HP/music: src/dungeon_cutscene.c:1976; examples in src/dungeon_cutscene_regis.c:324
- Behaviors: include/structs/dungeon_entity.h:422; AI: src/dungeon_ai.c:24
- Floor end condition: src/dungeon_engine.c:478

## Testing Checklist

- Enter the floor: `DisplayPreFightDialogue()` triggers, boss HP/music set, minions present.
- KO the boss: stairs appear at the anchor; custom loot drops if configured.
- With minions still alive: the floor remains active; you can take the stairs or clear the room.
- With “delay stairs” option: stairs only appear after all minions are defeated.
- Ensure the anchor tile (action `69`) exists; otherwise `sub_808B1CC` falls back to leader position.
