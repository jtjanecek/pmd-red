# Project Overview

## Run logs
The most recent logs (all the way from boot) with are in `execution.log` in the root of the directory. Read this to help debug.

## Building the ROM
After any code changes, build with the command `make` to ensure it compiles.

# Rogue Rescue Team
Information about the project in general.

## Vision
- Build a roguelike take on **Pokémon Mystery Dungeon: Red Rescue Team** that boots straight into repeatable dungeon runs.
- Remove every blocking story sequence, hub cutscene, and Town Square dependency so the run loop is: quiz → seed selection → dungeon grind.
- Drive all dungeon randomization from the player-provided personality quiz seed so two runs with the same seed produce the same dungeon order, floor layouts, spawns, and loot pacing.

## Player Setup & Options
- **Seed-first flow** – `src/personality_test1.c` now opens with the seed picker (`docs/runtime-dungeon-hooks.md` describes what the seed influences). Players can keep the vanilla story seed, roll a random seed, or enter a numeric seed before anything else.
- **Skip the script** – Skip Cutscenes defaults to ON (`src/save.c:41-110`). When enabled, the quiz ends by calling `ApplySkipPostgameBootstrap()` (`src/personality_test1.c:671-743`) which materializes the team, flags all story scenarios as “postgame”, seeds mail, and spawns the player directly inside the Team Base. All transitional cutscene stations are ignored via the guards in `src/ground_map.c:600-670`.
- **Skip the “basic rescues” wall** – `ThoroughlyResetScriptVars()` in `src/event_flag.c:1-80` bumps `CLEAR_COUNT` when the Skip Basic Rescues toggle is ON, so the campaign never forces filler jobs between story checks.
- **Recruiting presets** – The Recruit All prompt (Normal / All Recruitable / No Recruitable) persists through `SetRecruitAllSetting()` (`src/personality_test1.c:429-648`, `src/dungeon_mon_recruit.c:1-110`). Setting it to “All” removes evolution-stage restrictions; “No” hard-disables in-dungeon recruits for pure roguelike runs. Hold L+R+SELECT when picking “All Recruitable” to stash the hidden AutoRecruitAll debug preset (guaranteed asks while under the 5-mon cap).
- **Difficulty picker** – Normal / Hard / Nightmare choices (`src/data/personality_test1.h:33-56`) write into `SetGameDifficultySetting()` and show up on the load screen (`src/load_screen.c:300-338`). Hooks that scale spawns and boss density can use `GetGameDifficultySetting()`.
- **Full hero + partner choice** – We deleted the alignment quiz gate. `CreateStarterSelectionMenu()`/`CreatePartnerSelectionMenu()` in `src/personality_test2.c` expose the entire `gPartners` roster (`src/data/personality_test2.h`) so the player can pick any Pokémon as the main character or partner and nickname them immediately.
- **Friend Areas unlocked by default** – Starting a file calls `InitializeFriendAreas()` (`src/save.c:520-578`, `src/friend_area.c:19-37`) which simply sets every Friend Area flag to TRUE. Shops and recruitment logic can assume every sanctuary is already open.

## Seeded Dungeon Randomizer
- **Single source of truth** – The chosen seed lives in `TeamBasicInfo.customSeed` and is mirrored via `sub_8011C34()`/`sub_8011C40()` (`src/save.c:48-82`). Every runtime hook should grab it through these helpers.
- **Randomizer core** – `src/dungeon_seed_overrides.c` is the runtime that mixes `(seed, dungeonId, floor)` into deterministic RNG streams. It exposes:
  - `DungeonSeedOverrides_GenerateFloorConfig()` → per-floor tileset + spawn table mutator.
  - `DungeonSeedOverrides_GetFloorCount()` → per-dungeon floor count / starting floor overrides.
  - `DungeonSeedOverrides_GetDungeonName()` → prefix/suffix mashups cached per dungeon.
  - `DungeonSeedOverrides_GetDungeonRngSeed()` → deterministic input for the dungeon RNG.
- **Hook-in points** – `docs/runtime-dungeon-hooks.md` lists the precise functions on the vanilla call graph where we intercept names (`GetDungeonName1/2`), tilesets (`SetFloorItemMonsterSpawns` → `ApplyRuntimeFloorOverrides`), enemy spawns (`RandomizeFloorSpawns`), and RNG seeding (`RunDungeon_Async`). Any new runtime shuffle should thread through these checkpoints so it stays seed-stable.
- **Mini-boss loot & pacing** – `docs/custom_bossfights.md` documents the “Regi-style” pattern: fixed-room anchors record KO hooks that can spawn stairs and deterministic loot (`sub_808B1CC`). Combine it with the seed RNG helpers to randomize mini-boss drops in a reproducible way.

## Dungeon Content & Encounters
- **Boss/interrupt framework** – Use the Regi template for fights that should leave the floor active, and follow `docs/dungeon_interrupts.md` to inject floor-based cut-ins (Frosty Forest’s “Articuno warning” is the reference implementation).
- **Naming, counts, tilesets** – `docs/dungeons.md` explains how the global dungeon ID drives every table. It’s the reference when you touch `include/constants/dungeon.h` or the `gDungeonNames` table – critical when the randomizer wants to swap IDs around without corrupting the data tables.
- **Future hooks** – The RNG entry points leave room to shuffle spawn density, boss frequency, forced weather, and music per difficulty tier. The helper tables in `src/dungeon_seed_overrides.c` (`sSpeciesPools`, `sFloorBandTable`, etc.) are intentionally data-driven so we can grow them without rewriting engine code.

## Hub, Story, & Memory Changes
- **Pokémon Square stripped** – `NormalizeGroundMapId()` in `src/ground_map.c:660-704` aliases every Square warp to the Team Base exterior. `docs/town_square_deletion.md` and `docs/ewram-reduction.md` walk through the steps we already took (stubbing `gGroundScript_gs1`, redirecting map conversions, trimming live pools, shrinking the heap) and what’s still on the roadmap (delete Square map assets, guard any stray Square-only event calls).
- **Cutscene state reference** – `docs/story_flow.md` captures the scenario/state dump from a vanilla postgame save. `ApplySkipPostgameBootstrap()` mirrors that dump when Skip Cutscenes is ON so story scripts that still probe `ScenarioCalc` or `GroundScript` flags see the exact combination they expect.
- **Town NPC coverage** – Only the essentials (Kecleon, Felicity, Kangaskhan, the statue save helper) are kept alive on the base exterior per the plan in `docs/town_square_deletion.md`. Everything else is either stubbed or deleted so we reclaim ~110 KiB of EWRAM (`docs/ewram-reduction.md` has the breakdown).

## Reference Docs & Entry Points
- `docs/runtime-dungeon-hooks.md` – Hook map for runs that need to patch dungeon metadata at runtime.
- `docs/custom_bossfights.md` – Regi-style fight recipe (fixed rooms, KO hooks, stairs/loot spawns).
- `docs/dungeon_interrupts.md` – How to add “floor interrupt” sequences that pause the climb.
- `docs/dungeons.md` – Canonical list of dungeon IDs, name tables, and tileset lookups.
- `docs/story_flow.md` – Scenario/state dump from a postgame save; useful when tweaking script flags.
- `docs/town_square_deletion.md` & `docs/ewram-reduction.md` – Memory audits and the Square removal plan.

Keep this file short and up to date so any automation (or another AI agent) can immediately understand how Rogue Rescue Team differs from upstream pret/pmd-red and which files to inspect next when they need to touch seeds, dungeons, or hub behavior.
