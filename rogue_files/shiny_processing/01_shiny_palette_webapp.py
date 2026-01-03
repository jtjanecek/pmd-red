#!/usr/bin/env python3
import argparse
import binascii
import json
import os
import struct
import zlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlparse

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


def write_png_rgba_bytes(width, height, rgba_bytes):
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
    return write_png(chunks)


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


def apply_overrides(pixels, overrides):
    if not overrides or all(v is None for v in overrides):
        return pixels
    new_pixels = []
    for row in pixels:
        new_row = []
        for idx in row:
            replacement = overrides[idx]
            new_row.append(replacement if replacement is not None else idx)
        new_pixels.append(new_row)
    return new_pixels


class AppState:
    def __init__(self, frames_dir):
        self.frames_dir = frames_dir
        self.grid_dir = os.path.join(os.path.dirname(frames_dir), "shiny_idle_grids")
        self.cache = {}

    def list_monsters(self):
        if not os.path.isdir(self.frames_dir):
            return []
        return sorted(
            name for name in os.listdir(self.frames_dir)
            if os.path.isdir(os.path.join(self.frames_dir, name))
        )

    def load_frames(self, mon, palette_idx):
        key = (mon, palette_idx)
        cached = self.cache.get(key)
        if cached is not None:
            return cached

        palette_dir = f"palette_{palette_idx:02d}"
        mon_dir = os.path.join(self.frames_dir, mon, palette_dir)
        if not os.path.isdir(mon_dir):
            raise FileNotFoundError(f"Missing {mon}/{palette_dir} in shiny_idle_frames.")

        images = {}
        base_palette = None
        for direction in DIRECTION_NAMES:
            path = os.path.join(mon_dir, f"idle_{direction}.png")
            if not os.path.isfile(path):
                continue
            width, height, pixels, palette = decode_png_indexed(path)
            if base_palette is None:
                base_palette = palette
            images[direction] = (width, height, pixels)

        if not images or base_palette is None:
            raise FileNotFoundError(f"No idle frames found for {mon}/{palette_dir}.")

        payload = (images, base_palette)
        self.cache[key] = payload
        return payload

    def render_grid(self, mon, palette_idx, overrides):
        images, base_palette = self.load_frames(mon, palette_idx)
        max_w = max(img[0] for img in images.values())
        max_h = max(img[1] for img in images.values())
        padding = 4
        cell_w = max_w + padding
        grid_w = (cell_w * len(DIRECTION_NAMES)) - padding
        grid_h = max_h

        canvas = bytearray(grid_w * grid_h * 4)
        for y in range(grid_h):
            for x in range(grid_w):
                pos = (y * grid_w + x) * 4
                canvas[pos:pos + 4] = bytes((240, 240, 240, 255))

        for col, direction in enumerate(DIRECTION_NAMES):
            img = images.get(direction)
            if img is None:
                continue
            width, height, pixels = img
            remapped = apply_overrides(pixels, overrides)
            rgba = indexed_to_rgba(width, height, remapped, base_palette)
            x0 = col * cell_w + (max_w - width) // 2
            y0 = (max_h - height) // 2
            place_image(canvas, grid_w, grid_h, (width, height, rgba), x0, y0)

        return write_png_rgba_bytes(grid_w, grid_h, canvas)

    def read_grid_png(self, mon):
        filename = f"{mon}.png"
        safe_name = os.path.basename(filename)
        path = os.path.join(self.grid_dir, safe_name)
        if not os.path.isfile(path):
            raise FileNotFoundError(f"Missing grid image for {mon}.")
        with open(path, "rb") as f:
            return f.read()


