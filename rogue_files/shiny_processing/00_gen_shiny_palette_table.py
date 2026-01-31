#!/usr/bin/env python3
import argparse
import binascii
import csv
import os
import re
import shutil
import struct
import sys
import zlib

MAX_PALETTE_ID = 12
MAX_REMAP_INDEX = 15
REMAP_COLUMNS = [f"index{i}_remap" for i in range(16)]

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

BASE_X_OFFSET = 0x100
BASE_Y_OFFSET = 0x200

DIRECTION_NAMES = [
    "south",
    "southeast",
    "east",
    "northeast",
    "north",
    "northwest",
    "west",
    "southwest",
]

ANIM_PRESETS = {
    "idle": {
        "anim_id": 7,
        "output_dir": "gen/shiny_idle_frames",
        "prefix": "idle",
    },
    "sleep": {
        "anim_id": 5,
        "output_dir": "gen/shiny_sleep_frames",
        "prefix": "sleep",
    },
    "attack": {
        "anim_id": 1,
        "output_dir": "gen/shiny_attack_frames",
        "prefix": "attack",
    },
}
DEFAULT_ANIM_SETS = ["idle", "sleep", "attack"]


def normalize_header_label(value):
    if value is None:
        return ""
    label = value.strip().lower()
    if not label:
        return ""
    label = re.sub(r"[^a-z0-9]+", "_", label)
    return label.strip("_")


def find_first_index(labels, candidates):
    for idx, label in enumerate(labels):
        if label in candidates:
            return idx
    return None


def resolve_remap_columns(labels):
    mapping = {}
    for idx in range(16):
        candidates = [
            str(idx),
            f"index{idx}_remap",
            f"index{idx}",
        ]
        col_idx = find_first_index(labels, set(candidates))
        if col_idx is None:
            return None
        mapping[idx] = col_idx
    return mapping


def get_cell(row, idx):
    if idx is None or idx >= len(row):
        return ""
    return row[idx].strip()


def ensure_gen_monster_data(dest_path, source_path):
    norm_dest = os.path.normpath(dest_path)
    parts = norm_dest.split(os.sep)
    if not parts or parts[0] != "gen":
        return
    if not os.path.isfile(source_path):
        return
    os.makedirs(os.path.dirname(norm_dest), exist_ok=True)
    try:
        shutil.copy2(source_path, norm_dest)
    except OSError as exc:
        print(f"warn: failed to copy {source_path} to {norm_dest}: {exc}")

FONT_DIGITS = {
    "0": [
        "111",
        "101",
        "101",
        "101",
        "111",
    ],
    "1": [
        "010",
        "110",
        "010",
        "010",
        "111",
    ],
    "2": [
        "111",
        "001",
        "111",
        "100",
        "111",
    ],
    "3": [
        "111",
        "001",
        "111",
        "001",
        "111",
    ],
    "4": [
        "101",
        "101",
        "111",
        "001",
        "001",
    ],
    "5": [
        "111",
        "100",
        "111",
        "001",
        "111",
    ],
    "6": [
        "111",
        "100",
        "111",
        "101",
        "111",
    ],
    "7": [
        "111",
        "001",
        "001",
        "001",
        "001",
    ],
    "8": [
        "111",
        "101",
        "111",
        "101",
        "111",
    ],
    "9": [
        "111",
        "101",
        "111",
        "001",
        "111",
    ],
}
FONT_HEIGHT = 5
FONT_WIDTH = 3
FONT_SPACING = 1


def load_shiny_palette_csv(path):
    with open(path, newline="") as csv_file:
        reader = csv.reader(csv_file)
        header = None
        id_idx = None
        palette_idx = None
        remap_columns = None
        for row in reader:
            if not row or all(not cell.strip() for cell in row):
                continue
            normalized = [normalize_header_label(cell) for cell in row]
            if not any(normalized):
                continue
            id_idx = find_first_index(normalized, {"id", "number"})
            palette_idx = find_first_index(
                normalized, {"shiny_palette_id", "shiny_palette"}
            )
            remap_columns = resolve_remap_columns(normalized)
            if id_idx is None or palette_idx is None or remap_columns is None:
                continue
            header = row
            break

        if header is None:
            raise ValueError("CSV header is missing or invalid")

        rows = []
        for row in reader:
            if not row or all(not cell.strip() for cell in row):
                continue
            try:
                monster_id = int(get_cell(row, id_idx))
            except (TypeError, ValueError) as exc:
                raise ValueError(f"Invalid id value: {get_cell(row, id_idx)}") from exc
            palette_raw = get_cell(row, palette_idx)
            if palette_raw == "" or palette_raw.lower() == "null":
                palette_id = 0
            else:
                try:
                    palette_id = int(palette_raw)
                except (TypeError, ValueError) as exc:
                    raise ValueError(
                        f"Invalid shiny_palette_id value: {palette_raw}"
                    ) from exc
            remap = []
            for idx in range(16):
                col_idx = remap_columns.get(idx)
                raw = get_cell(row, col_idx)
                if raw == "" or raw.lower() == "null":
                    remap.append(None)
                    continue
                try:
                    remap_val = int(raw)
                except (TypeError, ValueError) as exc:
                    raise ValueError(f"Invalid index{idx}_remap value: {raw}") from exc
                if remap_val < 0 or remap_val > MAX_REMAP_INDEX:
                    raise ValueError(
                        "index{}_remap out of range (0-{}): {}".format(
                            idx, MAX_REMAP_INDEX, remap_val
                        )
                    )
                remap.append(remap_val)
            rows.append((monster_id, palette_id, remap))

    if not rows:
        raise ValueError("CSV has no data rows")

    seen_ids = set()
    max_id = -1
    for monster_id, palette_id, _remap in rows:
        if monster_id < 0:
            raise ValueError(f"Negative id value: {monster_id}")
        if monster_id in seen_ids:
            raise ValueError(f"Duplicate id value: {monster_id}")
        if palette_id < 0 or palette_id > MAX_PALETTE_ID:
            raise ValueError(
                f"Palette id out of range (0-{MAX_PALETTE_ID}): {palette_id}"
            )
        seen_ids.add(monster_id)
        if monster_id > max_id:
            max_id = monster_id

    expected_count = max_id + 1
    if len(rows) != expected_count:
        missing = sorted(set(range(expected_count)) - seen_ids)
        raise ValueError(f"Missing ids in CSV: {missing[:10]}")

    values = [0] * expected_count
    remaps = [list(range(16)) for _ in range(expected_count)]
    remap_active = [0] * expected_count
    for monster_id, palette_id, remap in rows:
        values[monster_id] = palette_id
        table = list(range(16))
        active = 0
        for idx, remap_val in enumerate(remap):
            if remap_val is None:
                continue
            table[idx] = remap_val
            if remap_val != idx:
                active = 1
        remaps[monster_id] = table
        remap_active[monster_id] = active

    return values, remaps, remap_active


