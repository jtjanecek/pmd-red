#!/usr/bin/env python3
"""
Export monster base stats and level-up data to CSV.

Reads base stats from data/monster/monster_data.json and LevelData entries
from lvmp%03d in the ROM's system file archive, then emits per-level totals.
"""

import argparse
import csv
import json
import struct
import sys
from pathlib import Path
from typing import Dict, List, Optional


ROM_VADDR = 0x08000000
SYSTEM_ARCHIVE_OFFSET = 0x300500  # gSystemFileArchive in ROM

MAX_LEVEL = 100
LEVEL_DATA_SIZE = 12
LEVEL_DATA_STRUCT = struct.Struct("<IHBBBBxx")


def decompress_at(src_data: bytes) -> Optional[bytes]:
    """Decompress AT3P/AT4P data (based on src/decompress_at.c)."""
    if len(src_data) < 7:
        return None

    if src_data[0:4] == b"AT4P":
        compressed_length = src_data[5] + (src_data[6] << 8)
        idx_start = 0x12
    elif src_data[0:4] == b"AT3P":
        compressed_length = src_data[5] + (src_data[6] << 8)
        idx_start = 0x10
    else:
        return None

    if src_data[4] == ord("N"):
        return bytes(src_data[7 : 7 + compressed_length])

    flags = [src_data[0x7 + i] + 3 for i in range(9)]

    dst = bytearray()
    cur_index = idx_start
    current_byte = 0
    cmd_bit = 8

    while cur_index < compressed_length:
        if cmd_bit == 8:
            current_byte = src_data[cur_index]
            cur_index += 1
            cmd_bit = 0

        if (current_byte & 0x80) == 0:
            command = (src_data[cur_index] >> 4) + 3
            tmp = (src_data[cur_index] & 0xF) << 8

            if command == flags[0]:
                command = 0x1F
            elif command == flags[1]:
                command = 0x1E
            elif command == flags[2]:
                command = 0x1D
            elif command == flags[3]:
                command = 0x1C
            elif command == flags[4]:
                command = 0x1B
            elif command == flags[5]:
                command = 0x1A
            elif command == flags[6]:
                command = 0x19
            elif command == flags[7]:
                command = 0x18
            elif command == flags[8]:
                command = 0x17

            if command == 0x1F:  # aaaa
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | (c & 0xF))
                dst.append(((c & 0xF) << 4) | (c & 0xF))
            elif command == 0x1E:  # abbb
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | ((c + 1) & 0xF))
                dst.append((((c + 1) & 0xF) << 4) | ((c + 1) & 0xF))
            elif command == 0x1D:  # babb
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | ((c - 1) & 0xF))
                dst.append(((c & 0xF) << 4) | (c & 0xF))
            elif command == 0x1C:  # bbab
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | (c & 0xF))
                dst.append((((c - 1) & 0xF) << 4) | (c & 0xF))
            elif command == 0x1B:  # bbba
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | (c & 0xF))
                dst.append(((c & 0xF) << 4) | ((c - 1) & 0xF))
            elif command == 0x1A:  # baaa
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | ((c - 1) & 0xF))
                dst.append((((c - 1) & 0xF) << 4) | ((c - 1) & 0xF))
            elif command == 0x19:  # abaa
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | ((c + 1) & 0xF))
                dst.append(((c & 0xF) << 4) | (c & 0xF))
            elif command == 0x18:  # aaba
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | (c & 0xF))
                dst.append((((c + 1) & 0xF) << 4) | (c & 0xF))
            elif command == 0x17:  # aaab
                c = src_data[cur_index] & 0xF
                cur_index += 1
                dst.append(((c & 0xF) << 4) | (c & 0xF))
                dst.append(((c & 0xF) << 4) | ((c + 1) & 0xF))
            else:
                cur_index += 1
                tmp += src_data[cur_index]
                cur_index += 1
                tmp = tmp + len(dst) - 0x1000
                for _ in range(command):
                    if 0 <= tmp < len(dst):
                        dst.append(dst[tmp])
                    else:
                        dst.append(0)
                    tmp += 1
        else:
            dst.append(src_data[cur_index])
            cur_index += 1

        cmd_bit += 1
        current_byte = (current_byte << 1) & 0xFF

    return bytes(dst)


def load_system_archive_index(rom) -> Dict[str, int]:
    rom.seek(SYSTEM_ARCHIVE_OFFSET)
    magic = rom.read(8)
    if magic != b"pksdir0\x00":
        raise RuntimeError(f"Unexpected system archive magic: {magic!r}")

    num_entries = struct.unpack("<I", rom.read(4))[0]
    table_ptr = struct.unpack("<I", rom.read(4))[0]

    rom.seek(table_ptr - ROM_VADDR)
    entries: Dict[str, int] = {}

    for _ in range(num_entries):
        name_ptr = struct.unpack("<I", rom.read(4))[0]
        data_ptr = struct.unpack("<I", rom.read(4))[0]

        cur_pos = rom.tell()
        rom.seek(name_ptr - ROM_VADDR)
        name_bytes = bytearray()
        while True:
            c = rom.read(1)
            if c == b"\x00" or not c or len(name_bytes) > 100:
                break
            name_bytes.extend(c)
        rom.seek(cur_pos)

        try:
            name = name_bytes.decode("ascii")
        except UnicodeDecodeError:
            continue

        entries[name] = data_ptr

    return entries


