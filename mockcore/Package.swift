// swift-tools-version: 5.9
import PackageDescription

// LTMockCore — Swift stand-in for libLTAppCore.
//
// Exports the REAL lt_* ABI (per ui/src/lt_bridge.h) from Swift so the Qt
// shell can exercise the true dlopen/dlsym binding path before the real
// library exists. Hand-written interesting functions live in
// Sources/LTMockCore/MockCore.swift; the mechanical remainder is generated
// at CMake configure time into GeneratedStubs.swift (cmake/gen_bridge.py).
//
// This is a dev/test artifact only — never shipped. The real libLTAppCore
// replaces it by pointing LTM_APPCORE_LIB at the real .so (no code change).
let package = Package(
    name: "LTMockCore",
    products: [
        .library(name: "LTMockCore", type: .dynamic, targets: ["LTMockCore"]),
    ],
    targets: [
        .target(name: "LTMockCore"),
    ]
)
