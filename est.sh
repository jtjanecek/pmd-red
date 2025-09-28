#!/usr/bin/env bash
set -euo pipefail

ROM_LIMIT_BYTES=$((32 * 1024 * 1024))

arm-none-eabi-objcopy -O binary pmd_red.elf pmd_red.raw

python3 - <<'PY'
import re, subprocess

ROM_START = 0x08000000
ROM_END   = 0x0A000000
ROM_LIMIT = 32 * 1024 * 1024

total = 0
readelf_out = subprocess.check_output([
    "arm-none-eabi-readelf", "-S", "pmd_red.elf"
], text=True)

pattern = re.compile(
    r"\s*\[\s*\d+\]\s+\S+\s+\S+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+(\S+)"
)

for line in readelf_out.splitlines():
    m = pattern.match(line)
    if not m:
        continue
    addr = int(m.group(1), 16)
    size = int(m.group(2), 16)
    flags = m.group(3)
    if ROM_START <= addr < ROM_END and 'A' in flags:
        total += size

free_bytes = ROM_LIMIT - total
percent_used = (total / ROM_LIMIT) * 100
percent_free = 100 - percent_used

print(f"ROM payload (no padding): {total} bytes ({total/1048576:.2f} MiB)")
print(f"Free space before padding: {free_bytes} bytes ({free_bytes/1048576:.2f} MiB)")
print(f"Usage: {percent_used:.2f}% used / {percent_free:.2f}% free")
PY

rm -f pmd_red.raw
