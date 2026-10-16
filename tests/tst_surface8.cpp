// tst_surface8.cpp — Surface 8 (capture) tests, offscreen.
//
// Drives the capture adapter + RecordingButton QML against the mock bridge:
//   - availability, screenshot → CaptureDone,
//   - screenshot copy/save-as Qt-side effects,
//   - recording: start → tick → stop → save handshake → done,
//   - recording button states (idle / elapsed / saving),
//   - capture prefs round-trip.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTCaptureAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface8 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTCaptureAdapter capture;
        QQmlEngine engine;
        Fixture() : capture(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("capture"), &capture);
            capture.setDeviceUuid(QString::fromLatin1(kIPodUuid));
        }
        QObject *load(const char *rel) {
            QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath(rel)));
            if (comp.isError()) {
                qWarning() << comp.errors();
                return nullptr;
            }
            return comp.create(engine.rootContext());
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface8"));
    }

    void screenshotFiresCaptureDone() {
        Fixture f;
        QSignalSpy done(&f.capture, SIGNAL(captureDone()));
        QVERIFY(done.isValid());
        f.capture.screenshot();
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 5000);
        QVERIFY(f.capture.lastCaptureOk());
        QCOMPARE(f.capture.lastCaptureKind(), 0); // kLTCaptureScreenshot
        QCOMPARE(f.capture.lastCapturePath(),
                 QStringLiteral("/tmp/mock-screenshot.png"));
    }

    void screenshotCopySignals() {
        Fixture f;
        QSignalSpy copied(&f.capture, SIGNAL(screenshotCopied()));
        QVERIFY(copied.isValid());
        // The mock path doesn't exist, so the image load fails and no
        // signal fires — the copy path is best-effort by design.
        f.capture.screenshotCopy();
        QTest::qWait(200); // queued no-op: the mock path has no image
        QCOMPARE(copied.size(), 0);
    }

    void recordingLifecycle() {
        Fixture f;
        QSignalSpy changed(&f.capture, SIGNAL(recordingChanged()));
        QVERIFY(changed.isValid());
        QSignalSpy saveNeeded(&f.capture,
                              SIGNAL(saveDestinationRequested(QString)));
        QVERIFY(saveNeeded.isValid());

        f.capture.startRecording();
        // phase → recording, tick → 3 s (queued through the bridge).
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.recordingPhase(), 2, 5000); // recording
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.elapsedSeconds(), quint64(3), 5000);

        // Stop (not discard): the two-step save handshake asks for a path.
        f.capture.stopRecording(false);
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.recordingPhase(), 3, 5000); // saving
        QTRY_COMPARE_WITH_TIMEOUT(saveNeeded.size(), 1, 5000);
        QCOMPARE(saveNeeded[0][0].toString(),
                 QStringLiteral("Mock Device Recording.mov"));

        // Qt shows the dialog, then calls saveRecording().
        QSignalSpy done(&f.capture, SIGNAL(captureDone()));
        f.capture.saveRecording(QStringLiteral("/tmp/mock-recording.mov"));
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 5000);
        QCOMPARE(f.capture.lastCaptureKind(), 1); // kLTCaptureRecording
        QCOMPARE(f.capture.lastCapturePath(),
                 QStringLiteral("/tmp/mock-recording.mov"));
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.recordingPhase(), 0, 5000); // idle
    }

    void recordingDiscardSkipsSave() {
        Fixture f;
        QSignalSpy saveNeeded(&f.capture,
                              SIGNAL(saveDestinationRequested(QString)));
        f.capture.startRecording();
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.recordingPhase(), 2, 5000);
        f.capture.stopRecording(true); // discard
        QTest::qWait(200); // queued: no save dialog for a discard
        QCOMPARE(saveNeeded.size(), 0);
        QTRY_COMPARE_WITH_TIMEOUT(f.capture.recordingPhase(), 0, 5000);
    }

    void recordingButtonStates() {
        Fixture f;
        QObject *btn = f.load("/views/RecordingButton.qml");
        QVERIFY2(btn != nullptr, "RecordingButton failed to load");
        QVERIFY(btn->property("text").toString().contains(QStringLiteral("Start Recording")));
        QVERIFY(!btn->property("active").toBool());

        // Recording: the label is the live elapsed mm:ss.
        f.bridge.mockFireRecordingPhase(2);
        f.bridge.mockFireRecordingTick(65);
        QTRY_VERIFY_WITH_TIMEOUT(btn->property("active").toBool(), 5000);
        QVERIFY(btn->property("text").toString().contains(QStringLiteral("01:05")));

        // Saving: spinner + "Saving Recording…".
        f.bridge.mockFireRecordingPhase(3);
        QTRY_VERIFY_WITH_TIMEOUT(
            btn->property("text").toString().contains(QStringLiteral("Saving")),
            5000);
    }

    void capturePrefsRoundTrip() {
        Fixture f;
        f.capture.refreshPrefs();
        QVERIFY(f.capture.prefs().contains(QStringLiteral("saveLocation")));
        f.capture.setPrefs(QStringLiteral("{\"saveLocation\":\"/tmp/x\",\"recordBezel\":true}"));
        QCOMPARE(f.capture.prefs().value(QStringLiteral("saveLocation")).toString(),
                 QStringLiteral("/tmp/x"));
        QVERIFY(f.capture.prefs().value(QStringLiteral("recordBezel")).toBool());
    }
};

QTEST_MAIN(TstSurface8)
#include "tst_surface8.moc"
