#!/usr/bin/env bash
# build-appimage.sh — build the LightTouch Qt app and bundle it as an AppImage.
#
# Produces: LightTouch-<version>-x86_64.AppImage containing
#   - the Qt 6 app (usr/bin/lighttouch)
#   - Qt 6 libraries + QML imports (via linuxdeploy-plugin-qt)
#   - libLTAppCore.so (the Swift core, real C ABI — no mock)
#   - the Swift runtime (libswiftCore et al. from the Swift toolchain)
#
# Build host requirements: Ubuntu 22.04 x86_64 (glibc 2.35 baseline, so the
# AppImage also runs on 24.04), Qt 6.4+ (qmake), cmake, a C++17 compiler,
# patchelf, file, curl, python3.
#
# System packages needed on the build host (Ubuntu 22.04):
#   cmake g++ patchelf file curl python3 ninja-build
#   libgl-dev libegl-dev libxkbcommon-dev          # QtGui configure-time deps
#   libgl1 libegl1 libxkbcommon0 libdbus-1-3      # Qt runtime deps
#   libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1
#   libxcb-randr0 libxcb-render-util0 libxcb-shape0 libxcb-sync1
#   libxcb-xfixes0 libxcb-xinerama0 libxcb-xkb1    # xcb platform plugin deps
# Qt itself comes from QT_DIR (aqtinstall recommended: see README below).
#
# Usage:
#   packaging/build-appimage.sh [--output FILE] [--skip-smoke] [--keep-appdir]
#
# Environment:
#   UI_DIR               Qt UI repo root (default: parent of this script's dir)
#   CROSS_DIR            lighttouch-cross checkout (default: <UI_DIR>/../port)
#                        Only used when LT_APPCORE_SO is unset and a Swift
#                        toolchain is available: builds Packages/LTAppCore.
#   LT_APPCORE_SO        prebuilt libLTAppCore.so to bundle (skips Swift build)
#   QT_DIR               Qt install prefix, e.g. /root/Qt/6.4.3/gcc_64
#                        (default: qmake found on PATH, else QT_DIR env)
#   LT_SWIFT_RUNTIME_DIR Swift runtime dir, e.g. <toolchain>/usr/lib/swift/linux
#                        (default: <swift bin>/../lib/swift/linux)
#   TOOLS_DIR            where linuxdeploy/appimagetool are cached
#                        (default: <UI_DIR>/build/packaging-tools)
#   VERSION              app version baked into the file name (default: 0.1.0)
#   BUILD_JOBS           parallel build jobs (default: nproc, max 4)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UI_DIR="${UI_DIR:-$(cd "$SCRIPT_DIR/.." && pwd)}"
CROSS_DIR="${CROSS_DIR:-$(cd "$UI_DIR/../port" 2>/dev/null && pwd || echo "$UI_DIR/../port")}"
VERSION="${VERSION:-0.1.0}"
TOOLS_DIR="${TOOLS_DIR:-$UI_DIR/build/packaging-tools}"
BUILD_DIR="${BUILD_DIR:-$UI_DIR/build/appimage-build}"
APPDIR="${APPDIR:-$UI_DIR/build/AppDir}"
OUTPUT=""
SKIP_SMOKE=0
KEEP_APPDIR=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --output) OUTPUT="$2"; shift 2;;
        --skip-smoke) SKIP_SMOKE=1; shift;;
        --keep-appdir) KEEP_APPDIR=1; shift;;
        *) echo "unknown arg: $1" >&2; exit 2;;
    esac
done
OUTPUT="${OUTPUT:-$UI_DIR/build/LightTouch-${VERSION}-x86_64.AppImage}"

log() { echo "[pack] $*" >&2; }
die() { echo "[pack] ERROR: $*" >&2; exit 1; }
need() { command -v "$1" >/dev/null 2>&1 || die "missing required tool: $1"; }

need cmake; need patchelf; need curl; need python3; file --version >/dev/null 2>&1 || die "missing: file"

