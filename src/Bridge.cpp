// Bridge.cpp — see Bridge.h for the threading contract.
//
// Portability: this file uses only Qt abstractions (QLibrary, not dlopen;
// QStandardPaths; QJsonDocument) plus strdup/free/snprintf from the C
// library, which MSVC also provides (strdup may warn C4996 there — benign).
// No Linux-only APIs appear in any UI code path; the same sources must
// compile for the Windows build.
//
// The event path, end to end:
//   Swift thread → ltBridgeEventTrampoline (extern "C", any thread)
//     → Bridge::postEvent: deep-copy the LTEvent (strdup strings),
//       lt_free_event_strings(original), queued re-post
//     → Bridge::dispatchEvent (UI thread): payload → QVariantMap,
//       emit ltEventReceived, free the copy
// Adapters only ever see ltEventReceived on the UI thread.

#include "Bridge.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDebug>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <memory>
#ifndef Q_OS_WIN
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <chrono>
#endif
#include <string>
#include <map>

namespace {

// ------------------------------------------------------------------
// Event string-field helpers. The union member with live strings depends
// on kind; these three functions must stay in sync with lt_bridge.h §5.
// ------------------------------------------------------------------

static void freeEventStringFields(LTEvent *e) {
    auto F = [](const char *s) { if (s) ::free(const_cast<char *>(s)); };
    switch (e->kind) {
    case kLTEventBootStage:            F(e->u.bootStage.detail); break;
    case kLTEventNoticeChanged:        F(e->u.noticeChanged.message);
                                      F(e->u.noticeChanged.operation); break;
    case kLTEventZoomChanged:          F(e->u.zoomChanged.mode); break;
    case kLTEventHiddenAppsListed:     F(e->u.hiddenAppsListed.appsJson); break;
    case kLTEventInstallProgress:      F(e->u.installProgress.bundleId);
                                      F(e->u.installProgress.phase); break;
    case kLTEventInstallDone:          F(e->u.installDone.bundleId);
                                      F(e->u.installDone.message); break;
    case kLTEventFilesListed:          F(e->u.filesListed.entriesJson); break;
    case kLTEventFilesProgress:        F(e->u.filesProgress.currentFile); break;
    case kLTEventFilesDone:            F(e->u.filesDone.message); break;
    case kLTEventLogLine:              F(e->u.logLine.source);
                                      F(e->u.logLine.text); break;
    case kLTEventLogRotated:           F(e->u.logRotated.source); break;
    case kLTEventCaptureDone:          F(e->u.captureDone.path);
                                      F(e->u.captureDone.message); break;
    case kLTEventRecordingSaveNeeded:  F(e->u.recordingSaveNeeded.suggestedName); break;
    case kLTEventJobProgress:          F(e->u.jobProgress.entryId);
                                      F(e->u.jobProgress.detail); break;
    case kLTEventJobDone:              F(e->u.jobDone.entryId);
                                      F(e->u.jobDone.message); break;
    case kLTEventToast:                F(e->u.toast.title);
                                      F(e->u.toast.body);
                                      F(e->u.toast.category); break;
    case kLTEventDropAction:           F(e->u.dropAction.action);
                                      F(e->u.dropAction.detailJson); break;
    default: break;  // payload-less kinds, or unknown future kinds
    }
}

static LTEvent *deepCopyEvent(const LTEvent *src) {
    LTEvent *dst = new LTEvent;
    *dst = *src;  // LTEvent is POD
    auto D = [](const char *s) -> const char * { return s ? ::strdup(s) : nullptr; };
    switch (dst->kind) {
    case kLTEventBootStage:            dst->u.bootStage.detail = D(src->u.bootStage.detail); break;
    case kLTEventNoticeChanged:        dst->u.noticeChanged.message = D(src->u.noticeChanged.message);
                                      dst->u.noticeChanged.operation = D(src->u.noticeChanged.operation); break;
    case kLTEventZoomChanged:          dst->u.zoomChanged.mode = D(src->u.zoomChanged.mode); break;
    case kLTEventHiddenAppsListed:     dst->u.hiddenAppsListed.appsJson = D(src->u.hiddenAppsListed.appsJson); break;
    case kLTEventInstallProgress:      dst->u.installProgress.bundleId = D(src->u.installProgress.bundleId);
                                      dst->u.installProgress.phase = D(src->u.installProgress.phase); break;
    case kLTEventInstallDone:          dst->u.installDone.bundleId = D(src->u.installDone.bundleId);
                                      dst->u.installDone.message = D(src->u.installDone.message); break;
    case kLTEventFilesListed:          dst->u.filesListed.entriesJson = D(src->u.filesListed.entriesJson); break;
    case kLTEventFilesProgress:        dst->u.filesProgress.currentFile = D(src->u.filesProgress.currentFile); break;
    case kLTEventFilesDone:            dst->u.filesDone.message = D(src->u.filesDone.message); break;
    case kLTEventLogLine:              dst->u.logLine.source = D(src->u.logLine.source);
                                      dst->u.logLine.text = D(src->u.logLine.text); break;
    case kLTEventLogRotated:           dst->u.logRotated.source = D(src->u.logRotated.source); break;
    case kLTEventCaptureDone:          dst->u.captureDone.path = D(src->u.captureDone.path);
                                      dst->u.captureDone.message = D(src->u.captureDone.message); break;
    case kLTEventRecordingSaveNeeded:  dst->u.recordingSaveNeeded.suggestedName = D(src->u.recordingSaveNeeded.suggestedName); break;
    case kLTEventJobProgress:          dst->u.jobProgress.entryId = D(src->u.jobProgress.entryId);
                                      dst->u.jobProgress.detail = D(src->u.jobProgress.detail); break;
    case kLTEventJobDone:              dst->u.jobDone.entryId = D(src->u.jobDone.entryId);
                                      dst->u.jobDone.message = D(src->u.jobDone.message); break;
    case kLTEventToast:                dst->u.toast.title = D(src->u.toast.title);
                                      dst->u.toast.body = D(src->u.toast.body);
                                      dst->u.toast.category = D(src->u.toast.category); break;
    case kLTEventDropAction:           dst->u.dropAction.action = D(src->u.dropAction.action);
                                      dst->u.dropAction.detailJson = D(src->u.dropAction.detailJson); break;
    default: break;
    }
    return dst;
}

static QVariantMap eventPayloadToMap(const LTEvent *e) {
    QVariantMap m;
    auto S = [](const char *s) -> QString { return s ? QString::fromUtf8(s) : QString(); };
    switch (e->kind) {
    case kLTEventDeviceRowChanged:
        m.insert(QStringLiteral("device"), quint64(e->u.rowChanged.device)); break;
    case kLTEventDeviceStatusChanged:
        m.insert(QStringLiteral("machine"), e->u.statusChanged.machine);
        m.insert(QStringLiteral("running"), bool(e->u.statusChanged.running));
        m.insert(QStringLiteral("sleeping"), bool(e->u.statusChanged.sleeping)); break;
    case kLTEventBootStage:
        m.insert(QStringLiteral("stage"), e->u.bootStage.stage);
        m.insert(QStringLiteral("detail"), S(e->u.bootStage.detail));
        m.insert(QStringLiteral("fraction"), e->u.bootStage.fraction); break;
    case kLTEventNoticeChanged:
        m.insert(QStringLiteral("message"), S(e->u.noticeChanged.message));
        m.insert(QStringLiteral("operation"), S(e->u.noticeChanged.operation));
        m.insert(QStringLiteral("offersErase"), bool(e->u.noticeChanged.offersErase)); break;
    case kLTEventDisplayLinkReady:
        m.insert(QStringLiteral("fd"), e->u.displayLinkReady.fd);
        m.insert(QStringLiteral("width"), e->u.displayLinkReady.width);
        m.insert(QStringLiteral("height"), e->u.displayLinkReady.height);
        m.insert(QStringLiteral("stride"), e->u.displayLinkReady.stride);
        m.insert(QStringLiteral("format"), e->u.displayLinkReady.format);
        m.insert(QStringLiteral("ringGeneration"), quint64(e->u.displayLinkReady.ringGeneration)); break;
    case kLTEventFrameAvailable:
        m.insert(QStringLiteral("frameSeq"), quint64(e->u.frameAvailable.frameSeq));
        m.insert(QStringLiteral("width"), e->u.frameAvailable.width);
        m.insert(QStringLiteral("height"), e->u.frameAvailable.height); break;
    case kLTEventRotationChanged:
        m.insert(QStringLiteral("degrees"), e->u.rotationChanged.degrees);
        m.insert(QStringLiteral("width"), e->u.rotationChanged.width);
        m.insert(QStringLiteral("height"), e->u.rotationChanged.height); break;
    case kLTEventZoomChanged:
        m.insert(QStringLiteral("points"), e->u.zoomChanged.points);
        m.insert(QStringLiteral("mode"), S(e->u.zoomChanged.mode)); break;
    case kLTEventFingerDotsChanged:
        m.insert(QStringLiteral("shown"), bool(e->u.fingerDotsChanged.shown)); break;
    case kLTEventHiddenAppsListed:
        m.insert(QStringLiteral("appsJson"), S(e->u.hiddenAppsListed.appsJson)); break;
    case kLTEventInstallProgress:
        m.insert(QStringLiteral("bundleId"), S(e->u.installProgress.bundleId));
        m.insert(QStringLiteral("fraction"), e->u.installProgress.fraction);
        m.insert(QStringLiteral("phase"), S(e->u.installProgress.phase)); break;
    case kLTEventInstallDone:
        m.insert(QStringLiteral("bundleId"), S(e->u.installDone.bundleId));
        m.insert(QStringLiteral("ok"), bool(e->u.installDone.ok));
        m.insert(QStringLiteral("message"), S(e->u.installDone.message)); break;
    case kLTEventFilesListed:
        m.insert(QStringLiteral("requestId"), quint64(e->u.filesListed.requestId));
        m.insert(QStringLiteral("entriesJson"), S(e->u.filesListed.entriesJson)); break;
    case kLTEventFilesProgress:
        m.insert(QStringLiteral("transferId"), quint64(e->u.filesProgress.transferId));
        m.insert(QStringLiteral("fraction"), e->u.filesProgress.fraction);
        m.insert(QStringLiteral("currentFile"), S(e->u.filesProgress.currentFile));
        m.insert(QStringLiteral("bytesDone"), qint64(e->u.filesProgress.bytesDone));
        m.insert(QStringLiteral("bytesTotal"), qint64(e->u.filesProgress.bytesTotal)); break;
    case kLTEventFilesDone:
        m.insert(QStringLiteral("transferId"), quint64(e->u.filesDone.transferId));
        m.insert(QStringLiteral("ok"), bool(e->u.filesDone.ok));
        m.insert(QStringLiteral("cancelled"), bool(e->u.filesDone.cancelled));
        m.insert(QStringLiteral("message"), S(e->u.filesDone.message)); break;
    case kLTEventFilesConnectionChanged:
        m.insert(QStringLiteral("state"), e->u.filesConnectionChanged.state); break;
    case kLTEventLogLine:
        m.insert(QStringLiteral("source"), S(e->u.logLine.source));
        m.insert(QStringLiteral("text"), S(e->u.logLine.text));
        m.insert(QStringLiteral("offset"), quint64(e->u.logLine.offset)); break;
    case kLTEventLogRotated:
        m.insert(QStringLiteral("source"), S(e->u.logRotated.source)); break;
    case kLTEventCaptureDone:
        m.insert(QStringLiteral("kind"), e->u.captureDone.kind);
        m.insert(QStringLiteral("path"), S(e->u.captureDone.path));
        m.insert(QStringLiteral("ok"), bool(e->u.captureDone.ok));
        m.insert(QStringLiteral("message"), S(e->u.captureDone.message)); break;
    case kLTEventRecordingTick:
        m.insert(QStringLiteral("elapsedSeconds"), quint64(e->u.recordingTick.elapsedSeconds)); break;
    case kLTEventRecordingPhaseChanged:
        m.insert(QStringLiteral("phase"), e->u.recordingPhaseChanged.phase); break;
    case kLTEventRecordingSaveNeeded:
        m.insert(QStringLiteral("suggestedName"), S(e->u.recordingSaveNeeded.suggestedName)); break;
    case kLTEventJobProgress:
        m.insert(QStringLiteral("entryId"), S(e->u.jobProgress.entryId));
        m.insert(QStringLiteral("fraction"), e->u.jobProgress.fraction);
        m.insert(QStringLiteral("detail"), S(e->u.jobProgress.detail));
        m.insert(QStringLiteral("etaSeconds"), e->u.jobProgress.etaSeconds);
        m.insert(QStringLiteral("step"), e->u.jobProgress.step);
        m.insert(QStringLiteral("steps"), e->u.jobProgress.steps); break;
    case kLTEventJobDone:
        m.insert(QStringLiteral("entryId"), S(e->u.jobDone.entryId));
        m.insert(QStringLiteral("ok"), bool(e->u.jobDone.ok));
        m.insert(QStringLiteral("cancelled"), bool(e->u.jobDone.cancelled));
        m.insert(QStringLiteral("message"), S(e->u.jobDone.message)); break;
    case kLTEventTaskbarProgress:
        m.insert(QStringLiteral("fraction"), e->u.taskbarProgress.fraction); break;
    case kLTEventToast:
        m.insert(QStringLiteral("title"), S(e->u.toast.title));
        m.insert(QStringLiteral("body"), S(e->u.toast.body));
        m.insert(QStringLiteral("category"), S(e->u.toast.category)); break;
    case kLTEventDropAction:
        m.insert(QStringLiteral("requestId"), quint64(e->u.dropAction.requestId));
        m.insert(QStringLiteral("action"), S(e->u.dropAction.action));
        m.insert(QStringLiteral("detailJson"), S(e->u.dropAction.detailJson)); break;
    case kLTEventNetworkAccessChanged:
        m.insert(QStringLiteral("state"), e->u.networkAccessChanged.state); break;
    default: break;  // payload-less or unknown kinds: empty map
    }
    return m;
}

// The C trampoline handed to Swift. Runs on an arbitrary Swift thread:
// no Qt work here — just hand off to Bridge::postEvent.
extern "C" void ltBridgeEventTrampoline(const LTEvent *event, void *ctx) {
    if (event && ctx)
        static_cast<Bridge *>(ctx)->postEvent(event);
}

} // namespace

