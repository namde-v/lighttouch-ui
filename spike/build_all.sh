#!/bin/bash
# Builds both halves of the spike from scratch.
set -u -e
SWIFT_BIN=/home/hatch/.local/share/swiftly/toolchains/6.4.1/usr/bin
export PATH="$SWIFT_BIN:$PATH"
export TMPDIR=/home/hatch/.cache/swift-tmp
mkdir -p "$TMPDIR"

HERE="$(cd "$(dirname "$0")" && pwd)"

echo "=== Swift shared library ==="
cd "$HERE/swift"
swift build

echo "=== Qt app ==="
mkdir -p "$HERE/qt/build"
cd "$HERE/qt/build"
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc)"

echo "=== built ==="
ls -la "$HERE/swift/.build/debug/libLTSpike.so" "$HERE/qt/build/spikeapp"
