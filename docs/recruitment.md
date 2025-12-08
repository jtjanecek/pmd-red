# Recruitment Calculations

## Where the roll happens
- `TryRecruitMonster()` (`src/dungeon_mon_recruit.c:85-217`) is the live path used when a defeated enemy asks to join. `code_806FDF4.c` holds a disabled legacy path.
- `HandleMonsterJoinSequence()` (`src/dungeon_mon_recruit.c:296-391`) is the accept/decline flow once the roll passes and enforces name entry, friend area unlocks, and spawn of the new ally.
- Eligibility helpers: `IsMonsterRecruitable()` (`src/dungeon_mon_recruit.c:253-294`) screens global toggles, recruit cap, banned species, and friend area capacity; `CanEntityBeRecruited()` (`src/dungeon_mon_recruit.c:394-441`) does the pre-roll body-size/team-slot checks.

## Other gates that must pass
- Player options: Recruit All setting from the quiz (`RECRUIT_ALL_NONE/ALL/AUTO`) can hard-disable recruitment or bypass many checks (Auto only enforces the overall cap and refuses clients/allies).
- Dungeon/story locks: per-dungeon flag `IsRecruitingEnabled()`, fixed-room/boss exclusions, and one-time legendary guard that rejects if the species is already recruited.
- Spatial requirements: must be adjacent (Chebyshev distance < 2) and visible (`CanSeeTarget`), and the target cannot be a client or ally.
- Capacity: `WouldExceedRecruitmentCap()` blocks if the global recruit count (saved mons + pending dungeon joins) would exceed `MAX_RECRUITED_POKEMON` or if current body-size strips and team member slots are full.
- Friend Areas: `IsMonsterRecruitable()` rejects if the species’ friend area is missing; our bootstrap defaults already unlock all areas, but the check is still present.

## Current formula (difficulty-based percent)
- `ComputeRecruitChancePercent()` (`src/dungeon_mon_recruit.c:60-83`) pulls difficulty-aware overrides from `DungeonSeedOverrides_GetRecruitOverride()` (`src/dungeon_seed_overrides.c:443-456`).
- Base chances: Normal 10%, Hard 5%, Nightmare 1%. Friend Bow adds +5/+3/+1 on top. Values are clamped to 0–100 and roll against `DungeonRandInt(100)`.
- Species-specific recruit rates are ignored for now (no `-999` blocks); eligibility still follows Recruit All / AutoRecruitAll, caps, adjacency, legendaries, and dungeon/story locks. AutoRecruitAll bypasses the roll but respects caps and client/allied bans.

## Legacy level-based formula (for reference, removed)
- Previously: base chance from `recruitRate` in `data/monster/monster_data.json` plus `gFriendBowRecruitRateUpValue` and `gRecruitRateByLevel[attackerLevel]`, rolled against `DungeonRandInt(1000)`. `-999` rates blocked recruitment unless AutoRecruitAll; this guard is now removed in favor of flat percent chances.

## Next steps
1. Add an optional compat layer to reinterpret existing `recruitRate` data (e.g., `raw / 10`) for species-specific tuning, then migrate `monster_data.json` to explicit percent values.
2. Extend `DungeonSeedOverrides_GetRecruitOverride()` to accept dungeon/species/difficulty context so seed-based runs can tweak probabilities without touching the core helper.
3. Update UI/readouts to display percent-based chances and run `make` plus a few dungeon runs to validate the new math, especially with AutoRecruitAll and Friend Bow equipped.