// ------------------------------------------------------------------
// Mock core (LTM_BRIDGE_MOCK=1). Hand-written interesting functions;
// the mechanical remainder comes from lt_mock_stubs.inc (generated).
// The mock owns its strings via strdup; mock_lt_free_string = free.
// ------------------------------------------------------------------

namespace {

// Stored event callback for the mock (mirrors the Swift side's global).
static lt_event_fn g_mockCallback = nullptr;
static void *g_mockCallbackCtx = nullptr;

// The Bridge in mock mode, so hand-written mocks (boot/halt) can reach the
// synthetic frame ring.
static Bridge *g_mockBridge = nullptr;
// One shm segment laid out per src/items/FrameRing.h. The writer thread
// publishes animated test frames at ~30fps using the real
// FrameRingWriter/Reader protocol. The real helper's ring replaces this when
// libLTAppCore lands; the DisplayItem cannot tell the difference.
// (POSIX-only: on Windows the mock exports no ring yet.)
#ifndef Q_OS_WIN
class MockFrameRing {
public:
    static constexpr int kWidth = 320;
    static constexpr int kHeight = 480;

    int fd = -1;
    uint8_t *base = nullptr;
    size_t size = 0;
    std::atomic<bool> stopFlag{false};
    std::thread writer;

    bool start() {
        const int stride = kWidth * 4;
        size = 4096 + 3 * static_cast<size_t>(kHeight) * stride;
        char name[64];
        snprintf(name, sizeof(name), "/lt-mock-ring-%d-%lu",
                 static_cast<int>(::getpid()),
                 static_cast<unsigned long>(::time(nullptr)));
        fd = ::shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
        if (fd < 0)
            return false;
        ::shm_unlink(name);
        if (::ftruncate(fd, static_cast<off_t>(size)) != 0) {
            ::close(fd);
            fd = -1;
            return false;
        }
        base = static_cast<uint8_t *>(
            ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (base == MAP_FAILED) {
            ::close(fd);
            fd = -1;
            base = nullptr;
            return false;
        }
        volatile uint64_t *st = status();
        st[0] = 0x4C544652494E4700ULL; // "LTFRING\0" magic
        st[1] = 1;                     // layoutVersion
        st[4] = 3;                     // front: nothing published yet
        st[5] = kWidth;
        st[6] = kHeight;
        st[7] = 1;                     // ringGeneration
        writer = std::thread([this]() { writeLoop(); });
        return true;
    }

    void stop() {
        stopFlag.store(true);
        if (writer.joinable())
            writer.join();
        if (base) {
            ::munmap(base, size);
            base = nullptr;
        }
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    volatile uint64_t *status() const {
        return reinterpret_cast<volatile uint64_t *>(base);
    }
    int dupFd() const { return ::dup(fd); }
    uint64_t latestSerial() const {
        return __atomic_load_n(status() + 3, __ATOMIC_SEQ_CST);
    }

private:
    void writeLoop() {
        const int stride = kWidth * 4;
        uint64_t serial = 0;
        int frame = 0;
        while (!stopFlag.load()) {
            volatile uint64_t *st = status();
            const int front = static_cast<int>(
                __atomic_load_n(st + 4, __ATOMIC_ACQUIRE));
            const int held = static_cast<int>(
                __atomic_load_n(st + 8, __ATOMIC_SEQ_CST)) - 1;
            int idx = -1;
            for (int i = 0; i < 3; ++i) {
                if (i != front && i != held) {
                    idx = i;
                    break;
                }
            }
            if (idx >= 0) {
                drawFrame(base + 4096 + static_cast<size_t>(idx) * kHeight * stride,
                          frame);
                __atomic_store_n(st + 4, static_cast<uint64_t>(idx), __ATOMIC_RELEASE);
                __atomic_store_n(st + 3, ++serial, __ATOMIC_SEQ_CST);
                ++frame;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

    void drawFrame(uint8_t *dst, int frame) {
        const int stride = kWidth * 4;
        // Animated rainbow bars + a bouncing white square + a frame-parity
        // stripe, so tests and eyes can tell frames apart.
        static const uint8_t palette[6][3] = {
            {0x5B, 0xCE, 0xFA}, // light blue
            {0xF5, 0xA9, 0xB8}, // pink
            {0xFF, 0xFF, 0xFF}, // white
            {0xF5, 0xA9, 0xB8}, // pink
            {0x5B, 0xCE, 0xFA}, // light blue
            {0x3A, 0x3A, 0x3A}, // dark gray
        };
        const int bx = (frame * 7) % (kWidth - 40);
        const int by = (frame * 5) % (kHeight - 40);
        for (int y = 0; y < kHeight; ++y) {
            uint8_t *row = dst + static_cast<size_t>(y) * stride;
            for (int x = 0; x < kWidth; ++x) {
                const int bar = ((x * 6 / kWidth) + frame / 12) % 6;
                uint8_t b = palette[bar][0], g = palette[bar][1], r = palette[bar][2];
                if (x >= bx && x < bx + 40 && y >= by && y < by + 40) {
                    b = 0xFF; g = 0xFF; r = 0xFF;
                }
                if ((frame / 30) % 2 == 0 && y < 8) {
                    b = 0x00; g = 0x00; r = 0x00;
                }
                row[x * 4 + 0] = b;
                row[x * 4 + 1] = g;
                row[x * 4 + 2] = r;
                row[x * 4 + 3] = 0xFF;
            }
        }
    }
};
#endif // Q_OS_WIN (MockFrameRing)
struct MockDevice {
    std::string uuid;
    std::string name;
    std::string board;
    std::string firmware;
    int status;  // kLTDeviceRowState…
    std::string statusText;
    std::string firmwareNote;  // empty = none
    bool prepared;  // has on-disk device data (delete asks; else instant remove)
};

// Seed data; copied into g_mockDevices by resetMockDevices() on every
// mock_lt_init so each Bridge (and each test) starts from the same state.
static const MockDevice kMockDeviceSeed[] = {
    { "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617", "iPhone 3GS", "n88ap",
      "iPhone OS 3.1.3", kLTDeviceRowStateOff, "Off", "", true },
    { "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7", "iPod touch (2nd gen)", "n72ap",
      "iOS 4.2.1", kLTDeviceRowStateRunning, "Running", "Jailbroken", true },
    { "a1b2c3d4-e5f6-4789-abcd-ef0123456789", "iPad", "k48ap",
      "iOS 5.1.1", kLTDeviceRowStateBooting, "Booting\xE2\x80\xA6", "", true },
};
static const int kMockDeviceSeedCount = 3;

// Mutable library: rename/delete/create mutate this, mirroring the real
// core's guarded-delete semantics closely enough for UI work.
static std::vector<MockDevice> g_mockDevices;
static lt_device_t g_mockNextHandle = 100;

static void resetMockJobs(); // defined with the jobs mock below
static void resetMockDevices() {
    g_mockDevices.assign(std::begin(kMockDeviceSeed), std::end(kMockDeviceSeed));
    g_mockNextHandle = 100;
    resetMockJobs();
}

static const MockDevice *mockDeviceByHandle(lt_device_t d) {
    if (d >= 1 && d <= static_cast<lt_device_t>(g_mockDevices.size()))
        return &g_mockDevices[d - 1];
    return nullptr;
}

static MockDevice *mockDeviceByHandleMut(lt_device_t d) {
    return const_cast<MockDevice *>(mockDeviceByHandle(d));
}

static std::string jsonEscape(const std::string &s) {
    std::string o;
    for (char c : s) {
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else o += c;
    }
    return o;
}

static std::string mockDeviceRowJson(const MockDevice *dev) {
    std::string s = "{\"id\":\"";
    s += dev->uuid;
    s += "\",\"name\":\"";
    s += jsonEscape(dev->name);
    s += "\",\"board\":\"";
    s += dev->board;
    s += "\",\"firmware\":\"";
    s += jsonEscape(dev->firmware);
    s += "\",\"status\":";
    s += std::to_string(dev->status);
    s += ",\"statusText\":\"";
    s += jsonEscape(dev->statusText);
    s += "\",\"busy\":null,\"firmwareNote\":";
    if (!dev->firmwareNote.empty()) { s += "\""; s += jsonEscape(dev->firmwareNote); s += "\""; }
    else s += "null";
    // Mock-only field: the real core does not expose this yet; the Qt delete
    // guard reads it when present and treats a missing field as "prepared"
    // (safe: asks rather than silently removing). See UI_COVERAGE.md §2 s1.
    s += ",\"prepared\":";
    s += dev->prepared ? "true" : "false";
    s += "}";
    return s;
}

// Fires kLTEventDeviceListChanged through the stored callback (the real
// core emits this after every library mutation).
static void mockFireListChangedNow() {
    if (!g_mockCallback) return;
    LTEvent e{};
    e.kind = kLTEventDeviceListChanged;
    g_mockCallback(&e, g_mockCallbackCtx);
}

static int32_t mock_lt_abi_version(void) { return kLTAbiVersionCurrent; }
static std::map<std::string, std::string> mockSettingsStore;


static LTResult mock_lt_init(const char *configJson) {
    (void)configJson;
    resetMockDevices();
    mockSettingsStore.clear();
    return kLTOk;
}

static LTResult mock_lt_shutdown(void) { return kLTOk; }

static LTResult mock_lt_set_event_callback(lt_event_fn fn, void *ctx) {
    g_mockCallback = fn;
    g_mockCallbackCtx = ctx;
    return kLTOk;
}

static void mock_lt_free_string(const char *s) { ::free(const_cast<char *>(s)); }

static void mock_lt_free_event_strings(const LTEvent *e) {
    freeEventStringFields(const_cast<LTEvent *>(e));
}

static const char *mock_lt_last_error(void) {
    return ::strdup("mock core: no failure recorded on this thread");
}

static LTResult mock_lt_version(const char **out) {
    if (out) *out = ::strdup("0.0.0-mock");
    return kLTOk;
}

static LTResult mock_lt_device_list(const char **out) {
    std::string s = "[";
    for (size_t i = 0; i < g_mockDevices.size(); ++i) {
        if (i) s += ",";
        s += mockDeviceRowJson(&g_mockDevices[i]);
    }
    s += "]";
    if (out) *out = ::strdup(s.c_str());
    return kLTOk;
}

static LTResult mock_lt_device_row(lt_device_t device, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    if (out) *out = ::strdup(mockDeviceRowJson(dev).c_str());
    return kLTOk;
}

static LTResult mock_lt_device_find(const char *uuid, lt_device_t *out) {
    if (!uuid) return kLTBadArgument;
    for (size_t i = 0; i < g_mockDevices.size(); ++i) {
        if (g_mockDevices[i].uuid == uuid) {
            if (out) *out = static_cast<lt_device_t>(i + 1);
            return kLTOk;
        }
    }
    return kLTNoSuchDevice;
}

static LTResult mock_lt_device_uuid(lt_device_t device, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    if (out) *out = ::strdup(dev->uuid.c_str());
    return kLTOk;
}

static LTResult mock_lt_device_create(const char *entryId, const char *name, lt_device_t *out) {
    if (!name) return kLTBadArgument;
    MockDevice dev;
    char uuidBuf[64];
    ::snprintf(uuidBuf, sizeof(uuidBuf), "mock-created-%llu",
               static_cast<unsigned long long>(g_mockNextHandle));
    dev.uuid = uuidBuf;
    dev.name = name;
    dev.board = "n88ap";
    dev.firmware = entryId ? entryId : "unprepared";
    dev.status = kLTDeviceRowStateOff;
    dev.statusText = "Off";
    dev.prepared = false;  // a fresh row: sidebar-only until prepared
    g_mockDevices.push_back(dev);
    const lt_device_t handle = static_cast<lt_device_t>(g_mockDevices.size());
    ++g_mockNextHandle;
    if (out) *out = handle;
    mockFireListChangedNow();
    return kLTOk;
}

static LTResult mock_lt_device_delete(lt_device_t device) {
    if (device < 1 || device > static_cast<lt_device_t>(g_mockDevices.size()))
        return kLTNoSuchDevice;
    g_mockDevices.erase(g_mockDevices.begin() + (device - 1));
    mockFireListChangedNow();
    return kLTOk;
}

static LTResult mock_lt_device_rename(lt_device_t device, const char *name) {
    MockDevice *dev = mockDeviceByHandleMut(device);
    if (!dev) return kLTNoSuchDevice;
    if (!name) return kLTBadArgument;
    dev->name = name;
    mockFireListChangedNow();
    return kLTOk;
}

// Mock-only firmware job: the iPad row (uuid …6789) is mid-prepare so the
// placeholder's per-job progress bar has something to bind. "deviceUuid" is
// a mock-only field (see UI_COVERAGE.md §2 s1); the real core will expose the
// row↔job linkage when it lands.
//
// The list is dynamic: job_start adds, job_cancel removes, so the adapter's
// refresh-on-done actually observes a change (MOCK-ONLY).
static std::string g_mockJobsJson;
static void resetMockJobs() {
    g_mockJobsJson =
        "[{\"entryId\":\"ipsw-k48ap-5.1.1\","
        "\"deviceUuid\":\"a1b2c3d4-e5f6-4789-abcd-ef0123456789\","
        "\"kind\":\"prepare\",\"progress\":0.42,"
        "\"preparation\":{\"step\":2,\"steps\":5,\"name\":\"Preparing\","
        "\"fraction\":0.42,\"detail\":\"Decrypting root filesystem\","
        "\"remaining\":\"12:34\"},\"failed\":null}]";
}
static LTResult mock_lt_jobs_list(const char **out) {
    if (out) *out = ::strdup(g_mockJobsJson.c_str());
    return kLTOk;
}

// Extension-only drop classification (mirrors DroppedFiles' extension
// check; ABI.md §3.10 notes classification is synchronous and cheap).
static LTResult mock_lt_drop_classify(const char *path, int32_t *out) {
    if (!path) return kLTBadArgument;
    std::string p = path;
    for (char &c : p) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    const std::string::size_type dot = p.find_last_of('.');
    const std::string ext = (dot == std::string::npos) ? "" : p.substr(dot);
    int32_t cls = kLTDropUnsupported;
    if (ext == ".ipsw") cls = kLTDropIPSW;
    else if (ext == ".ipa") cls = kLTDropIPA;
    else if (ext == ".mp3" || ext == ".m4a" || ext == ".aac" || ext == ".wav"
             || ext == ".mp4" || ext == ".m4v" || ext == ".mov"
             || ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif"
             || ext == ".bmp")
        cls = kLTDropMedia;
    if (out) *out = cls;
    return kLTOk;
}

// Plausible DeviceMenuState.validate answers from the mock device state:
// a running row's guest accepts input; anything else is dimmed.
// In-memory string prefs for lt_settings_get/set_string (the generated stub
// returns "{}", which can't round-trip).

static LTResult mock_lt_settings_get_string(const char *key, const char **out) {
    const auto it = mockSettingsStore.find(key ? key : "");
    const std::string v = (it == mockSettingsStore.end()) ? "" : it->second;
    if (out) *out = ::strdup(v.c_str());
    return kLTOk;
}

static LTResult mock_lt_settings_set_string(const char *key, const char *value) {
    mockSettingsStore[key ? key : ""] = value ? value : "";
    return kLTOk;
}

// Plausible catalog entries for the Add Device sheet: three boards, a mix of
// statuses (available / experimental / comingSoon / userIPSW). Field schema
// per lt_bridge.h §7.10.
static LTResult mock_lt_catalog_list(const char *filterJson, const char **out) {
    (void)filterJson;
    static const char *json = R"JSON([
{"id":"n88ap-3.1.3","board":"n88ap","productType":"iPhone2,1","marketingName":"iPhone 3GS","version":"3.1.3","build":"7E18","status":"available","statusNote":null,"prereleaseBadge":null,"media":[],"released":"2010-02-02","managesApps":true,"estimates":{"preparedBytes":2147483648}},
{"id":"n88ap-3.1.2","board":"n88ap","productType":"iPhone2,1","marketingName":"iPhone 3GS","version":"3.1.2","build":"7D11","status":"available","statusNote":null,"prereleaseBadge":null,"media":[],"released":"2009-10-08","managesApps":true,"estimates":{"preparedBytes":2147483648}},
{"id":"n72ap-4.2.1","board":"n72ap","productType":"iPod2,1","marketingName":"iPod touch (2nd gen)","version":"4.2.1","build":"8C148","status":"available","statusNote":null,"prereleaseBadge":null,"media":[],"released":"2010-11-22","managesApps":true,"estimates":{"preparedBytes":1073741824}},
{"id":"n72ap-4.2b3","board":"n72ap","productType":"iPod2,1","marketingName":"iPod touch (2nd gen)","version":"4.2","build":"8C5101c","status":"experimental","statusNote":"Untested on this board","prereleaseBadge":"Beta 3","media":[],"released":null,"managesApps":true,"estimates":{"preparedBytes":1073741824}},
{"id":"k48ap-5.1.1","board":"k48ap","productType":"iPad1,1","marketingName":"iPad","version":"5.1.1","build":"9B206","status":"available","statusNote":null,"prereleaseBadge":null,"media":[],"released":"2012-05-07","managesApps":true,"estimates":{"preparedBytes":3221225472}},
{"id":"k48ap-6.0","board":"k48ap","productType":"iPad1,1","marketingName":"iPad","version":"6.0","build":"10A403","status":"comingSoon","statusNote":"Not yet prepared","prereleaseBadge":null,"media":[],"released":null,"managesApps":true,"estimates":{"preparedBytes":3221225472}},
{"id":"k48ap-5.0","board":"k48ap","productType":"iPad1,1","marketingName":"iPad","version":"5.0","build":"9A334","status":"userIPSW","statusNote":null,"prereleaseBadge":null,"media":[],"released":"2011-10-12","managesApps":true,"estimates":{"preparedBytes":3221225472}}
])JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

static LTResult mock_lt_about_credits(const char **out) {
    static const char *json = R"({"credits":[
{"text":"LightTouch","isHeading":true,"isDetail":false},
{"text":"Classic iPhone, iPod and iPad emulation.","isHeading":false,"isDetail":true},
{"text":"Show Licenses","isHeading":false,"isDetail":false,"link":"lighttouch:licenses"}],
"licenses":[
{"name":"MockCore","text":"Mock license text.\n\nPermission is hereby granted, free of charge."},
{"name":"Qt","text":"The Qt framework is available under the GNU Lesser General Public License v3."}]})";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

static LTResult mock_lt_help_topics(const char **out) {
    static const char *json = R"([
{"title":"Getting Started","body":"Add a device from the firmware catalog, then start it. Your [Device] appears in the sidebar."},
{"title":"Installing Apps","body":"Drag a decrypted .ipa onto your [Device], or use Apps > Install App."},
{"title":"Keyboard","body":"With your [Device] running, keystrokes go to the guest when Send Keyboard Input is on."}])";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

static LTResult mock_lt_device_boot(lt_device_t device, const char *optionsJson) {
    (void)optionsJson;
    MockDevice *dev = mockDeviceByHandleMut(device);
    if (!dev)
        return kLTNoSuchDevice;
    dev->status = kLTDeviceRowStateRunning;
    dev->statusText = "Running";
    // The synthetic display link: the DisplayItem cannot tell it from the
    // real helper's ring (MOCK-ONLY).
    if (g_mockBridge)
        g_mockBridge->mockStartDisplayLink(device);
    mockFireListChangedNow();
    return kLTAccepted;
}

static LTResult mock_lt_device_halt(lt_device_t device) {
    MockDevice *dev = mockDeviceByHandleMut(device);
    if (!dev)
        return kLTNoSuchDevice;
    dev->status = kLTDeviceRowStateOff;
    dev->statusText = "Off";
    if (g_mockBridge)
        g_mockBridge->mockStopDisplayLink(device);
    mockFireListChangedNow();
    return kLTAccepted;
}

static LTResult mock_lt_menu_validate(lt_device_t device, int32_t item, int32_t arg, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    const bool running = dev->status == kLTDeviceRowStateRunning;
    std::string title, checked;
    bool enabled = false, hasTitle = false, hasChecked = false;
    switch (item) {
    case kLTMenuItemLock:  // 1
        enabled = running; hasTitle = true;
        title = running ? "Lock" : "Lock";
        break;
    case kLTMenuItemRotate:  // 2
        enabled = running;
        break;
    case kLTMenuItemCompassHeading:  // 3
        enabled = running; hasChecked = true;
        checked = (arg == 0) ? "true" : "false";
        break;
    case kLTMenuItemCarrier:  // 4
        enabled = running;
        break;
    case kLTMenuItemInput:  // 5
        enabled = running;
        break;
    case kLTMenuItemPause:  // 6
        enabled = running; hasTitle = true; hasChecked = true;
        title = "Pause"; checked = "false";
        break;
    case kLTMenuItemRestart:  // 7
        enabled = running;
        break;
    default:
        break;
    }
    std::string s = "{\"enabled\":";
    s += enabled ? "true" : "false";
    s += ",\"title\":";
    s += hasTitle ? ("\"" + title + "\"") : "null";
    s += ",\"checked\":";
    s += hasChecked ? checked : "null";
    s += "}";
    if (out) *out = ::strdup(s.c_str());
    return kLTOk;
}

static LTResult mock_lt_device_status(lt_device_t device, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    const int running = (dev->status == kLTDeviceRowStateRunning) ? 1 : 0;
    const int machine = running ? kLTVMStateRunning
                      : (dev->status == kLTDeviceRowStateBooting) ? kLTVMStateStarting
                      : kLTVMStateOff;
    char buf[512];
    ::snprintf(buf, sizeof(buf),
        "{\"machine\":%d,\"running\":%d,\"sleeping\":0,\"paused\":0,"
        "\"shuttingDown\":0,\"storageFailed\":0,\"installing\":0,"
        "\"acceptsInput\":%d,\"hasCompass\":%d,\"hasCellular\":%d,"
        "\"compassHeading\":0,\"editingText\":0}",
        machine, running, running, running, running);
    if (out) *out = ::strdup(buf);
    return kLTOk;
}

// ------------------------------------------------------------------
// Hand-written mocks for surfaces 3–9, 12–14 (MOCK-ONLY).
//
// Everything below is fenced MOCK-ONLY: canned JSON snapshots and
// scripted event sequences so the Qt UI can be built and tested before
// libLTAppCore lands. Nothing here ships or influences the real core.
// Field schemas follow ABI.md §3 / lt_bridge.h; where the ABI leaves the
// schema open (storage, catalog details) the mock invents a reasonable one
// and says so. Keep LT_MOCK_EXCLUDE_CSV (cmake/SwiftAppCore.cmake) and
// mockcore/Sources/LTMockCore/MockCore.swift in sync when adding entries.
// ------------------------------------------------------------------

// --- Surface 4: inspector panels -------------------------------------
// Tweaks (TweaksPanelModel): two tweaks, one unavailable on this firmware,
// plus finger-dots state carried separately by lt_display_get_finger_dots.
static LTResult mock_lt_tweaks_get(lt_device_t device, const char **out) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    static const char *json = R"JSON({
"running":true,
"tweaks":[
 {"id":"fingerDots","title":"Show Touches","caption":"View \u25b8 Show Finger Dots","on":false,"available":true,"section":"Interface"},
 {"id":"coreAnimationColors","title":"Core Animation Flashing","caption":"Flashes Core Animation layers","on":true,"available":true,"section":"Graphics"},
 {"id":"timeMachine","title":"Time Machine","caption":"Pins the guest clock","on":false,"available":false,"section":"System"}],
"color":2,"clock":null,"status":null,"message":null,"hiddenApps":null})JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

