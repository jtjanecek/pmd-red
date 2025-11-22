# Dungeon Hint & Type Selection System — Updated Final Design

This document describes the finalized system for generating dungeon themes in the roguelike Red Rescue Team ROM hack. It now supports showing **two hint options per dungeon**, with the player choosing one hint, and the game selecting the final dungeon type using a **50/50 random split** between the two types in that hint.

The system ensures:
- Strong player agency  
- Automatic type balancing  
- Never using more than 2 bosses per type  
- Guaranteed completion of all 20 dungeons  

---

# 1. Overview

Each dungeon presents the player with **two hints**.  
Each hint corresponds to a **pair of types**, for example:

- Hint A → (Bug, Rock)  
- Hint B → (Fire, Psychic)

The player picks **one hint**.

Once selected, the game determines the dungeon type by choosing **one of the two types at random (50/50)**.  
That chosen type becomes the dungeon’s boss.

This process repeats across **20 dungeons**.

---

# 2. Type Pick Limits

Each Pokémon type may be used **at most twice** during the entire run.

We track type usage through a dictionary structure like:

picked_count[type] = number of times that type has been the dungeon’s final boss

Once picked_count[type] reaches 2:
- That type becomes **capped**
- Any hint containing that type must not appear again

This ensures:
- No type is overused
- You never need more than two boss monsters per type

---

# 3. Hint Table Structure

A CSV contains **136 hints**, one for each unordered pair of the 17 Gen 3 types.

Each row includes:
- A poetic hint line  
- Type1  
- Type2  

Every unordered type combination appears exactly once.

---

# 4. Dungeon Generation Algorithm

For each dungeon (1 through 20):

### Step 1 — Filter Valid Hints
From all 136 hints, filter out any pair where either type has been picked twice.

Remaining pairs form the list of **valid hints**.

### Step 2 — Select Two Hints to Display
Randomly select **two distinct pairs** from the valid list.  
These become the hint choices shown to the player.

### Step 3 — Player Selects One Hint
The player chooses which hint to follow.

### Step 4 — Determine Final Dungeon Type (50/50)
The selected hint contains types t1 and t2.

The game randomly chooses between them with equal probability:

- 50% → t1  
- 50% → t2  

This randomly chosen type becomes the dungeon’s boss type.

### Step 5 — Update Type Usage
Increase the count:

picked_count[selected_type] += 1

Repeat for the next dungeon.

---

# 5. Mathematical Guarantee of Safety

To show two valid hints, the system must always have **at least 2 valid pairs** available.

A pair is valid only if both its types have been picked less than twice.

Let k = number of capped types.  
Let S = set of types that still have fewer than 2 picks (usable types).

We require |S| ≥ 3, because:
- With only 2 usable types, there is exactly 1 possible pair  
- With fewer than 2 usable types, no pairs exist  

Could |S| ever fall below 3 during a 20-dungeon run?

For |S| = 2, at least 15 types must be capped:
- 15 capped types × 2 picks each = 30 picks  
- But the full run only contains 20 picks total  
- Therefore this situation is impossible

Thus:
- There will always be at least 3 uncapped types  
- There will always be at least 3 valid pairs  
- Therefore selecting 2 valid hints is always possible  

The system cannot softlock or fail before all 20 dungeons are generated.

---

# 6. Probability Behavior

Important clarifications:

- The player chooses **which hint** is used.
- The two types inside a hint are chosen **randomly, 50/50**.
- No type is biased toward higher or lower probability.
- Only the capping system affects availability.
- As a result, runs naturally diversify over time.

---

# 7. Summary

This system provides:

- Two hint options per dungeon  
- A player-selected hint  
- A 50/50 random determination of the final type within that hint  
- Automatic removal of overused types  
- Guaranteed run completion  
- Balanced distribution of dungeon themes  
- A simple and robust implementation model  

It perfectly supports:
- Limited boss pools (2 per type)  
- Roguelike variety  
- Replayability  
- Stable type distribution across long runs  

---

