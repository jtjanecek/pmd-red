#!/bin/bash

rm -f -- execution.log
rm -f -- *.sav

# Write date header at top of log
echo "Log created on: $(date)" > execution.log
echo "" >> execution.log

sleep 3

echo

gba() {
    "$HOME/Documents/mGBA/mGBA.app/Contents/MacOS/mGBA" \
        -d "$HOME/Documents/rogue-rescue-team/pmd_red.gba" "$@"
}

# Append WARN lines under the date header
gba 2>&1 | tee >(grep "WARN" >> execution.log)
