// lt-spike Qt app: proves Swift .so -> C++ -> QML -> pixel-check, headless.
//
// Usage: spikeapp <path/to/libLTSpike.so> <path/to/main.qml>
// Exit code 0 + "SPIKE RESULT: PASS" means every link in the chain worked.
// Any failure prints "SPIKE RESULT: FAIL: <reason>" and exits nonzero.
//
// Steps:
//  1. dlopen the Swift .so (proves Swift runtime inits in-process)
//  2. dlsym the 4 @_cdecl exports
//  3. C++ -> Swift: lt_spike_version() must be 42
//  4. Swift -> C++: register callback, lt_spike_fire_event(1234) must deliver 1234
//     (also: unregistering (nil) then firing must not crash)
//  5. 120 repeated lt_spike_next_frame calls, C++-side buffer verified each time
//  6. Offscreen QML render of 10 frames; grabWindow pixel checks:
//     - background pixel (0,0) and (319,239) == deterministic color for frameIndex
//     - marker pixel (7,5) is pure white
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickImageProvider>
#include <QImage>
#include <QColor>
#include <QThread>

#include <dlfcn.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <atomic>

namespace {

// ---- C ABI of the Swift exports -------------------------------------------
using version_fn = int32_t (*)();
using frame_fn = void (*)(void *, int32_t, int32_t, int32_t);
using regcb_fn = void (*)(void (*)(int32_t));
using fire_fn = void (*)(int32_t);

std::atomic<int32_t> g_callbackValue{-1};
std::atomic<bool> g_providerError{false};

extern "C" void spikeEventCallback(int32_t value) {
    g_callbackValue.store(value, std::memory_order_relaxed);
}

#define FAIL(...)                                                                                 \
    do {                                                                                          \
        std::printf("SPIKE RESULT: FAIL: ");                                                      \
        std::printf(__VA_ARGS__);                                                                  \
        std::printf("\n");                                                                        \
        std::fflush(stdout);                                                                      \
        return 1;                                                                                 \
    } while (0)

#define STEP(...)                                                                                 \
    do {                                                                                          \
        std::printf("step: ");                                                                    \
        std::printf(__VA_ARGS__);                                                                  \
        std::printf("\n");                                                                        \
        std::fflush(stdout);                                                                      \
    } while (0)

#define CHECK(cond, ...)                                                                          \
    do {                                                                                          \
        if (!(cond))                                                                               \
            FAIL(__VA_ARGS__);                                                                     \
    } while (0)

constexpr int kW = 320;
constexpr int kH = 240;

// Must mirror SpikeLib.swift exactly.
void expectedColor(int32_t frame, int &r, int &g, int &b) {
    r = (int)((frame * 37) & 0xFF);
    g = (int)(((frame * 91) + 17) & 0xFF);
    b = (int)(((frame * 53) + 101) & 0xFF);
}

// Full-buffer cross-check of what Swift wrote (catches layout/stride bugs).
bool verifyBuffer(const uint32_t *buf, int w, int h, int32_t frame) {
    int r, g, b;
    expectedColor(frame, r, g, b);
    const uint32_t bg = (uint32_t)b | ((uint32_t)g << 8) | ((uint32_t)r << 16) | (0xFFu << 24);
    for (int i = 0; i < w * h; ++i) {
        const int x = i % w, y = i / w;
        const uint32_t want = (x == 7 && y == 5) ? 0xFFFFFFFFu : bg;
        if (buf[i] != want) {
            std::printf("  buffer mismatch at (%d,%d): got 0x%08X want 0x%08X (frame %d)\n",
                        x, y, buf[i], want, frame);
            return false;
        }
    }
    return true;
}

// QML image://spikeframes/<frameIndex> -> calls Swift for that frame.
class SpikeImageProvider : public QQuickImageProvider {
public:
    SpikeImageProvider(frame_fn f)
        : QQuickImageProvider(QQuickImageProvider::Image), m_frame(f) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &) override {
        bool ok = false;
        const int32_t idx = (int32_t)id.toInt(&ok);
        if (!ok) {
            std::printf("image provider got non-integer id '%s'\n", qPrintable(id));
            std::fflush(stdout);
            g_providerError.store(true, std::memory_order_relaxed);
            return QImage();
        }
        std::vector<uint32_t> buf((size_t)kW * kH);
        m_frame(buf.data(), kW, kH, idx);
        if (size)
            *size = QSize(kW, kH);
        // QImage borrows buf; copy() detaches before buf dies.
        return QImage(reinterpret_cast<const uchar *>(buf.data()), kW, kH,
                      QImage::Format_ARGB32)
            .copy();
    }

private:
    frame_fn m_frame;
};

