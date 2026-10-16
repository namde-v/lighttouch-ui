#!/bin/bash
# Re-runnable bridge spike for the LightTouchMac Qt-bridge claim (UI_APPROACH.md §4).
# Proves: Swift @_cdecl .so -> C++ dlopen/dlsym -> QML offscreen render -> pixel checks.
# Usage: ./run_spike.sh        (rebuilds nothing; builds via build_all.sh first if needed)
set -u
SWIFT_BIN=/home/hatch/.local/share/swiftly/toolchains/6.4.1/usr/bin
export TMPDIR=/home/hatch/.cache/swift-tmp
export LD_LIBRARY_PATH=/home/hatch/.local/share/swiftly/toolchains/6.4.1/usr/lib/swift/linux${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export QT_QPA_PLATFORM=offscreen

HERE="$(cd "$(dirname "$0")" && pwd)"
"$HERE/qt/build/spikeapp" \
    "$HERE/swift/.build/debug/libLTSpike.so" \
    "$HERE/qt/main.qml"
