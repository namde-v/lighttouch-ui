# Linux packaging

## Format decision: AppImage (only, for now)

**AppImage** was chosen as the single Linux format:

- Single file, no root needed, runs on Ubuntu 22.04 **and** 24.04 (built on a
  22.04 host → glibc 2.35 baseline).
- Bundles everything the app needs: Qt 6.4.3, the Swift runtime
  (`libswiftCore` et al.), and the real `libLTAppCore.so` — the target machine
  needs no Qt, no Swift, no extra packages.
- A `.deb` was considered and deferred: Ubuntu's archives only ship Qt 6.2
  while the UI requires Qt 6.4+, so a deb would either bundle Qt anyway
  (duplicating the AppImage) or depend on a third-party PPA. If distro-native
  installation becomes a requirement, the `AppDir` this script assembles is
  the natural input for `dpkg-deb`.

## Building

```sh
# Prerequisites (Ubuntu 22.04 x86_64):
#   Qt 6.4+ via aqtinstall, e.g.
#     pip install aqtinstall
#     aqt install-qt linux desktop 6.4.3 --outputdir /opt/Qt -m qtmultimedia
#   sudo apt install cmake g++ patchelf file curl python3 ninja-build \
#     libgl-dev libegl-dev libxkbcommon-dev libgl1 libegl1 libxkbcommon0 \
#     libdbus-1-3 libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 \
#     libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 libxcb-shape0 \
#     libxcb-sync1 libxcb-xfixes0 libxcb-xinerama0 libxcb-xkb1
#   Swift 6.4+ toolchain (to build libLTAppCore; or pass a prebuilt .so)

# Fastest path — prebuilt core library:
QT_DIR=/opt/Qt/6.4.3/gcc_64 \
LT_APPCORE_SO=/path/to/libLTAppCore.so \
LT_SWIFT_RUNTIME_DIR=/path/to/swift-6.4.1/usr/lib/swift/linux \
packaging/build-appimage.sh

# Or let the script build the core from a lighttouch-cross checkout:
QT_DIR=/opt/Qt/6.4.3/gcc_64 \
CROSS_DIR=/path/to/lighttouch-cross \
LT_SWIFT_RUNTIME_DIR=/path/to/swift-6.4.1/usr/lib/swift/linux \
packaging/build-appimage.sh
```

The script downloads `linuxdeploy`, `linuxdeploy-plugin-qt`, and `appimagetool`
into `build/packaging-tools` on first run, builds the Qt app in release mode,
assembles the `AppDir`, stages `libLTAppCore.so` (with `$ORIGIN` rpath) plus the
Swift runtime beside the binary, emits `LightTouch-<version>-x86_64.AppImage`,
and then **smoke-tests it**: extracts the AppImage and launches the app with
`QT_QPA_PLATFORM=offscreen`, requiring the real-core handshake
(`Bridge ready (mock: false)`) within 25 seconds.

Key environment variables: `UI_DIR`, `CROSS_DIR`, `LT_APPCORE_SO`,
`QT_DIR`, `LT_SWIFT_RUNTIME_DIR`, `TOOLS_DIR`, `BUILD_DIR`, `APPDIR`,
`VERSION`, `BUILD_JOBS`. Flags: `--output FILE`, `--skip-smoke`,
`--keep-appdir`.

## CI

`.github/workflows/appimage.yml` builds the AppImage on `ubuntu-22.04`
(Qt 6.4.3 via `install-qt-action`, Swift via `swiftly`, core built from
`namde-v/lighttouch-cross`), runs the in-script smoke test, and uploads the
AppImage as an artifact. It triggers on **version tags (`v*`) and manual
dispatch only** — a full Swift release build takes 20–40 minutes, so it does
not run on every push.

## What's in the AppImage

- `usr/bin/lighttouch` — the Qt 6 UI
- `usr/bin/libLTAppCore.so` — the real Swift core (release build, C ABI)
- `usr/bin/libswift*.so`, `libdispatch*`, `libFoundation*`, `lib_FoundationICU.so`
- `usr/lib/` — Qt 6.4.3 libraries, QML imports, platform plugins
  (xcb, offscreen, minimal), icon themes bits
- `lighttouch.desktop` + icon

## Known limitations (v0.1.0)

- No code signing on Linux (GPG `.sig`/AppImageUpdate out of scope for now).
- No auto-updates (no AppImageUpdate integration yet).
- Firmware catalog starts empty when no `firmware-catalog.json` is bundled;
  the core logs this and continues — catalog population is a runtime concern.
- `DevicePane.qml` logs a non-fatal QML binding-loop warning at startup
  (pre-existing, also seen in dev runs).
- Built for x86_64 only.
