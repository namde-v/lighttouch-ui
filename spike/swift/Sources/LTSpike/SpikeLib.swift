/// LTSpike — scratch Swift shared library for the Qt-bridge spike.
///
/// Proves the @_cdecl C-ABI bridge shape from UI_APPROACH.md §4:
/// Swift code -> @_cdecl exports in a .so -> dlopen/dlsym from C++ -> QML.
/// Plus the reverse direction: a stored C function pointer lets Swift
/// post events back to C++ (the LTEvent callback shape).
///
/// Boundary discipline: only C types cross (Int32, raw pointers, @convention(c)
/// function pointers). Exports are top-level (hence nonisolated). The one
/// piece of mutable global state uses nonisolated(unsafe); the real
/// libLTAppCore will guard this with a lock / serial queue.

// Stored event callback, set by lt_spike_register_callback.
// nonisolated(unsafe): single-writer-at-startup in the spike; the real
// bridge will use a lock because Swift may post events from any thread.
nonisolated(unsafe) private var storedCallback: (@convention(c) (Int32) -> Void)? = nil

/// Simple handshake: lets C++ verify it bound the right library.
@_cdecl("lt_spike_version")
public func lt_spike_version() -> Int32 {
    return 42
}

/// Writes a deterministic BGRA test pattern into the caller's buffer.
///
/// - Solid background color derived from frameIndex:
///     r = (frameIndex * 37) & 0xFF
///     g = (frameIndex * 91 + 17) & 0xFF
///     b = (frameIndex * 53 + 101) & 0xFF, alpha = 0xFF
/// - One fixed white pixel (0xFFFFFFFF) at coordinate (x=7, y=5), row-major.
/// Buffer layout: width*height UInt32 pixels, little-endian BGRA bytes,
/// which is exactly QImage::Format_ARGB32's memory layout on little-endian.
@_cdecl("lt_spike_next_frame")
public func lt_spike_next_frame(
    _ buffer: UnsafeMutableRawPointer,
    _ width: Int32,
    _ height: Int32,
    _ frameIndex: Int32
) {
    let w = Int(width)
    let h = Int(height)
    guard w > 0, h > 0 else { return }

    let r = UInt32((Int(frameIndex) * 37) & 0xFF)
    let g = UInt32((Int(frameIndex) * 91 + 17) & 0xFF)
    let b = UInt32((Int(frameIndex) * 53 + 101) & 0xFF)
    // Little-endian UInt32: bytes are B, G, R, A -> BGRA, alpha opaque.
    let bgPixel = b | (g << 8) | (r << 16) | (0xFF << 24)

    let pixels = buffer.bindMemory(to: UInt32.self, capacity: w * h)
    for i in 0..<(w * h) {
        pixels[i] = bgPixel
    }

    // Fixed white marker pixel at (7, 5).
    let wx = 7, wy = 5
    if wx < w && wy < h {
        pixels[wy * w + wx] = 0xFFFFFFFF
    }
}

/// Stores the C++-side event callback (may be nil to unregister).
@_cdecl("lt_spike_register_callback")
public func lt_spike_register_callback(_ cb: (@convention(c) (Int32) -> Void)?) {
    storedCallback = cb
}

/// Invokes the stored callback with `value`. No-op when none registered.
@_cdecl("lt_spike_fire_event")
public func lt_spike_fire_event(_ value: Int32) {
    storedCallback?(value)
}
