# Monster Stats and Level Progression

This document summarizes how base stats, level-up gains, and EXP thresholds are computed in Rogue Rescue Team (PMD Red).

## Data sources

- Base stats come from `data/monster/monster_data.json` (loaded into `sMonsterParameters`).
  - `baseHP`
  - `baseAtkSpAtk[2]` (index 0 = Attack, index 1 = Special Attack)
  - `baseDefSpDef[2]` (index 0 = Defense, index 1 = Special Defense)
- Level-up data comes from `lvmp%03d` entries in `gSystemFileArchive`.
  - Loaded via `GetLvlUpEntry()` in `src/pokemon.c`.
  - Structure is `LevelData` in `include/structs/str_pokemon.h`:
    - `expRequired` (total EXP required to reach this level)
    - `gainHP`
    - `gainAtt[2]`
    - `gainDef[2]`

`GetLvlUpEntry()` loads the `lvmp` file for a species, decompresses it, and caches 100 entries. The function indexes with `level - 1`, so entry 0 corresponds to level 1.

## Stat formulas (base + gains)

Stat totals are computed by summing base stats and per-level gains. The helpers in `src/dungeon_mon_spawn.c` show the runtime logic:

- `CalcSpeciesHPAtLevel()`
- `CalcSpeciesAtkAtLevel()`
- `CalcSpeciesDefAtLevel()`

For a species `S` at level `L`:

```
HP(L)  = baseHP + sum(gainHP for levels 2..L)
Atk(L) = baseAtkSpAtk[c] + sum(gainAtt[c] for levels 2..L)
Def(L) = baseDefSpDef[c] + sum(gainDef[c] for levels 2..L)
```

These totals are used when initializing new entities (see `sub_806AED8()` and `InitEntityFromSpawnInfo()` in `src/dungeon_mon_spawn.c`).

## Level-up application

`LevelUp()` in `src/dungeon_leveling.c` advances levels when `exp >= LevelData.expRequired` for the next level. For each level gained it:

- Adds `gainHP`, `gainAtt[]`, `gainDef[]` to the current stats.
- Clamps `maxHPStat` to 999 and Attack/Defense to 255.
- Updates IQ skill state (`LoadIQSkills()` + `sub_8079764()`).

`LevelDownTarget()` subtracts the same gains, clamping to a minimum of 1 for HP and stats.

## EXP yields and modifiers

- EXP required per level is stored in `LevelData.expRequired`.
- EXP awarded for defeating a monster uses `CalculateEXPGain()` in `src/pokemon.c`:
  ```
  expYield + (expYield * (level - 1)) / 10
  ```
- `AddExpPoints()` in `src/dungeon_leveling.c` applies a global +20% EXP bonus:
  ```
  exp = (exp * 12) / 10
  ```

## CSV export

Use `scripts/export_monster_level_stats.py` to dump base stats, per-level gains, and computed totals:

```
python3 scripts/export_monster_level_stats.py --rom baserom.gba --output monster_level_stats.csv
```

The CSV includes:

- Base stats from `data/monster/monster_data.json`
- Per-level gains and EXP requirements from `lvmp%03d`
- Computed totals using the same sums as `CalcSpecies*AtLevel()`