# --- Qt -----------------------------------------------------------------------
if [[ -n "${QT_DIR:-}" && -x "$QT_DIR/bin/qmake" ]]; then
    QMAKE="$QT_DIR/bin/qmake"
elif command -v qmake6 >/dev/null 2>&1; then
    QMAKE="$(command -v qmake6)"
elif command -v qmake >/dev/null 2>&1; then
    QMAKE="$(command -v qmake)"
else
    die "no Qt 6 qmake found (set QT_DIR, e.g. QT_DIR=/root/Qt/6.4.3/gcc_64)"
fi
QT_VER="$("$QMAKE" -query QT_VERSION)"
log "qmake: $QMAKE (Qt $QT_VER)"
[[ "${QT_VER%%.*}" == "6" ]] || die "Qt 6 required, found $QT_VER"

# --- libLTAppCore.so -----------------------------------------------------------
resolve_appcore() {
    if [[ -n "${LT_APPCORE_SO:-}" ]]; then
        [[ -f "$LT_APPCORE_SO" ]] || die "LT_APPCORE_SO not found: $LT_APPCORE_SO"
        echo "$LT_APPCORE_SO"; return
    fi
    local pkg="$CROSS_DIR/Packages/LTAppCore"
    if [[ -f "$pkg/Package.swift" ]] && command -v swift >/dev/null 2>&1; then
        log "building libLTAppCore (SwiftPM release) from $pkg"
        (cd "$pkg" && swift build -c release 2>&1 | tail -3 >&2)
        local so
        so="$(find "$pkg/.build" -name libLTAppCore.so -type f 2>/dev/null | head -1)"
        [[ -n "$so" ]] || die "SwiftPM build finished but libLTAppCore.so not found"
        echo "$so"; return
    fi
    die "no libLTAppCore.so: set LT_APPCORE_SO or provide Swift toolchain + $pkg"
}
APPCORE_SO="$(resolve_appcore)"
log "libLTAppCore.so: $APPCORE_SO ($(du -h "$APPCORE_SO" | cut -f1))"

# --- Swift runtime --------------------------------------------------------------
if [[ -z "${LT_SWIFT_RUNTIME_DIR:-}" ]]; then
    if command -v swift >/dev/null 2>&1; then
        SWIFT_BIN="$(command -v swift)"
        LT_SWIFT_RUNTIME_DIR="$(cd "$(dirname "$SWIFT_BIN")/../lib/swift/linux" && pwd)"
    else
        die "no swift on PATH and LT_SWIFT_RUNTIME_DIR unset — cannot bundle the Swift runtime"
    fi
fi
[[ -f "$LT_SWIFT_RUNTIME_DIR/libswiftCore.so" ]] || die "no libswiftCore.so in $LT_SWIFT_RUNTIME_DIR"
log "Swift runtime: $LT_SWIFT_RUNTIME_DIR"

# --- Build the Qt app ------------------------------------------------------------
JOBS="${BUILD_JOBS:-$(nproc)}"; [[ "$JOBS" -gt 4 ]] && JOBS=4
log "configuring Qt app ($JOBS jobs)"
cmake -S "$UI_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$("$QMAKE" -query QT_INSTALL_PREFIX)" \
    -DCMAKE_BUILD_PARALLEL_LEVEL="$JOBS" >"$BUILD_DIR-cmake.log" 2>&1 \
    || { tail -30 "$BUILD_DIR-cmake.log"; die "cmake configure failed"; }
log "building Qt app"
cmake --build "$BUILD_DIR" -j "$JOBS" >"$BUILD_DIR-build.log" 2>&1 \
    || { tail -40 "$BUILD_DIR-build.log"; die "Qt app build failed"; }
APP_BIN="$BUILD_DIR/lighttouch"
[[ -x "$APP_BIN" ]] || die "expected binary missing: $APP_BIN"
log "built: $APP_BIN"

