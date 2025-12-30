#!/usr/bin/env python3
import argparse
import binascii
import os
import re
import struct
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

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


def parse_palette_dirs(mon_dir):
    pattern = re.compile(r"^palette_(\d+)$")
    palettes = []
    for name in os.listdir(mon_dir):
        match = pattern.match(name)
        if match:
            palettes.append((int(match.group(1), 10), name))
    return sorted(palettes)


def load_idle_image(path):
    width, height, pixels, palette = decode_png_indexed(path)
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


def main():
    parser = argparse.ArgumentParser(
        description="Compose idle-direction palette grids per monster."
    )
    parser.add_argument(
        "--input-dir",
        default="build/shiny_idle_frames",
        help="Directory containing per-mon idle frames.",
    )
    parser.add_argument(
        "--output-dir",
        default="build/shiny_idle_grids",
        help="Destination directory for grid PNGs.",
    )
    parser.add_argument(
        "--mon",
        action="append",
        dest="mons",
        help="Monster directory name to process (repeatable).",
    )
    parser.add_argument(
        "--monster-data",
        default="data/monster/monster_data.json",
        help="Path to monster_data.json for vanilla palette lookup.",
    )
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)
    vanilla_palettes = load_vanilla_palettes(args.monster_data)

    for mon_name, mon_dir in iter_mon_dirs(args.input_dir, args.mons):
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
        for y in range(header_h):
            for x in range(grid_w):
                pos = (y * grid_w + x) * 4
                canvas[pos:pos + 4] = bytes((240, 240, 240, 255))

        mon_key = normalize_monster_name(mon_name)
        for col, (palette_idx, palette_dir) in enumerate(palette_dirs):
            label = str(palette_idx)
            label_w = (FONT_WIDTH * len(label)) + (FONT_SPACING * (len(label) - 1))
            label_x = col * max_w + (max_w - label_w) // 2
            label_y = 1
            is_vanilla = vanilla_palettes.get(mon_key) == palette_idx
            color = (220, 0, 0, 255) if is_vanilla else (0, 0, 0, 255)
            draw_text(
                canvas,
                grid_w,
                grid_h,
                label_x,
                label_y,
                label,
                color,
                bold=is_vanilla,
            )
            for row, direction in enumerate(DIRECTION_NAMES):
                img = images.get((palette_dir, direction))
                if img is None:
                    continue
                x0 = col * max_w + (max_w - img[0]) // 2
                y0 = header_h + row * max_h + (max_h - img[1]) // 2
                place_image(canvas, grid_w, grid_h, img, x0, y0)

        out_path = os.path.join(args.output_dir, f"{mon_name}.png")
        write_png_rgba(out_path, grid_w, grid_h, canvas)
        print(f"wrote {mon_name} -> {out_path}")


if __name__ == "__main__":
    main()
