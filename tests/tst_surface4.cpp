// tst_surface4.cpp — Surface 4 (device inspector panels) tests, offscreen.
//
// Drives the panel adapters + QML panes against the mock bridge:
//   - tweaks: rows with sections/captions, toggle, color/clock, hidden apps
//   - carrier: state snapshot, fields, call/SMS/location commands
//   - proxy: config get/set round-trip, sheet OK/Cancel semantics
//   - debug port: state, enable toggle, copy command
//   - storage: usage sections (devices/IPSWs/caches/apps)
//   - capture settings pane: prefs round-trip

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTTweaksAdapter.h"
#include "adapters/LTCarrierAdapter.h"
#include "adapters/LTProxyAdapter.h"
#include "adapters/LTDebugPortAdapter.h"
#include "adapters/LTStorageAdapter.h"
#include "adapters/LTCaptureAdapter.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTJobsAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface4 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTTweaksAdapter tweaks;
        LTCarrierAdapter carrier;
        LTProxyAdapter proxy;
        LTDebugPortAdapter debugPort;
        LTStorageAdapter storage;
        LTCaptureAdapter capture;
        LTDeviceListAdapter deviceList;
        LTJobsAdapter jobs;
        QQmlEngine engine;
        Fixture()
            : tweaks(&bridge), carrier(&bridge), proxy(&bridge),
              debugPort(&bridge), storage(&bridge), capture(&bridge),
              deviceList(&bridge), jobs(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("tweaks"), &tweaks);
            ctx->setContextProperty(QStringLiteral("carrier"), &carrier);
            ctx->setContextProperty(QStringLiteral("proxy"), &proxy);
            ctx->setContextProperty(QStringLiteral("debugPort"), &debugPort);
            ctx->setContextProperty(QStringLiteral("storage"), &storage);
            ctx->setContextProperty(QStringLiteral("capture"), &capture);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
            deviceList.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
            const QString uuid = QString::fromLatin1(kIPodUuid);
            tweaks.setDeviceUuid(uuid);
            carrier.setDeviceUuid(uuid);
            proxy.setDeviceUuid(uuid);
            debugPort.setDeviceUuid(uuid);
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface4"));
    }

    void tweaksRowsHaveSectionsAndCaptions() {
        Fixture f;
        QCOMPARE(f.tweaks.tweaks().size(), 3);
        const QVariantMap first = f.tweaks.tweaks()[0].toMap();
        QCOMPARE(first.value(QStringLiteral("id")).toString(),
                 QStringLiteral("fingerDots"));
        QVERIFY(!first.value(QStringLiteral("caption")).toString().isEmpty());
        QCOMPARE(first.value(QStringLiteral("section")).toString(),
                 QStringLiteral("Interface"));
        // Rows stay visible but disabled when the firmware lacks the tweak.
        const QVariantMap tm = f.tweaks.tweaks()[2].toMap();
        QCOMPARE(tm.value(QStringLiteral("id")).toString(),
                 QStringLiteral("timeMachine"));
        QVERIFY(!tm.value(QStringLiteral("available")).toBool());
        // Toggle + sub-options don't crash (mock accepts).
        f.tweaks.setTweak(QStringLiteral("fingerDots"), true);
        f.tweaks.setColor(1);
        f.tweaks.setClock(1167609600.0);
        f.tweaks.listHiddenApps();
    }

    void tweaksPanelQmlBinds() {
        Fixture f;
        QObject *pane = f.load("/panes/TweaksPanel.qml");
        QVERIFY2(pane != nullptr, "TweaksPanel failed to load");
        QVariant sections = pane->property("sections");
        QCOMPARE(sections.toList().size(), 3); // Interface, Graphics, System
        // The unavailable row exists in the model with available=false;
        // the QML binds Switch.enabled to modelData["available"].
        bool found = false;
        for (const QVariant &t : f.tweaks.tweaks()) {
            const QVariantMap m = t.toMap();
            if (m.value(QStringLiteral("id")).toString() == QStringLiteral("timeMachine")) {
                QVERIFY(!m.value(QStringLiteral("available")).toBool());
                found = true;
            }
        }
        QVERIFY2(found, "timeMachine tweak not in model");
    }

    void carrierStateBinds() {
        Fixture f;
        const QVariantMap s = f.carrier.state();
        QCOMPARE(s.value(QStringLiteral("carrierName")).toString(),
                 QStringLiteral("Light Touch"));
        QCOMPARE(s.value(QStringLiteral("mcc")).toString(), QStringLiteral("001"));
        QCOMPARE(s.value(QStringLiteral("callState")).toString(),
                 QStringLiteral("idle"));
        QVERIFY(s.value(QStringLiteral("canRing")).toBool());
        QVERIFY(!s.value(QStringLiteral("canAnswer")).toBool());
        const QVariantList sent = s.value(QStringLiteral("sent")).toList();
        QCOMPARE(sent.size(), 1);
        // Commands don't crash (mock accepts).
        f.carrier.setField(QStringLiteral("callNumber"), QStringLiteral("15555550123"));
        f.carrier.ring();
        f.carrier.answer();
        f.carrier.hangUp();
        f.carrier.setField(QStringLiteral("smsText"), QStringLiteral("hi"));
        f.carrier.sendSms();
        f.carrier.setLocation(37.3318, -122.0312);
        f.carrier.applyLocation();
        f.carrier.toggleWalk();
        // The 1 Hz poll loop is driven by the visible property.
        f.carrier.setVisible(true);
        QVERIFY(f.carrier.visible());
        f.carrier.setVisible(false);
    }

    void carrierPanelQmlBinds() {
        Fixture f;
        QObject *pane = f.load("/panes/CarrierPanel.qml");
        QVERIFY2(pane != nullptr, "CarrierPanel failed to load");
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("carrierNameField")) != nullptr);
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("ringButton")) != nullptr);
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("sendSmsButton")) != nullptr);
        // GPS section shows: the mock modem has GPS.
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("walkButton")) != nullptr);
        // Ring is enabled while idle (canRing).
        QObject *ring = pane->findChild<QObject *>(QStringLiteral("ringButton"));
        QVERIFY(ring->property("enabled").toBool());
    }

    void proxyRoundTrip() {
        Fixture f;
        QCOMPARE(f.proxy.config().value(QStringLiteral("mode")).toString(),
                 QStringLiteral("off"));
        f.proxy.apply(QStringLiteral("{\"mode\":\"archive\",\"archiveDate\":\"2020-06-01\"}"));
        QCOMPARE(f.proxy.config().value(QStringLiteral("mode")).toString(),
                 QStringLiteral("archive"));
        QCOMPARE(f.proxy.config().value(QStringLiteral("archiveDate")).toString(),
                 QStringLiteral("2020-06-01"));
    }

    void proxySheetDraftSemantics() {
        Fixture f;
        // Isolate from proxyRoundTrip (mock proxy state is global).
        f.proxy.apply(QStringLiteral("{\"mode\":\"off\"}"));
        QObject *editor = f.load("/panes/ProxySettings.qml");
        QVERIFY2(editor != nullptr, "ProxySettings failed to load");
        // Draft starts from the adapter; Cancel (loadFromAdapter) restores.
        QVERIFY(QMetaObject::invokeMethod(editor, "loadFromAdapter"));
        QObject *box = editor->findChild<QObject *>(QStringLiteral("proxyEnabledBox"));
        QVERIFY(box != nullptr);
        box->setProperty("checked", true);
        // currentJson reflects the draft without touching the adapter.
        QVariant ret;
        QVERIFY(QMetaObject::invokeMethod(editor, "currentJson",
                                          Q_RETURN_ARG(QVariant, ret)));
        QVERIFY(ret.toString().contains(QStringLiteral("\"mode\":\"direct\"")));
        QCOMPARE(f.proxy.config().value(QStringLiteral("mode")).toString(),
                 QStringLiteral("off")); // adapter untouched until OK
    }

    void debugPortStateAndToggle() {
        Fixture f;
        QVERIFY(!f.debugPort.state().value(QStringLiteral("enabled")).toBool());
        QCOMPARE(f.debugPort.state().value(QStringLiteral("port")).toInt(), 0); // null → 0
        f.debugPort.setEnabled(true);
        QVERIFY(f.debugPort.state().value(QStringLiteral("enabled")).toBool());
        QCOMPARE(f.debugPort.state().value(QStringLiteral("port")).toInt(), 1234);
        // Copy doesn't crash (clipboard may be unavailable offscreen).
        f.debugPort.copyCommand(QStringLiteral("lldb -o 'gdb-remote localhost:1234'"));
    }

    void debugPortQmlBinds() {
        Fixture f;
        // Isolate from debugPortStateAndToggle (mock port state is global).
        f.debugPort.setEnabled(false);
        QObject *pane = f.load("/panes/DebugPort.qml");
        QVERIFY2(pane != nullptr, "DebugPort failed to load");
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("debugPortSwitch")) != nullptr);
        // No port while disabled: no command rows.
        QCOMPARE(pane->property("port").toInt(), -1);
    }

    void storageUsageSections() {
        Fixture f;
        f.storage.refresh();
        const QVariantMap u = f.storage.usage();
        const QVariantList devices = u.value(QStringLiteral("devices")).toList();
        QCOMPARE(devices.size(), 2);
        QCOMPARE(devices[0].toMap().value(QStringLiteral("baseBytes")).toULongLong(),
                 2147483648ULL);
        const QVariantList ipsws = u.value(QStringLiteral("ipsws")).toList();
        QCOMPARE(ipsws.size(), 2);
        // Referenced IPSWs take no space (the note says so).
        QCOMPARE(ipsws[1].toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("referenced"));
        QCOMPARE(ipsws[1].toMap().value(QStringLiteral("bytes")).toULongLong(), 0ULL);
        QVERIFY(u.value(QStringLiteral("decryptedBytes")).toULongLong() > 0);
    }

    void storageSettingsQmlBinds() {
        Fixture f;
        QObject *pane = f.load("/panes/StorageSettings.qml");
        QVERIFY2(pane != nullptr, "StorageSettings failed to load");
        QVERIFY(pane->property("usage").toMap().contains(QStringLiteral("devices")));
    }

    void captureSettingsPrefsRoundTrip() {
        Fixture f;
        f.capture.refreshPrefs();
        QVERIFY(!f.capture.prefs().isEmpty());
        const QString loc = f.capture.prefs().value(QStringLiteral("saveLocation")).toString();
        QVERIFY(!loc.isEmpty());
        QObject *pane = f.load("/panes/CaptureSettings.qml");
        QVERIFY2(pane != nullptr, "CaptureSettings failed to load");
        QVERIFY(pane->findChild<QObject *>(QStringLiteral("saveLocationField")) != nullptr);
        // Toggling a pref writes it back through the adapter.
        f.capture.setPrefs(QStringLiteral(
            "{\"saveLocation\":\"/tmp/caps\",\"openFinderAfterCapture\":false,"
            "\"copyOnCapture\":true,\"soundEffectsEnabled\":false,\"recordBezel\":true}"));
        QCOMPARE(f.capture.prefs().value(QStringLiteral("copyOnCapture")).toBool(), true);
    }
};

QTEST_MAIN(TstSurface4)
#include "tst_surface4.moc"