# --- linuxdeploy tooling -----------------------------------------------------------
mkdir -p "$TOOLS_DIR"
fetch_tool() { # name url
    local dest="$TOOLS_DIR/$1"
    if [[ -x "$dest" ]]; then log "tool cached: $1"; return; fi
    log "downloading $1"
    curl -fsSL -o "$dest" "$2" || die "download failed: $2"
    chmod +x "$dest"
}
extract_appimage() { # appimage -> prints AppRun path (logs go to stderr)
    local img="$1" dest="$TOOLS_DIR/$2.extracted"
    if [[ -x "$dest/squashfs-root/AppRun" ]]; then echo "$dest/squashfs-root/AppRun"; return; fi
    log "extracting $(basename "$img") (no FUSE needed)"
    rm -rf "$dest" && mkdir -p "$dest"
    (cd "$dest" && "$img" --appimage-extract >/dev/null 2>&1) \
        || die "failed to extract $img"
    [[ -x "$dest/squashfs-root/AppRun" ]] || die "extraction produced no AppRun: $img"
    echo "$dest/squashfs-root/AppRun"
}
fetch_tool linuxdeploy-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
fetch_tool linuxdeploy-plugin-qt-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
fetch_tool appimagetool-x86_64.AppImage \
    https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
LINUXDEPLOY="$(extract_appimage "$TOOLS_DIR/linuxdeploy-x86_64.AppImage" linuxdeploy)"
PLUGIN_QT="$(extract_appimage "$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage" plugin-qt)"
APPIMAGETOOL="$(extract_appimage "$TOOLS_DIR/appimagetool-x86_64.AppImage" appimagetool)"
# linuxdeploy discovers input plugins as `linuxdeploy-plugin-<name>` on PATH
# (or next to its own binary). The extracted plugin AppRun isn't either, so
# link it into a private bindir.
mkdir -p "$TOOLS_DIR/bin"
ln -sf "$PLUGIN_QT" "$TOOLS_DIR/bin/linuxdeploy-plugin-qt"
export PATH="$TOOLS_DIR/bin:$PATH"

# --- Deploy ------------------------------------------------------------------------
rm -rf "$APPDIR"
mkdir -p "$(dirname "$OUTPUT")"
export QMAKE
export QML_SOURCES_PATHS="$UI_DIR/qml"   # QML lives in qrc: point the plugin at sources
# linuxdeploy resolves the executable's NEEDED entries itself: it must see the
# (possibly non-system) Qt libraries.
QT_LIBDIR="$("$QMAKE" -query QT_INSTALL_LIBS)"
export LD_LIBRARY_PATH="${QT_LIBDIR}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export VERSION
log "running linuxdeploy (Qt plugin)"
"$LINUXDEPLOY" --appdir "$APPDIR" \
    -e "$APP_BIN" \
    -d "$SCRIPT_DIR/lighttouch.desktop" \
    -i "$SCRIPT_DIR/lighttouch.png" \
    --plugin qt >"$BUILD_DIR-deploy.log" 2>&1 \
    || { tail -40 "$BUILD_DIR-deploy.log"; die "linuxdeploy failed"; }
log "linuxdeploy done"

# linuxdeploy-plugin-qt ships only the xcb platform plugin; also bundle
# offscreen + minimal so the AppImage runs headless (CI, servers, ssh).
# Their Qt deps already live in usr/lib; mirror the deployed plugins' rpath.
QT_PLUGINDIR="$("$QMAKE" -query QT_INSTALL_PLUGINS)"
for _p in libqoffscreen.so libqminimal.so; do
    if [[ -f "$QT_PLUGINDIR/platforms/$_p" ]]; then
        cp -f "$QT_PLUGINDIR/platforms/$_p" "$APPDIR/usr/plugins/platforms/"
        patchelf --force-rpath --set-rpath '$ORIGIN/../../lib:$ORIGIN' \
            "$APPDIR/usr/plugins/platforms/$_p"
        log "extra platform plugin: $_p"
    fi
done

