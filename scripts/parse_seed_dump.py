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
DEFAULT_PLOT_PATH = PROJECT_ROOT / "scripts" / "seed_spawn_ranges.pdf"
ITEM_POKE_ID = "105"  # ITEM_POKE constant from include/constants/item.h
MONSTER_DATA_PATH = PROJECT_ROOT / "data" / "monster" / "monster_data.json"
MONSTER_NAMES_PATH = PROJECT_ROOT / "data" / "monster" / "monster_names.s"

GLOBAL_COLUMN_REMOVALS = {"species", "boss_species"}
SECTION_COLUMN_REMOVALS = {
    "spawn_range": {"start_idx", "end_idx"},
    "spawn_entry": {"range_start_idx", "range_end_idx"},
}
SPECIES_NAME_COLUMNS = ("species_name", "boss_species_name")
DIFFICULTY_LABELS = {
    0: "Normal",
    1: "Hard",
    2: "Nightmare",
}
RECRUIT_SETTING_LABELS = {
    0: "Normal",
    1: "All Recruitable",
    2: "No Recruitable",
    3: "Auto Recruit All",
}


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


def parse_int(value: str | None) -> int | None:
    if value is None:
        return None
    text = str(value).strip()
    if not text:
        return None
    try:
        return int(text, 10)
    except ValueError:
        return None


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


def build_run_settings_line(rows_by_section: OrderedDict[str, List[Dict[str, str]]]) -> str | None:
    settings_row = None
    settings_rows = rows_by_section.get("save_overrides") or []
    if settings_rows:
        settings_row = settings_rows[0]
    else:
        settings_row = {}

    seed = settings_row.get("seed")
    if not seed:
        meta_rows = rows_by_section.get("meta") or []
        if meta_rows:
            seed = meta_rows[0].get("seed")

    items = []
    if seed:
        items.append(f"Seed {seed}")

    diff_value = parse_int(settings_row.get("diff"))
    if diff_value is not None:
        diff_label = DIFFICULTY_LABELS.get(diff_value, f"Unknown ({diff_value})")
        items.append(f"Difficulty {diff_label}")

    skip_value = parse_int(settings_row.get("skip"))
    if skip_value is not None:
        items.append(f"Skip Basic Rescues {'On' if skip_value else 'Off'}")

    recruit_value = parse_int(settings_row.get("recruit"))
    if recruit_value is not None:
        recruit_label = RECRUIT_SETTING_LABELS.get(recruit_value, f"Unknown ({recruit_value})")
        items.append(f"Recruit {recruit_label}")

    if not items:
        return None
    return "Run Settings: " + " | ".join(items)


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


def build_spawn_range_groups(rows_by_section: OrderedDict[str, List[Dict[str, str]]]
                             ) -> OrderedDict[int, List[Dict[str, object]]]:
    rows = rows_by_section.get("spawn_range") or []
    by_dungeon: OrderedDict[int, List[Dict[str, object]]] = OrderedDict()
    seen_by_dungeon: Dict[int, set[tuple[str, int | None, int, int]]] = {}

    for row in rows:
        dungeon_id = parse_int(row.get("dungeon_id"))
        start_flr = parse_int(row.get("start_flr"))
        end_flr = parse_int(row.get("end_flr"))
        if dungeon_id is None or start_flr is None or end_flr is None:
            continue
        if end_flr < start_flr:
            start_flr, end_flr = end_flr, start_flr

        species_name = row.get("species_name") or row.get("species") or "Unknown"
        level = parse_int(row.get("level"))
        bst = parse_int(row.get("bst"))
        index = parse_int(row.get("index")) or 0
        label = species_name
        if level is not None:
            label = f"{species_name} Lv{level}"

        seen_entries = seen_by_dungeon.setdefault(dungeon_id, set())
        entry_key = (species_name, level, start_flr, end_flr)
        if entry_key in seen_entries:
            continue
        seen_entries.add(entry_key)

        by_dungeon.setdefault(dungeon_id, []).append({
            "label": label,
            "start_flr": start_flr,
            "end_flr": end_flr,
            "index": index,
            "bst": bst,
        })

    for entries in by_dungeon.values():
        entries.sort(key=lambda entry: (entry["index"], entry["start_flr"],
                                        entry["end_flr"], entry["label"]))

    return by_dungeon


