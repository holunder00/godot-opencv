#!/bin/bash
set -e
PLATFORM=""
TARGET="template_debug"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
case "$(uname -s)" in
    Linux*)  PLATFORM="linux";;
    Darwin*) PLATFORM="macos";;
    MINGW*|MSYS*|CYGWIN*) PLATFORM="windows";;
esac
echo "=== Building godot-cpp ==="
cd godot-cpp && scons platform=$PLATFORM target=$TARGET -j$JOBS && cd ..
echo "=== Building OpenCV GDExtension ==="
scons platform=$PLATFORM target=$TARGET -j$JOBS
echo "=== Build complete ==="
