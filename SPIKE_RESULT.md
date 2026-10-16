# Bridge Spike Result — PASS (independently verified)

Date: 2026-10-16 · Phase 5, step 1 (the gate)

Claim proven: Swift core ↔ Qt 6 UI via in-process `@_cdecl` C-ABI bridging
(UI_APPROACH.md §4) works on Linux, headless.

## Verification

Worker-built spike under `ui/spike/` (scratch SwiftPM package `LTSpike`, Qt 6 C++
app `spikeapp`, QML scene). Coordinator re-ran `run_spike.sh` independently:
exit 0, `SPIKE RESULT: PASS: all links verified`.

| Link | Evidence |
|---|---|
| Swift `.so` build | `libLTSpike.so`, `llvm-nm -D` shows 4 C symbols |
| dlopen + dlsym | `RTLD_NOW`; bogus-.so negative control fails cleanly |
| C++ → Swift | `lt_spike_version()` → 42 |
| Swift → C++ callback | `lt_spike_fire_event(1234)` delivered 1234; nil-register + re-register safe |
| Offscreen QML render | `QT_QPA_PLATFORM=offscreen`, 10 frames via `QQuickImageProvider` + `grabWindow()` |
| Pixel check | Frames 0–9: bg color matched frameIndex-derived value; white marker at (7,5) pure white every frame |
| 120-frame stability | 120 calls, byte-compared buffers, no crash/mismatch/leak |
| Non-vacuous | 2×-scaled negative control failed as expected |

## Carry-forward notes for the real `libLTAppCore` build

1. `LD_LIBRARY_PATH` must include `<toolchain>/usr/lib/swift/linux` at dlopen+run,
   or bake `-rpath` / ship Swift runtime libs beside the binary.
2. Toolchain: `/home/hatch/.local/share/swiftly/toolchains/6.4.1/usr/bin` first on
   PATH; `TMPDIR=/home/hatch/.cache/swift-tmp` for every swift invocation.
3. `@_cdecl` exports must be top-level (implicitly nonisolated); the stored C
   callback global needs `nonisolated(unsafe)` + a lock (events post from arbitrary
   Swift threads).
4. QImage `Format_ARGB32` == Swift-written little-endian BGRA `UInt32` — exact match.
5. CI/apt images need `qml6-module-qtquick`, `qml6-module-qtquick-window`,
   `qml6-module-qtqml-workerscript` (not in stock qt6-base-dev).
6. Offscreen `grabWindow()` is color-exact for unscaled 1:1 images — good for the
   shm-frame `QQuickItem` path.

## Open (Windows)

`.def`-file export-list / MSVC+Swift name agreement untested — must be spiked on a
Windows runner before the real UI build. Linux-only proof so far.

`port/` and `upstream/` untouched. Nothing pushed.
