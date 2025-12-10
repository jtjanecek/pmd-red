#!/usr/bin/env python3
"""
Analyze Pokemon level-up EXP curves from PMD Red ROM.

This script extracts level data for all Pokemon species and outputs a CSV
with EXP requirements for each level.
"""

import struct
import csv
import sys
from pathlib import Path

# ROM constants
ROM_VADDR = 0x08000000
ROM_SIZE = 0x2000000

# Based on constants/monster.h, there are 600 monster IDs but not all are valid Pokemon
NUM_MONSTERS = 600

# LevelData structure from structs/str_pokemon.h:
# typedef struct LevelData {
#     s32 expRequired;  // offset 0x0, 4 bytes
#     u16 gainHP;       // offset 0x4, 2 bytes
#     u8 gainAtt[2];    // offset 0x6, 2 bytes
#     u8 gainDef[2];    // offset 0x8, 2 bytes
#     u16 fillA;        // offset 0xA, 2 bytes (padding)
# } LevelData;
LEVEL_DATA_STRUCT = struct.Struct('<IHBBBBxx')  # <I = s32, H = u16, 4xB = bytes, xx = padding
LEVEL_DATA_SIZE = 12  # 0xC bytes

MAX_LEVEL = 100  # 0x64 levels per Pokemon


def decompress_at(src_data):
    """
    Decompress AT-compressed data.
    Based on src/decompress_at.c
    """
    if len(src_data) < 7:
        return None

    # Check magic
    if src_data[0:4] == b'AT4P':
        compressed_length = src_data[5] + (src_data[6] << 8)
        decompressed_size = src_data[0x10] + (src_data[0x11] << 8)
        idx_start = 0x12
    elif src_data[0:4] == b'AT3P':
        compressed_length = src_data[5] + (src_data[6] << 8)
        idx_start = 0x10
        decompressed_size = 0
    else:
        return None

    # Check for uncompressed mode
    if src_data[4] == ord('N'):
        return bytes(src_data[7:7+compressed_length])

    # Parse flags
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
            # Process command
            command = (src_data[cur_index] >> 4) + 3
            tmp = (src_data[cur_index] & 0xf) << 8

            # Map flags
            if command == flags[0]: command = 0x1f
            elif command == flags[1]: command = 0x1e
            elif command == flags[2]: command = 0x1d
            elif command == flags[3]: command = 0x1c
            elif command == flags[4]: command = 0x1b
            elif command == flags[5]: command = 0x1a
            elif command == flags[6]: command = 0x19
            elif command == flags[7]: command = 0x18
            elif command == flags[8]: command = 0x17

            if command == 0x1f:  # aaaa
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | (c & 0xf))
                dst.append(((c & 0xf) << 4) | (c & 0xf))
            elif command == 0x1e:  # abbb
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | ((c + 1) & 0xf))
                dst.append((((c + 1) & 0xf) << 4) | ((c + 1) & 0xf))
            elif command == 0x1d:  # babb
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | ((c - 1) & 0xf))
                dst.append(((c & 0xf) << 4) | (c & 0xf))
            elif command == 0x1c:  # bbab
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | (c & 0xf))
                dst.append((((c - 1) & 0xf) << 4) | (c & 0xf))
            elif command == 0x1b:  # bbba
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | (c & 0xf))
                dst.append(((c & 0xf) << 4) | ((c - 1) & 0xf))
            elif command == 0x1a:  # baaa
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | ((c - 1) & 0xf))
                dst.append((((c - 1) & 0xf) << 4) | ((c - 1) & 0xf))
            elif command == 0x19:  # abaa
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | ((c + 1) & 0xf))
                dst.append(((c & 0xf) << 4) | (c & 0xf))
            elif command == 0x18:  # aaba
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | (c & 0xf))
                dst.append((((c + 1) & 0xf) << 4) | (c & 0xf))
            elif command == 0x17:  # aaab
                c = src_data[cur_index] & 0xf
                cur_index += 1
                dst.append(((c & 0xf) << 4) | (c & 0xf))
                dst.append(((c & 0xf) << 4) | ((c + 1) & 0xf))
            else:
                # Back reference
                cur_index += 1
                tmp += src_data[cur_index]
                cur_index += 1
                tmp = tmp + len(dst) - 0x1000
                for _ in range(command):
                    if tmp >= 0 and tmp < len(dst):
                        dst.append(dst[tmp])
                        tmp += 1
                    else:
                        dst.append(0)
                        tmp += 1
        else:
            # Copy byte directly
            dst.append(src_data[cur_index])
            cur_index += 1

        cmd_bit += 1
        current_byte = (current_byte << 1) & 0xFF

    return bytes(dst)