// Carrier (CarrierPanelModel): the 3GS's modem state for the mock iPhone 3GS.
static LTResult mock_lt_carrier_get(lt_device_t device, const char **out) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    static const char *json = R"JSON({
"carrierName":"Light Touch","mcc":"001","mnc":"01",
"settings":{"registered":true,"simPresent":true,"bars":4},
"status":{"emergencyCall":false,"lastDialed":"15555550123","hasGPS":true},
"sent":[{"id":"sms-1","number":"15555550100","text":"On my way"}],
"message":null,"networkValid":true,"networkEdited":false,"applyingNetwork":false,
"callNumber":"","smsNumber":"","smsText":"",
"callState":"idle","canRing":true,"canAnswer":false,"canHangUp":false,
"latitude":"37.3349","longitude":"-122.0090","walking":false,
"reportedLocation":{"latitude":37.3349,"longitude":-122.009}})JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

// Storage: mock-invented schema (ABI.md §6 gap 6: the Mac view's backers
// live in the app target; the real lt_storage_get schema is TBD on the
// Swift side). Mirrors StorageSettingsView's sections: devices, IPSWs,
// decrypted caches, logs, IPA library.
static LTResult mock_lt_storage_get(lt_device_t device, const char **out) {
    (void)device;
    static const char *json = R"JSON({
"devices":[
 {"id":"3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617","name":"iPhone 3GS","firmware":"iPhone OS 3.1.3",
  "baseBytes":2147483648,"dataBytes":536870912,"snapshotBytes":134217728,"canDelete":true},
 {"id":"a1b2c3d4-e5f6-4789-abcd-ef0123456789","name":"iPad","firmware":"iOS 5.1.1",
  "baseBytes":3221225472,"dataBytes":1073741824,"snapshotBytes":0,"canDelete":false}],
"ipsws":[
 {"entryId":"n88ap-3.1.3","title":"iPhone 3GS iOS 3.1.3","kind":"downloaded","file":null,"bytes":2147483648,"busy":false},
 {"entryId":"k48ap-5.0","title":"iPad iOS 5.0","kind":"referenced","file":"~/firmware/iPad1,1_5.0_9A334_Restore.ipsw","bytes":0,"busy":true}],
"decryptedBytes":1073741824,"logsBytes":16777216,
"libraryIpas":3,"libraryBytes":268435456,"unusedBytes":67108864})JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

