#!/usr/bin/env python3
"""
Generate the type boss table from rogue_files/type_bosses.csv.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
import pathlib
import re
import sys
from typing import Dict, List

MAX_BOSSES_PER_TYPE = 2
MINIONS_PER_BOSS = 2
MAX_MOVES_PER_BOSS = 4
STAT_COUNT = 5
CHANCE_SCALE = 1000
PROJECT_ROOT = pathlib.Path(__file__).resolve().parent.parent
MONSTER_HEADER_PATH = PROJECT_ROOT / "include" / "constants" / "monster.h"
MOVE_HEADER_PATH = PROJECT_ROOT / "include" / "constants" / "move_id.h"

TYPE_NAMES = [
    "NONE",
    "NORMAL",
    "FIRE",
    "WATER",
    "GRASS",
    "ELECTRIC",
    "ICE",
    "FIGHTING",
    "POISON",
    "GROUND",
    "FLYING",
    "PSYCHIC",
    "BUG",
    "ROCK",
    "GHOST",
    "DRAGON",
    "DARK",
    "STEEL",
]

TYPE_MAP: Dict[str, str] = {
    "NONE": "TYPE_NONE",
    "NORMAL": "TYPE_NORMAL",
    "FIRE": "TYPE_FIRE",
    "WATER": "TYPE_WATER",
    "GRASS": "TYPE_GRASS",
    "ELECTRIC": "TYPE_ELECTRIC",
    "ICE": "TYPE_ICE",
    "FIGHTING": "TYPE_FIGHTING",
    "POISON": "TYPE_POISON",
    "GROUND": "TYPE_GROUND",
    "FLYING": "TYPE_FLYING",
    "PSYCHIC": "TYPE_PSYCHIC",
    "BUG": "TYPE_BUG",
    "ROCK": "TYPE_ROCK",
    "GHOST": "TYPE_GHOST",
    "DRAGON": "TYPE_DRAGON",
    "DARK": "TYPE_DARK",
    "STEEL": "TYPE_STEEL",
}

WEATHER_MAP: Dict[str, str] = {
    "SUNNY": "WEATHER_SUNNY",
    "SANDSTORM": "WEATHER_SANDSTORM",
    "FOG": "WEATHER_FOG",
    "HAIL": "WEATHER_HAIL",
    "SNOW": "WEATHER_SNOW",
}


@dataclass
class BossRow:
    species: str
    weather: str | None
    chances: Dict[str, int]  # difficulty -> scaled probability
    minions: List[str]
    boss_moves_nightmare: List[str]
    boss_has_custom_moves_nightmare: bool
    boss_moves_non_nightmare: List[str]
    boss_has_custom_moves_non_nightmare: bool
    minion_moves_nightmare: List[List[str]]
    minion_has_custom_moves_nightmare: List[bool]
    minion_moves_non_nightmare: List[List[str]]
    minion_has_custom_moves_non_nightmare: List[bool]
    boss_stat_tiers: List[str]
    minion_stat_tiers: List[List[str]]


def load_monster_constants(path: pathlib.Path) -> List[str]:
    try:
        text = path.read_text(encoding="utf-8")
    except FileNotFoundError as exc:
        raise SystemExit(f"Monster header not found at {path}") from exc

    names = set()
    for match in re.finditer(r"MONSTER_([A-Z0-9_]+)", text):
        names.add(f"MONSTER_{match.group(1)}")
    if not names:
        raise SystemExit(f"No monster constants found in {path}")
    return list(names)


def load_move_constants(path: pathlib.Path) -> List[str]:
    try:
        text = path.read_text(encoding="utf-8")
    except FileNotFoundError as exc:
        raise SystemExit(f"Move header not found at {path}") from exc

    names = set()
    for match in re.finditer(r"MOVE_([A-Z0-9_]+)", text):
        names.add(f"MOVE_{match.group(1)}")
    if not names:
        raise SystemExit(f"No move constants found in {path}")
    return list(names)


SPECIES_OVERRIDES: Dict[str, str] = {
    # Default to Normal form when CSV specifies generic Deoxys.
    "DEOXYS": "MONSTER_DEOXYS_NORMAL",
}
VALID_SPECIES = set(load_monster_constants(MONSTER_HEADER_PATH))
VALID_MOVES = set(load_move_constants(MOVE_HEADER_PATH))
MOVE_OVERRIDES: Dict[str, str] = {
    "NONE": "MOVE_NOTHING",
    "NOTHING": "MOVE_NOTHING",
}

NON_NIGHTMARE_STATUS_MOVES = {
    "MOVE_ACID_ARMOR",
    "MOVE_AGILITY",
    "MOVE_AMNESIA",
    "MOVE_BARRIER",
    "MOVE_BELLY_DRUM",
    "MOVE_BULK_UP",
    "MOVE_CALM_MIND",
    "MOVE_CONFUSE_RAY",
    "MOVE_COSMIC_POWER",
    "MOVE_CURSE",
    "MOVE_DETECT",
    "MOVE_DOUBLE_TEAM",
    "MOVE_DRAGON_DANCE",
    "MOVE_EGG_BOMB",
    "MOVE_ENCORE",
    "MOVE_FEATHERDANCE",
    "MOVE_FOCUS_ENERGY",
    "MOVE_GLARE",
    "MOVE_HAZE",
    "MOVE_HEAL_BELL",
    "MOVE_HOWL",
    "MOVE_HYPNOSIS",
    "MOVE_LEECH_SEED",
    "MOVE_LIGHT_SCREEN",
    "MOVE_LOCK_ON",
    "MOVE_MEAN_LOOK",
    "MOVE_MEMENTO",
    "MOVE_METRONOME",
    "MOVE_MIND_READER",
    "MOVE_MINIMIZE",
    "MOVE_MORNING_SUN",
    "MOVE_MIRROR_COAT",
    "MOVE_NIGHT_SHADE",
    "MOVE_POISONPOWDER",
    "MOVE_PIN_MISSILE",
    "MOVE_PROTECT",
    "MOVE_PURSUIT",
    "MOVE_RECOVER",
    "MOVE_REFLECT",
    "MOVE_REST",
    "MOVE_ROAR",
    "MOVE_SAFEGUARD",
    "MOVE_SAND_ATTACK",
    "MOVE_SANDSTORM",
    "MOVE_SCARY_FACE",
    "MOVE_SCREECH",
    "MOVE_SILVER_WIND",
    "MOVE_SING",
    "MOVE_SLEEP_POWDER",
    "MOVE_SMOKESCREEN",
    "MOVE_STUN_SPORE",
    "MOVE_SUPERSONIC",
    "MOVE_SYNTHESIS",
    "MOVE_SWORDS_DANCE",
    "MOVE_TELEPORT",
    "MOVE_THUNDER_WAVE",
    "MOVE_TRANSFORM",
    "MOVE_FLAMETHROWER",
    "MOVE_FURY_SWIPES",
    "MOVE_WHIRLWIND",
    "MOVE_WILL_O_WISP",
    "MOVE_WISH",
    "MOVE_YAWN",
}

STAT_TIER_MAP: Dict[str, str] = {
    "LOW": "STAT_TIER_LOW",
    "MEDIUM": "STAT_TIER_MEDIUM",
    "HIGH": "STAT_TIER_HIGH",
}


def normalize_key(value: str) -> str:
    return value.strip().upper().replace("-", "_").replace(" ", "_")


def parse_probability(raw: str) -> int:
    raw = raw.strip()
    if not raw:
        return 0

    try:
        value = float(raw)
    except ValueError as exc:
        raise SystemExit(f"Invalid probability '{raw}'") from exc

    if value < 0 or value > 1:
        raise SystemExit(f"Probability '{raw}' must be between 0 and 1")

    scaled = round(value * CHANCE_SCALE)
    return max(0, min(scaled, CHANCE_SCALE))


def parse_weather(raw: str) -> str | None:
    key = normalize_key(raw)
    if not key:
        return None
    if key not in WEATHER_MAP:
        raise SystemExit(f"Unknown weather '{raw}'")
    return WEATHER_MAP[key]


def parse_species(raw: str, field_name: str) -> str:
    key = normalize_key(raw)
    if not key:
        raise SystemExit(f"Missing species for column '{field_name}'")

    if key in SPECIES_OVERRIDES:
        return SPECIES_OVERRIDES[key]

    candidate = f"MONSTER_{key}"
    if candidate not in VALID_SPECIES:
        raise SystemExit(f"Unknown species '{raw}' in column '{field_name}'")
    return candidate


def parse_move(raw: str, field_name: str, default_move: str = "MOVE_NOTHING") -> str:
    key = normalize_key(raw)

    if key in MOVE_OVERRIDES:
        mapped = MOVE_OVERRIDES[key]
    elif not key:
        mapped = default_move
    else:
        candidate = key if key.startswith("MOVE_") else f"MOVE_{key}"
        if candidate not in VALID_MOVES:
            raise SystemExit(f"Unknown move '{raw}' in column '{field_name}'")
        mapped = candidate

    return mapped


def parse_non_nightmare_move(raw: str, field_name: str) -> str:
    move = parse_move(raw, field_name)
    if move in NON_NIGHTMARE_STATUS_MOVES:
        raise SystemExit(
            f"Status move '{raw}' is not allowed in '{field_name}'. "
            "Non-nightmare move columns must contain offensive moves only."
        )
    return move


def parse_stat_tier(raw: str, field_name: str) -> str:
    key = normalize_key(raw)
    if not key:
        raise SystemExit(f"Missing stat tier for column '{field_name}'")
    if key not in STAT_TIER_MAP:
        raise SystemExit(f"Unknown stat tier '{raw}' in column '{field_name}'")
    return STAT_TIER_MAP[key]


def parse_csv(path: pathlib.Path) -> Dict[str, List[BossRow]]:
    pools: Dict[str, List[BossRow]] = {name: [] for name in TYPE_NAMES}

    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            name = normalize_key(row.get("Pokemon", ""))
            type_name = normalize_key(row.get("Type", ""))
            weather = parse_weather(row.get("weather", ""))
            chances = {
                "normal": parse_probability(row.get("weather_chance_normal", "")),
                "hard": parse_probability(row.get("weather_chance_hard", "")),
                "nightmare": parse_probability(row.get("weather_chance_nightmare", "")),
            }
            minions = [
                parse_species(row.get("Minion1", ""), "Minion1"),
                parse_species(row.get("Minion2", ""), "Minion2"),
            ]
            boss_moves_nightmare = [
                parse_move(row.get("BossMove1", ""), "BossMove1"),
                parse_move(row.get("BossMove2", ""), "BossMove2"),
                parse_move(row.get("BossMove3", ""), "BossMove3"),
                parse_move(row.get("BossMove4", ""), "BossMove4"),
            ]
            boss_moves_non_nightmare = [
                parse_non_nightmare_move(row.get("BossMove1NonNightmare", ""), "BossMove1NonNightmare"),
                parse_non_nightmare_move(row.get("BossMove2NonNightmare", ""), "BossMove2NonNightmare"),
                parse_non_nightmare_move(row.get("BossMove3NonNightmare", ""), "BossMove3NonNightmare"),
                parse_non_nightmare_move(row.get("BossMove4NonNightmare", ""), "BossMove4NonNightmare"),
            ]
            boss_stat_tiers = [
                "STAT_TIER_MEDIUM",
                parse_stat_tier(row.get("BossAtk", ""), "BossAtk"),
                parse_stat_tier(row.get("BossSpAtk", ""), "BossSpAtk"),
                parse_stat_tier(row.get("BossDef", ""), "BossDef"),
                parse_stat_tier(row.get("BossSpDef", ""), "BossSpDef"),
            ]
            minion1_stat_tiers = [
                "STAT_TIER_MEDIUM",
                parse_stat_tier(row.get("Minion1Atk", ""), "Minion1Atk"),
                parse_stat_tier(row.get("Minion1SpAtk", ""), "Minion1SpAtk"),
                parse_stat_tier(row.get("Minion1Def", ""), "Minion1Def"),
                parse_stat_tier(row.get("Minion1SpDef", ""), "Minion1SpDef"),
            ]
            minion2_stat_tiers = [
                "STAT_TIER_MEDIUM",
                parse_stat_tier(row.get("Minion2Atk", ""), "Minion2Atk"),
                parse_stat_tier(row.get("Minion2SpAtk", ""), "Minion2SpAtk"),
                parse_stat_tier(row.get("Minion2Def", ""), "Minion2Def"),
                parse_stat_tier(row.get("Minion2SpDef", ""), "Minion2SpDef"),
            ]
            minion1_moves_nightmare = [
                parse_move(row.get("Minion1Move1", ""), "Minion1Move1"),
                parse_move(row.get("Minion1Move2", ""), "Minion1Move2"),
                parse_move(row.get("Minion1Move3", ""), "Minion1Move3"),
                parse_move(row.get("Minion1Move4", ""), "Minion1Move4"),
            ]
            minion2_moves_nightmare = [
                parse_move(row.get("Minion2Move1", ""), "Minion2Move1"),
                parse_move(row.get("Minion2Move2", ""), "Minion2Move2"),
                parse_move(row.get("Minion2Move3", ""), "Minion2Move3"),
                parse_move(row.get("Minion2Move4", ""), "Minion2Move4"),
            ]
            minion1_moves_non_nightmare = [
                parse_non_nightmare_move(row.get("Minion1Move1NonNightmare", ""), "Minion1Move1NonNightmare"),
                parse_non_nightmare_move(row.get("Minion1Move2NonNightmare", ""), "Minion1Move2NonNightmare"),
                parse_non_nightmare_move(row.get("Minion1Move3NonNightmare", ""), "Minion1Move3NonNightmare"),
                parse_non_nightmare_move(row.get("Minion1Move4NonNightmare", ""), "Minion1Move4NonNightmare"),
            ]
            minion2_moves_non_nightmare = [
                parse_non_nightmare_move(row.get("Minion2Move1NonNightmare", ""), "Minion2Move1NonNightmare"),
                parse_non_nightmare_move(row.get("Minion2Move2NonNightmare", ""), "Minion2Move2NonNightmare"),
                parse_non_nightmare_move(row.get("Minion2Move3NonNightmare", ""), "Minion2Move3NonNightmare"),
                parse_non_nightmare_move(row.get("Minion2Move4NonNightmare", ""), "Minion2Move4NonNightmare"),
            ]
            species = parse_species(row.get("Pokemon", ""), "Pokemon")

            if not name or not type_name:
                raise SystemExit(f"Invalid row in {path}: {row!r}")
            if type_name not in TYPE_MAP:
                raise SystemExit(f"Unknown type '{type_name}' in {path}")
            if len(pools[type_name]) >= MAX_BOSSES_PER_TYPE:
                raise SystemExit(f"Too many bosses for type {type_name} (max {MAX_BOSSES_PER_TYPE})")

            if weather is None and any(chances.values()):
                raise SystemExit(f"Weather chances set for {name} but no weather provided")

            pools[type_name].append(
                BossRow(
                    species=species,
                    weather=weather,
                    chances=chances,
                    minions=minions,
                    boss_moves_nightmare=boss_moves_nightmare,
                    boss_has_custom_moves_nightmare=any(move != "MOVE_NOTHING" for move in boss_moves_nightmare),
                    boss_moves_non_nightmare=boss_moves_non_nightmare,
                    boss_has_custom_moves_non_nightmare=any(move != "MOVE_NOTHING" for move in boss_moves_non_nightmare),
                    minion_moves_nightmare=[minion1_moves_nightmare, minion2_moves_nightmare],
                    minion_has_custom_moves_nightmare=[
                        any(move != "MOVE_NOTHING" for move in minion1_moves_nightmare),
                        any(move != "MOVE_NOTHING" for move in minion2_moves_nightmare),
                    ],
                    minion_moves_non_nightmare=[minion1_moves_non_nightmare, minion2_moves_non_nightmare],
                    minion_has_custom_moves_non_nightmare=[
                        any(move != "MOVE_NOTHING" for move in minion1_moves_non_nightmare),
                        any(move != "MOVE_NOTHING" for move in minion2_moves_non_nightmare),
                    ],
                    boss_stat_tiers=boss_stat_tiers,
                    minion_stat_tiers=[minion1_stat_tiers, minion2_stat_tiers],
                )
            )

    for type_name in TYPE_NAMES:
        if type_name == "NONE":
            continue
        count = len(pools[type_name])
        if count != MAX_BOSSES_PER_TYPE:
            raise SystemExit(
                f"Type {type_name} has {count} bosses; expected {MAX_BOSSES_PER_TYPE}. "
                f"Check rogue_files/type_bosses.csv."
            )

    return pools


def write_c_file(path: pathlib.Path, pools: Dict[str, List[BossRow]]) -> None:
    with path.open("w", encoding="utf-8") as handle:
        handle.write("// Auto-generated by scripts/build_type_boss_table.py — do not edit.\n")
        handle.write('#include "global.h"\n')
        handle.write('#include "type_selection.h"\n')
        handle.write('#include "constants/monster.h"\n')
        handle.write('#include "constants/move_id.h"\n')
        handle.write('#include "constants/type.h"\n\n')
        handle.write("const TypeBossPool gTypeBossTable[NUM_TYPES] = {\n")

        for type_name in TYPE_NAMES:
            type_const = TYPE_MAP[type_name]
            rows = list(pools.get(type_name, []))
            species = [row.species for row in rows]
            count = len(species)
            while len(species) < MAX_BOSSES_PER_TYPE:
                species.append("MONSTER_NONE")

            handle.write(f"    [{type_const}] = {{\n")
            handle.write(f"        .species = {{{', '.join(species)}}},\n")
            handle.write("        .minions = {\n")
            for idx in range(MAX_BOSSES_PER_TYPE):
                if idx < len(rows):
                    minions = list(rows[idx].minions)
                else:
                    minions = []
                while len(minions) < MINIONS_PER_BOSS:
                    minions.append("MONSTER_NONE")
                handle.write(f"            {{{', '.join(minions)}}},\n")
            handle.write("        },\n")
            handle.write("        .moves = {\n")
            for variant in ("non_nightmare", "nightmare"):
                handle.write("            {\n")
                for idx in range(MAX_BOSSES_PER_TYPE):
                    if idx < len(rows):
                        if variant == "nightmare":
                            moves = list(rows[idx].boss_moves_nightmare)
                        else:
                            moves = list(rows[idx].boss_moves_non_nightmare)
                    else:
                        moves = []
                    while len(moves) < MAX_MOVES_PER_BOSS:
                        moves.append("MOVE_NOTHING")
                    handle.write(f"                {{{', '.join(moves)}}},\n")
                handle.write("            },\n")
            handle.write("        },\n")
            handle.write("        .hasCustomMoves = {\n")
            for variant in ("non_nightmare", "nightmare"):
                has_custom_moves = []
                for idx in range(MAX_BOSSES_PER_TYPE):
                    if idx < len(rows):
                        if variant == "nightmare":
                            flag = rows[idx].boss_has_custom_moves_nightmare
                        else:
                            flag = rows[idx].boss_has_custom_moves_non_nightmare
                        has_custom_moves.append("TRUE" if flag else "FALSE")
                    else:
                        has_custom_moves.append("FALSE")
                handle.write(f"            {{{', '.join(has_custom_moves)}}},\n")
            handle.write("        },\n")
            handle.write("        .minionMoves = {\n")
            for variant in ("non_nightmare", "nightmare"):
                handle.write("            {\n")
                for idx in range(MAX_BOSSES_PER_TYPE):
                    if idx < len(rows):
                        if variant == "nightmare":
                            minion_moves = list(rows[idx].minion_moves_nightmare)
                        else:
                            minion_moves = list(rows[idx].minion_moves_non_nightmare)
                    else:
                        minion_moves = []
                    while len(minion_moves) < MINIONS_PER_BOSS:
                        minion_moves.append([])
                    handle.write("                {\n")
                    for minion_idx in range(MINIONS_PER_BOSS):
                        moves = list(minion_moves[minion_idx])
                        while len(moves) < MAX_MOVES_PER_BOSS:
                            moves.append("MOVE_NOTHING")
                        handle.write(f"                    {{{', '.join(moves)}}},\n")
                    handle.write("                },\n")
                handle.write("            },\n")
            handle.write("        },\n")
            handle.write("        .minionHasCustomMoves = {\n")
            for variant in ("non_nightmare", "nightmare"):
                handle.write("            {\n")
                for idx in range(MAX_BOSSES_PER_TYPE):
                    if idx < len(rows):
                        if variant == "nightmare":
                            flags = list(rows[idx].minion_has_custom_moves_nightmare)
                        else:
                            flags = list(rows[idx].minion_has_custom_moves_non_nightmare)
                    else:
                        flags = []
                    while len(flags) < MINIONS_PER_BOSS:
                        flags.append(False)
                    flag_str = ", ".join("TRUE" if flag else "FALSE" for flag in flags)
                    handle.write(f"                {{{flag_str}}},\n")
                handle.write("            },\n")
            handle.write("        },\n")
            handle.write("        .bossStatTiers = {\n")
            for idx in range(MAX_BOSSES_PER_TYPE):
                if idx < len(rows):
                    tiers = list(rows[idx].boss_stat_tiers)
                else:
                    tiers = []
                while len(tiers) < STAT_COUNT:
                    tiers.append("STAT_TIER_MEDIUM")
                handle.write(f"            {{{', '.join(tiers)}}},\n")
            handle.write("        },\n")
            handle.write("        .minionStatTiers = {\n")
            for idx in range(MAX_BOSSES_PER_TYPE):
                if idx < len(rows):
                    minion_tiers = list(rows[idx].minion_stat_tiers)
                else:
                    minion_tiers = []
                while len(minion_tiers) < MINIONS_PER_BOSS:
                    minion_tiers.append(["STAT_TIER_MEDIUM"] * STAT_COUNT)
                handle.write("            {\n")
                for minion_idx in range(MINIONS_PER_BOSS):
                    tiers = list(minion_tiers[minion_idx])
                    while len(tiers) < STAT_COUNT:
                        tiers.append("STAT_TIER_MEDIUM")
                    handle.write(f"                {{{', '.join(tiers)}}},\n")
                handle.write("            },\n")
            handle.write("        },\n")
            handle.write(f"        .count = {count},\n")
            handle.write("    },\n")

        handle.write("};\n")

        handle.write("\nconst TypeBossWeatherPool gTypeBossWeatherTable[NUM_TYPES] = {\n")
        for type_name in TYPE_NAMES:
            type_const = TYPE_MAP[type_name]
            rows = list(pools.get(type_name, []))
            count = len(rows)

            handle.write(f"    [{type_const}] = {{\n")
            handle.write("        .bosses = {\n")
            for idx in range(MAX_BOSSES_PER_TYPE):
                if idx < len(rows):
                    row = rows[idx]
                    enabled = row.weather is not None and any(row.chances.values())
                    weather_const = row.weather or "WEATHER_CLEAR"
                    chance_normal = row.chances["normal"]
                    chance_hard = row.chances["hard"]
                    chance_nightmare = row.chances["nightmare"]
                    handle.write(
                        "            { .enabled = %s, .weather = %s, .chance = {%d, %d, %d} },\n"
                        % (
                            "TRUE" if enabled else "FALSE",
                            weather_const,
                            chance_normal,
                            chance_hard,
                            chance_nightmare,
                        )
                    )
                else:
                    handle.write("            {0},\n")

            handle.write("        },\n")
            handle.write(f"        .count = {count},\n")
            handle.write("    },\n")
        handle.write("};\n")


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=pathlib.Path, help="Path to rogue_files/type_bosses.csv")
    parser.add_argument("out", type=pathlib.Path, help="Output C file path")
    args = parser.parse_args(argv)

    pools = parse_csv(args.csv)
    write_c_file(args.out, pools)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
