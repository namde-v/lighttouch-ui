// tst_bridge.cpp — Bridge unit tests.
//
// Covers: mock-mode init, ABI version handshake, query round-trips with
// string ownership (takeString frees via lt_free_string), the full event
// path (trampoline → deep copy → queued dispatch → signal), clean failure
// when the library is missing, and the true dlopen/dlsym binding against
// the Swift-built libLTMockCore.so stand-in.

#include <QtTest>
#include <QFile>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>

#include "Bridge.h"

class TstBridge : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    void initializeAndVersion() {
        Bridge b;
        QString err;
        QVERIFY2(b.initialize(&err), qPrintable(err));
        QVERIFY(b.isMock());
        QVERIFY(b.isReady());
        QCOMPARE(b.api().lt_abi_version(), kLTAbiVersionCurrent);

        const char *raw = nullptr;
        QCOMPARE(b.api().lt_version(&raw), kLTOk);
        QCOMPARE(b.takeString(raw), QStringLiteral("0.0.0-mock"));

        b.shutdown();
        QVERIFY(!b.isReady());
    }

    void deviceListQuery() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));

        const char *raw = nullptr;
        QCOMPARE(b.api().lt_device_list(&raw), kLTOk);
        const QString json = b.takeString(raw);
        const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        QVERIFY(doc.isArray());
        QCOMPARE(doc.array().size(), 3);
        QCOMPARE(doc.array().first().toObject().value(QStringLiteral("name")).toString(),
                 QStringLiteral("iPhone 3GS"));

        // find/uuid round-trip through handles
        lt_device_t h = kLTInvalidDevice;
        QCOMPARE(b.api().lt_device_find("7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7", &h), kLTOk);
        QCOMPARE(h, lt_device_t(2));
        raw = nullptr;
        QCOMPARE(b.api().lt_device_uuid(h, &raw), kLTOk);
        QCOMPARE(b.takeString(raw),
                 QStringLiteral("7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"));
        QCOMPARE(b.api().lt_device_find("no-such-uuid", &h), kLTNoSuchDevice);

        b.shutdown();
    }

    void eventRoundTrip() {
        // Full path: mockFire* → postEvent (deep copy + free original) →
        // QueuedConnection → dispatchEvent → ltEventReceived on this thread.
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));

        QSignalSpy spy(&b, &Bridge::ltEventReceived);
        QVERIFY(spy.isValid());
        b.mockFireToast(QStringLiteral("Hello"), QStringLiteral("world"),
                        QStringLiteral("test"));
        QVERIFY2(spy.wait(3000), "toast event never arrived");
        QCOMPARE(spy.size(), 1);
        const QList<QVariant> args = spy.takeFirst();
        QCOMPARE(args.at(0).toInt(), kLTEventToast);
        QCOMPARE(args.at(1).toULongLong(), quint64(0));  // not device-scoped
        const QVariantMap payload = args.at(3).toMap();
        QCOMPARE(payload.value(QStringLiteral("title")).toString(), QStringLiteral("Hello"));
        QCOMPARE(payload.value(QStringLiteral("body")).toString(), QStringLiteral("world"));
        QCOMPARE(payload.value(QStringLiteral("category")).toString(), QStringLiteral("test"));

        // Device-scoped event keeps its handle.
        b.mockFireDeviceStatus(2, kLTVMStateRunning, true, false);
        QVERIFY2(spy.wait(3000), "status event never arrived");
        const QList<QVariant> sargs = spy.takeFirst();
        QCOMPARE(sargs.at(0).toInt(), kLTEventDeviceStatusChanged);
        QCOMPARE(sargs.at(1).toULongLong(), quint64(2));
        const QVariantMap spayload = sargs.at(3).toMap();
        QCOMPARE(spayload.value(QStringLiteral("machine")).toInt(), kLTVMStateRunning);
        QVERIFY(spayload.value(QStringLiteral("running")).toBool());

        b.shutdown();
    }

    void missingLibraryFailsCleanly() {
        qputenv("LTM_BRIDGE_MOCK", "0");
        qputenv("LTM_APPCORE_LIB", "/nonexistent/libLTAppCore.so");
        Bridge b;
        QString err;
        QVERIFY(!b.initialize(&err));  // must fail, never half-init
        QVERIFY(!err.isEmpty());
        QVERIFY(err.contains(QStringLiteral("/nonexistent/libLTAppCore.so")));
        QVERIFY(!b.isReady());
        qputenv("LTM_BRIDGE_MOCK", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    void realBindingAgainstSwiftStandIn() {
        // The true dlopen/dlsym path (not mock mode) against the Swift-built
        // libLTMockCore.so, which exports the real lt_* ABI. Swapping
        // LTM_APPCORE_LIB to the real library later needs no code change.
        const QByteArray so = qgetenv("LT_MOCKCORE_SO");
        if (so.isEmpty())
            QSKIP("LT_MOCKCORE_SO not set — Swift stand-in not built");
        if (!QFile::exists(QString::fromLocal8Bit(so)))
            QSKIP("Swift stand-in .so not staged yet");

        qputenv("LTM_BRIDGE_MOCK", "0");
        qputenv("LTM_APPCORE_LIB", so);
        Bridge b;
        QString err;
        QVERIFY2(b.initialize(&err), qPrintable(err));
        QVERIFY(!b.isMock());
        QVERIFY(b.isReady());
        QCOMPARE(b.api().lt_abi_version(), 1);

        const char *raw = nullptr;
        QCOMPARE(b.api().lt_device_list(&raw), kLTOk);
        const QString json = b.takeString(raw);
        QVERIFY2(json.contains(QStringLiteral("iPhone 3GS")),
                 qPrintable(json.left(200)));

        // A pure function through the real binding too.
        QCOMPARE(b.api().lt_zoom_draws_nearest(2.0, 2.0), int32_t(0));

        b.shutdown();
        qputenv("LTM_BRIDGE_MOCK", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }
};

QTEST_MAIN(TstBridge)
#include "tst_bridge.moc"
