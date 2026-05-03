#!/usr/bin/env python3
"""Generate per-monster idle-animation GIFs (normal, shiny, side-by-side).

For each Pokemon, writes three GIFs with transparent backgrounds into
gen/shiny_gifs/:

  {mon}_normal.gif   - idle facing south using the vanilla palette
  {mon}_shiny.gif    - idle facing south using the shiny palette + remap
  {mon}_compare.gif  - normal on the left, shiny on the right (single GIF)

Reuses parsing/rendering helpers from 00_gen_shiny_palette_table.py and
implements a stdlib-only GIF89a + LZW encoder (no Pillow dependency).
"""
import argparse
import csv
import importlib.util
import os
import re
import struct
import sys


HERE = os.path.dirname(os.path.abspath(__file__))


def _load_helpers():
    spec = importlib.util.spec_from_file_location(
        "shiny_table_gen",
        os.path.join(HERE, "00_gen_shiny_palette_table.py"),
    )
    if spec is None or spec.loader is None:
        raise ImportError("Could not load 00_gen_shiny_palette_table.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


helpers = _load_helpers()
DIRECTION_NAMES = helpers.DIRECTION_NAMES
SOUTH_INDEX = 0
GAME_FPS = 60


def load_dir_to_species_id(table_path):
    """Parse src/monster_files_table.c into a map from ax sprite directory
    name to species ID. Needed because data/monster/monster_data.json and the
    CSV CodeName column collapse all form variants (Deoxys/Castform/Unown) to
    a single name, so name-based lookup is ambiguous."""
    if not os.path.isfile(table_path):
        return {}
    with open(table_path, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()
    pattern = re.compile(
        r'\[\s*MONSTER_\w+\s*-\s*1\s*\]\s*=\s*\{\s*"ax(\d+)"\s*,\s*&gAx(\w+)\s*,?\s*\}'
    )
    out = {}
    for match in pattern.finditer(text):
        species_id = int(match.group(1), 10)
        dir_name = match.group(2).lower()
        out[dir_name] = species_id
    return out


def load_vanilla_palettes_by_id(monster_data_path):
    if not os.path.isfile(monster_data_path):
        return []
    data = helpers.json_load(monster_data_path)
    out = []
    for entry in data:
        pal = entry.get("overworldPalette")
        out.append(int(pal) if pal is not None else None)
    return out


def parse_anim_frames_with_duration(block):
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
                off_x = 0
                off_y = 0
                if offset_match:
                    off_x = int(offset_match.group(1), 10)
                    off_y = int(offset_match.group(2), 10)
                duration_match = re.search(r"\.frames\s*=\s*(-?\d+)", entry)
                duration = (
                    int(duration_match.group(1), 10) if duration_match else 1
                )
                if duration <= 0:
                    duration = 1
                frames.append((pose_id, off_x, off_y, duration))
    return frames


def render_south_idle(mon_name, mon_dir, anim_id):
    ax_path = os.path.join("src", "data", "ax", f"{mon_name}.h")
    if not os.path.isfile(ax_path):
        return None
    with open(ax_path, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()
    try:
        poses_list, anims_list, sprite_chunks, pose_blocks = (
            helpers.parse_ax_data(text)
        )
    except ValueError:
        return None
    if anim_id >= len(anims_list):
        return None
    table_name = anims_list[anim_id]
    dir_anims = helpers.parse_name_list(text, table_name)
    if not dir_anims or len(dir_anims) <= SOUTH_INDEX:
        return None
    south_name = dir_anims[SOUTH_INDEX]
    block = helpers.find_array_block(text, south_name)
    if block is None:
        return None
    raw_frames = parse_anim_frames_with_duration(block)
    if not raw_frames:
        return None

    sprite_cache = {}
    png_cache = {}

    last_pose = None
    resolved = []
    for pose_id, off_x, off_y, dur in raw_frames:
        rp = pose_id
        if rp < 0:
            if last_pose is None:
                continue
            rp = last_pose
        if rp >= len(poses_list):
            continue
        last_pose = rp
        resolved.append((rp, off_x, off_y, dur))
    if not resolved:
        return None

    frame_data = []
    bounds = []
    for pose_id, off_x, off_y, dur in resolved:
        pose_name = poses_list[pose_id]
        entries = pose_blocks.get(pose_name)
        parts = []
        if entries:
            parts = helpers.build_pose_parts(
                mon_dir,
                sprite_cache,
                png_cache,
                sprite_chunks,
                entries,
                (off_x, off_y),
            )
        if parts:
            min_x = min(p[0] for p in parts)
            min_y = min(p[1] for p in parts)
            max_x = max(p[0] + p[3] for p in parts)
            max_y = max(p[1] + p[4] for p in parts)
            bounds.append((min_x, min_y, max_x, max_y))
        frame_data.append((parts, dur))

    if not bounds:
        return None
    g_min_x = min(b[0] for b in bounds)
    g_min_y = min(b[1] for b in bounds)
    g_max_x = max(b[2] for b in bounds)
    g_max_y = max(b[3] for b in bounds)
    out_w = max(1, g_max_x - g_min_x)
    out_h = max(1, g_max_y - g_min_y)

    canvases = []
    for parts, dur in frame_data:
        canvas = [[0] * out_w for _ in range(out_h)]
        for x, y, sprite_pixels, w_px, h_px in parts:
            dx = x - g_min_x
            dy = y - g_min_y
            for py in range(h_px):
                row = sprite_pixels[py]
                for px in range(w_px):
                    idx = row[px]
                    if idx == 0:
                        continue
                    cx = dx + px
                    cy = dy + py
                    if 0 <= cx < out_w and 0 <= cy < out_h:
                        canvas[cy][cx] = idx
        canvases.append((canvas, dur))

    return out_w, out_h, canvases


def load_csv_codename_map(csv_path):
    """Returns (codename_to_id, shiny_palette_by_id, remap_by_id)."""
    values, remaps, _active = helpers.load_shiny_palette_csv(csv_path)
    code_to_id = {}
    with open(csv_path, newline="") as f:
        reader = csv.reader(f)
        id_idx = None
        name_idx = None
        header_done = False
        for row in reader:
            if not row or all(not c.strip() for c in row):
                continue
            if not header_done:
                normalized = [helpers.normalize_header_label(c) for c in row]
                if not any(normalized):
                    continue
                cand_id = helpers.find_first_index(
                    normalized, {"id", "number"}
                )
                cand_name = helpers.find_first_index(
                    normalized, {"codename", "code_name"}
                )
                if cand_id is not None and cand_name is not None:
                    id_idx = cand_id
                    name_idx = cand_name
                    header_done = True
                continue
            if id_idx is None or name_idx is None:
                continue
            try:
                mon_id = int(row[id_idx].strip())
            except (ValueError, IndexError):
                continue
            if name_idx >= len(row):
                continue
            name_value = row[name_idx].strip()
            if not name_value:
                continue
            code_to_id[helpers.normalize_monster_name(name_value)] = mon_id
    return code_to_id, values, remaps


def load_palette_colors(pal_dir, palette_id):
    path = os.path.join(pal_dir, f"{palette_id}.pmdpal")
    if not os.path.isfile(path):
        return None
    return helpers.read_pmd_palette(path)


def frames_to_centiseconds(frames):
    return max(2, round(frames * 100 / GAME_FPS))


class _BitWriter:
    def __init__(self):
        self._buf = bytearray()
        self._cur = 0
        self._bits = 0

    def write(self, value, n_bits):
        self._cur |= (value & ((1 << n_bits) - 1)) << self._bits
        self._bits += n_bits
        while self._bits >= 8:
            self._buf.append(self._cur & 0xFF)
            self._cur >>= 8
            self._bits -= 8

    def to_bytes(self):
        if self._bits > 0:
            self._buf.append(self._cur & 0xFF)
            self._cur = 0
            self._bits = 0
        return bytes(self._buf)


def _lzw_compress(indices, min_code_size):
    if min_code_size < 2:
        min_code_size = 2
    clear_code = 1 << min_code_size
    eoi_code = clear_code + 1
    code_size = min_code_size + 1

    table = {(i,): i for i in range(clear_code)}
    next_code = eoi_code + 1

    bw = _BitWriter()
    bw.write(clear_code, code_size)

    w = ()
    for k in indices:
        wk = w + (k,)
        if wk in table:
            w = wk
            continue
        bw.write(table[w], code_size)
        if next_code < 4096:
            if next_code == (1 << code_size) and code_size < 12:
                code_size += 1
            table[wk] = next_code
            next_code += 1
        else:
            bw.write(clear_code, code_size)
            code_size = min_code_size + 1
            table = {(i,): i for i in range(clear_code)}
            next_code = eoi_code + 1
        w = (k,)
    if w:
        bw.write(table[w], code_size)
    bw.write(eoi_code, code_size)
    return bw.to_bytes()


def _split_subblocks(data):
    out = bytearray()
    i = 0
    while i < len(data):
        chunk = data[i:i + 255]
        out.append(len(chunk))
        out.extend(chunk)
        i += 255
    out.append(0)
    return bytes(out)


def _palette_size_field(n_colors):
    size = 2
    field = 0
    while size < n_colors:
        size *= 2
        field += 1
    return size, field


def encode_gif(width, height, frames, palette, transparent_idx=0):
    n_in = max(2, len(palette))
    padded_size, size_field = _palette_size_field(n_in)
    if padded_size > 256:
        raise ValueError("Palette too large for GIF")
    palette_bytes = bytearray()
    for i in range(padded_size):
        if i < len(palette):
            r, g, b = palette[i]
        else:
            r = g = b = 0
        palette_bytes.extend(bytes((r & 0xFF, g & 0xFF, b & 0xFF)))

    out = bytearray()
    out.extend(b"GIF89a")

    packed_lsd = 0x80 | (size_field << 4) | size_field
    out.extend(struct.pack("<HHBBB", width, height, packed_lsd, 0, 0))
    out.extend(palette_bytes)

    out.extend(b"\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00")

    lzw_min = max(2, size_field + 1)

    for canvas, delay_cs in frames:
        gce_packed = (2 << 2) | 0x01
        out.extend(b"\x21\xF9\x04")
        out.append(gce_packed)
        out.extend(struct.pack("<H", max(2, int(delay_cs))))
        out.append(transparent_idx & 0xFF)
        out.append(0)

        out.append(0x2C)
        out.extend(struct.pack("<HHHH", 0, 0, width, height))
        out.append(0)

        flat = []
        for row in canvas:
            flat.extend(row)

        out.append(lzw_min)
        compressed = _lzw_compress(flat, lzw_min)
        out.extend(_split_subblocks(compressed))

    out.append(0x3B)
    return bytes(out)


def make_combined_palette(normal_pal, shiny_pal):
    combined = list(normal_pal[:16])
    while len(combined) < 16:
        combined.append((0, 0, 0))
    shiny_slice = list(shiny_pal[:16])
    while len(shiny_slice) < 16:
        shiny_slice.append((0, 0, 0))
    combined.extend(shiny_slice)
    return combined


def build_side_by_side_canvas(normal_canvas, shiny_canvas, gap):
    h = max(len(normal_canvas), len(shiny_canvas))
    w_n = max((len(row) for row in normal_canvas), default=0)
    w_s = max((len(row) for row in shiny_canvas), default=0)
    total_w = max(1, w_n + gap + w_s)
    h = max(1, h)
    out = [[0] * total_w for _ in range(h)]
    for y, row in enumerate(normal_canvas):
        for x, val in enumerate(row):
            out[y][x] = val
    for y, row in enumerate(shiny_canvas):
        for x, val in enumerate(row):
            if val == 0:
                continue
            out[y][w_n + gap + x] = (val & 0xF) + 16
    return out, total_w, h


def apply_remap(canvas, remap):
    table = list(remap) + list(range(len(remap), 16))
    result = []
    for row in canvas:
        result.append([table[v] if v < len(table) else v for v in row])
    return result


def build_global_palette(mons, extra_colors=()):
    """Map every mon's combined palette into a single shared palette.

    Index 0 is reserved as transparent. Returns (palette, remap_per_mon,
    extra_idx) where extra_idx is a list of global indices for the
    extra_colors entries in the same order.
    """
    palette = [(0, 0, 0)]
    color_to_idx = {}

    def intern(color):
        key = tuple(color)
        idx = color_to_idx.get(key)
        if idx is None:
            idx = len(palette)
            if idx >= 256:
                raise ValueError(
                    "Mega grid exceeds 256 unique colors; quantization needed"
                )
            palette.append(color)
            color_to_idx[key] = idx
        return idx

    remap_per_mon = []
    for m in mons:
        local = m["palette"]
        remap = [0] * len(local)
        for j, color in enumerate(local):
            if j == 0:
                continue
            remap[j] = intern(color)
        remap_per_mon.append(remap)

    extra_idx = [intern(c) for c in extra_colors]
    return palette, remap_per_mon, extra_idx



def render_mon_compare(mon_name, mon_dir, anim_id, vanilla_palettes,
                       code_to_id, shiny_palette_ids, shiny_remaps,
                       get_palette, dir_to_id=None,
                       vanilla_palettes_by_id=None):
    mon_norm = helpers.normalize_monster_name(mon_name)
    species_id = (dir_to_id or {}).get(mon_name)
    if species_id is None:
        species_id = (dir_to_id or {}).get(mon_norm)

    vanilla_pid = None
    if (species_id is not None and vanilla_palettes_by_id is not None
            and species_id < len(vanilla_palettes_by_id)):
        vanilla_pid = vanilla_palettes_by_id[species_id]
    if vanilla_pid is None:
        vanilla_pid = vanilla_palettes.get(mon_norm)
    if vanilla_pid is None:
        return None
    normal_pal = get_palette(vanilla_pid)
    if normal_pal is None:
        return None

    mon_id = species_id if species_id is not None else code_to_id.get(mon_norm)
    shiny_pid = None
    remap = list(range(16))
    if mon_id is not None and mon_id < len(shiny_palette_ids):
        shiny_pid = shiny_palette_ids[mon_id]
        remap = list(shiny_remaps[mon_id])
    if shiny_pid is None:
        shiny_pid = vanilla_pid
    shiny_pal = get_palette(shiny_pid)
    if shiny_pal is None:
        return None

    rendered = render_south_idle(mon_name, mon_dir, anim_id)
    if rendered is None:
        return None
    width, height, raw_canvases = rendered

    delays = [frames_to_centiseconds(dur) for _c, dur in raw_canvases]
    combined_pal = make_combined_palette(normal_pal, shiny_pal)

    sbs_canvases = []
    sbs_w = sbs_h = 0
    for canvas, _dur in raw_canvases:
        shiny_canvas = apply_remap(canvas, remap)
        combined, total_w, total_h = build_side_by_side_canvas(
            canvas, shiny_canvas, 0
        )
        sbs_w = total_w
        sbs_h = total_h
        sbs_canvases.append(combined)

    return {
        "name": mon_name,
        "dex_id": mon_id if mon_id is not None else -1,
        "w": sbs_w,
        "h": sbs_h,
        "frames": sbs_canvases,
        "delays": delays,
        "palette": combined_pal,
    }


def pad_canvas(canvas, target_w, target_h):
    cur_h = len(canvas)
    cur_w = len(canvas[0]) if cur_h else 0
    pad_x = (target_w - cur_w) // 2
    pad_y = (target_h - cur_h) // 2
    out = [[0] * target_w for _ in range(target_h)]
    for y, row in enumerate(canvas):
        dst_y = pad_y + y
        if not (0 <= dst_y < target_h):
            continue
        for x, val in enumerate(row):
            dst_x = pad_x + x
            if 0 <= dst_x < target_w:
                out[dst_y][dst_x] = val
    return out


def frame_at_time(delays, t):
    cycle = sum(delays)
    if cycle <= 0:
        return 0
    t = t % cycle
    acc = 0
    for i, d in enumerate(delays):
        acc += d
        if t < acc:
            return i
    return len(delays) - 1


FONT_GLYPHS = {
    "A": ["111", "1.1", "111", "1.1", "1.1"],
    "B": ["11.", "1.1", "11.", "1.1", "11."],
    "C": ["111", "1..", "1..", "1..", "111"],
    "D": ["11.", "1.1", "1.1", "1.1", "11."],
    "E": ["111", "1..", "11.", "1..", "111"],
    "F": ["111", "1..", "11.", "1..", "1.."],
    "G": ["111", "1..", "1.1", "1.1", "111"],
    "H": ["1.1", "1.1", "111", "1.1", "1.1"],
    "I": ["111", ".1.", ".1.", ".1.", "111"],
    "J": ["111", "..1", "..1", "1.1", ".1."],
    "K": ["1.1", "1.1", "11.", "1.1", "1.1"],
    "L": ["1..", "1..", "1..", "1..", "111"],
    "M": ["1.1", "111", "111", "1.1", "1.1"],
    "N": ["1.1", "111", "111", "111", "1.1"],
    "O": ["111", "1.1", "1.1", "1.1", "111"],
    "P": ["111", "1.1", "111", "1..", "1.."],
    "Q": ["111", "1.1", "1.1", "111", "..1"],
    "R": ["11.", "1.1", "11.", "1.1", "1.1"],
    "S": ["111", "1..", "111", "..1", "111"],
    "T": ["111", ".1.", ".1.", ".1.", ".1."],
    "U": ["1.1", "1.1", "1.1", "1.1", "111"],
    "V": ["1.1", "1.1", "1.1", "1.1", ".1."],
    "W": ["1.1", "1.1", "1.1", "111", "1.1"],
    "X": ["1.1", "1.1", ".1.", "1.1", "1.1"],
    "Y": ["1.1", "1.1", ".1.", ".1.", ".1."],
    "Z": ["111", "..1", ".1.", "1..", "111"],
    "/": ["..1", "..1", ".1.", "1..", "1.."],
    ":": ["...", ".1.", "...", ".1.", "..."],
    ".": ["...", "...", "...", "...", ".1."],
    "-": ["...", "...", "111", "...", "..."],
    " ": ["...", "...", "...", "...", "..."],
}
for _d, _p in helpers.FONT_DIGITS.items():
    FONT_GLYPHS[_d] = _p

GLYPH_W = helpers.FONT_WIDTH
GLYPH_H = helpers.FONT_HEIGHT
GLYPH_SPACING = helpers.FONT_SPACING


def _draw_text(canvas, x0, y0, text, color_idx, glyphs=None, scale=1):
    if glyphs is None:
        glyphs = helpers.FONT_DIGITS
    cx = x0
    cw = len(canvas[0]) if canvas else 0
    ch = len(canvas)
    glyph_w = GLYPH_W * scale
    spacing = GLYPH_SPACING * scale
    for raw_ch in text:
        key = raw_ch.upper() if glyphs is FONT_GLYPHS else raw_ch
        pattern = glyphs.get(key)
        if pattern is None:
            cx += glyph_w + spacing
            continue
        for ry, row in enumerate(pattern):
            for rx, c in enumerate(row):
                if c != "1":
                    continue
                for dy in range(scale):
                    yy = y0 + ry * scale + dy
                    if not (0 <= yy < ch):
                        continue
                    dst = canvas[yy]
                    for dx in range(scale):
                        xx = cx + rx * scale + dx
                        if 0 <= xx < cw:
                            dst[xx] = color_idx
        cx += glyph_w + spacing


def _text_pixel_width(text, scale=1):
    n = len(text)
    if n == 0:
        return 0
    return (n * GLYPH_W + (n - 1) * GLYPH_SPACING) * scale


def _layout_for_cols(mons, cols):
    n = len(mons)
    rows = (n + cols - 1) // cols
    col_widths = [0] * cols
    row_heights = [0] * rows
    for i, m in enumerate(mons):
        c = i % cols
        r = i // cols
        if m["w"] > col_widths[c]:
            col_widths[c] = m["w"]
        if m["h"] > row_heights[r]:
            row_heights[r] = m["h"]
    return rows, col_widths, row_heights


def build_mega_gif(mons, tick_cs, num_ticks, cell_w=None, cell_h=None,
                   grid_color=(64, 64, 64), text_color=(255, 255, 255),
                   title_lines=(), title_scale=6):
    n = len(mons)

    if cell_w is not None and cell_h is not None:
        cols = max(1, int(round((n * cell_h / cell_w) ** 0.5)))
        rows = (n + cols - 1) // cols
        while True:
            new_cols = max(1, cols - 1)
            new_rows = (n + new_cols - 1) // new_cols
            cur_score = abs(cols * cell_w - rows * cell_h)
            new_score = abs(new_cols * cell_w - new_rows * cell_h)
            if new_score < cur_score:
                cols = new_cols
                rows = new_rows
                continue
            break
        col_widths = [cell_w] * cols
        row_heights = [cell_h] * rows
    else:
        best = None
        for cand_cols in range(1, n + 1):
            cand_rows, cw_list, rh_list = _layout_for_cols(mons, cand_cols)
            tw = sum(cw_list)
            th = sum(rh_list)
            score = abs(tw - th)
            if best is None or score < best[0]:
                best = (score, cand_cols, cand_rows, cw_list, rh_list)
        _, cols, rows, col_widths, row_heights = best

    grid_w = sum(col_widths)
    grid_h = sum(row_heights)

    title_lines = [t for t in title_lines if t]
    if title_lines:
        title_pad_top = max(4, title_scale)
        title_line_gap = max(4, title_scale)
        title_pad_bottom = max(6, 2 * title_scale)
        line_h = GLYPH_H * title_scale
        header_h = (
            title_pad_top
            + line_h * len(title_lines)
            + title_line_gap * (len(title_lines) - 1)
            + title_pad_bottom
        )
        max_title_w = max(
            _text_pixel_width(t, scale=title_scale) for t in title_lines
        )
        total_w = max(grid_w, max_title_w + 2 * title_scale)
    else:
        header_h = 0
        total_w = grid_w
    total_h = grid_h + header_h
    grid_x_origin = (total_w - grid_w) // 2

    col_x = [grid_x_origin]
    for w in col_widths[:-1]:
        col_x.append(col_x[-1] + w)
    row_y = [header_h]
    for h in row_heights[:-1]:
        row_y.append(row_y[-1] + h)

    palette, remaps, extras = build_global_palette(
        mons, extra_colors=(grid_color, text_color)
    )
    grid_idx, text_idx = extras

    remapped_frames = []
    for i, m in enumerate(mons):
        rmap = remaps[i]
        out_frames = []
        for canvas in m["frames"]:
            new_canvas = []
            for row in canvas:
                new_row = [rmap[v] if v < len(rmap) else 0 for v in row]
                new_canvas.append(new_row)
            out_frames.append(new_canvas)
        remapped_frames.append(out_frames)

    frames_out = []
    for t in range(num_ticks):
        time_cs = t * tick_cs
        big = [[0] * total_w for _ in range(total_h)]

        if title_lines:
            ty = title_pad_top
            for line in title_lines:
                lw = _text_pixel_width(line, scale=title_scale)
                tx = (total_w - lw) // 2
                _draw_text(
                    big, tx, ty, line, text_idx,
                    glyphs=FONT_GLYPHS, scale=title_scale,
                )
                ty += line_h + title_line_gap

        for i, m in enumerate(mons):
            col = i % cols
            row = i // cols
            cell_x = col_x[col]
            cell_y = row_y[row]
            cell_w_i = col_widths[col]
            cell_h_i = row_heights[row]
            f_idx = frame_at_time(m["delays"], time_cs)
            cell = remapped_frames[i][f_idx]
            ch = len(cell)
            cw = len(cell[0]) if ch else 0
            x0 = cell_x + (cell_w_i - cw) // 2
            y0 = cell_y + (cell_h_i - ch)
            for y, src_row in enumerate(cell):
                dy = y0 + y
                if not (cell_y <= dy < cell_y + cell_h_i - 1):
                    continue
                dst = big[dy]
                for x, val in enumerate(src_row):
                    if val == 0:
                        continue
                    dx = x0 + x
                    if cell_x <= dx < cell_x + cell_w_i - 1:
                        dst[dx] = val

            label = str(m["dex_id"]) if m["dex_id"] > 0 else "?"
            _draw_text(big, cell_x + 1, cell_y + 1, label, text_idx)

        for r in range(rows):
            y = row_y[r] + row_heights[r] - 1
            if y < total_h:
                row_pixels = big[y]
                for x in range(grid_x_origin, grid_x_origin + grid_w):
                    row_pixels[x] = grid_idx
        for c in range(cols):
            x = col_x[c] + col_widths[c] - 1
            if x < total_w:
                for y in range(header_h, total_h):
                    big[y][x] = grid_idx

        frames_out.append((big, tick_cs))

    return total_w, total_h, frames_out, palette


def run(args):
    csv_path = args.input
    pal_dir = args.palette_dir
    out_dir = args.output_dir
    base_dir = args.input_dir
    monster_data_path = args.monster_data
    anim_id = args.anim_id

    code_to_id, shiny_palette_ids, shiny_remaps = load_csv_codename_map(
        csv_path
    )
    vanilla_palettes = helpers.load_vanilla_palettes(monster_data_path)
    vanilla_palettes_by_id = load_vanilla_palettes_by_id(monster_data_path)
    dir_to_id = load_dir_to_species_id(args.species_table)

    os.makedirs(out_dir, exist_ok=True)

    pal_cache = {}

    def get_palette(idx):
        if idx not in pal_cache:
            pal_cache[idx] = load_palette_colors(pal_dir, idx)
        return pal_cache[idx]

    targets = list(helpers.iter_mon_dirs(base_dir, args.mons))
    if not targets:
        print("no monsters to process")
        return 0

    if args.mega:
        rendered_mons = []
        for mon_name, mon_dir in targets:
            data = render_mon_compare(
                mon_name, mon_dir, anim_id, vanilla_palettes,
                code_to_id, shiny_palette_ids, shiny_remaps, get_palette,
                dir_to_id=dir_to_id,
                vanilla_palettes_by_id=vanilla_palettes_by_id,
            )
            if data is None:
                print(f"skip {mon_name}: failed to render")
                continue
            rendered_mons.append(data)
        if not rendered_mons:
            print("no mons rendered for mega")
            return 0
        rendered_mons.sort(
            key=lambda m: (m["dex_id"] if m["dex_id"] > 0 else 10**9, m["name"])
        )
        if args.mega_title is None:
            title_lines = (
                "Red Rescue Team Shiny Patch v1.0.0",
                "https://github.com/jtjanecek/rogue-rescue-team/releases/tag/shiny-patch-v1.0.0",
            )
        else:
            title_lines = tuple(t for t in args.mega_title if t)
        total_w, total_h, frames_out, palette = build_mega_gif(
            rendered_mons,
            args.mega_tick_cs,
            args.mega_ticks,
            cell_w=args.mega_cell_w,
            cell_h=args.mega_cell_h,
            title_lines=title_lines,
            title_scale=args.mega_title_scale,
        )
        mega_bytes = encode_gif(total_w, total_h, frames_out, palette, 0)
        out_path = os.path.join(out_dir, args.mega_filename)
        with open(out_path, "wb") as f:
            f.write(mega_bytes)
        print(
            f"wrote {out_path}: {total_w}x{total_h}, "
            f"{len(rendered_mons)} mons, {args.mega_ticks} ticks"
        )
        return 0

    written = 0
    for mon_name, mon_dir in targets:
        mon_norm = helpers.normalize_monster_name(mon_name)

        species_id = dir_to_id.get(mon_name)
        if species_id is None:
            species_id = dir_to_id.get(mon_norm)

        vanilla_pid = None
        if (species_id is not None
                and species_id < len(vanilla_palettes_by_id)):
            vanilla_pid = vanilla_palettes_by_id[species_id]
        if vanilla_pid is None:
            vanilla_pid = vanilla_palettes.get(mon_norm)
        if vanilla_pid is None:
            print(f"skip {mon_name}: no vanilla palette in monster_data.json")
            continue
        normal_pal = get_palette(vanilla_pid)
        if normal_pal is None:
            print(f"skip {mon_name}: missing palette {vanilla_pid}")
            continue

        mon_id = (
            species_id if species_id is not None
            else code_to_id.get(mon_norm)
        )
        shiny_pid = None
        remap = list(range(16))
        if mon_id is not None and mon_id < len(shiny_palette_ids):
            shiny_pid = shiny_palette_ids[mon_id]
            remap = list(shiny_remaps[mon_id])
        if shiny_pid is None:
            shiny_pid = vanilla_pid
        shiny_pal = get_palette(shiny_pid)
        if shiny_pal is None:
            print(f"skip {mon_name}: missing shiny palette {shiny_pid}")
            continue

        rendered = render_south_idle(mon_name, mon_dir, anim_id)
        if rendered is None:
            print(f"skip {mon_name}: failed to render idle frames")
            continue
        width, height, raw_canvases = rendered

        delays = [frames_to_centiseconds(dur) for _canvas, dur in raw_canvases]
        normal_frames = [
            (canvas, delays[i])
            for i, (canvas, _dur) in enumerate(raw_canvases)
        ]
        shiny_frames = []
        for i, (canvas, _dur) in enumerate(raw_canvases):
            shiny_frames.append((apply_remap(canvas, remap), delays[i]))

        normal_gif = encode_gif(width, height, normal_frames, normal_pal, 0)
        shiny_gif = encode_gif(width, height, shiny_frames, shiny_pal, 0)

        combined_pal = make_combined_palette(normal_pal, shiny_pal)
        sbs_frames = []
        sbs_w = sbs_h = 0
        for i, (canvas, _dur) in enumerate(raw_canvases):
            shiny_canvas = shiny_frames[i][0]
            combined, total_w, total_h = build_side_by_side_canvas(
                canvas, shiny_canvas, args.gap
            )
            sbs_w = total_w
            sbs_h = total_h
            sbs_frames.append((combined, delays[i]))
        compare_gif = encode_gif(sbs_w, sbs_h, sbs_frames, combined_pal, 0)

        with open(os.path.join(out_dir, f"{mon_name}_normal.gif"), "wb") as f:
            f.write(normal_gif)
        with open(os.path.join(out_dir, f"{mon_name}_shiny.gif"), "wb") as f:
            f.write(shiny_gif)
        with open(os.path.join(out_dir, f"{mon_name}_compare.gif"), "wb") as f:
            f.write(compare_gif)

        written += 1
        print(f"wrote {mon_name} (3 gifs)")

    print(f"done: {written} monster(s)")
    return 0


def main():
    parser = argparse.ArgumentParser(
        description="Generate per-monster idle GIFs (normal, shiny, compare)."
    )
    parser.add_argument(
        "--input",
        default="rogue_files/shiny_palette.csv",
        help="Path to shiny_palette.csv.",
    )
    parser.add_argument(
        "--monster-data",
        default="data/monster/monster_data.json",
        help="Path to monster_data.json (for vanilla overworldPalette).",
    )
    parser.add_argument(
        "--species-table",
        default="src/monster_files_table.c",
        help="Path to monster_files_table.c (for ax dir -> species ID map).",
    )
    parser.add_argument(
        "--input-dir",
        default="graphics/ax/mon",
        help="Directory containing per-monster ax sprite folders.",
    )
    parser.add_argument(
        "--palette-dir",
        default="graphics/ax/pal",
        help="Directory containing base .pmdpal files.",
    )
    parser.add_argument(
        "--output-dir",
        default="gen/shiny_gifs",
        help="Destination directory for output GIFs.",
    )
    parser.add_argument(
        "--anim-id",
        type=int,
        default=7,
        help="Animation ID (default: 7 for idle).",
    )
    parser.add_argument(
        "--mon",
        action="append",
        dest="mons",
        help="Monster directory name to process (repeatable).",
    )
    parser.add_argument(
        "--gap",
        type=int,
        default=0,
        help="Pixel gap between normal and shiny in the compare gif.",
    )
    parser.add_argument(
        "--mega",
        action="store_true",
        help="Build a single square-ish grid GIF of every mon's compare.",
    )
    parser.add_argument(
        "--mega-filename",
        default="mega_compare.gif",
        help="Output filename for the mega grid (relative to --output-dir).",
    )
    parser.add_argument(
        "--mega-tick-cs",
        type=int,
        default=8,
        help="Centiseconds per global animation tick in the mega gif.",
    )
    parser.add_argument(
        "--mega-ticks",
        type=int,
        default=16,
        help="Number of global animation ticks in the mega gif.",
    )
    parser.add_argument(
        "--mega-cell-w",
        type=int,
        default=None,
        help="Cell width in mega grid (default: 90th percentile of mon widths).",
    )
    parser.add_argument(
        "--mega-cell-h",
        type=int,
        default=None,
        help="Cell height in mega grid (default: 90th percentile of mon heights).",
    )
    parser.add_argument(
        "--mega-title",
        action="append",
        default=None,
        help="Title line drawn above the mega grid (repeatable for multiple lines). "
             "Pass --mega-title '' to disable. Defaults to the v1.0.0 release banner.",
    )
    parser.add_argument(
        "--mega-title-scale",
        type=int,
        default=6,
        help="Integer pixel-scale for the title font (each 3x5 glyph rendered SxS).",
    )
    args = parser.parse_args()
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