# 8. Benefits

- Smooth, self-correcting dungeon variety  
- Keeps player agency intact  
- No type dominates or repeats excessively  
- Automatically avoids edge cases or softlocks  
- Easy to code during ROM hacking  
- Easy to tune, expand, or replace with weighted logic later  

---

# 9. Implementation Plan

## 9.1 Content pipeline
- Keep `docs/type_choices.csv` as the single editable source of hint text and type pairs. We will add a Python helper under `tools/build_type_hint_table.py` that ingests the CSV, asserts that all 136 unordered combinations of the 17 Gen 3 types exist, and emits a generated include such as `src/data/type_hint_table.inc`.
- Define `typedef struct { const u8 *message; u8 type1; u8 type2; } TypeHintDefinition;` in a new `include/type_selection.h`. The generated include will populate `const TypeHintDefinition gTypeHintTable[]` and a `NUM_TYPE_HINTS` constant so the game never has to parse CSV at runtime.
- Alongside the table include, extend `src/strings.c` (or add `src/strings/type_hint_strings.inc`) with the localized text blobs that the generated table references. This keeps all hint text inside the existing string system so we can reuse the dialogue font renderer without new assets.

## 9.2 Runtime state and APIs
- Add `src/type_selection.c` plus the header mentioned above. This module keeps the entire per-run state in an `EWRAM_DATA TypeSelectionState` structure containing:
  - `u8 pickCount[NUM_TYPES];`
  - `s16 pendingHintIds[2];` and `u8 pendingHintCount;`
  - `u8 completedDungeons;`
  - `u8 currentDungeonType;` (the type committed for the upcoming dungeon) and `u8 hasCommittedType;`
  - A lightweight deterministic RNG pointer/index that is derived from `TeamBasicInfo.customSeed`, `completedDungeons`, and the global dungeon seed helpers in `src/dungeon_seed_overrides.c`.
- Expose helpers: `void TypeSelection_Init(void);`, `void TypeSelection_ResetForNewRun(void);`, `bool8 TypeSelection_EnsurePendingHints(void);`, `const TypeHintDefinition *TypeSelection_GetHint(u32 index);`, `bool8 TypeSelection_SelectHint(u32 optionIndex);`, `u8 TypeSelection_GetCommittedType(void);`, and `void TypeSelection_AdvanceToNextDungeon(void);`.
- `TypeSelection_EnsurePendingHints` filters `gTypeHintTable` to the pairs where both `pickCount[type] < 2`, uses the deterministic RNG to pick two distinct hint IDs, stores them in `pendingHintIds`, and marks the state dirty for saving.
- `TypeSelection_SelectHint` reads the chosen hint, flips a 50/50 coin between `type1` and `type2`, increments the winning type’s count, records `currentDungeonType`, bumps `completedDungeons`, clears `pendingHintIds`, and returns whether a dungeon slot is now ready.
- `TypeSelection_GetCommittedType` becomes the single source of truth for other systems (boss pools, spawn overrides, UI banners) that need to know which type governs the next dungeon. `DungeonSeedOverrides_GenerateFloorConfig` and any boss logic under `src/dungeon_seed_overrides.c` will call it before populating spawns so every hook stays in sync.

## 9.3 Save / load integration
- Define a packed `typedef struct { u8 pickCount[NUM_TYPES]; s16 pendingHintIds[2]; u8 pendingHintCount; u8 completedDungeons; u8 currentDungeonType; u8 hasCommittedType; } TypeSelectionSaveData;`.
- Extend both save blocks that already persist custom seed + difficulty:
  - `struct UnkStruct_sub_8011DAC` in `include/save.h` (full adventure save).
  - `struct unk_struct` in `src/save.c` (the small metadata sector used during auto-save / quick-save).
  Each struct gains a `TypeSelectionSaveData typeSelection;` field before its padding, and the padding array shrinks accordingly so total size stays constant.