// Proxy: WebProxyConfiguration JSON; the set call round-trips in memory.
static std::string g_mockProxyJson =
    R"JSON({"mode":"off","archiveDate":"2026-01-01"})JSON";
static LTResult mock_lt_proxy_get(lt_device_t device, const char **out) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (out) *out = ::strdup(g_mockProxyJson.c_str());
    return kLTOk;
}
static LTResult mock_lt_proxy_set(lt_device_t device, const char *json) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (json) g_mockProxyJson = json;
    return kLTAccepted;
}

// Debug port: enabled flag round-trips; the port is this boot's.
static int g_mockDebugPortEnabled = 0;
static LTResult mock_lt_debug_port_get(lt_device_t device, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    char buf[512];
    ::snprintf(buf, sizeof(buf),
        "{\"enabled\":%d,\"port\":%s,\"shortName\":\"%s\",\"lldbWithSymbols\":null}",
        g_mockDebugPortEnabled,
        g_mockDebugPortEnabled ? "1234" : "null",
        dev->name.c_str());
    if (out) *out = ::strdup(buf);
    return kLTOk;
}
static LTResult mock_lt_debug_port_set(lt_device_t device, const char *json) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    g_mockDebugPortEnabled = (json && ::strstr(json, "\"enabled\":true")) ? 1 : 0;
    return kLTAccepted;
}