def find_file_in_archive(rom_data, filename):
    """
    Find a file in the system file archive (pksdir0).
    Returns offset to the file data, or None if not found.
    """
    # The gSystemFileArchive starts at 0x8300500 (0x300500 in file)
    archive_offset = 0x300500

    rom_data.seek(archive_offset)
    magic = rom_data.read(8)
    if magic != b'pksdir0\x00':
        print(f"Warning: Expected pksdir0 magic, got {magic}", file=sys.stderr)
        return None

    num_entries = struct.unpack('<I', rom_data.read(4))[0]
    table_ptr = struct.unpack('<I', rom_data.read(4))[0]

    # Read directory table
    rom_data.seek(table_ptr - ROM_VADDR)

    for i in range(num_entries):
        name_ptr = struct.unpack('<I', rom_data.read(4))[0]
        data_ptr = struct.unpack('<I', rom_data.read(4))[0]

        # Read name
        cur_pos = rom_data.tell()
        rom_data.seek(name_ptr - ROM_VADDR)
        name = b''
        while True:
            c = rom_data.read(1)
            if c == b'\x00' or len(name) > 100:
                break
            name += c
        rom_data.seek(cur_pos)

        if name.decode('ascii') == filename:
            return data_ptr

    return None


def extract_level_data(rom_path, species_id):
    """Extract level data for a specific Pokemon species."""
    filename = f'lvmp{species_id:03d}'

    with open(rom_path, 'rb') as rom:
        # Find the file in the archive
        file_ptr = find_file_in_archive(rom, filename)

        if file_ptr is None:
            return None

        # Read SIRO header
        rom.seek(file_ptr - ROM_VADDR)
        magic = rom.read(4)

        if magic not in (b'SIRO', b'SIR0'):
            # Might be AT compressed directly
            rom.seek(file_ptr - ROM_VADDR)
            compressed_data = rom.read(MAX_LEVEL * LEVEL_DATA_SIZE + 0x100)
            decompressed = decompress_at(compressed_data)

            if decompressed is None:
                return None
        else:
            # SIRO archive - read data pointer
            data_ptr = struct.unpack('<I', rom.read(4))[0]
            rom.seek(data_ptr - ROM_VADDR)
            compressed_data = rom.read(MAX_LEVEL * LEVEL_DATA_SIZE + 0x100)
            decompressed = decompress_at(compressed_data)

            if decompressed is None:
                return None

        # Parse level data
        level_data = []
        for level in range(MAX_LEVEL):
            offset = level * LEVEL_DATA_SIZE
            if offset + LEVEL_DATA_SIZE > len(decompressed):
                break

            exp_required, gain_hp, gain_att, gain_sp_att, gain_def, gain_sp_def = \
                LEVEL_DATA_STRUCT.unpack(decompressed[offset:offset+LEVEL_DATA_SIZE])

            level_data.append({
                'level': level + 1,
                'exp_required': exp_required,
                'gain_hp': gain_hp,
                'gain_att': gain_att,
                'gain_sp_att': gain_sp_att,
                'gain_def': gain_def,
                'gain_sp_def': gain_sp_def
            })

        return level_data


def load_monster_names(rom_path):
    """Load monster names from the monster_data.json file."""
    import json

    names = {}
    json_path = Path('data/monster/monster_data.json')

    if json_path.exists():
        try:
            with open(json_path, 'r') as f:
                monster_data = json.load(f)
                for i, mon in enumerate(monster_data):
                    # Extract clean name from the constant name
                    name = mon.get('name', f'Species_{i:03d}')
                    # Convert from "MonsterNameBulbasaur" to "Bulbasaur"
                    if name.startswith('MonsterName'):
                        name = name[11:]  # Remove "MonsterName" prefix
                    names[i] = name
        except Exception as e:
            print(f"Warning: Could not load monster names: {e}", file=sys.stderr)
            return {i: f'Species_{i:03d}' for i in range(NUM_MONSTERS)}
    else:
        print("Warning: monster_data.json not found, using generic names", file=sys.stderr)
        return {i: f'Species_{i:03d}' for i in range(NUM_MONSTERS)}

    return names


def main():
    rom_path = Path('pmd_red.gba')
    if not rom_path.exists():
        rom_path = Path('baserom.gba')
    if not rom_path.exists():
        print("Error: Could not find pmd_red.gba or baserom.gba", file=sys.stderr)
        sys.exit(1)

    output_file = 'pokemon_exp_curves.csv'

    print(f"Extracting EXP curve data from {rom_path}...")

    # Load monster names
    monster_names = load_monster_names(rom_path)

    # Open output CSV
    with open(output_file, 'w', newline='') as csvfile:
        fieldnames = ['species_id', 'species_name', 'level', 'exp_required',
                     'gain_hp', 'gain_att', 'gain_sp_att', 'gain_def', 'gain_sp_def']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        # Extract data for each species
        successful = 0
        failed = 0

        for species_id in range(1, NUM_MONSTERS):  # Start from 1, skip 0
            level_data = extract_level_data(rom_path, species_id)

            if level_data is None:
                failed += 1
                continue

            species_name = monster_names.get(species_id, f'Unknown_{species_id}')

            for entry in level_data:
                writer.writerow({
                    'species_id': species_id,
                    'species_name': species_name,
                    **entry
                })

            successful += 1
            if successful % 50 == 0:
                print(f"Processed {successful} species...")

        print(f"\nDone! Extracted data for {successful} species.")
        print(f"Failed to extract: {failed} species")
        print(f"Output written to: {output_file}")


if __name__ == '__main__':
    main()
