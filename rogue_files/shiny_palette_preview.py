#!/usr/bin/env python3
import argparse
import binascii
import os
import re
import struct

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


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


def replace_palette(png_data, plte_data):
    chunks = read_png_chunks(png_data)
    replaced = False
    new_chunks = []
    for chunk_type, chunk_data in chunks:
        if chunk_type == b"PLTE":
            new_chunks.append((chunk_type, plte_data))
            replaced = True
        else:
            new_chunks.append((chunk_type, chunk_data))
    if not replaced:
        raise ValueError("PNG has no PLTE chunk")
    return write_png(new_chunks)


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


def pick_sprite_png(mon_dir, preferred):
    preferred_path = os.path.join(mon_dir, preferred)
    if os.path.isfile(preferred_path):
        return preferred_path

    best = None
    best_index = None
    pattern = re.compile(r"^sprite_(\d+)\.png$")
    for name in os.listdir(mon_dir):
        match = pattern.match(name)
        if match:
            idx = int(match.group(1))
            if best_index is None or idx < best_index:
                best_index = idx
                best = name
    if best:
        return os.path.join(mon_dir, best)

    for name in sorted(os.listdir(mon_dir)):
        if name.startswith("sprite_") and name.endswith(".png"):
            return os.path.join(mon_dir, name)
    return None


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


def main():
    parser = argparse.ArgumentParser(
        description="Generate per-palette previews for monster sprites."
    )
    parser.add_argument(
        "--mon",
        action="append",
        dest="mons",
        help="Monster dir name under graphics/ax/mon (repeatable).",
    )
    parser.add_argument(
        "--sprite",
        default="sprite_1.png",
        help="Preferred sprite PNG filename to use for previews.",
    )
    parser.add_argument(
        "--palette-dir",
        default="graphics/ax/pal",
        help="Directory containing base .pmdpal files.",
    )
    parser.add_argument(
        "--palette-count",
        type=int,
        default=13,
        help="Number of base palette rows to preview.",
    )
    parser.add_argument(
        "--output-dir",
        default="build/shiny_palette_previews",
        help="Destination root for output PNGs.",
    )
    args = parser.parse_args()

    base_dir = "graphics/ax/mon"
    os.makedirs(args.output_dir, exist_ok=True)

    palette_bytes = {}
    for idx in range(args.palette_count):
        pal_path = os.path.join(args.palette_dir, f"{idx}.pmdpal")
        colors = read_pmd_palette(pal_path)
        palette_bytes[idx] = palette_to_plte(colors, 16)

    for mon_name, mon_dir in iter_mon_dirs(base_dir, args.mons):
        sprite_path = pick_sprite_png(mon_dir, args.sprite)
        if sprite_path is None:
            print(f"skip {mon_name}: no sprite PNG found")
            continue
        with open(sprite_path, "rb") as f:
            png_data = f.read()

        out_dir = os.path.join(args.output_dir, mon_name)
        os.makedirs(out_dir, exist_ok=True)
        for idx, plte_data in palette_bytes.items():
            out_path = os.path.join(out_dir, f"palette_{idx:02d}.png")
            out_data = replace_palette(png_data, plte_data)
            with open(out_path, "wb") as f:
                f.write(out_data)

        print(f"wrote {mon_name} -> {out_dir}")


if __name__ == "__main__":
    main()