bool checkPixel(const QImage &img, int x, int y, int er, int eg, int eb, const char *what) {
    const QColor c = img.pixelColor(x, y);
    if (c.red() != er || c.green() != eg || c.blue() != eb || c.alpha() != 255) {
        std::printf("  pixel check failed for %s at (%d,%d): got rgba(%d,%d,%d,%d) want rgb(%d,%d,%d)\n",
                    what, x, y, c.red(), c.green(), c.blue(), c.alpha(), er, eg, eb);
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3) {
        std::printf("usage: %s <libLTSpike.so> <main.qml>\n", argv[0]);
        return 2;
    }
    const char *soPath = argv[1];
    const char *qmlPath = argv[2];

    // ---- 1. dlopen ---------------------------------------------------------
    STEP("dlopen %s", soPath);
    dlerror();
    void *handle = dlopen(soPath, RTLD_NOW | RTLD_LOCAL);
    CHECK(handle != nullptr, "dlopen failed: %s", dlerror());

    // ---- 2. dlsym ----------------------------------------------------------
    STEP("dlsym exports");
    dlerror();
    auto lt_version = reinterpret_cast<version_fn>(dlsym(handle, "lt_spike_version"));
    auto lt_frame = reinterpret_cast<frame_fn>(dlsym(handle, "lt_spike_next_frame"));
    auto lt_register = reinterpret_cast<regcb_fn>(dlsym(handle, "lt_spike_register_callback"));
    auto lt_fire = reinterpret_cast<fire_fn>(dlsym(handle, "lt_spike_fire_event"));
    const char *symErr = dlerror();
    CHECK(lt_version && lt_frame && lt_register && lt_fire,
          "dlsym missing symbol(s): %s", symErr ? symErr : "(unknown)");

    // ---- 3. C++ -> Swift call ----------------------------------------------
    STEP("lt_spike_version()");
    const int32_t v = lt_version();
    CHECK(v == 42, "lt_spike_version() returned %d, want 42", v);

    // ---- 4. Swift -> C++ callback ------------------------------------------
    STEP("register callback + fire_event(1234)");
    lt_register(spikeEventCallback);
    lt_fire(1234);
    CHECK(g_callbackValue.load() == 1234, "callback delivered %d, want 1234",
          g_callbackValue.load());

    STEP("unregister (nil) + fire_event must not crash");
    lt_register(nullptr);
    g_callbackValue.store(-1);
    lt_fire(777); // must be a silent no-op
    CHECK(g_callbackValue.load() == -1, "callback fired after unregister!");
    lt_register(spikeEventCallback); // re-arm for later
    lt_fire(5);
    CHECK(g_callbackValue.load() == 5, "re-registered callback got %d, want 5",
          g_callbackValue.load());

    // ---- 5. 120-frame stability, full-buffer verify ------------------------
    STEP("120 repeated lt_spike_next_frame calls with buffer verify");
    {
        std::vector<uint32_t> buf((size_t)kW * kH);
        for (int32_t f = 1000; f < 1120; ++f) {
            lt_frame(buf.data(), kW, kH, f);
            CHECK(verifyBuffer(buf.data(), kW, kH, f), "stability loop failed at frame %d", f);
        }
    }
    STEP("120-frame stability: no crash, no mismatch");

    // ---- 6. Offscreen QML render + pixel checks -----------------------------
    STEP("Qt offscreen QML render, 10 frames");
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.addImageProvider("spikeframes", new SpikeImageProvider(lt_frame));
    engine.load(QUrl::fromLocalFile(qmlPath));
    CHECK(!engine.rootObjects().isEmpty(), "QML failed to load: %s", qPrintable(qmlPath));

    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    CHECK(window != nullptr, "QML root is not a Window");

    window->show();
    int waited = 0;
    while (!window->isExposed() && waited < 5000) {
        QCoreApplication::processEvents();
        QThread::msleep(20);
        waited += 20;
    }
    CHECK(window->isExposed(), "window never exposed under offscreen platform");

    for (int32_t frame = 0; frame < 10; ++frame) {
        window->setProperty("frameIndex", frame);
        // Let the scene graph pick up the new image source and render.
        for (int i = 0; i < 10; ++i) {
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
        const QImage shot = window->grabWindow();
        CHECK(!g_providerError.load(), "image provider reported an error");
        CHECK(shot.width() == kW && shot.height() == kH,
              "grabWindow gave %dx%d, want %dx%d", shot.width(), shot.height(), kW, kH);

        int r, g, b;
        expectedColor(frame, r, g, b);
        CHECK(checkPixel(shot, 0, 0, r, g, b, "background(0,0)"),
              "frame %d background pixel wrong", frame);
        CHECK(checkPixel(shot, kW - 1, kH - 1, r, g, b, "background(319,239)"),
              "frame %d corner pixel wrong", frame);
        CHECK(checkPixel(shot, 7, 5, 255, 255, 255, "white marker(7,5)"),
              "frame %d white marker pixel wrong", frame);
        STEP("frame %d rendered + pixel-checked (bg rgb(%d,%d,%d), marker white)", frame, r, g, b);
    }

    dlclose(handle);
    std::printf("SPIKE RESULT: PASS: all links verified\n");
    return 0;
}
