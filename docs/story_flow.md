Story Flow Notes (GS/GO/Gating)

This document collects quick notes while iterating on scene skipping and the
early‑game story dungeon selection (GO) flow.

Note on scope

- All runtime changes described here run only when `SkipCutscenes=ON`
  (`GetSkipCutscenesSetting() == TRUE`). With skip OFF, vanilla scripts and
  flows are preserved.

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

Quick summary (up to Great Canyon)

- Scene 3 → Thunderwave Cave is GO; Tiny Woods cleared.
- Scene 4 → Mt. Steel becomes GO; Thunderwave Cave cleared.
- Scene 5 → Sinister Woods is enforced as sole GO (SC hidden until SW clear).
- Scene 6 → Silent Chasm becomes GO; any lingering SW GO cleared; SC preselected.
- Scene 7 → Mt. Thunder becomes GO; SC GO cleared; MT preselected.
- Scene 8 → Great Canyon becomes GO; MT GO cleared; GC preselected.
- After GC clear → Jump to Pokémon Square sleeping (scene 11.2); GC GO cleared. Partner talk triggers Lapis Cave (no auto‑GO).

Post‑GC to Mt. Blaze (skip mode)

- Square “sleeping” (scene 11.* or 14): Partner “All set!” redirects to the
  Lapis Cave entrance cutscene (no Lapis GO set; player chooses Lapis/Rock Path).
- After Lapis Cave clear: Immediately promote Mt. Blaze as GO and warp to
  the Mt. Blaze entrance. Scenario advances into the Mt. Blaze arc (`12.2`).
  Lapis Cave GO is cleared and its conquered flag set.

Post‑Mt. Blaze to Frosty Forest (skip mode)

- Target behavior: Skip the long "We’ve come a really long way…" interlude.
- After Mt. Blaze clear (Peak): Immediately promote Frosty Forest as GO and
  warp to `MAP_FROSTY_FOREST_ENTRY`. Scenario advances into the Frosty arc
  (`13.2`). Mt. Blaze is marked conquered and any lingering MB GO is cleared.
- Frosty Forest END station (skip mode): auto‑promote Frosty Grotto as GO and
  warp to `MAP_FROSTY_FOREST_MID` (save point), mirroring the Lapis→Mt. Blaze
  handoff. This avoids stray "nobody is here" lines at the end station.

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
  - scen==6 (post‑SW, pre‑SC clear) → enforce SC as active GO, clear SW GO, preselect SC.
  - scen==7 (post‑SC, pre‑Mt. Thunder clear) → enforce Mt. Thunder as active GO, clear SC GO, preselect MT.
  - scen==8 (post‑MT, pre‑GC clear) → enforce Great Canyon as active GO, clear MT GO, preselect GC.
  - After GC clear on resume → clear GC GO and fast‑forward to Square sleeping (scene 11.2).
  - These guards are in `src/ground_script.c` and are no‑ops once each dungeon is conquered.
- Mini‑scene skips at Team Base: in skip mode we skip the outside Team Base
  “Team Meanies + Caterpie” (scene 5) and the post‑SW “thank‑you” (scene 6)
  by reloading the inside free‑roam station.
- After Mt. Thunder (scene 8), we also skip the outside partner prompt
  about heading to Great Canyon and go straight to inside free‑roam with
  GC already set as GO. This uses `EVENT_S08E01[0]` as a one‑shot guard so
  the skip only triggers immediately after returning from Mt. Thunder and
  does not bounce you back inside on later exits.

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

Return detection normalization

- Some returns encode `DUNGEON_ENTER` with sentinels (0x50/0x51/0x52) and place
  the true script dungeon id in `DUNGEON_ENTER_INDEX`.
- The Team Base and Pokémon Square resume handlers now normalize the last‑enter
  value before checking for Silent Chasm or Great Canyon clears. This ensures we
  correctly promote Mt. Thunder after SC and fast‑forward after GC even when
  logs show `D_ENTER=81`.