def write_table(values, remaps, remap_active, output_path, input_path):
    with open(output_path, "w", newline="\n") as out:
        out.write(
            "// Auto-generated by rogue_files/shiny_processing/00_gen_shiny_palette_table.py\n"
        )
        out.write(f"// Source: {input_path}\n\n")
        out.write('#include "global.h"\n')
        out.write('#include "constants/monster.h"\n')
        out.write('#include "shiny_palette_table.h"\n\n')
        out.write("const u8 gMonsterShinyPalette[MONSTER_MAX] = {\n")
        for i, value in enumerate(values):
            if i % 16 == 0:
                out.write("    ")
            out.write(f"{value},")
            if i % 16 == 15 or i == len(values) - 1:
                out.write("\n")
            else:
                out.write(" ")
        out.write("};\n")
        out.write("\n")
        out.write("const bool8 gMonsterShinyIndexRemapActive[MONSTER_MAX] = {\n")
        for i, value in enumerate(remap_active):
            if i % 16 == 0:
                out.write("    ")
            out.write(f"{value},")
            if i % 16 == 15 or i == len(remap_active) - 1:
                out.write("\n")
            else:
                out.write(" ")
        out.write("};\n")
        out.write("\n")
        out.write("const u8 gMonsterShinyIndexRemap[MONSTER_MAX][16] = {\n")
        for i, row in enumerate(remaps):
            out.write("    {")
            for j, value in enumerate(row):
                out.write(f"{value}")
                if j < len(row) - 1:
                    out.write(", ")
            if i < len(remaps) - 1:
                out.write("},\n")
            else:
                out.write("}\n")
        out.write("};\n")


def read_png_chunks(data):
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("Not a PNG file")
    offset = len(PNG_SIGNATURE)
    chunks = []
    while offset < len(data):
        if offset + 8 > len(data):
            raise ValueError("Truncated PNG chunk header")
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        chunk_type = data[offset + 4:offset + 8]
        chunk_data_start = offset + 8
        chunk_data_end = chunk_data_start + length
        if chunk_data_end + 4 > len(data):
            raise ValueError("Truncated PNG chunk data")
        chunk_data = data[chunk_data_start:chunk_data_end]
        chunks.append((chunk_type, chunk_data))
        offset = chunk_data_end + 4
    return chunks


def write_png(chunks):
    out = bytearray(PNG_SIGNATURE)
    for chunk_type, chunk_data in chunks:
        out += struct.pack(">I", len(chunk_data))
        out += chunk_type
        out += chunk_data
        crc = binascii.crc32(chunk_type)
        crc = binascii.crc32(chunk_data, crc) & 0xFFFFFFFF
        out += struct.pack(">I", crc)
    return bytes(out)