// --- Surface 5: apps inspector ---------------------------------------
// Rows follow AppsInspectorRows: pending install rows above installed apps
// (kind "install"), home-screen order, banner for the stale case.
static LTResult mock_lt_apps_list(lt_device_t device, const char **out) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    static const char *json = R"JSON({
"banner":{"text":null},
"device":{"canQueueInstall":true,"canReachDevice":true,"installing":false,
 "preparingDevice":false,"hasFileTransfer":true,"reconnecting":false,"transfersPaused":false},
"rows":[
 {"identity":"com.mock.installing","title":"MockApp","subtitle":"Installing","kind":"install","progress":0.42,"enabled":false},
 {"identity":"com.apple.Maps","title":"Maps","subtitle":"Apple","kind":"app","progress":null,"enabled":true},
 {"identity":"com.apple.MobileSMS","title":"Messages","subtitle":"Apple","kind":"app","progress":null,"enabled":true}]})JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}

// Install: scripted progress then success, plus the list-changed event, so
// the inspector's pending-row machinery has something to track.
static LTResult mock_lt_apps_install(lt_device_t device, const char *ipaPath) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge) {
        g_mockBridge->mockFireInstallProgress(QStringLiteral("com.mock.installing"), 0.5,
                                              QStringLiteral("Copying"));
        g_mockBridge->mockFireInstallDone(QStringLiteral("com.mock.installing"), true, QString());
        g_mockBridge->mockFireAppsChanged();
    }
    (void)ipaPath;
    return kLTAccepted;
}

// --- Surface 6: files browser ----------------------------------------
// The mock device's filesystem: a tiny canned tree per path.
static const char *mockFilesEntries(const char *path) {
    if (!path || !*path)
        return "[{\"name\":\"DCIM\",\"path\":\"DCIM\",\"isDirectory\":true,\"size\":0},"
               "{\"name\":\"iTunes_Control\",\"path\":\"iTunes_Control\",\"isDirectory\":true,\"size\":0},"
               "{\"name\":\"photo.jpg\",\"path\":\"photo.jpg\",\"isDirectory\":false,\"size\":1048576}]";
    if (::strcmp(path, "DCIM") == 0)
        return "[{\"name\":\"100APPLE\",\"path\":\"DCIM/100APPLE\",\"isDirectory\":true,\"size\":0}]";
    if (::strcmp(path, "DCIM/100APPLE") == 0)
        return "[{\"name\":\"IMG_0001.jpg\",\"path\":\"DCIM/100APPLE/IMG_0001.jpg\",\"isDirectory\":false,\"size\":2097152}]";
    return "[]";
}
static uint64_t g_mockFilesRequestId = 1000;
static LTResult mock_lt_files_state(const char **out) {
    // The mock iPhone 3GS (handle 1) is off; the iPod (handle 2) runs:
    // connected follows the running row.
    static const char *json = R"JSON({"state":"connected","device":"7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"})JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}
static LTResult mock_lt_files_list(lt_device_t device, const char *path, uint64_t *requestIdOut) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    const uint64_t id = ++g_mockFilesRequestId;
    if (requestIdOut) *requestIdOut = id;
    if (g_mockBridge)
        g_mockBridge->mockFireFilesListed(id, QString::fromUtf8(mockFilesEntries(path)));
    return kLTAccepted;
}
static uint64_t g_mockFilesTransferId = 2000;
static LTResult mock_lt_files_transfer(lt_device_t device, const char *opJson, uint64_t *transferIdOut) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    const uint64_t id = ++g_mockFilesTransferId;
    if (transferIdOut) *transferIdOut = id;
    if (g_mockBridge) {
        g_mockBridge->mockFireFilesProgress(id, 0.5, QStringLiteral("photo.jpg"), 524288, 1048576);
        g_mockBridge->mockFireFilesDone(id, true, false, QString());
    }
    (void)opJson;
    return kLTAccepted;
}
static LTResult mock_lt_files_cancel(uint64_t transferId) {
    if (g_mockBridge)
        g_mockBridge->mockFireFilesDone(transferId, false, true,
                                        QStringLiteral("Cancelled"));
    return kLTAccepted;
}

// --- Surface 7: log window + console ---------------------------------
static LTResult mock_lt_log_sources(const char **out) {
    static const char *json =
        R"JSON(["app","helper:3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"])JSON";
    if (out) *out = ::strdup(json);
    return kLTOk;
}
static LTResult mock_lt_log_read(const char *source, uint64_t from, const char **out) {
    static const char *json = R"JSON({
"text":"[app] LightTouch 0.1.0 starting\n[app] mock core initialized\n","rotated":false,"offset":58})JSON";
    (void)from;
    if (out) *out = ::strdup(json);
    (void)source;
    return kLTOk;
}

