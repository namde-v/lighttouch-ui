// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "LTSpike",
    products: [
        // Built as a shared library: libLTSpike.so
        .library(name: "LTSpike", type: .dynamic, targets: ["LTSpike"]),
    ],
    targets: [
        .target(name: "LTSpike"),
    ]
)
