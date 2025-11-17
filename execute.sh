#!/bin/bash

rm -f -- execution.log

# Write date header at top of log
echo "Log created on: $(date)" > execution.log
echo "" >> execution.log

echo

gba() {
    /Users/johnjanecek/Documents/pmd/mGBA.app/Contents/MacOS/mGBA \
        -d /Users/johnjanecek/Documents/pmd/pmd-red/pmd_red.gba "$@"
}

# Append WARN lines under the date header
gba 2>&1 | tee >(grep "WARN" >> execution.log)