def paeth_predictor(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def decode_png_indexed(path):
    data = open(path, "rb").read()
    chunks = read_png_chunks(data)
    ihdr = None
    idat = []
    for chunk_type, chunk_data in chunks:
        if chunk_type == b"IHDR":
            ihdr = chunk_data
        elif chunk_type == b"IDAT":
            idat.append(chunk_data)
    if ihdr is None:
        raise ValueError(f"{path}: missing IHDR")
    width, height, bit_depth, color_type, comp, filt, interlace = struct.unpack(
        ">IIBBBBB", ihdr
    )
    if color_type != 3:
        raise ValueError(f"{path}: expected indexed PNG (color_type=3)")
    if bit_depth not in (4, 8):
        raise ValueError(f"{path}: unsupported bit depth {bit_depth}")
    if interlace != 0:
        raise ValueError(f"{path}: interlaced PNGs not supported")
    raw = zlib.decompress(b"".join(idat))

    row_bytes = (width * bit_depth + 7) // 8
    bpp = 1
    rows = []
    offset = 0
    prev = bytes([0] * row_bytes)
    for _ in range(height):
        if offset + 1 + row_bytes > len(raw):
            raise ValueError(f"{path}: truncated image data")
        filter_type = raw[offset]
        offset += 1
        row = bytearray(raw[offset:offset + row_bytes])
        offset += row_bytes
        recon = bytearray(row_bytes)
        if filter_type == 0:
            recon[:] = row
        elif filter_type == 1:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + left) & 0xFF
        elif filter_type == 2:
            for i in range(row_bytes):
                recon[i] = (row[i] + prev[i]) & 0xFF
        elif filter_type == 3:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + ((left + prev[i]) // 2)) & 0xFF
        elif filter_type == 4:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                up = prev[i]
                up_left = prev[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + paeth_predictor(left, up, up_left)) & 0xFF
        else:
            raise ValueError(f"{path}: unsupported filter {filter_type}")
        prev = bytes(recon)
        rows.append(recon)

    pixels = []
    if bit_depth == 8:
        for row in rows:
            pixels.append(list(row[:width]))
    else:
        for row in rows:
            row_pixels = []
            for byte in row:
                row_pixels.append((byte >> 4) & 0xF)
                if len(row_pixels) < width:
                    row_pixels.append(byte & 0xF)
                if len(row_pixels) >= width:
                    break
            pixels.append(row_pixels)

    return width, height, pixels


def decode_png_indexed_with_palette(path):
    data = open(path, "rb").read()
    chunks = read_png_chunks(data)
    ihdr = None
    plte = None
    idat = []
    for chunk_type, chunk_data in chunks:
        if chunk_type == b"IHDR":
            ihdr = chunk_data
        elif chunk_type == b"PLTE":
            plte = chunk_data
        elif chunk_type == b"IDAT":
            idat.append(chunk_data)
    if ihdr is None:
        raise ValueError(f"{path}: missing IHDR")
    if plte is None:
        raise ValueError(f"{path}: missing PLTE")

    width, height, bit_depth, color_type, comp, filt, interlace = struct.unpack(
        ">IIBBBBB", ihdr
    )
    if color_type != 3:
        raise ValueError(f"{path}: expected indexed PNG (color_type=3)")
    if bit_depth not in (4, 8):
        raise ValueError(f"{path}: unsupported bit depth {bit_depth}")
    if interlace != 0:
        raise ValueError(f"{path}: interlaced PNGs not supported")

    palette = []
    for i in range(0, len(plte), 3):
        palette.append((plte[i], plte[i + 1], plte[i + 2]))

    raw = zlib.decompress(b"".join(idat))
    row_bytes = (width * bit_depth + 7) // 8
    bpp = 1
    rows = []
    offset = 0
    prev = bytes([0] * row_bytes)
    for _ in range(height):
        if offset + 1 + row_bytes > len(raw):
            raise ValueError(f"{path}: truncated image data")
        filter_type = raw[offset]
        offset += 1
        row = bytearray(raw[offset:offset + row_bytes])
        offset += row_bytes
        recon = bytearray(row_bytes)
        if filter_type == 0:
            recon[:] = row
        elif filter_type == 1:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + left) & 0xFF
        elif filter_type == 2:
            for i in range(row_bytes):
                recon[i] = (row[i] + prev[i]) & 0xFF
        elif filter_type == 3:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + ((left + prev[i]) // 2)) & 0xFF
        elif filter_type == 4:
            for i in range(row_bytes):
                left = recon[i - bpp] if i >= bpp else 0
                up = prev[i]
                up_left = prev[i - bpp] if i >= bpp else 0
                recon[i] = (row[i] + paeth_predictor(left, up, up_left)) & 0xFF
        else:
            raise ValueError(f"{path}: unsupported filter {filter_type}")
        prev = bytes(recon)
        rows.append(recon)

    pixels = []
    if bit_depth == 8:
        for row in rows:
            pixels.append(list(row[:width]))
    else:
        for row in rows:
            row_pixels = []
            for byte in row:
                row_pixels.append((byte >> 4) & 0xF)
                if len(row_pixels) < width:
                    row_pixels.append(byte & 0xF)
                if len(row_pixels) >= width:
                    break
            pixels.append(row_pixels)

    return width, height, pixels, palette


def write_png_indexed(path, width, height, pixels, plte_data, bit_depth=4):
    if bit_depth not in (4, 8):
        raise ValueError("bit_depth must be 4 or 8")
    row_bytes = (width * bit_depth + 7) // 8
    raw_rows = []
    for row in pixels:
        packed = bytearray()
        if bit_depth == 8:
            packed.extend(row[:width])
        else:
            it = iter(row[:width])
            for left in it:
                right = next(it, 0)
                packed.append(((left & 0xF) << 4) | (right & 0xF))
        if len(packed) < row_bytes:
            packed.extend([0] * (row_bytes - len(packed)))
        raw_rows.append(bytes([0]) + packed)
    raw = b"".join(raw_rows)
    compressed = zlib.compress(raw)

    ihdr = struct.pack(
        ">IIBBBBB", width, height, bit_depth, 3, 0, 0, 0
    )
    chunks = [
        (b"IHDR", ihdr),
        (b"PLTE", plte_data),
        (b"IDAT", compressed),
        (b"IEND", b""),
    ]
    with open(path, "wb") as f:
        f.write(write_png(chunks))


def read_pmd_palette(path):
    data = open(path, "rb").read()
    if len(data) % 4 != 0:
        raise ValueError(f"Unexpected palette size for {path}")
    colors = []
    for i in range(0, len(data), 4):
        colors.append((data[i], data[i + 1], data[i + 2]))
    return colors


def palette_to_plte(colors, count=16):
    if len(colors) < count:
        raise ValueError("Palette does not have enough colors")
    out = bytearray()
    for r, g, b in colors[:count]:
        out += bytes((r, g, b))
    return bytes(out)


def strip_c_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*", "", text)
    return text


def split_args(text):
    args = []
    depth = 0
    start = 0
    for i, ch in enumerate(text):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 0:
            args.append(text[start:i].strip())
            start = i + 1
    tail = text[start:].strip()
    if tail:
        args.append(tail)
    return args


def extract_paren_content(text, start_idx):
    i = start_idx
    while i < len(text) and text[i].isspace():
        i += 1
    if i >= len(text) or text[i] != "(":
        raise ValueError("Expected '('")
    depth = 0
    begin = i + 1
    for j in range(i, len(text)):
        if text[j] == "(":
            depth += 1
        elif text[j] == ")":
            depth -= 1
            if depth == 0:
                return text[begin:j], j + 1
    raise ValueError("Unterminated '('")


def get_shape_size_dims(shape, size):
    if shape == 0:
        sizes = [(8, 8), (16, 16), (32, 32), (64, 64)]
    elif shape == 1:
        sizes = [(16, 8), (32, 8), (32, 16), (64, 32)]
    elif shape == 2:
        sizes = [(8, 16), (8, 32), (16, 32), (32, 64)]
    else:
        raise ValueError(f"Unknown shape {shape}")
    return sizes[size]


def eval_atom(expr, constants):
    expr = expr.strip()
    if expr in constants:
        return constants[expr]
    if expr.startswith("0x") or expr.startswith("-0x"):
        return int(expr, 16)
    if expr.startswith("-") and expr[1:].isdigit():
        return int(expr, 10)
    if expr.isdigit():
        return int(expr, 10)
    raise ValueError(f"Unknown token '{expr}'")


def eval_expr(expr, constants):
    expr = expr.strip()
    if expr.startswith("FLIP"):
        args_str, _ = extract_paren_content(expr, len("FLIP"))
        args = split_args(args_str)
        if len(args) != 2:
            raise ValueError(f"FLIP expects 2 args: {expr}")
        h = eval_atom(args[0], constants)
        v = eval_atom(args[1], constants)
        return (1 if h else 0) | (2 if v else 0)
    if expr.startswith("OAM1"):
        args_str, _ = extract_paren_content(expr, len("OAM1"))
        args = split_args(args_str)
        if len(args) != 3:
            raise ValueError(f"OAM1 expects 3 args: {expr}")
        y = eval_atom(args[0], constants)
        shape = eval_atom(args[1], constants)
        unk = eval_atom(args[2], constants)
        return (y & 0xFF) | ((shape & 3) << 14) | (unk << 8)
    if expr.startswith("OAM2"):
        args_str, _ = extract_paren_content(expr, len("OAM2"))
        args = split_args(args_str)
        if len(args) != 5:
            raise ValueError(f"OAM2 expects 5 args: {expr}")
        x = eval_atom(args[0], constants)
        size = eval_atom(args[1], constants)
        flip = eval_expr(args[2], constants)
        unk1 = eval_atom(args[3], constants)
        unk2 = eval_atom(args[4], constants)
        return (
            (x & 0x1FF)
            | ((size & 3) << 14)
            | (flip << (9 + 3))
            | (unk1 << 9)
            | (unk2 << 8)
        )
    if expr.startswith("OAM3"):
        args_str, _ = extract_paren_content(expr, len("OAM3"))
        args = split_args(args_str)
        if len(args) != 3:
            raise ValueError(f"OAM3 expects 3 args: {expr}")
        tile = eval_atom(args[0], constants)
        prio = eval_atom(args[1], constants)
        pal = eval_atom(args[2], constants)
        return (tile & 0x3FF) | ((prio & 3) << 10) | ((pal & 15) << 12)
    return eval_atom(expr, constants)


def parse_pose_block(block, constants):
    entries = []
    i = 0
    while True:
        idx = block.find("AX_POSE", i)
        if idx == -1:
            break
        if block.startswith("AX_POSE_TERMINATOR", idx):
            i = idx + len("AX_POSE_TERMINATOR")
            continue
        if block.startswith("AX_POSE_FULL", idx):
            macro = "AX_POSE_FULL"
        else:
            macro = "AX_POSE"
        args_str, end_idx = extract_paren_content(block, idx + len(macro))
        args = split_args(args_str)
        if macro == "AX_POSE":
            if len(args) != 4:
                raise ValueError(f"{macro} expects 4 args")
            sprite_id = eval_expr(args[0], constants)
            flags1 = eval_expr(args[1], constants)
            flags2 = eval_expr(args[2], constants)
            flags3 = eval_expr(args[3], constants)
            unk2_0 = 0
            unk2_1 = 0
        else:
            if len(args) != 6:
                raise ValueError(f"{macro} expects 6 args")
            sprite_id = eval_expr(args[0], constants)
            unk2_0 = eval_expr(args[1], constants)
            unk2_1 = eval_expr(args[2], constants)
            flags1 = eval_expr(args[3], constants)
            flags2 = eval_expr(args[4], constants)
            flags3 = eval_expr(args[5], constants)
        entries.append(
            {
                "sprite_id": sprite_id,
                "flags1": flags1,
                "flags2": flags2,
                "flags3": flags3,
                "unk2_0": unk2_0,
                "unk2_1": unk2_1,
            }
        )
        i = end_idx
    return entries


def find_array_block(text, name):
    pattern = re.compile(
        r"static const [^;]*\b" + re.escape(name) + r"\b\s*\[\]\s*=\s*\{(.*?)\};",
        re.S,
    )
    match = pattern.search(text)
    if not match:
        return None
    return match.group(1)


def parse_name_list(text, array_name):
    block = find_array_block(text, array_name)
    if block is None:
        return None
    block = strip_c_comments(block)
    items = []
    for token in block.split(","):
        token = token.strip()
        if not token:
            continue
        items.append(token)
    return items


def parse_anim_frames(block):
    frames = []
    depth = 0
    start = None
    for i, ch in enumerate(block):
        if ch == "{":
            if depth == 0:
                start = i + 1
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0 and start is not None:
                entry = block[start:i]
                start = None
                if "AX_ANIM_TERMINATOR" in entry:
                    continue
                pose_match = re.search(r"\.poseId\s*=\s*(-?\d+)", entry)
                if not pose_match:
                    continue
                pose_id = int(pose_match.group(1), 10)
                offset_match = re.search(
                    r"\.offset\s*=\s*\{\s*(-?\d+)\s*,\s*(-?\d+)\s*\}",
                    entry,
                )
                if offset_match:
                    off_x = int(offset_match.group(1), 10)
                    off_y = int(offset_match.group(2), 10)
                else:
                    off_x = 0
                    off_y = 0
                frames.append((pose_id, off_x, off_y))
    return frames


def parse_gfx_map(text):
    gfx_map = {}
    pattern = re.compile(
        r"static const u(?:8|16|32)\s+(\w+)\s*\[\]\s*=\s*INCBIN_U(?:8|16|32)\(\"([^\"]+)\"\);"
    )
    for match in pattern.finditer(text):
        name = match.group(1)
        path = match.group(2)
        gfx_map[name] = path
    return gfx_map


def parse_byte_count(expr):
    expr = expr.strip()
    if expr.startswith("0x") or expr.startswith("0X"):
        return int(expr, 16)
    if expr.isdigit():
        return int(expr, 10)
    return None


def parse_sprite_chunk_arrays(text):
    arrays = {}
    for match in re.finditer(
        r"static const ax_sprite\s+(\w+)\s*\[\]\s*=\s*\{(.*?)\};",
        text,
        re.S,
    ):
        name = match.group(1)
        block = strip_c_comments(match.group(2))
        entries = []
        for entry in re.finditer(r"\{\s*([^,]+)\s*,\s*([^}]+)\}", block):
            token = entry.group(1).strip()
            count_expr = entry.group(2).strip()
            if token == "NULL" and parse_byte_count(count_expr) == 0:
                break
            entries.append((token, count_expr))
        arrays[name] = entries
    return arrays


def parse_ax_data(text):
    axmain_match = re.search(
        r"(?:static\s+)?const\s+axmain\s+\w+\s*=\s*\{(.*?)\};",
        text,
        re.S,
    )
    if not axmain_match:
        raise ValueError("Could not locate axmain block")
    axmain_block = axmain_match.group(1)
    poses_name = re.search(r"\.poses\s*=\s*(\w+)", axmain_block)
    anims_name = re.search(r"\.animations\s*=\s*(\w+)", axmain_block)
    sprites_name = re.search(r"\.spriteData\s*=\s*(\w+)", axmain_block)
    if not poses_name or not anims_name or not sprites_name:
        raise ValueError("axmain is missing required fields")
    poses_name = poses_name.group(1)
    anims_name = anims_name.group(1)
    sprites_name = sprites_name.group(1)

    poses_list = parse_name_list(text, poses_name)
    anims_list = parse_name_list(text, anims_name)
    sprite_list = parse_name_list(text, sprites_name)
    if poses_list is None or anims_list is None or sprite_list is None:
        raise ValueError("Failed to parse ax lists")

    gfx_map = parse_gfx_map(text)
    sprite_arrays = parse_sprite_chunk_arrays(text)
    sprite_chunks = []
    for sprite_name in sprite_list:
        entries = sprite_arrays.get(sprite_name, [])
        chunk_entries = []
        for symbol, count_expr in entries:
            if symbol == "NULL":
                byte_count = parse_byte_count(count_expr)
                if byte_count is None:
                    continue
                tile_count = byte_count // 32
                if tile_count > 0:
                    chunk_entries.append({"kind": "blank", "tiles": tile_count})
                continue
            raw_path = gfx_map.get(symbol)
            if not raw_path:
                continue
            if raw_path.endswith(".4bpp"):
                raw_path = raw_path[:-5] + ".png"
            chunk_entries.append({"kind": "gfx", "path": raw_path})
        sprite_chunks.append(chunk_entries)

    constants = {
        "ST_OAM_SQUARE": 0,
        "ST_OAM_H_RECTANGLE": 1,
        "ST_OAM_V_RECTANGLE": 2,
        "ST_OAM_SIZE_0": 0,
        "ST_OAM_SIZE_1": 1,
        "ST_OAM_SIZE_2": 2,
        "ST_OAM_SIZE_3": 3,
    }

    pose_blocks = {}
    for match in re.finditer(
        r"static const ax_pose\s+(\w+)\s*\[\]\s*=\s*\{(.*?)\};",
        text,
        re.S,
    ):
        name = match.group(1)
        block = match.group(2)
        entries = parse_pose_block(block, constants)
        pose_blocks[name] = entries

    return poses_list, anims_list, sprite_chunks, pose_blocks


def tiles_from_pixels(width, height, pixels):
    tiles = []
    tiles_w = width // 8
    tiles_h = height // 8
    for ty in range(tiles_h):
        for tx in range(tiles_w):
            tile = []
            for py in range(8):
                row = []
                sy = ty * 8 + py
                for px in range(8):
                    sx = tx * 8 + px
                    row.append(pixels[sy][sx])
                tile.append(row)
            tiles.append(tile)
    return tiles


def build_tile_list(chunk_entries, png_cache):
    tiles = []
    for entry in chunk_entries:
        if entry["kind"] == "blank":
            for _ in range(entry["tiles"]):
                tiles.append([[0] * 8 for _ in range(8)])
            continue
        path = entry["path"]
        if not os.path.isfile(path):
            continue
        if path not in png_cache:
            png_cache[path] = decode_png_indexed(path)
        width, height, pixels = png_cache[path]
        tiles.extend(tiles_from_pixels(width, height, pixels))
    return tiles


def extract_oam_sprite(tile_list, tile_num, width_px, height_px):
    tiles_w = width_px // 8
    tiles_h = height_px // 8
    out = [[0 for _ in range(width_px)] for _ in range(height_px)]
    for ty in range(tiles_h):
        for tx in range(tiles_w):
            tile_index = tile_num + (ty * tiles_w) + tx
            if tile_index >= len(tile_list):
                continue
            tile = tile_list[tile_index]
            for py in range(8):
                for px in range(8):
                    out[ty * 8 + py][tx * 8 + px] = tile[py][px]
    return out


def apply_flips(pixels, hflip, vflip):
    out = pixels
    if hflip:
        out = [list(reversed(row)) for row in out]
    if vflip:
        out = list(reversed(out))
    return out


def get_sprite_tiles(mon_dir, sprite_cache, png_cache, sprite_chunks, sprite_id):
    tile_list = sprite_cache.get(sprite_id)
    if tile_list is not None:
        return tile_list
    chunk_entries = []
    if 0 <= sprite_id < len(sprite_chunks):
        chunk_entries = sprite_chunks[sprite_id]
    if not chunk_entries:
        sprite_path = os.path.join(mon_dir, f"sprite_{sprite_id + 1}.png")
        chunk_entries = [{"kind": "gfx", "path": sprite_path}]
    tile_list = build_tile_list(chunk_entries, png_cache)
    sprite_cache[sprite_id] = tile_list
    return tile_list


def build_pose_parts(mon_dir, sprite_cache, png_cache, sprite_chunks, entries, offset):
    pose_tiles = []
    seen_ids = set()
    for entry in entries:
        sprite_id = entry["sprite_id"]
        if sprite_id < 0:
            continue
        if sprite_id in seen_ids:
            continue
        seen_ids.add(sprite_id)
        tile_list = get_sprite_tiles(
            mon_dir, sprite_cache, png_cache, sprite_chunks, sprite_id
        )
        if tile_list:
            pose_tiles.extend(tile_list)

    parts = []
    for entry in entries:
        sprite_id = entry["sprite_id"]
        if sprite_id < 0:
            continue

        flags1 = entry["flags1"]
        flags2 = entry["flags2"]
        flags3 = entry["flags3"]

        shape = (flags1 >> 14) & 3
        size = (flags2 >> 14) & 3
        width_px, height_px = get_shape_size_dims(shape, size)
        tile_num = flags3 & 0x3FF

        x = (flags2 & 0x1FF) - BASE_X_OFFSET + offset[0]
        y = (flags1 & 0x3FF) - BASE_Y_OFFSET + offset[1]

        hflip = (flags2 >> 12) & 1
        vflip = (flags2 >> 13) & 1

        sprite_pixels = extract_oam_sprite(
            pose_tiles, tile_num, width_px, height_px
        )
        sprite_pixels = apply_flips(sprite_pixels, hflip, vflip)

        parts.append((x, y, sprite_pixels, width_px, height_px))

    return parts


def render_pose(mon_dir, sprite_cache, png_cache, sprite_chunks, entries, offset):
    parts = build_pose_parts(
        mon_dir, sprite_cache, png_cache, sprite_chunks, entries, offset
    )
    if not parts:
        return [[0]]

    min_x = min(p[0] for p in parts)
    min_y = min(p[1] for p in parts)
    max_x = max(p[0] + p[3] for p in parts)
    max_y = max(p[1] + p[4] for p in parts)
    out_w = max_x - min_x
    out_h = max_y - min_y
    if out_w <= 0:
        out_w = 1
    if out_h <= 0:
        out_h = 1
    canvas = [[0 for _ in range(out_w)] for _ in range(out_h)]

    for x, y, sprite_pixels, width_px, height_px in parts:
        dest_x = x - min_x
        dest_y = y - min_y
        for py in range(height_px):
            row = sprite_pixels[py]
            for px in range(width_px):
                idx = row[px]
                if idx == 0:
                    continue
                cx = dest_x + px
                cy = dest_y + py
                if 0 <= cx < out_w and 0 <= cy < out_h:
                    canvas[cy][cx] = idx

    return crop_canvas(canvas)


def crop_canvas(canvas):
    if not canvas:
        return [[0]]
    height = len(canvas)
    width = len(canvas[0])
    min_x = width
    min_y = height
    max_x = -1
    max_y = -1
    for y, row in enumerate(canvas):
        for x, val in enumerate(row):
            if val != 0:
                if x < min_x:
                    min_x = x
                if y < min_y:
                    min_y = y
                if x > max_x:
                    max_x = x
                if y > max_y:
                    max_y = y
    if max_x < 0 or max_y < 0:
        return [[0]]
    cropped = []
    for y in range(min_y, max_y + 1):
        cropped.append(canvas[y][min_x:max_x + 1])
    return cropped


def iter_mon_dirs(base_dir, filters):
    if filters:
        for name in filters:
            path = os.path.join(base_dir, name)
            if os.path.isdir(path):
                yield name, path
        return
    for name in sorted(os.listdir(base_dir)):
        path = os.path.join(base_dir, name)
        if os.path.isdir(path):
            yield name, path


def parse_palette_rows(arg):
    rows = []
    for part in arg.split(","):
        part = part.strip()
        if not part:
            continue
        rows.append(int(part, 10))
    return rows


def indexed_to_rgba(width, height, pixels, palette, transparent_index=0):
    out = bytearray(width * height * 4)
    for y in range(height):
        row = pixels[y]
        for x in range(width):
            idx = row[x]
            r, g, b = palette[idx]
            a = 0 if idx == transparent_index else 255
            pos = (y * width + x) * 4
            out[pos:pos + 4] = bytes((r, g, b, a))
    return out


def write_png_rgba(path, width, height, rgba_bytes):
    raw_rows = []
    stride = width * 4
    for y in range(height):
        start = y * stride
        raw_rows.append(b"\x00" + rgba_bytes[start:start + stride])
    raw = b"".join(raw_rows)
    compressed = zlib.compress(raw)

    ihdr = struct.pack(
        ">IIBBBBB", width, height, 8, 6, 0, 0, 0
    )
    chunks = [
        (b"IHDR", ihdr),
        (b"IDAT", compressed),
        (b"IEND", b""),
    ]
    with open(path, "wb") as f:
        f.write(write_png(chunks))


def parse_palette_dirs(mon_dir):
    pattern = re.compile(r"^palette_(\d+)$")
    palettes = []
    for name in os.listdir(mon_dir):
        match = pattern.match(name)
        if match:
            palettes.append((int(match.group(1), 10), name))
    return sorted(palettes)


def load_idle_image(path):
    width, height, pixels, palette = decode_png_indexed_with_palette(path)
    rgba = indexed_to_rgba(width, height, pixels, palette)
    return width, height, rgba


def place_image(canvas, canvas_w, canvas_h, img, x0, y0):
    width, height, rgba = img
    for y in range(height):
        for x in range(width):
            pos = (y * width + x) * 4
            a = rgba[pos + 3]
            if a == 0:
                continue
            cx = x0 + x
            cy = y0 + y
            if 0 <= cx < canvas_w and 0 <= cy < canvas_h:
                dst = (cy * canvas_w + cx) * 4
                canvas[dst:dst + 4] = rgba[pos:pos + 4]


def draw_digit(canvas, canvas_w, canvas_h, x0, y0, digit, color):
    pattern = FONT_DIGITS.get(digit)
    if not pattern:
        return
    r, g, b, a = color
    for y, row in enumerate(pattern):
        for x, ch in enumerate(row):
            if ch != "1":
                continue
            cx = x0 + x
            cy = y0 + y
            if 0 <= cx < canvas_w and 0 <= cy < canvas_h:
                pos = (cy * canvas_w + cx) * 4
                canvas[pos:pos + 4] = bytes((r, g, b, a))


def draw_text(canvas, canvas_w, canvas_h, x0, y0, text, color, bold=False):
    x = x0
    for ch in text:
        if ch.isdigit():
            draw_digit(canvas, canvas_w, canvas_h, x, y0, ch, color)
            if bold:
                draw_digit(canvas, canvas_w, canvas_h, x + 1, y0, ch, color)
            x += FONT_WIDTH + FONT_SPACING
        else:
            x += FONT_WIDTH + FONT_SPACING


def normalize_monster_name(name):
    if name.startswith("MonsterName"):
        name = name[len("MonsterName"):]
    name = re.sub(r"[^A-Za-z0-9]", "", name)
    return name.lower()


def load_vanilla_palettes(path):
    if not os.path.isfile(path):
        return {}
    data = json_load(path)
    result = {}
    for entry in data:
        name = entry.get("name", "")
        palette = entry.get("overworldPalette")
        if palette is None:
            continue
        norm = normalize_monster_name(name)
        result[norm] = int(palette)
    return result


def json_load(path):
    import json

    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def run_table(args):
    values, remaps, remap_active = load_shiny_palette_csv(args.input)
    write_table(values, remaps, remap_active, args.output, args.input)


def run_anim_frames(args, anim_id, output_dir, prefix):
    base_dir = "graphics/ax/mon"
    os.makedirs(output_dir, exist_ok=True)

    if args.frames_palette_rows:
        palette_rows = parse_palette_rows(args.frames_palette_rows)
    else:
        palette_rows = list(range(args.frames_palette_count))

    palette_bytes = {}
    missing_rows = []
    for idx in palette_rows:
        pal_path = os.path.join(args.frames_palette_dir, f"{idx}.pmdpal")
        if not os.path.isfile(pal_path):
            missing_rows.append(idx)
            continue
        colors = read_pmd_palette(pal_path)
        palette_bytes[idx] = palette_to_plte(colors, 16)

    if missing_rows:
        existing = sorted(palette_bytes.keys())
        if not existing:
            raise FileNotFoundError(
                f"No palette files found in {args.frames_palette_dir}"
            )
        for idx in missing_rows:
            fallback = max((i for i in existing if i < idx), default=existing[0])
            palette_bytes[idx] = palette_bytes[fallback]
            print(
                f"warn: missing palette {idx}, using {fallback} instead"
            )

    for mon_name, mon_dir in iter_mon_dirs(base_dir, args.frames_mons):
        ax_path = os.path.join("src", "data", "ax", f"{mon_name}.h")
        if not os.path.isfile(ax_path):
            print(f"skip {mon_name}: missing {ax_path}")
            continue

        text = open(ax_path, "r", encoding="utf-8", errors="ignore").read()
        try:
            poses_list, anims_list, sprite_chunks, pose_blocks = parse_ax_data(text)
        except ValueError as exc:
            print(f"skip {mon_name}: {exc}")
            continue
        if anim_id >= len(anims_list):
            print(f"skip {mon_name}: anim_id {anim_id} out of range")
            continue
        anim_table_name = anims_list[anim_id]
        dir_anim_names = parse_name_list(text, anim_table_name)
        if dir_anim_names is None or len(dir_anim_names) < len(DIRECTION_NAMES):
            print(f"skip {mon_name}: missing anim table {anim_table_name}")
            continue

        sprite_cache = {}
        png_cache = {}
        for dir_idx, dir_name in enumerate(DIRECTION_NAMES):
            anim_name = dir_anim_names[dir_idx]
            anim_block = find_array_block(text, anim_name)
            if anim_block is None:
                print(f"skip {mon_name}: missing anim {anim_name}")
                break
            frames = parse_anim_frames(anim_block)
            if not frames:
                print(f"skip {mon_name} {dir_name}: no frames found")
                continue

            resolved_frames = []
            last_pose_id = None
            for pose_id, off_x, off_y in frames:
                resolved_pose = pose_id
                if resolved_pose < 0:
                    if last_pose_id is None:
                        resolved_frames.append(None)
                        continue
                    resolved_pose = last_pose_id
                if resolved_pose >= len(poses_list):
                    print(
                        f"skip {mon_name} {dir_name}: pose_id {resolved_pose} out of range"
                    )
                    resolved_frames.append(None)
                    continue
                last_pose_id = resolved_pose
                resolved_frames.append((resolved_pose, off_x, off_y))

            base_frame = args.frames_frame_index
            if base_frame < 0 or base_frame >= len(resolved_frames):
                base_frame = 0
            if resolved_frames and resolved_frames[base_frame] is None:
                base_frame = next(
                    (idx for idx, entry in enumerate(resolved_frames) if entry is not None),
                    base_frame,
                )

            frame_parts = []
            frame_bounds = []
            for frame_idx, entry in enumerate(resolved_frames):
                if entry is None:
                    frame_parts.append([])
                    continue
                pose_id, off_x, off_y = entry
                pose_name = poses_list[pose_id]
                entries = pose_blocks.get(pose_name)
                if entries is None:
                    print(
                        f"skip {mon_name} {dir_name}: missing pose {pose_name}"
                    )
                    frame_parts.append([])
                    continue
                parts = build_pose_parts(
                    mon_dir,
                    sprite_cache,
                    png_cache,
                    sprite_chunks,
                    entries,
                    (off_x, off_y),
                )
                if not parts:
                    frame_parts.append([])
                    continue
                min_x = min(p[0] for p in parts)
                min_y = min(p[1] for p in parts)
                max_x = max(p[0] + p[3] for p in parts)
                max_y = max(p[1] + p[4] for p in parts)
                frame_bounds.append((min_x, min_y, max_x, max_y))
                frame_parts.append(parts)

            if not frame_bounds:
                print(f"skip {mon_name} {dir_name}: no drawable frames")
                continue
            global_min_x = min(b[0] for b in frame_bounds)
            global_min_y = min(b[1] for b in frame_bounds)
            global_max_x = max(b[2] for b in frame_bounds)
            global_max_y = max(b[3] for b in frame_bounds)
            out_w = global_max_x - global_min_x
            out_h = global_max_y - global_min_y
            if out_w <= 0:
                out_w = 1
            if out_h <= 0:
                out_h = 1

            frame_canvases = []
            for parts in frame_parts:
                if parts is None:
                    parts = []
                canvas = [[0 for _ in range(out_w)] for _ in range(out_h)]
                for x, y, sprite_pixels, width_px, height_px in parts:
                    dest_x = x - global_min_x
                    dest_y = y - global_min_y
                    for py in range(height_px):
                        row = sprite_pixels[py]
                        for px in range(width_px):
                            idx = row[px]
                            if idx == 0:
                                continue
                            cx = dest_x + px
                            cy = dest_y + py
                            if 0 <= cx < out_w and 0 <= cy < out_h:
                                canvas[cy][cx] = idx
                frame_canvases.append(canvas)

            for palette_idx, plte_data in palette_bytes.items():
                out_dir = os.path.join(
                    output_dir, mon_name, f"palette_{palette_idx:02d}"
                )
                os.makedirs(out_dir, exist_ok=True)
                for frame_idx, canvas in enumerate(frame_canvases):
                    if canvas is None:
                        continue
                    out_path = os.path.join(
                        out_dir, f"{prefix}_{dir_name}_frame{frame_idx:02d}.png"
                    )
                    write_png_indexed(
                        out_path,
                        len(canvas[0]),
                        len(canvas),
                        canvas,
                        plte_data,
                        bit_depth=4,
                    )
                    if frame_idx == base_frame:
                        base_path = os.path.join(out_dir, f"{prefix}_{dir_name}.png")
                        write_png_indexed(
                            base_path,
                            len(canvas[0]),
                            len(canvas),
                            canvas,
                            plte_data,
                            bit_depth=4,
                        )
        print(f"wrote {mon_name} ({prefix})")


def run_idle_frames(args):
    run_anim_frames(args, args.frames_anim_id, args.frames_output_dir, "idle")


def run_idle_grid(args):
    os.makedirs(args.grid_output_dir, exist_ok=True)
    vanilla_palettes = load_vanilla_palettes(args.grid_monster_data)

    for mon_name, mon_dir in iter_mon_dirs(args.grid_input_dir, args.grid_mons):
        palette_dirs = parse_palette_dirs(mon_dir)
        if not palette_dirs:
            print(f"skip {mon_name}: no palette dirs")
            continue

        images = {}
        max_w = 1
        max_h = 1
        for palette_idx, palette_dir in palette_dirs:
            for direction in DIRECTION_NAMES:
                path = os.path.join(mon_dir, palette_dir, f"idle_{direction}.png")
                if not os.path.isfile(path):
                    continue
                img = load_idle_image(path)
                images[(palette_dir, direction)] = img
                max_w = max(max_w, img[0])
                max_h = max(max_h, img[1])

        if max_w <= 0 or max_h <= 0:
            print(f"skip {mon_name}: no images found")
            continue

        header_h = FONT_HEIGHT + 2
        grid_w = max_w * len(palette_dirs)
        grid_h = header_h + (max_h * len(DIRECTION_NAMES))
        canvas = bytearray(grid_w * grid_h * 4)

        for col_idx, (palette_idx, palette_dir) in enumerate(palette_dirs):
            x0 = col_idx * max_w
            label = f"{palette_idx:02d}"
            label_w = (FONT_WIDTH * len(label)) + (FONT_SPACING * (len(label) - 1))
            label_x = x0 + max(0, (max_w - label_w) // 2)
            label_y = 1
            draw_text(
                canvas,
                grid_w,
                grid_h,
                label_x,
                label_y,
                label,
                (255, 255, 255, 255),
                bold=True,
            )

            for row_idx, direction in enumerate(DIRECTION_NAMES):
                y0 = header_h + row_idx * max_h
                key = (palette_dir, direction)
                img = images.get(key)
                if img is None:
                    continue
                place_image(canvas, grid_w, grid_h, img, x0, y0)

        norm = normalize_monster_name(mon_name)
        vanilla_palette = vanilla_palettes.get(norm)
        if vanilla_palette is not None:
            banner = f"vanilla {vanilla_palette:02d}"
            banner_w = (FONT_WIDTH * len(banner)) + (FONT_SPACING * (len(banner) - 1))
            banner_x = max(0, (grid_w - banner_w) // 2)
            banner_y = grid_h - FONT_HEIGHT - 1
            draw_text(
                canvas,
                grid_w,
                grid_h,
                banner_x,
                banner_y,
                banner,
                (255, 215, 0, 255),
            )

        out_path = os.path.join(args.grid_output_dir, f"{mon_name}.png")
        write_png_rgba(out_path, grid_w, grid_h, canvas)


def main():
    parser = argparse.ArgumentParser(
        description="Generate shiny palette table, animation frames, and grids."
    )
    parser.add_argument(
        "--mode",
        default="all",
        choices=["all", "table", "frames", "grid"],
        help="Select which steps to run (default: all).",
    )
    parser.add_argument(
        "--input",
        default="rogue_files/shiny_palette.csv",
        help="Path to shiny_palette.csv (table generation).",
    )
    parser.add_argument(
        "--output",
        default="src/data/shiny_palette_table.c",
        help="Path to output .c file (table generation).",
    )
    parser.add_argument(
        "--frames-mon",
        action="append",
        dest="frames_mons",
        help="Monster dir name under graphics/ax/mon (repeatable).",
    )
    parser.add_argument(
        "--frames-anim-id",
        type=int,
        default=7,
        help="Animation ID to use (default: 7 for idle).",
    )
    parser.add_argument(
        "--frames-anim",
        action="append",
        dest="frames_anims",
        choices=DEFAULT_ANIM_SETS,
        help="Animation preset to render (repeatable). Default: idle,sleep,attack.",
    )
    parser.add_argument(
        "--frames-frame-index",
        type=int,
        default=0,
        help="Frame index within the animation sequence (default: 0).",
    )
    parser.add_argument(
        "--frames-palette-dir",
        default="graphics/ax/pal",
        help="Directory containing base .pmdpal files.",
    )
    parser.add_argument(
        "--frames-palette-count",
        type=int,
        default=16,
        help="Number of base palette rows to preview (starting at 0).",
    )
    parser.add_argument(
        "--frames-palette-rows",
        default=None,
        help="Comma-separated palette row list (overrides --frames-palette-count).",
    )
    parser.add_argument(
        "--frames-output-dir",
        default="gen/shiny_idle_frames",
        help="Destination root for output PNGs.",
    )
    parser.add_argument(
        "--grid-input-dir",
        default="gen/shiny_idle_frames",
        help="Directory containing per-mon idle frames.",
    )
    parser.add_argument(
        "--grid-output-dir",
        default="gen/shiny_idle_grids",
        help="Destination directory for grid PNGs.",
    )
    parser.add_argument(
        "--grid-mon",
        action="append",
        dest="grid_mons",
        help="Monster directory name to process (repeatable).",
    )
    parser.add_argument(
        "--grid-monster-data",
        default="gen/monster_data.json",
        help="Path to monster_data.json for vanilla palette lookup.",
    )
    args = parser.parse_args()

    try:
        ensure_gen_monster_data(args.grid_monster_data, "data/monster/monster_data.json")
        if args.mode in ("all", "table"):
            run_table(args)
        if args.mode in ("all", "frames"):
            frame_sets = args.frames_anims or DEFAULT_ANIM_SETS
            for anim_name in frame_sets:
                if anim_name == "idle":
                    run_anim_frames(
                        args,
                        args.frames_anim_id,
                        args.frames_output_dir,
                        "idle",
                    )
                    continue
                preset = ANIM_PRESETS.get(anim_name)
                if not preset:
                    continue
                run_anim_frames(
                    args,
                    preset["anim_id"],
                    preset["output_dir"],
                    preset["prefix"],
                )
        if args.mode in ("all", "grid"):
            run_idle_grid(args)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
