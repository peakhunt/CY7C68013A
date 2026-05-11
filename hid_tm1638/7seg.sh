#!/bin/bash
DEV="/dev/hidraw5"
#PATTERNS=(1 2 4 8 16 32 64 128 64 32 16 8 4 2)
PATTERNS=(1 3 7 15 31 63 127 255 127 63 31 15 3 1 0)

while true; do
    for p in "${PATTERNS[@]}"; do
        # This sends exactly 10 bytes: 0, 0, and your pattern repeated 8 times.
        # No printf, no UTF-8 mangling, no c280 garbage.
        #python3 -c "import sys; sys.stdout.buffer.write(bytes([0] + [$p]*9))" > "$DEV"
        oct=$(printf '%03o' "$p")
        
        # Send 1 null byte followed by 9 pattern bytes
        # \000 is the null byte; \${oct} is your pattern
        printf "\000\\${oct}\\${oct}\\${oct}\\${oct}\\${oct}\\${oct}\\${oct}\\${oct}\\${oct}" > "$DEV"
        sleep 0.1
    done
done
