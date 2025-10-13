Story Flow Notes (GS/GO/Gating)

This document collects quick notes while iterating on scene skipping and the
early‑game story dungeon selection (GO) flow.

Terminology

- Script dungeon id: value from `include/constants/script_dungeon_id.h` used by
  scripts and the GO/Conquered arrays.
- List id (display id): the id used by the Dungeons list UI. Some script ids
  map to a different display id for naming purposes (e.g., Sinister Woods).
- GO (job) flag: `RESCUE_SCENARIO_JOB_LIST` entry for a script dungeon id,
  queried via `sub_8097384(scriptId)`.

Key mappings and helpers

- `sub_80A26B8(scriptIdOrListId)` maps a value to its canonical display id.
  It is used internally by a number of helpers to collapse aliases.
- `sub_80A270C(listId)` returns the runtime dungeon index used by
  `sub_80974A0()` to fetch a human‑readable name.

Early‑game ids of interest

- 0: Tiny Woods
- 1: Thunderwave Cave
- 2: Mt. Steel
- 3: SCRIPT_DUNGEON_3 (display id for “Sinister Woods”)
- 4: SCRIPT_DUNGEON_SINISTER_WOODS (script id used by GO flag for SW)
- 6: SCRIPT_DUNGEON_SILENT_CHASM

Important nuance (Sinister Woods) and resolution

- Scripts set GO on script id 4 (SINISTER_WOODS), while the Dungeons list is
  built over display ids 0..45. The canonical mapping from a display id to a
  script id is `sub_80A26B8(displayId)` (backed by `gUnknown_8116F9A`).
- The original list code sometimes compared display ids directly to GO flags,
  which caused SW’s GO to appear on the Silent Chasm row (display id 4) since
  both values matched numerically.
- Fix: In `sub_802FBF4()` we now consistently:
  - Resolve `scriptId = sub_80A26B8(displayId)`
  - Evaluate GO/Conquered using `scriptId`
  - Force‑allow the SW row during scene 5 if SW is GO
  - Hide the Silent Chasm row during scene 5 until SW is conquered
  - No display‑id remapping is needed when GO checks use `scriptId`.

Post‑SW (Silent Chasm unlock)

- After clearing SW, scripts set GO on Silent Chasm (script id 6). Because the
  list code now checks GO by script id derived from each display id, the GO
  badge correctly appears on the Silent Chasm row, not a duplicate SW row.

Runtime enforcement for robustness

- On entering Team Base or Pokémon Square with skip enabled:
  - scen==5 and SW not conquered → enforce SW as sole GO (clear SC GO).
 - scen==6 (post‑SW, pre‑SC clear) → enforce SC as active GO and clear any
  lingering SW GO, also preselect SC in `DUNGEON_SELECT`.
 - scen==7 (post‑SC, pre‑Mt. Thunder clear) → enforce Mt. Thunder as active GO
   and clear any lingering SC GO; preselect Mt. Thunder.
 - These guards are in `src/ground_script.c` and are no‑ops once SC is conquered.
  - Mini‑scene skips at Team Base: in skip mode we skip the outside Team Base
    “Team Meanies + Caterpie” (scene 5) and the post‑SW “thank‑you” (scene 6)
    by reloading the inside free‑roam station.

Script alias note

- Some story scripts use an alias id (e.g., `SCRIPT_DUNGEON_3`) when marking a
  stage as conquered. With cutscene skipping, this could leave the real SW GO
  flag set. We add a small fix in `ground_script.c` (case 0xb0 handler) to clear
  SW’s GO if its alias is conquered, keeping UI and state consistent.

Skip‑cutscene integration (Team Base / Pokémon Square)

- `src/ground_script.c` enforces the post–Mt. Steel state:
  - Clears any premature SC GO
  - Sets SW GO and preselects SW on the world map
  - Blocks scripts from re‑setting Mt. Steel as GO once scen >= 5

Debug strings

- `[GS] …` ground script enforcement/logs
- `[GO] …` GO/Conquered changes
- `[WM] …` world‑map/dungeon‑list traces added while debugging SW/SC
- Great Canyon early appearance (fix)

- Cause: the Dungeons list used the display index to check GO/Conquered in
  `sub_80A27CC()`. For Great Canyon (display id 6), this overlapped with
  Silent Chasm’s script id (6), so when SC was GO, GC erroneously appeared.
- Fix: `sub_802FBF4()` now passes the resolved script id to
  `sub_80A27CC()`, ensuring GC only appears after its proper story trigger
  (post–Mt. Thunder).

Mt. Thunder visibility

- After SC clear (scene 7), runtime guards set Mt. Thunder GO and clear SC GO
  when cutscene skip is on. UI also refuses to show SW/SC as GO for
  scene>=6/7, respectively, as a last resort.

End‑room cutscene skips

- To keep scenario progression consistent while skipping:
  - Tiny Woods end: marks TW conquered; sets TWC GO (already present).
  - Mt. Steel end: marks MS conquered; sets SW GO (already present).
  - Sinister Woods end: marks SW conquered; sets Silent Chasm GO (skips the
    Metapod/Caterpie thank‑you scene).
  - Silent Chasm end: marks SC conquered; sets Mt. Thunder GO.
  - Mt. Thunder end: marks MT conquered; sets Great Canyon GO.
  - Each skip advances `SCENARIO_MAIN` to the next major stage and returns to
    Team Base for free roam.

Square “Sleeping” after Great Canyon

- Target behavior (skip mode): After GC clear, do not auto‑enter Lapis Cave or
  jump into the Blaze/Freeze chain. Instead, enter Pokémon Square “sleeping”
  where partner talk triggers Lapis Cave.
- Implementation details:
  - Fast‑forward to `SCENARIO_MAIN = 11.2` (not 14.0) and warp to Pokémon
    Square. This matches the pre‑Lapis sleeping setup and avoids Mt. Blaze/
    Mt. Freeze arcs.
  - Detection is resilient: on returning to Team Base or Square, if the last
    dungeon result indicates success (observed values 6/9/11/12) and GC is the
    active GO, mark GC conquered, clear its GO, then move to 11.2 + Square.
  - End‑room path (Hill of the Ancients) is also hooked to apply the same
    fast‑forward if reached directly.
  - We intentionally do NOT set Lapis Cave GO; the partner talk initiates LC.

Skip‑mode vs vanilla

- All UI gating, end‑room skips, and enforcement live behind the
  `GetSkipCutscenesSetting()` check. With skip OFF, visibility and GO badges
  follow vanilla flow; with skip ON, scene‑aware guards apply.

Logging quick reference

- `[GS] TB|SQ resume: D_RESULT=… D_ENTER=… scen=…` — return diagnostics
- `[GS] enforce TB|SQ: …` — scene‑aware GO corrections
- `[GS] skip … end` — end‑room skip progression
- `[WM] allow … / print …` — Dungeons list gating and rendering traces