// --- Surface 8: capture ----------------------------------------------
// Availability follows the mock device state: the running iPod can capture.
static LTResult mock_lt_capture_availability(lt_device_t device, const char **out) {
    const MockDevice *dev = mockDeviceByHandle(device);
    if (!dev) return kLTNoSuchDevice;
    const int running = (dev->status == kLTDeviceRowStateRunning) ? 1 : 0;
    char buf[256];
    ::snprintf(buf, sizeof(buf),
        "{\"canTakeScreenshot\":%d,\"canStartRecording\":%d,\"canToggleRecording\":%d,"
        "\"screenshotBusy\":0,\"recordingSaving\":0,\"recordingCanStop\":0}",
        running, running, running);
    if (out) *out = ::strdup(buf);
    return kLTOk;
}
static LTResult mock_lt_capture_screenshot(lt_device_t device) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge)
        g_mockBridge->mockFireCaptureDone(kLTCaptureScreenshot,
                                          QStringLiteral("/tmp/mock-screenshot.png"),
                                          true, QString());
    return kLTAccepted;
}
static LTResult mock_lt_capture_record_start(lt_device_t device) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge) {
        g_mockBridge->mockFireRecordingPhase(kLTRecordingPhaseRecording);
        g_mockBridge->mockFireRecordingTick(3);
        g_mockBridge->mockFireCaptureAvailability();
    }
    return kLTAccepted;
}
// The two-step save handshake (ABI.md §3.8): stop → save-needed (or straight
// to done when discarding) → Qt's save dialog → record_save / record_discard.
static LTResult mock_lt_capture_record_stop(lt_device_t device, int32_t discard) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge) {
        g_mockBridge->mockFireRecordingPhase(kLTRecordingPhaseSaving);
        if (discard)
            g_mockBridge->mockFireRecordingPhase(kLTRecordingPhaseIdle);
        else
            g_mockBridge->mockFireRecordingSaveNeeded(
                QStringLiteral("Mock Device Recording.mov"));
    }
    return kLTAccepted;
}
static LTResult mock_lt_capture_record_save(lt_device_t device, const char *path) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge) {
        g_mockBridge->mockFireCaptureDone(kLTCaptureRecording,
                                          QString::fromUtf8(path ? path : ""),
                                          true, QString());
        g_mockBridge->mockFireRecordingPhase(kLTRecordingPhaseIdle);
        g_mockBridge->mockFireCaptureAvailability();
    }
    return kLTAccepted;
}
static LTResult mock_lt_capture_record_discard(lt_device_t device) {
    if (!mockDeviceByHandle(device)) return kLTNoSuchDevice;
    if (g_mockBridge) {
        g_mockBridge->mockFireRecordingPhase(kLTRecordingPhaseIdle);
        g_mockBridge->mockFireCaptureAvailability();
    }
    return kLTAccepted;
}
// Capture prefs: round-trip in memory (mirrors CapturePreferences).
static std::string g_mockCapturePrefsJson =
    R"JSON({"saveLocation":"~/Pictures","openFinderAfterCapture":true,"copyOnCapture":false,"soundEffectsEnabled":true,"recordBezel":false})JSON";
static LTResult mock_lt_capture_prefs_get(const char **out) {
    if (out) *out = ::strdup(g_mockCapturePrefsJson.c_str());
    return kLTOk;
}
static LTResult mock_lt_capture_prefs_set(const char *json) {
    if (json) g_mockCapturePrefsJson = json;
    return kLTAccepted;
}

// --- Surface 9: firmware catalog & jobs -------------------------------
// Catalog details (CatalogDetailsModel): mock-invented schema pending the
// real Swift model (requirements, keys state, recipe steps, emulator notes).
static LTResult mock_lt_catalog_details(const char *entryId, const char **out) {
    static const char *json = R"JSON({
"id":"n88ap-3.1.3","marketingName":"iPhone 3GS","version":"3.1.3","build":"7E18",
"requirements":["Downloaded IPSW","Prepared device image"],
"keysState":"bundled","recipeSteps":["Decrypt root filesystem","Build NOR image","Assemble device image"],
"emulatorNotes":"Runs at full speed on most hosts.","installedVersion":null,
"offersJailbreak":true,"offersSkipSetup":true})JSON";
    (void)entryId;
    if (out) *out = ::strdup(json);
    return kLTOk;
}
// A started job runs its scripted arc: progress → done → jobs list changed.
static LTResult mock_lt_job_start(const char *entryId, const char *optionsJson) {
    (void)optionsJson;
    const std::string id = entryId ? entryId : "";
    if (g_mockBridge) {
        // The started job joins the list so the adapter's refresh-on-done
        // observes a change. (Reset by resetMockDevices per lt_init.)
        if (g_mockJobsJson.find("\"entryId\":\"" + id + "\"") == std::string::npos) {
            g_mockJobsJson =
                "[{\"entryId\":\"" + id + "\","
                "\"deviceUuid\":\"a1b2c3d4-e5f6-4789-abcd-ef0123456789\","
                "\"kind\":\"download\",\"progress\":0.0,"
                "\"preparation\":{\"step\":1,\"steps\":5,\"name\":\"Downloading\","
                "\"fraction\":0.0,\"detail\":\"Starting\",\"remaining\":\"--:--\"},"
                "\"failed\":null}," + g_mockJobsJson.substr(1);
        }
        g_mockBridge->mockFireJobProgress(QString::fromUtf8(id.c_str()), 0.75);
        g_mockBridge->mockFireJobDone(QString::fromUtf8(id.c_str()),
                                      true, false, QString());
    }
    return kLTAccepted;
}
static LTResult mock_lt_job_cancel(const char *entryId) {
    const std::string id = entryId ? entryId : "";
    // The cancelled job leaves the list (the adapter's refresh observes it).
    const std::string needle = "\"entryId\":\"" + id + "\"";
    const size_t at = g_mockJobsJson.find(needle);
    if (at != std::string::npos) {
        const size_t objStart = g_mockJobsJson.rfind('{', at);
        size_t objEnd = g_mockJobsJson.size();
        int depth = 0;
        for (size_t i = objStart; i < g_mockJobsJson.size(); ++i) {
            if (g_mockJobsJson[i] == '{') ++depth;
            if (g_mockJobsJson[i] == '}') { --depth; if (depth == 0) { objEnd = i; break; } }
        }
        g_mockJobsJson.erase(objStart, objEnd - objStart + 1);
        if (objStart < g_mockJobsJson.size() && g_mockJobsJson[objStart] == ',')
            g_mockJobsJson.erase(objStart, 1);
        else if (objStart > 1 && g_mockJobsJson[objStart - 1] == ',')
            g_mockJobsJson.erase(objStart - 1, 1);
    }
    if (g_mockBridge)
        g_mockBridge->mockFireJobDone(QString::fromUtf8(id.c_str()),
                                      false, true,
                                      QStringLiteral("Cancelled"));
    return kLTAccepted;
}
static LTResult mock_lt_jobs_can_download(int32_t *out) {
    if (out) *out = 1;  // the preparer tool is present in the mock world
    return kLTOk;
}

// --- Surface 12: drop routing -----------------------------------------
// Routes by extension (mirrors lt_drop_classify); the router's answer is
// the kLTEventDropAction event.
static LTResult mock_lt_drop_route(const char *pathsJson, const char *target,
                                   uint64_t *requestIdOut) {
    static uint64_t nextId = 3000;
    const uint64_t id = ++nextId;
    if (requestIdOut) *requestIdOut = id;
    QString action = QString::fromUtf8(kLTDropActionUnsupported);
    const QString paths = QString::fromUtf8(pathsJson ? pathsJson : "");
    if (paths.contains(QStringLiteral(".ipsw"), Qt::CaseInsensitive))
        action = QString::fromUtf8(kLTDropActionImportIPSW);
    else if (paths.contains(QStringLiteral(".ipa"), Qt::CaseInsensitive))
        action = QString::fromUtf8(kLTDropActionInstallIPA);
    else if (paths.contains(QStringLiteral(".mp3"), Qt::CaseInsensitive)
             || paths.contains(QStringLiteral(".png"), Qt::CaseInsensitive))
        action = QString::fromUtf8(kLTDropActionImportMedia);
    if (g_mockBridge)
        g_mockBridge->mockFireDropAction(id, action,
                                         QStringLiteral("{\"target\":\"%1\"}")
                                             .arg(QString::fromUtf8(target ? target : "")));
    return kLTAccepted;
}

#include "lt_mock_stubs.inc"  // generated generic mock implementations

} // namespace

// ------------------------------------------------------------------
// Bridge
// ------------------------------------------------------------------

Bridge::Bridge(QObject *parent) : QObject(parent) {}

Bridge::~Bridge() {
    if (m_ready)
        shutdown();
}

bool Bridge::initialize(QString *errorMessage) {
    if (m_ready)
        return true;

    auto fail = [&](const QString &msg) {
        if (errorMessage) *errorMessage = msg;
        qCritical().noquote() << "Bridge::initialize failed:" << msg;
        return false;
    };

    m_mock = (qgetenv("LTM_BRIDGE_MOCK") == "1");
    if (m_mock) {
        qInfo() << "Bridge: mock mode (LTM_BRIDGE_MOCK=1) — no library loaded";
        g_mockBridge = this;
        fillMockApi();
    } else {
        QString path;
        if (!resolveLibraryPath(path, errorMessage))
            return false;
        m_lib.setFileName(path);
        if (!m_lib.load())
            return fail(QStringLiteral("could not load %1: %2").arg(path, m_lib.errorString()));
        qInfo() << "Bridge: loaded" << path;
        if (!bindApi(errorMessage)) {
            m_lib.unload();
            return false;
        }
        const int32_t v = m_api.lt_abi_version();
        if (v < kLTAbiVersionMin) {
            m_lib.unload();
            return fail(QStringLiteral("libLTAppCore ABI version %1 < minimum %2")
                            .arg(v).arg(kLTAbiVersionMin));
        }
        qInfo() << "Bridge: ABI version" << v;
    }

    // lt_init: storage paths are Qt-owned on Linux/Windows (no Bundle.main).
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString appDir = QCoreApplication::applicationDirPath();
    QJsonObject cfg{
        { QStringLiteral("appSupportDir"), appData },
        { QStringLiteral("logDir"), appData + QStringLiteral("/logs") },
        { QStringLiteral("resourceDir"), appDir + QStringLiteral("/resources") },
    };
    const QByteArray cfgBytes = QJsonDocument(cfg).toJson(QJsonDocument::Compact);
    if (m_api.lt_init(cfgBytes.constData()) != kLTOk)
        return fail(QStringLiteral("lt_init failed: %1").arg(lastErrorString()));

    if (m_api.lt_set_event_callback(ltBridgeEventTrampoline, this) != kLTOk)
        return fail(QStringLiteral("lt_set_event_callback failed: %1").arg(lastErrorString()));

    m_ready = true;
    emit readyChanged(true);
    if (m_mock)
        startMockEventLoop();
    return true;
}

