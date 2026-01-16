#!/usr/bin/env python3
"""
Parse execution.log for SEED_DUMP rows and export a CSV while printing tables.
"""

from __future__ import annotations

import argparse
import csv
import json
import pathlib
import re
import sys
from collections import OrderedDict
from typing import Dict, List

PROJECT_ROOT = pathlib.Path(__file__).resolve().parent.parent
DEFAULT_LOG_PATH = PROJECT_ROOT / "execution.log"
DEFAULT_OUT_PATH = PROJECT_ROOT / "gen" / "seed_dump.csv"
ITEM_POKE_ID = "105"  # ITEM_POKE constant from include/constants/item.h
MONSTER_DATA_PATH = PROJECT_ROOT / "data" / "monster" / "monster_data.json"
MONSTER_NAMES_PATH = PROJECT_ROOT / "data" / "monster" / "monster_names.s"

GLOBAL_COLUMN_REMOVALS = {"species", "boss_species"}
SECTION_COLUMN_REMOVALS = {
    "spawn_range": {"start_idx", "end_idx"},
    "spawn_entry": {"range_start_idx", "range_end_idx"},
}
SPECIES_NAME_COLUMNS = ("species_name", "boss_species_name")


def extract_payload(line: str) -> str | None:
    marker = "SEED_DUMP"
    idx = line.find(marker)
    if idx < 0:
        return None
    return line[idx:].strip()


def clean_value(value: str) -> str:
    """Strip leading control tokens (e.g., '?O') and non-printable chars."""
    if not isinstance(value, str):
        return str(value)
    cleaned = value
    # Remove leading '?X' tokens commonly produced by control bytes.
    while cleaned.startswith("?") and len(cleaned) > 1 and cleaned[1].isalnum():
        cleaned = cleaned[2:].lstrip()
    # Keep only printable ASCII.
    cleaned = "".join(ch for ch in cleaned if 32 <= ord(ch) < 127)
    return cleaned.strip()


def parse_monster_names(path: pathlib.Path) -> Dict[str, str]:
    names: Dict[str, str] = {}
    current_label = None
    try:
        with path.open(encoding="utf-8") as handle:
            for line in handle:
                line = line.strip()
                if line.endswith(":"):
                    label = line[:-1]
                    current_label = label if label.startswith("MonsterName") else None
                    continue
                if current_label and (line.startswith(".string") or line.startswith(".asciz")):
                    match = re.search(r"\"([^\"]*)\"", line)
                    if not match:
                        continue
                    value = match.group(1).replace("\\0", "")
                    names[current_label] = value
                    current_label = None
    except FileNotFoundError:
        return {}
    return names


def load_bst_lookup() -> Dict[str, str]:
    if not MONSTER_DATA_PATH.exists() or not MONSTER_NAMES_PATH.exists():
        return {}
    names_by_label = parse_monster_names(MONSTER_NAMES_PATH)
    try:
        with MONSTER_DATA_PATH.open(encoding="utf-8") as handle:
            data = json.load(handle)
    except FileNotFoundError:
        return {}

    bst_by_name: Dict[str, str] = {}
    for entry in data:
        label = entry.get("name")
        display = names_by_label.get(label)
        if not display:
            continue
        base_hp = int(entry.get("baseHP", 0))
        atk_sp = entry.get("baseAtkSpAtk") or [0, 0]
        def_sp = entry.get("baseDefSpDef") or [0, 0]
        try:
            bst = base_hp + int(atk_sp[0]) + int(atk_sp[1]) + int(def_sp[0]) + int(def_sp[1])
        except (TypeError, ValueError, IndexError):
            continue
        bst_value = str(bst)
        if display not in bst_by_name:
            bst_by_name[display] = bst_value
        cleaned = clean_value(display)
        if cleaned and cleaned not in bst_by_name:
            bst_by_name[cleaned] = bst_value
    return bst_by_name


def insert_bst_header(headers: List[str]) -> List[str]:
    if "bst" in headers:
        return headers
    for i, name in enumerate(headers):
        if name in SPECIES_NAME_COLUMNS:
            return headers[:i + 1] + ["bst"] + headers[i + 1:]
    return headers


def apply_seed_dump_overrides(rows_by_section: OrderedDict[str, List[Dict[str, str]]],
                              headers_by_section: Dict[str, List[str]]) -> None:
    bst_lookup = load_bst_lookup()
    for section, rows in rows_by_section.items():
        removals = set(GLOBAL_COLUMN_REMOVALS)
        removals.update(SECTION_COLUMN_REMOVALS.get(section, set()))
        headers = headers_by_section.get(section)
        if headers:
            headers = [h for h in headers if h not in removals]
            if any(h in SPECIES_NAME_COLUMNS for h in headers):
                headers = insert_bst_header(headers)
            headers_by_section[section] = headers
        for row in rows:
            for column in removals:
                row.pop(column, None)
            bst_value = None
            for name_key in SPECIES_NAME_COLUMNS:
                species_name = row.get(name_key)
                if not species_name:
                    continue
                bst_value = bst_lookup.get(species_name)
                if bst_value is None:
                    bst_value = bst_lookup.get(clean_value(species_name))
                if bst_value is not None:
                    break
            if bst_value is not None:
                row["bst"] = bst_value


