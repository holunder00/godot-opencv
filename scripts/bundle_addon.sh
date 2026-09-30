#!/bin/bash
set -e

ADDON="addons/gdopencv"
VERSION="1.0.0"

echo "=== Packaging GDOpenCV Addon v${VERSION} ==="

# Sanity check: binaries must be self-contained (static OpenCV)
if command -v ldd &>/dev/null; then
    for f in "$ADDON"/bin/linux/*.so; do
        [ -f "$f" ] || continue
        if ldd "$f" | grep -E "opencv|libav|libgstreamer"; then
            echo "ERROR: $f has unexpected shared dependencies!"
            exit 1
        fi
    done
fi

# Remove leftover libs/ folders from the old bundling approach
rm -rf "$ADDON"/bin/*/libs

ARCHIVE="gdopencv-v${VERSION}.zip"
cd addons
zip -r "../$ARCHIVE" gdopencv/ -x "*.git*"
cd ..

echo "=== Created: $ARCHIVE ($(du -h $ARCHIVE | awk '{print $1}')) ==="