void Bridge::shutdown() {
    if (!m_ready)
        return;
    if (m_mockTimer) {
        m_mockTimer->stop();
        m_mockTimer->deleteLater();
        m_mockTimer = nullptr;
    }
    mockStopDisplayLink();
    g_mockBridge = nullptr;
    // Unregister first so no event can arrive during teardown.
    if (m_api.lt_set_event_callback)
        m_api.lt_set_event_callback(nullptr, nullptr);
    if (m_api.lt_shutdown)
        m_api.lt_shutdown();
    if (m_lib.isLoaded())
        m_lib.unload();
    m_ready = false;
    emit readyChanged(false);
}

bool Bridge::resolveLibraryPath(QString &pathOut, QString *errorMessage) {
    QStringList tried;
#ifdef Q_OS_WIN
    const QString leaf = QStringLiteral("LTAppCore.dll");
#else
    const QString leaf = QStringLiteral("libLTAppCore.so");
#endif
    const QByteArray env = qgetenv("LTM_APPCORE_LIB");
    if (!env.isEmpty()) {
        const QString p = QString::fromLocal8Bit(env);
        tried << p;
        if (!QFile::exists(p)) {
            if (errorMessage)
                *errorMessage = QStringLiteral("LTM_APPCORE_LIB points at a missing file: %1").arg(p);
            return false;
        }
        pathOut = p;
        return true;
    }
    const QString appDirPath =
        QCoreApplication::applicationDirPath() + QLatin1Char('/') + leaf;
    tried << appDirPath;
    if (QFile::exists(appDirPath)) {
        pathOut = appDirPath;
        return true;
    }
    tried << (leaf + QStringLiteral(" (system search path)"));
    pathOut = leaf;  // let QLibrary::load() do the system search; it reports the error
    qInfo().noquote() << "Bridge: trying system search path for" << leaf
                      << "(also tried: " << tried.join(QStringLiteral(", ")) << ")";
    return true;
}