# --- Stage libLTAppCore + Swift runtime ----------------------------------------------
log "staging libLTAppCore.so + Swift runtime into AppDir"
cp -f "$APPCORE_SO" "$APPDIR/usr/bin/libLTAppCore.so"
# $ORIGIN rpath so the core finds the bundled Swift runtime regardless of host
patchelf --force-rpath --set-rpath '$ORIGIN' "$APPDIR/usr/bin/libLTAppCore.so"
shopt -s nullglob
SWIFT_LIBS=( "$LT_SWIFT_RUNTIME_DIR"/libswift*.so* "$LT_SWIFT_RUNTIME_DIR"/libdispatch*.so* \
             "$LT_SWIFT_RUNTIME_DIR"/libFoundation*.so* "$LT_SWIFT_RUNTIME_DIR"/libBlocksRuntime*.so* \
             "$LT_SWIFT_RUNTIME_DIR"/lib_FoundationICU.so* )
[[ ${#SWIFT_LIBS[@]} -gt 0 ]] || die "no Swift runtime libs matched in $LT_SWIFT_RUNTIME_DIR"
for lib in "${SWIFT_LIBS[@]}"; do
    [[ -f "$lib" ]] || continue
    cp -fL "$lib" "$APPDIR/usr/bin/"
done
shopt -u nullglob
log "bundled Swift libs: $(ls "$APPDIR/usr/bin"/libswift* "$APPDIR/usr/bin"/libdispatch* 2>/dev/null | wc -l) files"

# Sanity: every NEEDED entry of the staged core must resolve inside the AppDir
# or be a stock system lib.
log "checking staged library linkage"
UNRESOLVED="$(LD_LIBRARY_PATH="$APPDIR/usr/bin:$APPDIR/usr/lib" ldd "$APPDIR/usr/bin/libLTAppCore.so" 2>/dev/null | grep "not found" || true)"
[[ -z "$UNRESOLVED" ]] || die "unresolved deps in staged libLTAppCore.so:\n$UNRESOLVED"
UNRESOLVED_BIN="$(LD_LIBRARY_PATH="$APPDIR/usr/bin:$APPDIR/usr/lib" ldd "$APPDIR/usr/bin/lighttouch" 2>/dev/null | grep "not found" || true)"
[[ -z "$UNRESOLVED_BIN" ]] || die "unresolved deps in staged lighttouch:\n$UNRESOLVED_BIN"

# --- Build the AppImage ---------------------------------------------------------------
log "building AppImage -> $OUTPUT"
ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "$OUTPUT" >"$BUILD_DIR-appimage.log" 2>&1 \
    || { tail -30 "$BUILD_DIR-appimage.log"; die "appimagetool failed"; }
chmod +x "$OUTPUT"
log "AppImage: $OUTPUT ($(du -h "$OUTPUT" | cut -f1))"
sha256sum "$OUTPUT" | tee "$OUTPUT.sha256"

# --- Smoke test: headless launch against the REAL core ---------------------------------
if [[ "$SKIP_SMOKE" -eq 0 ]]; then
    log "smoke test: extracting + launching headless (offscreen)"
    SMOKE_DIR="$BUILD_DIR/smoke"
    rm -rf "$SMOKE_DIR"; mkdir -p "$SMOKE_DIR"
    (cd "$SMOKE_DIR" && "$OUTPUT" --appimage-extract >/dev/null 2>&1) || die "AppImage extraction failed"
    SMOKE_LOG="$BUILD_DIR-smoke.log"
    # The app runs its event loop forever; timeout is expected (124).
    set +e
    QT_QPA_PLATFORM=offscreen timeout 25 "$SMOKE_DIR/squashfs-root/AppRun" >"$SMOKE_LOG" 2>&1
    rc=$?
    set -e
    log "smoke run exit: $rc (124 == still running after 25s: expected)"
    grep -a "Bridge ready (mock: false)" "$SMOKE_LOG" \
        || { log "smoke log:"; cat "$SMOKE_LOG"; die "bridge handshake failed (no 'Bridge ready (mock: false)')"; }
    grep -aq "failed to load qrc:/qml/main.qml" "$SMOKE_LOG" && die "QML failed to load"
    log "SMOKE PASS: bridge handshake OK against real libLTAppCore (mock: false)"
fi

[[ "$KEEP_APPDIR" -eq 1 ]] || rm -rf "$APPDIR"
log "DONE: $OUTPUT"
