#!/bin/bash
set -e

ADDON="addons/gdopencv"
VERSION="1.0.0"

echo "=== Bundling GDOpenCV Addon v${VERSION} ==="

# --- Linux ---
if [ -f "$ADDON/bin/linux/libgdopencv.linux.template_debug.x86_64.so" ]; then
    echo "Bundling Linux dependencies..."
    mkdir -p "$ADDON/bin/linux/libs"
    
    EXT="$ADDON/bin/linux/libgdopencv.linux.template_debug.x86_64.so"
    
    ldd "$EXT" | grep -v "linux-vdso\|ld-linux\|libc.so\|libm.so\|libdl.so\|libpthread\|librt.so\|libstdc++\|libgcc_s\|libGL\|libX\|libdrm\|libwayland" | grep "=> /" | awk '{print $3}' | while read lib; do
        cp -n "$lib" "$ADDON/bin/linux/libs/" 2>/dev/null || true
    done
    
    # Fix RPATH
    if command -v patchelf &>/dev/null; then
        for ext in "$ADDON"/bin/linux/libgdopencv.linux.*.so; do
            patchelf --set-rpath '$ORIGIN/libs' "$ext"
        done
        for lib in "$ADDON"/bin/linux/libs/*.so*; do
            patchelf --set-rpath '$ORIGIN' "$lib" 2>/dev/null || true
        done
    fi
    echo "  Linux done: $(ls $ADDON/bin/linux/libs/ | wc -l) libs bundled"
fi

# --- Create archive ---
echo "Creating archive..."
ARCHIVE="gdopencv-v${VERSION}.zip"
cd addons
zip -r "../$ARCHIVE" gdopencv/ -x "*.git*"
cd ..

echo "=== Created: $ARCHIVE ==="
echo "=== Size: $(du -h $ARCHIVE | awk '{print $1}') ==="