bool Bridge::bindApi(QString *errorMessage) {
    QStringList missing;
#define BIND(name)                                                                       \
    do {                                                                                 \
        m_api.name = reinterpret_cast<decltype(&::name)>(m_lib.resolve(#name));          \
        if (!m_api.name)                                                                 \
            missing.append(QStringLiteral(#name));                                       \
    } while (0)
#include "lt_api_bind.inc"  // generated from lt_bridge.h — every export bound
#undef BIND
    if (!missing.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("library is missing %1 required symbol(s): %2")
                                .arg(missing.size()).arg(missing.join(QStringLiteral(", ")));
        return false;
    }
    return true;
}

void Bridge::fillMockApi() {
    Api api{};  // value-init: every member starts null, then all are assigned
    api.lt_abi_version = mock_lt_abi_version;
    api.lt_init = mock_lt_init;
    api.lt_shutdown = mock_lt_shutdown;
    api.lt_set_event_callback = mock_lt_set_event_callback;
    api.lt_free_string = mock_lt_free_string;
    api.lt_free_event_strings = mock_lt_free_event_strings;
    api.lt_last_error = mock_lt_last_error;
    api.lt_version = mock_lt_version;
    api.lt_device_list = mock_lt_device_list;
    api.lt_device_row = mock_lt_device_row;
    api.lt_device_find = mock_lt_device_find;
    api.lt_device_uuid = mock_lt_device_uuid;
    api.lt_device_create = mock_lt_device_create;
    api.lt_device_delete = mock_lt_device_delete;
    api.lt_device_rename = mock_lt_device_rename;
    api.lt_device_status = mock_lt_device_status;
    api.lt_jobs_list = mock_lt_jobs_list;
    api.lt_drop_classify = mock_lt_drop_classify;
    api.lt_menu_validate = mock_lt_menu_validate;
    api.lt_settings_get_string = mock_lt_settings_get_string;
    api.lt_settings_set_string = mock_lt_settings_set_string;
    api.lt_catalog_list = mock_lt_catalog_list;
    api.lt_about_credits = mock_lt_about_credits;
    api.lt_help_topics = mock_lt_help_topics;
    api.lt_device_boot = mock_lt_device_boot;
    api.lt_device_halt = mock_lt_device_halt;
    // Surfaces 4–9, 12 (MOCK-ONLY hand-written mocks).
    api.lt_tweaks_get = mock_lt_tweaks_get;
    api.lt_carrier_get = mock_lt_carrier_get;
    api.lt_storage_get = mock_lt_storage_get;
    api.lt_proxy_get = mock_lt_proxy_get;
    api.lt_proxy_set = mock_lt_proxy_set;
    api.lt_debug_port_get = mock_lt_debug_port_get;
    api.lt_debug_port_set = mock_lt_debug_port_set;
    api.lt_apps_list = mock_lt_apps_list;
    api.lt_apps_install = mock_lt_apps_install;
    api.lt_files_state = mock_lt_files_state;
    api.lt_files_list = mock_lt_files_list;
    api.lt_files_transfer = mock_lt_files_transfer;
    api.lt_files_cancel = mock_lt_files_cancel;
    api.lt_log_sources = mock_lt_log_sources;
    api.lt_log_read = mock_lt_log_read;
    api.lt_capture_availability = mock_lt_capture_availability;
    api.lt_capture_screenshot = mock_lt_capture_screenshot;
    api.lt_capture_record_start = mock_lt_capture_record_start;
    api.lt_capture_record_stop = mock_lt_capture_record_stop;
    api.lt_capture_record_save = mock_lt_capture_record_save;
    api.lt_capture_record_discard = mock_lt_capture_record_discard;
    api.lt_capture_prefs_get = mock_lt_capture_prefs_get;
    api.lt_capture_prefs_set = mock_lt_capture_prefs_set;
    api.lt_catalog_details = mock_lt_catalog_details;
    api.lt_job_start = mock_lt_job_start;
    api.lt_job_cancel = mock_lt_job_cancel;
    api.lt_jobs_can_download = mock_lt_jobs_can_download;
    api.lt_drop_route = mock_lt_drop_route;
#include "lt_mock_bind.inc"  // generated: api.<name> = mock_<name> for the rest
    m_api = api;
}

QString Bridge::takeString(const char *s) {
    if (!s)
        return QString();
    const QString out = QString::fromUtf8(s);
    if (m_api.lt_free_string)
        m_api.lt_free_string(s);
    return out;
}

QString Bridge::lastErrorString() {
    if (!m_ready || !m_api.lt_last_error)
        return QString();
    return takeString(m_api.lt_last_error());
}

void Bridge::postEvent(const LTEvent *e) {
    if (!e || !m_ready)
        return;
    LTEvent *copy = deepCopyEvent(e);
    // Release the producer's strings NOW (ABI.md §1.3); the copy owns its own.
    if (m_api.lt_free_event_strings)
        m_api.lt_free_event_strings(e);
    QMetaObject::invokeMethod(this, [this, copy]() { dispatchEvent(copy); },
                              Qt::QueuedConnection);
}

void Bridge::dispatchEvent(LTEvent *copy) {
    const int kind = copy->kind;
    const quint64 device = copy->device;
    const quint64 seq = copy->seq;
    const QVariantMap payload = eventPayloadToMap(copy);
    freeEventStringFields(copy);
    delete copy;
    emit ltEventReceived(kind, device, seq, payload);
}

// ------------------------------------------------------------------
// Mock event synthesis
// ------------------------------------------------------------------

void Bridge::mockFireToast(const QString &title, const QString &body, const QString &category) {
    if (!m_mock || !m_ready)
        return;
    LTEvent e{};
    e.kind = kLTEventToast;
    e.u.toast.title = ::strdup(title.toUtf8().constData());
    e.u.toast.body = ::strdup(body.toUtf8().constData());
    e.u.toast.category = ::strdup(category.toUtf8().constData());
    postEvent(&e);
}

void Bridge::mockFireDeviceListChanged() {
    if (!m_mock || !m_ready)
        return;
    LTEvent e{};
    e.kind = kLTEventDeviceListChanged;
    postEvent(&e);
}

void Bridge::mockFireDeviceStatus(lt_device_t device, int machine, bool running, bool sleeping) {
    if (!m_mock || !m_ready)
        return;
    LTEvent e{};
    e.kind = kLTEventDeviceStatusChanged;
    e.device = device;
    e.u.statusChanged.machine = machine;
    e.u.statusChanged.running = running ? 1 : 0;
    e.u.statusChanged.sleeping = sleeping ? 1 : 0;
    postEvent(&e);
}

void Bridge::mockFireBootStage(lt_device_t device, int stage, const QString &detail, double fraction) {
    if (!m_mock || !m_ready)
        return;
    LTEvent e{};
    e.kind = kLTEventBootStage;
    e.device = device;
    e.u.bootStage.stage = stage;
    e.u.bootStage.detail = ::strdup(detail.toUtf8().constData());
    e.u.bootStage.fraction = fraction;
    postEvent(&e);
}

void Bridge::mockFireJobProgress(const QString &entryId, double fraction) {
    if (!m_mock || !m_ready)
        return;
    LTEvent e{};
    e.kind = kLTEventJobProgress;
    e.u.jobProgress.entryId = ::strdup(entryId.toUtf8().constData());
    e.u.jobProgress.fraction = fraction;
    e.u.jobProgress.detail = ::strdup("Downloading…");
    e.u.jobProgress.etaSeconds = 42.0;
    e.u.jobProgress.step = 2;
    e.u.jobProgress.steps = 5;
    postEvent(&e);
}

// --- Surface 4–9, 12–14 mock event synthesis (MOCK-ONLY) ----------------
// Each builds the LTEvent with owned strings and posts it through the same
// postEvent() path the real trampoline uses (copy → queued dispatch).

void Bridge::mockFireTweaksChanged() {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventTweaksChanged; postEvent(&e);
}
void Bridge::mockFireCarrierChanged() {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventCarrierChanged; postEvent(&e);
}
void Bridge::mockFireCaptureAvailability() {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventCaptureAvailabilityChanged; postEvent(&e);
}
void Bridge::mockFireCaptureDone(int kind, const QString &path, bool ok,
                                 const QString &message) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventCaptureDone;
    e.u.captureDone.kind = kind;
    e.u.captureDone.path = ::strdup(path.toUtf8().constData());
    e.u.captureDone.ok = ok ? 1 : 0;
    e.u.captureDone.message = ::strdup(message.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireRecordingTick(quint64 elapsedSeconds) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventRecordingTick;
    e.u.recordingTick.elapsedSeconds = elapsedSeconds; postEvent(&e);
}
void Bridge::mockFireRecordingPhase(int phase) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventRecordingPhaseChanged;
    e.u.recordingPhaseChanged.phase = phase; postEvent(&e);
}
void Bridge::mockFireRecordingSaveNeeded(const QString &suggestedName) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventRecordingSaveNeeded;
    e.u.recordingSaveNeeded.suggestedName =
        ::strdup(suggestedName.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireAppsChanged() {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventAppsChanged; postEvent(&e);
}
void Bridge::mockFireInstallProgress(const QString &bundleId, double fraction,
                                     const QString &phase) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventInstallProgress;
    e.u.installProgress.bundleId = ::strdup(bundleId.toUtf8().constData());
    e.u.installProgress.fraction = fraction;
    e.u.installProgress.phase = ::strdup(phase.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireInstallDone(const QString &bundleId, bool ok,
                                 const QString &message) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventInstallDone;
    e.u.installDone.bundleId = ::strdup(bundleId.toUtf8().constData());
    e.u.installDone.ok = ok ? 1 : 0;
    e.u.installDone.message = ::strdup(message.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireFilesListed(quint64 requestId, const QString &entriesJson) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventFilesListed;
    e.u.filesListed.requestId = requestId;
    e.u.filesListed.entriesJson = ::strdup(entriesJson.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireFilesProgress(quint64 transferId, double fraction,
                                   const QString &currentFile, qint64 bytesDone,
                                   qint64 bytesTotal) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventFilesProgress;
    e.u.filesProgress.transferId = transferId;
    e.u.filesProgress.fraction = fraction;
    e.u.filesProgress.currentFile = ::strdup(currentFile.toUtf8().constData());
    e.u.filesProgress.bytesDone = bytesDone;
    e.u.filesProgress.bytesTotal = bytesTotal;
    postEvent(&e);
}
void Bridge::mockFireFilesDone(quint64 transferId, bool ok, bool cancelled,
                               const QString &message) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventFilesDone;
    e.u.filesDone.transferId = transferId;
    e.u.filesDone.ok = ok ? 1 : 0;
    e.u.filesDone.cancelled = cancelled ? 1 : 0;
    e.u.filesDone.message = ::strdup(message.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireFilesConnection(int state) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventFilesConnectionChanged;
    e.u.filesConnectionChanged.state = state; postEvent(&e);
}
void Bridge::mockFireLogLine(const QString &source, const QString &text,
                             quint64 offset) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventLogLine;
    e.u.logLine.source = ::strdup(source.toUtf8().constData());
    e.u.logLine.text = ::strdup(text.toUtf8().constData());
    e.u.logLine.offset = offset; postEvent(&e);
}
void Bridge::mockFireLogRotated(const QString &source) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventLogRotated;
    e.u.logRotated.source = ::strdup(source.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireJobDone(const QString &entryId, bool ok, bool cancelled,
                             const QString &message) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventJobDone;
    e.u.jobDone.entryId = ::strdup(entryId.toUtf8().constData());
    e.u.jobDone.ok = ok ? 1 : 0;
    e.u.jobDone.cancelled = cancelled ? 1 : 0;
    e.u.jobDone.message = ::strdup(message.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireTaskbarProgress(double fraction) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventTaskbarProgress;
    e.u.taskbarProgress.fraction = fraction; postEvent(&e);
}
void Bridge::mockFireDropAction(quint64 requestId, const QString &action,
                                const QString &detailJson) {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventDropAction;
    e.u.dropAction.requestId = requestId;
    e.u.dropAction.action = ::strdup(action.toUtf8().constData());
    e.u.dropAction.detailJson = ::strdup(detailJson.toUtf8().constData());
    postEvent(&e);
}
void Bridge::mockFireCatalogChanged() {
    if (!m_mock || !m_ready) return;
    LTEvent e{}; e.kind = kLTEventCatalogChanged; postEvent(&e);
}

void Bridge::startMockEventLoop() {
    // Tests set LTM_BRIDGE_MOCK_QUIET=1 for deterministic assertions; the
    // app leaves it unset so the shell demonstrates live event flow.
    if (qgetenv("LTM_BRIDGE_MOCK_QUIET") == "1") {
        qInfo() << "Bridge: mock event loop suppressed (LTM_BRIDGE_MOCK_QUIET=1)";
        return;
    }
    // One list-changed shortly after startup so adapters populate.
    QTimer::singleShot(400, this, [this]() { mockFireDeviceListChanged(); });
    m_mockTimer = new QTimer(this);
    connect(m_mockTimer, &QTimer::timeout, this, [this]() {
        ++m_mockTick;
        switch (m_mockTick % 4) {
        case 1:
            mockFireToast(QStringLiteral("Mock core"),
                          QStringLiteral("heartbeat %1 — libLTAppCore not loaded yet").arg(m_mockTick),
                          QStringLiteral("mock"));
            break;
        case 2:
            mockFireJobProgress(QStringLiteral("mock-ipsw-1"),
                                0.13 * (m_mockTick % 8));
            break;
        case 3:
            mockFireBootStage(3, kLTBootStageKernel,
                              QString::fromUtf8("Loading kernel\xE2\x80\xA6"), 0.42);
            break;
        default:
            mockFireDeviceStatus(2, kLTVMStateRunning, true, false);
            break;
        }
    });
    m_mockTimer->start(2200);
}

// --- Synthetic display link (mock-only) ----------------------------------
// mock_lt_device_boot starts the ring and emits kLTEventDisplayLinkReady;
// a 33ms timer forwards new frames as kLTEventFrameAvailable. mock_halt
// tears it down with kLTEventDisplayLinkClosed.

#ifndef Q_OS_WIN
static std::unique_ptr<MockFrameRing> g_mockRing;
static uint64_t g_mockRingLastSent = 0;
#endif

void Bridge::mockStartDisplayLink(lt_device_t device) {
#ifndef Q_OS_WIN
    if (!m_mock || !m_ready)
        return;
    mockStopDisplayLink();
    g_mockRing = std::make_unique<MockFrameRing>();
    if (!g_mockRing->start()) {
        g_mockRing.reset();
        return;
    }
    g_mockRingLastSent = 0;
    LTEvent e{};
    e.kind = kLTEventDisplayLinkReady;
    e.device = device;
    e.u.displayLinkReady.fd = g_mockRing->dupFd();
    e.u.displayLinkReady.width = MockFrameRing::kWidth;
    e.u.displayLinkReady.height = MockFrameRing::kHeight;
    e.u.displayLinkReady.stride = MockFrameRing::kWidth * 4;
    e.u.displayLinkReady.format = kLTPixelFormatBGRA8888;
    e.u.displayLinkReady.ringGeneration = 1;
    postEvent(&e);
    m_mockFrameTimer = new QTimer(this);
    connect(m_mockFrameTimer, &QTimer::timeout, this, [this, device]() {
        if (!g_mockRing)
            return;
        const uint64_t serial = g_mockRing->latestSerial();
        if (serial == g_mockRingLastSent)
            return;
        g_mockRingLastSent = serial;
        LTEvent f{};
        f.kind = kLTEventFrameAvailable;
        f.device = device;
        f.u.frameAvailable.frameSeq = serial;
        f.u.frameAvailable.width = MockFrameRing::kWidth;
        f.u.frameAvailable.height = MockFrameRing::kHeight;
        postEvent(&f);
    });
    m_mockFrameTimer->start(33);
#else
    Q_UNUSED(device);
#endif
}

void Bridge::mockStopDisplayLink(lt_device_t device) {
    if (m_mockFrameTimer) {
        m_mockFrameTimer->stop();
        m_mockFrameTimer->deleteLater();
        m_mockFrameTimer = nullptr;
    }
#ifndef Q_OS_WIN
    if (g_mockRing) {
        g_mockRing->stop();
        g_mockRing.reset();
        LTEvent e{};
        e.kind = kLTEventDisplayLinkClosed;
        e.device = device;
        postEvent(&e);
    }
#else
    Q_UNUSED(device);
#endif
}
