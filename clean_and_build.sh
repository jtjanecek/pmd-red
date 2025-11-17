#!/bin/bash

rm -f -- execution.log
rm -f -- *.sav
make clean && make


# Write date header at top of log
echo "Log created on: $(date)" > execution.log
echo "" >> execution.log

echo

gba() {
    "$HOME/Documents/pmd/mGBA.app/Contents/MacOS/mGBA" \
        -d "$HOME/Documents/pmd/pmd-red/pmd_red.gba" "$@"
}

# Append WARN lines under the date header
gba 2>&1 | tee >(grep "WARN" >> execution.log)
