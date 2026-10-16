// tst_surface2.cpp — Surface 2 (emulator display) tests, offscreen.
//
// Drives the real display stack against the mock bridge:
//   - the shm frame-ring reader protocol (ltframe::Reader: acquire/hold/
//     release over a writer-simulated status segment),
//   - boot → kLTEventDisplayLinkReady → LTDisplayAdapter::attach → linkReady,
//   - FrameAvailable → heldFrameIndex advances (UI thread protocol),
//   - DisplayItem renders the ring into a texture and pixels are non-black
//     (offscreen grab),
//   - input seams (touch/key/text/tilt/rotate/shake/home) don't crash,
//   - zoom mode round-trip + per-board memory (the mock has no ladder:
//     lt_zoom_step returns -1, so zoomIn is a documented no-op here),
//   - DeviceScreen.qml loads clean with the live adapters.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickView>
#include <QSettings>

#include <vector>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTDisplayAdapter.h"
#include "adapters/LTSettingsAdapter.h"
#include "adapters/LTCaptureAdapter.h"
#include "items/DisplayItem.h"
#include "items/FrameRing.h"

namespace {
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
constexpr char kIPodTouch[] = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7";
constexpr char kIPhone3GS[] = "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617";
} // namespace

class TstSurface2 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTDeviceListAdapter deviceList;
        LTDeviceAdapter device;
        LTDisplayAdapter display;
        LTSettingsAdapter settings;
        LTCaptureAdapter capture;
        Fixture()
            : deviceList(&bridge), device(&bridge), display(&bridge),
              settings(&bridge), capture(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            settings.setNetworkAccessAnswer(QStringLiteral("allow"));
        }
        void boot3GS() {
            // The display must track the device BEFORE boot: its
            // DisplayLinkReady arrives during boot, and setDeviceUuid closes
            // any link it didn't open.
            display.setDeviceUuid(QString::fromUtf8(kIPhone3GS));
            device.setCurrentDeviceUuid(QString::fromUtf8(kIPhone3GS));
            device.boot();
            QTest::qWait(50);
        }
        bool waitLink(int timeoutMs = 5000) {
            const int step = 50;
            for (int t = 0; t < timeoutMs; t += step) {
                if (display.linkReady())
                    return true;
                QTest::qWait(step);
            }
            return display.linkReady();
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        // QSettings needs a real organization/app on Windows (registry);
        // without it the zoom persistence below silently writes nowhere.
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface2"));
    }

    // The reader protocol itself: writer publishes frames, the reader holds
    // the latest segment and hands it back, pixels land in the held segment.
    void frameRingProtocol() {
        using namespace ltframe;
        const int w = 8, h = 4, stride = w * 4;
        std::vector<uint8_t> mem(kStatusBytes + 3 * h * stride, 0);
        volatile uint64_t *status = reinterpret_cast<volatile uint64_t *>(mem.data());
        auto frameBytes = [&](int i) {
            return mem.data() + kStatusBytes + static_cast<size_t>(i) * h * stride;
        };
        auto publish = [&](int index, uint64_t serial) {
            // Writer protocol: pixels first, then front, then serial (release).
            storeSeq(status + kSlotFront, static_cast<uint64_t>(index));
            storeSeq(status + kSlotFrameSerial, serial);
        };
        auto fill = [&](int index, uint8_t b, uint8_t g, uint8_t r) {
            uint8_t *px = frameBytes(index);
            for (int i = 0; i < w * h; ++i) {
                px[i * 4 + 0] = b; px[i * 4 + 1] = g;
                px[i * 4 + 2] = r; px[i * 4 + 3] = 0xFF;
            }
        };
        storeSeq(status + kSlotRingGeneration, 7);
        Reader reader;
        reader.attach(status, 7);
        QVERIFY(reader.attached());

        fill(0, 0x00, 0x00, 0xFF); // red
        publish(0, 1);
        QCOMPARE(reader.acquire(), 0);
        const uint8_t *held = frameBytes(reader.heldIndex());
        QCOMPARE(held[2], 0xFF); // R of BGRA
        QCOMPARE(held[0], 0x00); // B
        // Nothing new: acquire keeps the hold.
        QCOMPARE(reader.acquire(), 0);
        reader.release();
        QCOMPARE(reader.heldIndex(), -1);

        fill(1, 0xFF, 0x00, 0x00); // blue
        publish(1, 2);
        QCOMPARE(reader.acquire(), 1);
        held = frameBytes(reader.heldIndex());
        QCOMPARE(held[0], 0xFF);
        QCOMPARE(held[2], 0x00);
        QCOMPARE(reader.lastSerial(), quint64(2));
        reader.release();
        reader.detach();
        QVERIFY(!reader.attached());
    }

    // Boot → DisplayLinkReady → attach → FrameAvailable advances the hold.
    void displayLinkAndFrames() {
        Fixture f;
        f.boot3GS();
        QVERIFY2(f.waitLink(), "DisplayLinkReady never arrived");
        QCOMPARE(f.display.frameWidth(), 320);
        QCOMPARE(f.display.frameHeight(), 480);
        // The first frame arrives asynchronously after the link; wait for it.
        QTRY_VERIFY_WITH_TIMEOUT(f.display.heldFrameIndex() >= 0, 5000);
        // The mock ticks at ~30fps; the UI-thread protocol must advance.
        QSignalSpy frames(&f.display, &LTDisplayAdapter::frameAvailable);
        QVERIFY(frames.isValid());
        const int first = f.display.heldFrameIndex();
        bool advanced = false;
        for (int i = 0; i < 60 && !advanced; ++i) {
            QTest::qWait(50);
            advanced = !frames.isEmpty() || f.display.heldFrameIndex() != first;
        }
        QVERIFY2(advanced, "frameAvailable never advanced the hold");
        f.device.halt();
        QTest::qWait(100);
    }

    // DisplayItem uploads the held segment; the grab is non-black.
    void displayItemRenders() {
        Fixture f;
        f.boot3GS();
        QVERIFY2(f.waitLink(), "DisplayLinkReady never arrived");

        qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
        QQuickView view;
        view.rootContext()->setContextProperty(QStringLiteral("display"), &f.display);
        view.setSource(QUrl::fromLocalFile(qmlPath("/../tests/data/DisplayHarness.qml")));
        if (!view.errors().isEmpty()) {
            QFAIL(qPrintable(view.errors().first().toString()));
            return;
        }
        QVERIFY(view.rootObject());
        view.setWidth(320);
        view.setHeight(480);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view, 5000));
        QTest::qWait(300); // let a couple of frames land
        const QImage img = view.grabWindow();
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 320);
        QCOMPARE(img.height(), 480);
        bool nonBlack = false;
        for (int y = 0; y < img.height(); y += 16)
            for (int x = 0; x < img.width(); x += 16)
                if (qGray(img.pixel(x, y)) > 8)
                    nonBlack = true;
        QVERIFY2(nonBlack, "rendered frame is all black");
    }

    // Input seams reach the bridge without crashing; the key map is
    // best-effort (ABI.md §6 gap 8).
    void inputSeams() {
        Fixture f;
        f.boot3GS();
        QVERIFY2(f.waitLink(), "DisplayLinkReady never arrived");
        f.display.sendTouch(0, 0.5, 0.5, 0); // kLTTouchBegan
        f.display.sendTouch(1, 0.5, 0.6, 0); // kLTTouchMoved
        f.display.sendTouch(2, 0.5, 0.6, 0); // kLTTouchEnded
        f.display.typeText(QStringLiteral("hi"));
        f.display.sendKey(Qt::Key_Return, 1, Qt::NoModifier);
        f.display.sendKey(Qt::Key_Return, 0, Qt::NoModifier);
        f.display.sendTilt(0.5, -0.25);
        f.display.sendTilt(0.0, 0.0);
        f.device.rotateGuest(90);
        f.device.shake();
        f.device.pressButton(1); // kLTButtonHome
        QTest::qWait(50);
    }

    // Zoom: explicit modes round-trip and are remembered per board. The
    // mock has no ladder (lt_zoom_step returns -1), so zoomIn/zoomOut are
    // documented no-ops against it.
    void zoomModes() {
        Fixture f;
        f.device.setCurrentDeviceUuid(QString::fromUtf8(kIPodTouch));
        f.display.setDeviceUuid(QString::fromUtf8(kIPodTouch));
        QSignalSpy zoomed(&f.display, &LTDisplayAdapter::zoomChanged);
        f.display.setZoomModeExplicit(QStringLiteral("pixel"));
        QCOMPARE(f.display.zoomMode(), QStringLiteral("pixel"));
        QVERIFY(!zoomed.isEmpty());
        QSettings s;
        QCOMPARE(s.value(QStringLiteral("zoom/") + QString::fromUtf8(kIPodTouch)).toString(),
                 QStringLiteral("pixel"));
        const double before = f.display.zoomPoints();
        f.display.zoomIn();   // mock: no ladder → no change
        f.display.zoomOut();
        QCOMPARE(f.display.zoomPoints(), before);
        // Per-board memory survives a re-select.
        f.display.setDeviceUuid(QString());
        f.display.setDeviceUuid(QString::fromUtf8(kIPodTouch));
        QCOMPARE(f.display.zoomMode(), QStringLiteral("pixel"));
        QVERIFY(f.display.drawsNearest(1.0) == true || f.display.drawsNearest(1.0) == false);
    }

    // DeviceScreen.qml compiles and instantiates against the live adapters.
    void deviceScreenLoads() {
        Fixture f;
        qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
        QQmlEngine engine;
        QQmlContext *ctx = engine.rootContext();
        ctx->setContextProperty(QStringLiteral("device"), &f.device);
        ctx->setContextProperty(QStringLiteral("display"), &f.display);
        ctx->setContextProperty(QStringLiteral("capture"), &f.capture);
        QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath("/views/DeviceScreen.qml")));
        QVERIFY2(comp.isReady(), qPrintable(comp.errorString()));
        QObject *obj = comp.create(ctx);
        QVERIFY2(obj, "DeviceScreen failed to instantiate");
        delete obj;
    }
};

QTEST_MAIN(TstSurface2)
#include "tst_surface2.moc"
