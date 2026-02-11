#!/usr/bin/env python3
"""
make_bps_pure.py
Create a BPS patch (Binary Patching System, byuu) from base -> modified.

- Pure Python, no external tools.
- Emits SourceRead for runs where base[i] == mod[i] at the same offset,
  and TargetRead for differing runs. This is spec-valid and robust.
- Footer includes CRC32 of source, target, and patch (little-endian).

Usage:
  python3 make_bps_pure.py /path/to/base.gba /path/to/mod.gba /path/to/out.bps
"""
import sys
import os
import zlib
from io import BytesIO
from typing import ByteString

# ---- BPS variable-length integer encoding (byuu spec) ----
# Number encoding:
#  - Encode low 7 bits per byte; MSB=1 marks final byte
#  - After emitting a non-final byte, decrement the remaining value by 1
def bps_write_number(buf: BytesIO, value: int) -> None:
    if value < 0:
        raise ValueError("bps_write_number: negative value")
    while True:
        x = value & 0x7F
        value >>= 7
        if value == 0:
            buf.write(bytes([0x80 | x]))
            break
        buf.write(bytes([x]))
        value -= 1

# Action word packs: (length-1) << 2 | action_id
# action_id: 0=SourceRead, 1=TargetRead, 2=SourceCopy, 3=TargetCopy
def bps_write_action(buf: BytesIO, action_id: int, length: int) -> None:
    if length <= 0:
        return
    word = ((length - 1) << 2) | (action_id & 0x3)
    bps_write_number(buf, word)

def crc32le(data: ByteString) -> bytes:
    # BPS footer stores CRC32 as 32-bit little-endian
    return zlib.crc32(data).to_bytes(4, "little")

def make_bps(base: bytes, mod: bytes, metadata: bytes = b"") -> bytes:
    out = BytesIO()

    # Header
    out.write(b"BPS1")
    bps_write_number(out, len(base))
    bps_write_number(out, len(mod))
    bps_write_number(out, len(metadata))
    if metadata:
        out.write(metadata)

    # Linear construction: walk target from 0..len(mod)-1
    i = 0
    n_base = len(base)
    n_mod = len(mod)

    while i < n_mod:
        # SourceRead run: bytes equal at same offset (and within base)
        if i < n_base and base[i] == mod[i]:
            start = i
            i += 1
            while i < n_mod and i < n_base and base[i] == mod[i]:
                i += 1
            run_len = i - start
            bps_write_action(out, 0, run_len)  # SourceRead
            continue

        # TargetRead run: bytes differ or beyond base length
        start = i
        i += 1
        while i < n_mod and not (i < n_base and base[i] == mod[i]):
            i += 1
        run_len = i - start
        bps_write_action(out, 1, run_len)  # TargetRead
        out.write(mod[start:i])

    # Footer CRCs
    # 1) CRC32 of source (base)
    out.write(crc32le(base))
    # 2) CRC32 of target (mod)
    out.write(crc32le(mod))
    # 3) CRC32 of entire patch up to (but not including) this checksum
    patch_bytes = out.getvalue()
    out.write(crc32le(patch_bytes))

    return out.getvalue()

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 make_bps_pure.py base.gba mod.gba out.bps")
        sys.exit(2)

    base_path, mod_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]

    if not os.path.isfile(base_path):
        print(f"ERR: Base ROM not found: {base_path}")
        sys.exit(1)
    if not os.path.isfile(mod_path):
        print(f"ERR: Modified ROM not found: {mod_path}")
        sys.exit(1)

    with open(base_path, "rb") as f:
        base = f.read()
    with open(mod_path, "rb") as f:
        mod = f.read()

    # Optional metadata (can be empty). You can put UTF-8/XML, etc.
    metadata = b""

    patch = make_bps(base, mod, metadata)

    with open(out_path, "wb") as f:
        f.write(patch)

    print(f"[+] BPS patch written: {out_path}")
    print(f"[+] Base bytes:   {len(base)}")
    print(f"[+] Target bytes: {len(mod)}")
    print(f"[+] Patch bytes:  {len(patch)}")

if __name__ == "__main__":
    main()

