# LightTouch Qt UI shell

Qt 6 (C++/QML) application shell for the LightTouchMac cross-platform port.
The Swift core (`libLTAppCore`) is consumed **only** through the C ABI in
`src/lt_bridge.h` — translated from `ui/ABI.md`; deviations are listed in the
header's top comment.

## One top-level build

```sh
cd ui
cmake -B build && cmake --build build && ctest --test-dir build
```

## Running

```sh
# Mock mode — no library needed; synthesizes devices + events for UI work:
LTM_BRIDGE_MOCK=1 ./build/lighttouch

# Real (or stand-in) library:
LTM_APPCORE_LIB=/path/to/libLTAppCore.so ./build/lighttouch
# or: ./build/lighttouch            # app-dir-relative, then system path
```

Headless: prefix any of the above with `QT_QPA_PLATFORM=offscreen`.

Library resolution order: `$LTM_APPCORE_LIB` → `<appDir>/libLTAppCore.so`
(`LTAppCore.dll` on Windows) → system search path. Missing symbols are a
**clean fatal** listing every missing export — never silent.

## Swapping in the real libLTAppCore

No code change. Build the real library explicitly (it is another worker's
in-progress artifact and intentionally not part of the default build):

```sh
cmake --build build --target lt_appcore   # SwiftPM release build, staged next to the binary
./build/lighttouch                        # app-dir-relative lookup finds build/libLTAppCore.so
```

or point `LTM_APPCORE_LIB` at it. The `Bridge::Api` struct binds all 125 exports at
startup and refuses to start when the library's `lt_abi_version()` is older
than `kLTAbiVersionMin`.

## Swift stand-in (libLTMockCore.so)

`mockcore/` is a tiny SwiftPM package exporting the real `lt_*` ABI with
canned data. It is staged next to the binary at build time and used by
`tst_bridge` (`LT_MOCKCORE_SO`) to prove the true dlopen/dlsym path against a
Swift-built library. Dev-only; never shipped.

## Codegen contract

`cmake/gen_bridge.py` parses `src/lt_bridge.h` at configure time and emits:

- `lt_api_members.inc` / `lt_api_bind.inc` — `Bridge::Api` members + the dlsym
  bind loop (all 125 exports; can never drift from the header),
- `lt_mock_stubs.inc` / `lt_mock_bind.inc` — generic C++ mock implementations,
- `mockcore/…/GeneratedStubs.swift` — generic Swift stand-in implementations.

13 interesting functions have hand-written mocks (`Bridge.cpp`, `MockCore.swift`);
`LT_MOCK_EXCLUDE_CSV` in `cmake/SwiftAppCore.cmake` keeps both emitters in sync.

## Swift runtime (shipping note)

The Swift runtime lives in `<toolchain>/usr/lib/swift/linux`
(see `ui/SPIKE_RESULT.md`). Dev builds set rpath to `$ORIGIN` plus that dir,
and tests also get `LD_LIBRARY_PATH`. Ship `libswiftCore` et al. beside the
binary (`$ORIGIN` covers it) or repoint rpath at a bundled runtime dir.
Windows: same shape with `LTAppCore.dll` + the toolchain's runtime DLLs.

## Layout

- `src/lt_bridge.h` — the C ABI contract (single source of truth).
- `src/Bridge.{h,cpp}` — dlopen/dlsym, event trampoline → queued re-post,
  mock mode.
- `src/adapters/` — one thin QObject per ABI domain (no business logic).
- `qml/` — app shell: menu skeleton, sidebar, display placeholder.
- `tests/` — Qt Test suite (offscreen): `tst_bridge`, `tst_adapters`,
  `tst_applaunch`.
- `cmake/` — `SwiftAppCore.cmake` (SwiftPM targets), `gen_bridge.py`.
- `mockcore/` — Swift stand-in library (dev/test only).
