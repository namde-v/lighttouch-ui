// FrameRing.h — the Qt side of the helper frame ring (surface 2).
//
// Pure C++ (no Qt): maps the POSIX shm segment from kLTEventDisplayLinkReady
// and implements the app's half of the FrameRingWriter/Reader protocol from
// DeviceRuntime/SharedStatus.swift.
//
// LAYOUT (mock-only until the real helper exists — documented, not guessed:
// the ABI's DisplayLinkReady carries a single fd, so the mock exports ONE
// shm segment laid out as):
//   offset 0:              4096-byte status block, UInt64 slots:
//     [0] magic, [1] layoutVersion (=1), [2] heartbeat, [3] frameSerial,
//     [4] front, [5] width, [6] height, [7] ringGeneration, [8] held
//   offset 4096:           frame 0: height * stride bytes, tightly packed BGRA
//   offset 4096 + 1*frame: frame 1
//   offset 4096 + 2*frame: frame 2
// The real helper (port/Packages/DeviceRuntime) uses 4 separate segments;
// libLTAppCore will re-export whatever the Qt side needs when it lands —
// until then this layout is what the SYNTHETIC mock ring writes and what
// DisplayItem reads. Atomics mirror ltm_shm.h: seq-cst for frameSerial/held,
// acquire for the rest.

#pragma once

#include <atomic>
#include <cstdint>

namespace ltframe {

// Status-block slot indices (mirror StatusSlot in SharedStatus.swift).
constexpr int kSlotMagic = 0;
constexpr int kSlotLayoutVersion = 1;
constexpr int kSlotHeartbeat = 2;
constexpr int kSlotFrameSerial = 3;
constexpr int kSlotFront = 4;
constexpr int kSlotWidth = 5;
constexpr int kSlotHeight = 6;
constexpr int kSlotRingGeneration = 7;
constexpr int kSlotHeld = 8;

constexpr uint64_t kLayoutVersion = 1;
constexpr size_t kStatusBytes = 4096;
constexpr int kFrameCount = 3;

#ifdef _MSC_VER
// MSVC has no __atomic_* builtins. Emulate with MSVC's volatile
// acquire/release semantics plus seq-cst fences (the standard
// Batty-et-al. mappings for seq-cst load/store).
inline uint64_t loadSeq(const volatile uint64_t *p) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return *p;
}
inline void storeSeq(volatile uint64_t *p, uint64_t v) {
    *p = v;
    std::atomic_thread_fence(std::memory_order_seq_cst);
}
inline uint64_t loadAcq(const volatile uint64_t *p) {
    return *p;  // MSVC volatile read is acquire
}
#else
inline uint64_t loadSeq(const volatile uint64_t *p) {
    return __atomic_load_n(p, __ATOMIC_SEQ_CST);
}
inline void storeSeq(volatile uint64_t *p, uint64_t v) {
    __atomic_store_n(p, v, __ATOMIC_SEQ_CST);
}
inline uint64_t loadAcq(const volatile uint64_t *p) {
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}
#endif

// Reader side of the protocol (mirrors FrameRingReader.front()). Runs on the
// UI thread in response to kLTEventFrameAvailable; the render thread only
// reads pixels from the held segment, which the writer never touches.
class Reader {
public:
    Reader() = default;

    void attach(volatile uint64_t *status, uint64_t generation) {
        m_status = status;
        m_generation = generation;
        m_lastSerial = 0;
        m_currentIndex = -1;
    }
    void detach() {
        release();
        m_status = nullptr;
        m_currentIndex = -1;
    }
    bool attached() const { return m_status != nullptr; }

    // Returns the held frame index (0..2), or -1 when nothing new / invalid.
    // After a successful acquire the caller holds that segment until the next
    // acquire or release().
    int acquire() {
        if (!m_status)
            return -1;
        const uint64_t s1 = loadSeq(m_status + kSlotFrameSerial);
        if (s1 == m_lastSerial || loadAcq(m_status + kSlotRingGeneration) != m_generation)
            return m_currentIndex; // nothing new (or ring replaced): keep hold
        const int index = static_cast<int>(loadAcq(m_status + kSlotFront));
        if (index < 0 || index >= kFrameCount)
            return m_currentIndex;
        storeSeq(m_status + kSlotHeld, static_cast<uint64_t>(index + 1));
        if (loadSeq(m_status + kSlotFrameSerial) != s1) {
            // Moved under us: keep the last good one.
            storeSeq(m_status + kSlotHeld, static_cast<uint64_t>(m_currentIndex + 1));
            return m_currentIndex;
        }
        m_lastSerial = s1;
        m_currentIndex = index;
        return index;
    }

    void release() {
        if (m_status)
            storeSeq(m_status + kSlotHeld, 0);
        m_currentIndex = -1;
    }

    int heldIndex() const { return m_currentIndex; }
    uint64_t lastSerial() const { return m_lastSerial; }

private:
    volatile uint64_t *m_status = nullptr;
    uint64_t m_generation = 0;
    uint64_t m_lastSerial = 0;
    int m_currentIndex = -1;
};

} // namespace ltframe