Square “Sleeping” after Great Canyon

- Target behavior (skip mode): After GC clear, do not auto‑enter Lapis Cave or
  jump into the Blaze/Freeze chain. Instead, enter Pokémon Square “sleeping”
  where partner talk triggers the Lapis Cave entrance cutscene (with the
  Kangaskhan statue) before entering the dungeon.
- Implementation details:
  - Fast‑forward to `SCENARIO_MAIN = 11.2` and warp to Pokémon Square. Some
    resume paths may instead set scene `14`; both represent a pre‑Lapis
    “sleeping” free‑roam state in skip mode.
  - Detection is resilient: on returning to Team Base or Square, if the last
    dungeon result indicates success (observed values 6/9/11/12), normalize the
    last entered dungeon id (handle 0x50/0x51/0x52 via `DUNGEON_ENTER_INDEX`),
    and if it was Great Canyon, mark GC conquered, clear its GO, then move to
    11.2 + Square.
  - End‑room path (Hill of the Ancients) is also hooked to apply the same
    fast‑forward if reached directly.
  - We intentionally do NOT set Lapis Cave GO; the partner talk initiates LC
    by redirecting to `MAP_LAPIS_CAVE_ENTRY`. The redirect triggers for scene
    11.* and 14 when Great Canyon is conquered and Lapis Cave is not. If a
    partner prompt attempts to run the long fugitive scene
    (`EVENT_M01E07A_L002`), skip mode intercepts it and immediately warps to
    the Lapis Cave entrance cutscene instead.

After Lapis Cave clear (skip mode)

- Target behavior: Do not return to base/entrance loops. Promote Mt. Blaze and
  arrive at `MAP_MT_BLAZE_ENTRY` ready to proceed.
- Implementation details:
  - On TB/SQ resume, normalize last‑enter id. If it was `Lapis Cave` and the
    result was a success code (6/9/11/12):
    - Mark Lapis Cave conquered; clear any lingering Lapis GO.
    - Set Mt. Blaze as GO and select it.
    - Fast‑forward scenario to `SCENARIO_MAIN = 12.2` and warp to
      `MAP_MT_BLAZE_ENTRY`.
  - Early hook: the same detection also runs at the start of any ground‑map
    enter (before any map reroutes) to prevent Team Base → Lapis overrides from
    re‑triggering immediately after Lapis clear.
  - All of the above runs only when `SkipCutscenes=ON`.

Lapis Cave handoff details (skip mode)

- Fugitive-scene interception: Partner “All set!” can invoke
  `EVENT_M01E07A_L002`. We intercept this and perform a direct warp to
  `MAP_LAPIS_CAVE_ENTRY`, aborting the remainder of the event chain to avoid
  bouncing into Team Base/save flows.
- Base override during Square sleeping: If anything tries to route to Team Base
  (inside/outside) while in scene 11.* or 14 (GC done, LC not), we override the
  destination to Lapis Cave entrance. This guarantees we don’t land in base
  after pressing “All set!”.
- Entrance station enforcement: On arrival at `MAP_LAPIS_CAVE_ENTRY`, we force
  the entrance event station (g4 s0) so the cutscene runs and the partner is
  present. The cutscene then hands off to the partner prompt “Which way should
  we go?” (Lapis / Rock Path).
  - One‑shot guard: enforcement applies only on first arrival (group 0/sector 0)
    and sets a local guard (`MAP_LOCAL[0]`) to prevent bouncing back after
    internal station hops.
  - Loop protection at Lapis place: if already at a Lapis place (entrance/exit),
    the partner “ready” redirect is suppressed to avoid re‑warping.

Expected logs (skip mode)

- Post–Mt. Thunder return:
  - `[GS] enforce TB: set Great Canyon GO + select=…`