def extract_level_data(rom, entries: Dict[str, int], species_id: int) -> Optional[List[Dict[str, int]]]:
    filename = f"lvmp{species_id:03d}"
    file_ptr = entries.get(filename)
    if file_ptr is None:
        return None

    rom.seek(file_ptr - ROM_VADDR)
    magic = rom.read(4)

    if magic in (b"SIRO", b"SIR0"):
        data_ptr = struct.unpack("<I", rom.read(4))[0]
        rom.seek(data_ptr - ROM_VADDR)
        compressed_data = rom.read(MAX_LEVEL * LEVEL_DATA_SIZE + 0x100)
    else:
        rom.seek(file_ptr - ROM_VADDR)
        compressed_data = rom.read(MAX_LEVEL * LEVEL_DATA_SIZE + 0x100)

    decompressed = decompress_at(compressed_data)
    if decompressed is None:
        return None

    level_data: List[Dict[str, int]] = []
    for level_index in range(MAX_LEVEL):
        offset = level_index * LEVEL_DATA_SIZE
        chunk = decompressed[offset : offset + LEVEL_DATA_SIZE]
        if len(chunk) < LEVEL_DATA_SIZE:
            break
        exp_required, gain_hp, gain_att, gain_sp_att, gain_def, gain_sp_def = (
            LEVEL_DATA_STRUCT.unpack(chunk)
        )
        level_data.append(
            {
                "exp_required": exp_required,
                "gain_hp": gain_hp,
                "gain_att": gain_att,
                "gain_sp_att": gain_sp_att,
                "gain_def": gain_def,
                "gain_sp_def": gain_sp_def,
            }
        )

    return level_data


def normalize_name(raw_name: str) -> str:
    if raw_name.startswith("MonsterName"):
        return raw_name[len("MonsterName") :]
    return raw_name


def find_default_rom(repo_root: Path) -> Optional[Path]:
    for candidate in ("baserom.gba", "pmd_red.gba"):
        path = repo_root / candidate
        if path.exists():
            return path
    return None


def load_monster_data(path: Path) -> List[Dict[str, object]]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Export monster base stats and level-up data to CSV."
    )
    parser.add_argument(
        "--rom",
        type=Path,
        default=None,
        help="Path to baserom.gba or pmd_red.gba (defaults to repo root).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Output CSV path (defaults to repo root/monster_level_stats.csv).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[1]

    monster_data_path = repo_root / "data" / "monster" / "monster_data.json"
    if not monster_data_path.exists():
        print(f"Missing monster data: {monster_data_path}", file=sys.stderr)
        return 1

    rom_path = args.rom or find_default_rom(repo_root)
    if rom_path is None or not rom_path.exists():
        print("ROM not found. Provide --rom to a baserom.gba or pmd_red.gba.", file=sys.stderr)
        return 1

    output_path = args.output or (repo_root / "monster_level_stats.csv")

    monster_data = load_monster_data(monster_data_path)

    fieldnames = [
        "species_id",
        "species_name",
        "level",
        "exp_required",
        "gain_hp",
        "gain_atk",
        "gain_sp_atk",
        "gain_def",
        "gain_sp_def",
        "base_hp",
        "base_atk",
        "base_sp_atk",
        "base_def",
        "base_sp_def",
        "total_hp",
        "total_atk",
        "total_sp_atk",
        "total_def",
        "total_sp_def",
    ]

    missing_count = 0
    with rom_path.open("rb") as rom, output_path.open("w", newline="", encoding="utf-8") as output:
        entries = load_system_archive_index(rom)
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()

        for species_id, monster in enumerate(monster_data):
            level_data = extract_level_data(rom, entries, species_id)
            if level_data is None:
                missing_count += 1
                continue

            base_hp = int(monster.get("baseHP", 0))
            base_atk, base_sp_atk = monster.get("baseAtkSpAtk", [0, 0])
            base_def, base_sp_def = monster.get("baseDefSpDef", [0, 0])

            total_hp = base_hp
            total_atk = [int(base_atk), int(base_sp_atk)]
            total_def = [int(base_def), int(base_sp_def)]

            name = normalize_name(str(monster.get("name", f"Species{species_id}")))

            for level_index, level in enumerate(level_data, start=1):
                if level_index > 1:
                    total_hp += level["gain_hp"]
                    total_atk[0] += level["gain_att"]
                    total_atk[1] += level["gain_sp_att"]
                    total_def[0] += level["gain_def"]
                    total_def[1] += level["gain_sp_def"]

                writer.writerow(
                    {
                        "species_id": species_id,
                        "species_name": name,
                        "level": level_index,
                        "exp_required": level["exp_required"],
                        "gain_hp": level["gain_hp"],
                        "gain_atk": level["gain_att"],
                        "gain_sp_atk": level["gain_sp_att"],
                        "gain_def": level["gain_def"],
                        "gain_sp_def": level["gain_sp_def"],
                        "base_hp": base_hp,
                        "base_atk": base_atk,
                        "base_sp_atk": base_sp_atk,
                        "base_def": base_def,
                        "base_sp_def": base_sp_def,
                        "total_hp": total_hp,
                        "total_atk": total_atk[0],
                        "total_sp_atk": total_atk[1],
                        "total_def": total_def[0],
                        "total_sp_def": total_def[1],
                    }
                )

    if missing_count:
        print(f"Skipped {missing_count} species with no lvmp data.", file=sys.stderr)

    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