INDEX_HTML = """<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <title>Shiny Palette Web Preview</title>
  <style>
    :root {
      color-scheme: light;
      font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
    }
    body {
      margin: 16px;
      background: #f6f5f2;
      color: #222;
    }
    .panel {
      display: flex;
      gap: 12px;
      align-items: center;
      flex-wrap: wrap;
      margin-bottom: 12px;
    }
    label {
      font-weight: 600;
      margin-right: 6px;
    }
    select, input[type="number"] {
      padding: 4px 6px;
      font-size: 14px;
    }
    .preview-wrap {
      background: #fff;
      border: 1px solid #d9d9d9;
      padding: 16px;
      min-height: 240px;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    #preview {
      image-rendering: pixelated;
    }
    #overrides {
      display: grid;
      grid-template-columns: repeat(16, minmax(40px, 1fr));
      gap: 6px;
    }
    .override-cell {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 4px;
    }
    .status {
      min-height: 20px;
      color: #555;
    }
  </style>
</head>
<body>
  <div class="panel" style="margin-top:12px;">
    <strong>Color Overrides</strong>
  </div>
  <div id="overrides"></div>
  <div class="status" id="status"></div>

  <div class="panel">
    <div>
      <label for="pokemon">Pokemon</label>
      <select id="pokemon"></select>
    </div>
    <div>
      <label for="palette">Palette</label>
      <select id="palette"></select>
    </div>
    <div>
      <label for="scale">Scale</label>
      <select id="scale"></select>
    </div>
    <div>
      <label for="grid-scale">Grid Scale</label>
      <select id="grid-scale"></select>
    </div>
  </div>

  <div class="preview-wrap">
    <img id="preview" alt="Preview" />
  </div>

  <div class="panel" style="margin-top:16px;">
    <strong>Existing Grid Output</strong>
  </div>
  <div class="preview-wrap">
    <img id="grid" alt="Grid Output" style="image-rendering: pixelated;" />
  </div>

  <script>
    const pokemonSelect = document.getElementById("pokemon");
    const paletteSelect = document.getElementById("palette");
    const scaleSelect = document.getElementById("scale");
    const gridScaleSelect = document.getElementById("grid-scale");
    const overridesWrap = document.getElementById("overrides");
    const preview = document.getElementById("preview");
    const statusEl = document.getElementById("status");
    const gridImg = document.getElementById("grid");
    let debounceTimer = null;
    let currentUrl = null;

    function setStatus(text) {
      statusEl.textContent = text || "";
    }

    function buildOverridesInputs() {
      overridesWrap.innerHTML = "";
      for (let i = 0; i < 16; i++) {
        const cell = document.createElement("div");
        cell.className = "override-cell";
        const label = document.createElement("div");
        label.textContent = i.toString();
        const input = document.createElement("input");
        input.type = "number";
        input.min = "0";
        input.max = "15";
        input.placeholder = "-";
        input.dataset.index = i;
        input.addEventListener("input", scheduleRender);
        cell.appendChild(label);
        cell.appendChild(input);
        overridesWrap.appendChild(cell);
      }
    }

    function getOverrides() {
      const overrides = [];
      overridesWrap.querySelectorAll("input").forEach((input) => {
        const raw = input.value.trim();
        if (raw === "") {
          overrides.push(null);
          return;
        }
        const value = Number(raw);
        if (Number.isNaN(value)) {
          overrides.push(null);
          return;
        }
        overrides.push(Math.max(0, Math.min(15, value)));
      });
      return overrides;
    }

    function scheduleRender() {
      if (debounceTimer) {
        clearTimeout(debounceTimer);
      }
      debounceTimer = setTimeout(renderPreview, 150);
    }

    async function renderPreview() {
      const mon = pokemonSelect.value;
      const palette = Number(paletteSelect.value);
      const overrides = getOverrides();
      const payload = { mon, palette, overrides };
      setStatus("Rendering...");
      const response = await fetch("/api/render", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });
      if (!response.ok) {
        const text = await response.text();
        setStatus(text || "Render failed");
        return;
      }
      const blob = await response.blob();
      if (currentUrl) {
        URL.revokeObjectURL(currentUrl);
      }
      currentUrl = URL.createObjectURL(blob);
      preview.onload = () => {
        const scale = Number(scaleSelect.value);
        preview.style.width = (preview.naturalWidth * scale) + "px";
        preview.style.height = (preview.naturalHeight * scale) + "px";
        setStatus("");
      };
      preview.src = currentUrl;

      const gridUrl = `/api/grid?mon=${encodeURIComponent(mon)}&v=${Date.now()}`;
      gridImg.onload = () => {
        const scale = Number(gridScaleSelect.value);
        gridImg.style.width = (gridImg.naturalWidth * scale) + "px";
        gridImg.style.height = (gridImg.naturalHeight * scale) + "px";
      };
      gridImg.src = gridUrl;
    }

    async function init() {
      buildOverridesInputs();
      for (let i = 0; i <= 15; i++) {
        const opt = document.createElement("option");
        opt.value = i.toString();
        opt.textContent = i.toString();
        paletteSelect.appendChild(opt);
      }
      for (let i = 1; i <= 10; i++) {
        const opt = document.createElement("option");
        opt.value = i.toString();
        opt.textContent = i.toString() + "x";
        if (i === 5) {
          opt.selected = true;
        }
        scaleSelect.appendChild(opt);
      }
      for (let i = 1; i <= 10; i++) {
        const opt = document.createElement("option");
        opt.value = i.toString();
        opt.textContent = i.toString() + "x";
        if (i === 3) {
          opt.selected = true;
        }
        gridScaleSelect.appendChild(opt);
      }
      const response = await fetch("/api/monsters");
      const mons = await response.json();
      mons.forEach((mon) => {
        const opt = document.createElement("option");
        opt.value = mon;
        opt.textContent = mon;
        pokemonSelect.appendChild(opt);
      });
      pokemonSelect.addEventListener("change", scheduleRender);
      paletteSelect.addEventListener("change", scheduleRender);
      scaleSelect.addEventListener("change", scheduleRender);
      gridScaleSelect.addEventListener("change", scheduleRender);
      if (mons.length > 0) {
        pokemonSelect.value = mons[0];
      }
      scheduleRender();
    }

    init().catch((err) => {
      setStatus("Init failed: " + err);
    });
  </script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    def _send(self, status, content_type, data):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path == "/" or parsed.path == "/index.html":
            self._send(200, "text/html; charset=utf-8", INDEX_HTML.encode("utf-8"))
            return
        if parsed.path == "/api/monsters":
            mons = self.server.app_state.list_monsters()
            data = json.dumps(mons).encode("utf-8")
            self._send(200, "application/json", data)
            return
        if parsed.path == "/api/render":
            params = parse_qs(parsed.query)
            try:
                mon = params.get("mon", [""])[0]
                palette = int(params.get("palette", ["0"])[0])
            except ValueError:
                self._send(400, "text/plain; charset=utf-8", b"Invalid palette.")
                return
            overrides = []
            raw = params.get("overrides", [""])[0]
            if raw:
                parts = raw.split(",")
                for i in range(16):
                    try:
                        value = parts[i]
                    except IndexError:
                        value = ""
                    if value == "":
                        overrides.append(None)
                    else:
                        try:
                            num = int(value, 10)
                        except ValueError:
                            overrides.append(None)
                        else:
                            overrides.append(max(0, min(15, num)))
            else:
                overrides = [None] * 16
            try:
                png = self.server.app_state.render_grid(mon, palette, overrides)
            except Exception as exc:
                self._send(400, "text/plain; charset=utf-8", str(exc).encode("utf-8"))
                return
            self._send(200, "image/png", png)
            return
        if parsed.path == "/api/grid":
            params = parse_qs(parsed.query)
            mon = params.get("mon", [""])[0]
            try:
                png = self.server.app_state.read_grid_png(mon)
            except Exception as exc:
                self._send(404, "text/plain; charset=utf-8", str(exc).encode("utf-8"))
                return
            self._send(200, "image/png", png)
            return
        self._send(404, "text/plain; charset=utf-8", b"Not found.")

    def do_POST(self):
        parsed = urlparse(self.path)
        if parsed.path != "/api/render":
            self._send(404, "text/plain; charset=utf-8", b"Not found.")
            return
        length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(length) if length else b""
        try:
            payload = json.loads(body.decode("utf-8"))
        except json.JSONDecodeError:
            self._send(400, "text/plain; charset=utf-8", b"Invalid JSON.")
            return
        mon = payload.get("mon", "")
        try:
            palette = int(payload.get("palette", 0))
        except ValueError:
            self._send(400, "text/plain; charset=utf-8", b"Invalid palette.")
            return
        palette = max(0, min(15, palette))
        overrides = payload.get("overrides", [])
        normalized = []
        for i in range(16):
            try:
                value = overrides[i]
            except IndexError:
                value = None
            if value is None:
                normalized.append(None)
                continue
            try:
                num = int(value)
            except (TypeError, ValueError):
                normalized.append(None)
            else:
                normalized.append(max(0, min(15, num)))
        try:
            png = self.server.app_state.render_grid(mon, palette, normalized)
        except Exception as exc:
            self._send(400, "text/plain; charset=utf-8", str(exc).encode("utf-8"))
            return
        self._send(200, "image/png", png)


def main():
    parser = argparse.ArgumentParser(description="Shiny palette preview webapp")
    parser.add_argument("--host", default="127.0.0.1", help="Host to bind.")
    parser.add_argument("--port", default=8000, type=int, help="Port to bind.")
    args = parser.parse_args()

    frames_dir = "gen/shiny_idle_frames"
    server = ThreadingHTTPServer((args.host, args.port), Handler)
    server.app_state = AppState(frames_dir)
    print(f"Serving on http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