- After Great Canyon clear:
  - `[GS] TB detect GC clear: scen=11.2 -> Square sleeping (partner talk)` or
    `[GS] SQ detect GC clear: scen=11.2 -> Square sleeping (partner talk)`
- Partner “All set!” at Square sleeping:
  - `[GS] redirect: partner 'ready' -> Lapis Cave entrance cutscene` or
    `[GS] suppress redirect at Lapis place=22 (avoid loop)`
  - `[GS] reroute Lapis entry -> entrance event station (g4 s0)` (one‑shot)
- After Lapis Cave clear:
  - `[GS] enter detect LC clear: scen=12.2 -> Mt. Blaze entrance (set MB GO)` or
    `[GS] TB|SQ detect LC clear: scen=12.2 -> Mt. Blaze entrance (set MB GO)`

Key log breadcrumbs while debugging

- Intercepts
  - `[GS] intercept EXECUTE_FUNCTION*: skip fugitive -> Lapis Cave entrance` (src/ground_script.c:1712)
  - `[GS] intercept EXECUTE_FUNCTION: skip fugitive -> Lapis Cave entrance` (src/ground_script.c:1731)
- Overrides/reroutes
  - `[GS] override TB -> Lapis entry (g4 s0) during Square sleeping pre-LC` (src/ground_script.c:1798)
  - `[GS] reroute Lapis entry -> entrance event station (g4 s0)` (src/ground_script.c:1812)

Skip‑mode vs vanilla

- All UI gating, end‑room skips, and enforcement live behind the
  `GetSkipCutscenesSetting()` check. With skip OFF, visibility and GO badges
  follow vanilla flow; with skip ON, scene‑aware guards apply.

Logging quick reference

- `[GS] TB|SQ resume: D_RESULT=… D_ENTER=… scen=…` — return diagnostics
- `[GS] enforce TB|SQ: …` — scene‑aware GO corrections
- `[GS] skip … end` — end‑room skip progression
- `[WM] allow … / print …` — Dungeons list gating and rendering traces


# Examples:
POSTGAME:
Leaving base:
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=2 GE=9 GEL=0 GO=12 GM=9 GP=10 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[0,0,0,0]
[WARN] GBA Debug: [GS] exec station map=11 group=7 sector=0 set=1 place=10
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=2 GE=9 GEL=0 GO=12 GM=9 GP=10 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[0,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=2 GE=9 GEL=0 GO=12 GM=9 GP=10 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[0,0,0,0]

There were no GS updates on saving.


After completing a random dungeon in post game:
Waking up in post game (no cutscene)
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=4 group=11 sector=0 set=1 place=4
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=11 group=6 sector=0 set=1 place=10
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=178 GEL=0 GO=9 GM=9 GP=0 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]

[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=12 GEL=0 GO=12 GM=12 GP=11 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=14 group=8 sector=0 set=1 place=11
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=12 GP=11 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=12 GP=11 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]



# Post Game title screen GS sets in postgame loading the save (vanilla)
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=12 GEL=0 GO=12 GM=227 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=226 group=1 sector=0 set=1 place=37
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=227 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=227 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=12 GEL=0 GO=12 GM=226 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=5 group=5 sector=0 set=1 place=5
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=226 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=226 GP=37 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(pre-station): scen=19 start=9 GE=12 GEL=0 GO=12 GM=5 GP=5 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] exec station map=224 group=4 sector=0 set=1 place=37
[WARN] GBA Debug: [GS] dump(pre-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=5 GP=5 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]
[WARN] GBA Debug: [GS] dump(post-exec): scen=19 start=9 GE=12 GEL=0 GO=12 GM=5 GP=5 DS=47 DE=81 DEI=0 DR=9 BK=0 BL=2 WL=0 s1=31 s2=33 s3=0 s4=0 s5=0 s6=0 s7=0 s8=0 s9=0 E8=[1,0,0,0]

this is postgame log example above before we even load into base and click continue, just loading the rom