def write_spawn_range_plots(rows_by_section: OrderedDict[str, List[Dict[str, str]]],
                            out_path: pathlib.Path) -> bool:
    by_dungeon = build_spawn_range_groups(rows_by_section)
    if not by_dungeon:
        print("No spawn_range rows found for plotting.")
        return False

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        from matplotlib.backends.backend_pdf import PdfPages
        from matplotlib.cm import ScalarMappable
        from matplotlib.colors import Normalize
        from matplotlib.ticker import MaxNLocator
    except ImportError:
        print("matplotlib not available; skipping spawn_range plot output.", file=sys.stderr)
        return False

    if out_path.suffix.lower() != ".pdf":
        print(f"Plot output path {out_path} is not a .pdf; skipping plot output.")
        return False

    out_path.parent.mkdir(parents=True, exist_ok=True)

    bst_values = []
    for entries in by_dungeon.values():
        for entry in entries:
            bst = entry.get("bst")
            if isinstance(bst, int):
                bst_values.append(bst)
    bst_min = min(bst_values) if bst_values else None
    bst_max = max(bst_values) if bst_values else None

    def draw_spawn_chart(fig, ax, dungeon_id: int, entries: List[Dict[str, object]]) -> None:
        entry_count = len(entries)
        if entry_count == 0:
            return
        max_floor = max(int(entry["end_flr"]) for entry in entries)
        labels = [str(entry["label"]) for entry in entries]
        y_positions = list(range(entry_count))
        lefts = [int(entry["start_flr"]) - 0.5 for entry in entries]
        widths = [int(entry["end_flr"]) - int(entry["start_flr"]) + 1 for entry in entries]

        colors = []
        if bst_min is not None and bst_max is not None and bst_min != bst_max:
            norm = Normalize(vmin=bst_min, vmax=bst_max)
            cmap = plt.get_cmap("YlGn")
            for entry in entries:
                bst = entry.get("bst")
                if isinstance(bst, int):
                    colors.append(cmap(norm(bst)))
                else:
                    colors.append("#d0d4d8")
            sm = ScalarMappable(norm=norm, cmap=cmap)
            sm.set_array([])
        else:
            sm = None
            colors = ["#9ad66b"] * entry_count

        ax.barh(y_positions, widths, left=lefts, height=0.7,
                color=colors, edgecolor="#3d7f2f")
        ax.set_yticks(y_positions)
        if entry_count > 60:
            label_size = 5
        elif entry_count > 40:
            label_size = 6
        elif entry_count > 25:
            label_size = 7
        else:
            label_size = 8
        ax.set_yticklabels(labels, fontsize=label_size)
        ax.invert_yaxis()
        ax.set_xlabel("Floor")
        ax.set_ylabel("Pokemon (level)")
        ax.set_title(f"Dungeon {dungeon_id} Spawn Ranges")
        ax.set_xlim(0.5, max_floor + 0.5)
        if max_floor <= 30:
            ax.set_xticks(range(1, max_floor + 1))
        else:
            ax.xaxis.set_major_locator(MaxNLocator(integer=True, nbins=12))
        ax.grid(axis="x", linestyle=":", linewidth=0.6, color="#c0c3c7")
        ax.set_axisbelow(True)
        if sm is not None:
            colorbar = fig.colorbar(sm, ax=ax, orientation="horizontal",
                                    pad=0.08, fraction=0.05)
            colorbar.set_label("BST")

    settings_line = build_run_settings_line(rows_by_section)
    with PdfPages(out_path) as pdf:
        for dungeon_id in sorted(by_dungeon):
            entries = by_dungeon[dungeon_id]
            entry_count = len(entries)
            max_floor = max(int(entry["end_flr"]) for entry in entries)
            width = max(9.0, min(16.0, max_floor * 0.25))
            height = min(40.0, max(3.0, 0.3 * entry_count + 1.5))
            fig, ax = plt.subplots(figsize=(width, height))
            draw_spawn_chart(fig, ax, dungeon_id, entries)
            if settings_line:
                fig.tight_layout(rect=(0, 0, 1, 0.94))
                fig.text(0.5, 0.985, settings_line, ha="center", va="top", fontsize=9)
            else:
                fig.tight_layout()
            pdf.savefig(fig, bbox_inches="tight")
            plt.close(fig)

    return True


def main() -> int:
    parser = argparse.ArgumentParser(description="Parse SEED_DUMP rows from execution.log.")
    parser.add_argument("--log", type=pathlib.Path, default=DEFAULT_LOG_PATH,
                        help=f"Path to log file (default: {DEFAULT_LOG_PATH})")
    parser.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT_PATH,
                        help=f"Output CSV path (default: {DEFAULT_OUT_PATH})")
    parser.add_argument("--plot-out", type=pathlib.Path, default=DEFAULT_PLOT_PATH,
                        help=f"Output PDF for spawn ranges (default: {DEFAULT_PLOT_PATH})")
    parser.add_argument("--no-print", action="store_true",
                        help="Skip printing tables to the terminal")
    parser.add_argument("--no-plot", action="store_true",
                        help="Skip spawn range plot output")
    args = parser.parse_args()

    rows_by_section, headers_by_section = parse_log(args.log)
    if not rows_by_section:
        print("No SEED_DUMP rows found.")
        return 0

    apply_seed_dump_overrides(rows_by_section, headers_by_section)

    csv_headers = build_csv_headers(rows_by_section, headers_by_section)
    write_csv(rows_by_section, csv_headers, args.out)

    print(f"Wrote CSV: {args.out}")
    if not args.no_plot:
        if write_spawn_range_plots(rows_by_section, args.plot_out):
            print(f"Wrote spawn range plots: {args.plot_out}")
    if not args.no_print:
        print_tables(rows_by_section, headers_by_section)

    return 0


if __name__ == "__main__":
    sys.exit(main())