def parse_log(path: pathlib.Path) -> tuple[OrderedDict[str, List[Dict[str, str]]], Dict[str, List[str]]]:
    headers_by_section: Dict[str, List[str]] = {}
    rows_by_section: OrderedDict[str, List[Dict[str, str]]] = OrderedDict()
    section_counters: Dict[str, int] = {}

    try:
        with path.open(encoding="utf-8") as handle:
            for line in handle:
                payload = extract_payload(line)
                if payload is None:
                    continue
                parts = [part.strip() for part in payload.split(",")]
                if len(parts) < 2:
                    continue
                tag = parts[0]
                section = parts[1]
                if section not in rows_by_section:
                    rows_by_section[section] = []
                    section_counters[section] = 0

                if tag == "SEED_DUMP_HEADER":
                    headers_by_section[section] = parts[2:]
                    continue
                if tag != "SEED_DUMP":
                    continue

                values = parts[2:]
                header = headers_by_section.get(section)
                if header:
                    row = {name: values[i] if i < len(values) else "" for i, name in enumerate(header)}
                    if len(values) > len(header):
                        extra_start = len(header)
                        for i, value in enumerate(values[extra_start:], start=0):
                            row[f"extra_{i}"] = value
                else:
                    row = {f"col_{i}": value for i, value in enumerate(values)}

                row["section_index"] = str(section_counters[section])
                row = {k: clean_value(v) for k, v in row.items()}
                # Special-case money so the name doesn't vanish after cleaning.
                if "item_id" in row and row.get("item_id") == ITEM_POKE_ID:
                    row["item_name"] = "Poke (money)"
                rows_by_section[section].append(row)
                section_counters[section] += 1
    except FileNotFoundError as exc:
        raise SystemExit(f"Log not found at {path}") from exc

    return rows_by_section, headers_by_section


def build_csv_headers(rows_by_section: OrderedDict[str, List[Dict[str, str]]],
                      headers_by_section: Dict[str, List[str]]) -> List[str]:
    columns: List[str] = ["section"]

    for section in rows_by_section:
        for name in headers_by_section.get(section, []):
            if name not in columns:
                columns.append(name)

    if "section_index" not in columns:
        columns.append("section_index")

    for rows in rows_by_section.values():
        for row in rows:
            for name in row:
                if name not in columns:
                    columns.append(name)

    return columns


def write_csv(rows_by_section: OrderedDict[str, List[Dict[str, str]]],
              headers: List[str],
              out_path: pathlib.Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=headers)
        writer.writeheader()
        for section, rows in rows_by_section.items():
            for row in rows:
                record = {name: "" for name in headers}
                record["section"] = section
                for key, value in row.items():
                    record[key] = value
                writer.writerow(record)


def format_table(headers: List[str], rows: List[Dict[str, str]]) -> str:
    if not rows:
        return "(no rows)"

    headers = list(headers)
    if "section_index" in headers:
        headers = ["section_index"] + [h for h in headers if h != "section_index"]

    widths = []
    for header in headers:
        width = len(header)
        for row in rows:
            width = max(width, len(str(row.get(header, ""))))
        widths.append(width)

    def render_row(row: Dict[str, str]) -> str:
        return " | ".join(str(row.get(header, "")).ljust(width)
                          for header, width in zip(headers, widths))

    header_row = render_row({header: header for header in headers})
    separator = "-+-".join("-" * width for width in widths)
    lines = [header_row, separator]
    for row in rows:
        lines.append(render_row(row))
    return "\n".join(lines)


def print_tables(rows_by_section: OrderedDict[str, List[Dict[str, str]]],
                 headers_by_section: Dict[str, List[str]]) -> None:
    for section, rows in rows_by_section.items():
        headers = headers_by_section.get(section)
        if not headers and rows:
            headers = list(rows[0].keys())
        headers = headers or []
        print(f"\n[{section}] {len(rows)} rows")
        if headers:
            print(format_table(headers, rows))
        else:
            print("(no columns)")


def main() -> int:
    parser = argparse.ArgumentParser(description="Parse SEED_DUMP rows from execution.log.")
    parser.add_argument("--log", type=pathlib.Path, default=DEFAULT_LOG_PATH,
                        help=f"Path to log file (default: {DEFAULT_LOG_PATH})")
    parser.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT_PATH,
                        help=f"Output CSV path (default: {DEFAULT_OUT_PATH})")
    parser.add_argument("--no-print", action="store_true",
                        help="Skip printing tables to the terminal")
    args = parser.parse_args()

    rows_by_section, headers_by_section = parse_log(args.log)
    if not rows_by_section:
        print("No SEED_DUMP rows found.")
        return 0

    apply_seed_dump_overrides(rows_by_section, headers_by_section)

    csv_headers = build_csv_headers(rows_by_section, headers_by_section)
    write_csv(rows_by_section, csv_headers, args.out)

    print(f"Wrote CSV: {args.out}")
    if not args.no_print:
        print_tables(rows_by_section, headers_by_section)

    return 0


if __name__ == "__main__":
    sys.exit(main())
