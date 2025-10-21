#!/usr/bin/env python3
"""
apply_bps.py with tqdm progress
Apply a BPS patch to a base ROM.

Usage:
  python3 apply_bps.py base.gba patch.bps out.gba
"""
import sys
from pathlib import Path
from io import BytesIO
from typing import Tuple
from tqdm import tqdm
import zlib

# -------- BPS number coding (byuu) --------
# Unsigned "byuu varint": low 7 bits per byte, MSB=1 marks final.
# For each non-final byte consumed, add 1<<(7*k) to the accumulated value.
def read_bps_number(f: BytesIO) -> int:
    value = 0
    shift = 0
    while True:
        b_raw = f.read(1)
        if not b_raw:
            raise ValueError("Unexpected EOF while reading BPS number")
        b = b_raw[0]
        value += (b & 0x7F) << shift
        if b & 0x80:
            break
        shift += 7
        value += 1 << shift
    return value

def read_bps_signed(f: BytesIO) -> int:
    n = read_bps_number(f)
    # zigzag decode
    return (n >> 1) ^ -(n & 1)

def crc32le(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF

def read_exact(f: BytesIO, n: int) -> bytes:
    b = f.read(n)
    if len(b) != n:
        raise ValueError("Unexpected EOF while reading BPS data")
    return b

def parse_header(patch: bytes) -> Tuple[int, int, bytes, BytesIO]:
    f = BytesIO(patch)
    magic = read_exact(f, 4)
    if magic != b"BPS1":
        raise ValueError("Not a BPS file: missing 'BPS1' header")
    src_size = read_bps_number(f)
    dst_size = read_bps_number(f)
    meta_len = read_bps_number(f)
    metadata = read_exact(f, meta_len) if meta_len else b""
    return src_size, dst_size, metadata, f

def count_actions(stream_pos: int, patch: bytes) -> int:
    # Lightweight pre-pass to count actions for tqdm total.
    f = BytesIO(patch)
    f.seek(stream_pos)
    count = 0
    try:
        while f.tell() <= len(patch) - 12:  # leave room for 3 CRCs
            word = read_bps_number(f)
            action = word & 0x3
            length = (word >> 2) + 1
            if action == 0:      # SourceRead
                pass
            elif action == 1:    # TargetRead
                # skip literal bytes
                f.seek(length, 1)
            elif action == 2:    # SourceCopy
                _ = read_bps_signed(f)  # relative addr
            elif action == 3:    # TargetCopy
                _ = read_bps_signed(f)  # relative addr
            count += 1
    except Exception:
        # On any parse issue, fall back to unknown total.
        return 0
    return count

def apply_bps(base: bytes, patch: bytes) -> bytes:
    src_size, dst_size, _meta, f = parse_header(patch)

    if len(base) != src_size:
        # Some tools allow base larger with extra zeros; we enforce exact match for safety.
        raise ValueError(f"Source size mismatch: patch expects {src_size}, got {len(base)}")

    # Prepare output and state
    out = bytearray(dst_size)
    src = base

    src_rel = 0  # SourceCopy relative pointer
    dst_rel = 0  # TargetCopy relative pointer
    out_pos = 0

    # Progress setup
    start_pos = f.tell()
    total_actions = count_actions(start_pos, patch)
    pbar = tqdm(total=total_actions or None, desc="Applying BPS actions")

    # Action stream
    while f.tell() <= len(patch) - 12 and out_pos < dst_size:
        word = read_bps_number(f)
        action = word & 0x3
        length = (word >> 2) + 1

        if action == 0:  # SourceRead: copy from src at *current out_pos*
            end = out_pos + length
            if end > dst_size:
                raise ValueError("SourceRead exceeds target size")
            # If out_pos beyond src, bytes are considered 0
            seg = src[out_pos:end] if end <= len(src) else (src[out_pos:len(src)] + b"\x00" * (end - len(src)))
            out[out_pos:end] = seg
            out_pos = end

        elif action == 1:  # TargetRead: read literals
            data = read_exact(f, length)
            end = out_pos + length
            if end > dst_size:
                raise ValueError("TargetRead exceeds target size")
            out[out_pos:end] = data
            out_pos = end

        elif action == 2:  # SourceCopy: copy from src at (src_rel += signed)
            src_rel += read_bps_signed(f)
            src_off = src_rel
            end = out_pos + length
            if end > dst_size:
                raise ValueError("SourceCopy exceeds target size")
            if src_off < 0:
                raise ValueError("SourceCopy negative source offset")
            # Copy from src_off
            slice_end = src_off + length
            seg = src[src_off:slice_end]
            if len(seg) < length:
                # Spec allows reading past EOF as zero-fill
                seg = seg + b"\x00" * (length - len(seg))
            out[out_pos:end] = seg
            out_pos = end

        elif action == 3:  # TargetCopy: copy from out at (dst_rel += signed)
            dst_rel += read_bps_signed(f)
            src_off = dst_rel
            if src_off < 0 or src_off >= out_pos:
                raise ValueError("TargetCopy refers to invalid/unwritten region")
            end = out_pos + length
            if end > dst_size:
                raise ValueError("TargetCopy exceeds target size")
            # Handle potential overlap with forward copy semantics
            for _ in range(length):
                out[out_pos] = out[src_off]
                out_pos += 1
                src_off += 1

        else:
            raise ValueError("Unknown BPS action")

        if total_actions:
            pbar.update(1)

    pbar.close()

    # Footer CRCs
    if len(patch) - f.tell() != 12:
        raise ValueError("Malformed BPS: missing CRC footer")
    crc_src_expect = int.from_bytes(read_exact(f, 4), "little")
    crc_dst_expect = int.from_bytes(read_exact(f, 4), "little")
    crc_patch_expect = int.from_bytes(read_exact(f, 4), "little")

    # Validate CRCs
    crc_src_actual = crc32le(src)
    if crc_src_actual != crc_src_expect:
        raise ValueError(f"Source CRC mismatch: got 0x{crc_src_actual:08X}, expected 0x{crc_src_expect:08X}")

    crc_patch_actual = crc32le(patch[:-4])  # CRC over patch excluding final CRC
    if crc_patch_actual != crc_patch_expect:
        raise ValueError(f"Patch CRC mismatch: got 0x{crc_patch_actual:08X}, expected 0x{crc_patch_expect:08X}")

    crc_dst_actual = crc32le(out)
    if crc_dst_actual != crc_dst_expect:
        raise ValueError(f"Target CRC mismatch: got 0x{crc_dst_actual:08X}, expected 0x{crc_dst_expect:08X}")

    return bytes(out)

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 apply_bps.py base.gba patch.bps out.gba")
        sys.exit(2)

    base_path = Path(sys.argv[1])
    patch_path = Path(sys.argv[2])
    out_path = Path(sys.argv[3])

    base = base_path.read_bytes()
    bps  = patch_path.read_bytes()

    out = apply_bps(base, bps)
    out_path.write_bytes(out)

    print(f"Patched ROM written to {out_path} ({len(out)} bytes)")

if __name__ == "__main__":
    main()