- Add two glue helpers in `type_selection.c`: `void TypeSelection_WriteSaveData(TypeSelectionSaveData *out);` and `void TypeSelection_ReadSaveData(const TypeSelectionSaveData *in);`. `ReadSaveFromPak`, `WriteSavetoPak`, `sub_8011FA8`, and `sub_80121E0` will call the helpers right after handling the seed/difficulty fields so the nightly menu always resumes exactly where the player left off.
- Reset the state inside `TypeSelection_Init` when `InitializePlayerData()` builds a brand-new file so old save junk cannot leak into a new run.

## 9.4 Nightly UI hook
- Patch the Team Base evening script (`src/data/ground/ground_data_b01p01a_station.h`, entry `s_gs9_g6_s2_lives0_dlg0`) so the `MSG` that currently prints “That was good work today” is replaced with a script call into a new ground function: e.g., `{ 0xXX, 0x00, ..., GroundScript_ShowTypeSelectionMenu }`.
- Implement `GroundScript_ShowTypeSelectionMenu` (exact signature defined in `ground_script.h`) inside a new file such as `src/ground/type_selection_ui.c`. The function should:
  1. Call `TypeSelection_EnsurePendingHints()` to guarantee two hints exist; bail out gracefully if the player already locked in the next dungeon type.
  2. Render a two-option personality-quiz style menu using the existing `MenuItem` / `CreateDialogueBoxAndPortrait` helpers. The lower dialogue box always prints `"What path should we take tomorrow?"` while each option renders the exact hint text (formatted as `Hint N: <poetic line>`). We intentionally hide the actual type pair in the UI so players only see flavor hints, but the mgba log can keep printing the underlying types for debugging.
  3. On confirm, call `TypeSelection_SelectHint(selectionIndex)`; on cancel, keep the menu open so the player must commit before auto-save continues.
  4. Print a short confirmation line (“You’ve chosen the [Fire / Psychic] omen!”) before the script continues to the `TEXTBOX_CLEAR` and cue commands that already lead into the auto-save.
- The script hook fires right after returning from a dungeon and before the nightly `sub_80121E0` path, satisfying the requirement that the choice always happens before the automatic save writes.

## 9.5 Dungeon + progression glue
- Extend `DungeonSeedOverrides` with a `u8 sPendingDungeonType; bool8 sTypeValid;` pair and helpers (`DungeonSeedOverrides_SetPendingType`, `DungeonSeedOverrides_GetPendingType`) that simply proxy to the `TypeSelection` module. This keeps dungeon generation code ignorant of how the type was decided.
- When `TypeSelection_SelectHint` finalizes a type, immediately call `DungeonSeedOverrides_SetPendingType`; this feeds into `PopulateBossFightConfig` (boss pools), `PopulateSpawnTable` (spawn tables can swap species sets based on type), and any other runtime hook that wants to theme the dungeon.
- Track how many dungeons the player has cleared by reading `TypeSelectionState.completedDungeons` instead of re-deriving it from `RescueScenarioConquered`. Once the counter hits 20, mark the sequential dungeon list as complete (e.g., auto-conquer any remaining entries or trigger credits) so we stay aligned with the “20 dungeons per run” contract from this document.

## 9.6 Validation steps
- Unit-test the CSV converter so it fails the build if fewer than 136 unique unordered type pairs exist or if any type identifier doesn’t map to `include/constants/type.h`.
- Add mgba logging inside `TypeSelection_SelectHint` guarded by `#ifdef` during development so we can confirm the RNG stream is deterministic for a given seed / dungeon index.
- Manual QA checklist:
  1. Start a fresh file, clear three dungeons, and confirm the nightly menu always appears with two hints and enforces the “pick something” rule.
  2. Force-save and reload mid-run; verify the pending hints and per-type counts survive the reload and the menu offers the same two hints.
  3. Attempt to push one type beyond two selections by repeatedly picking the same hint; the menu should stop surfacing any hint that contains a capped type.
  4. Complete 20 dungeons; confirm the counter hits 20, no hints remain, and the run transitions into credits / postgame as expected.
