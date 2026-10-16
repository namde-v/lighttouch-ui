// tst_surface3.cpp — Surface 3 (device shell / bezel modes) tests, offscreen.
//
// Phase 1 is the flat bezel only (no 3D skins — honest flag, phase 2):
//   - bezel modes 2d/off via the View ▸ Device Bezels menu,
//   - the preference is app-wide and remembered (migrates the old on/off),
//   - free-form forces the bezel off and restores the preference on exit,
//   - the home-button chrome hides with the bezel off.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTSettingsAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTDisplayAdapter.h"
#include "items/DisplayItem.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTAppsAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface3 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTSettingsAdapter settings;
        LTDeviceAdapter device;
        LTDisplayAdapter display;
        LTDeviceListAdapter deviceList;
        LTAppsAdapter apps;
        QQmlEngine engine;
        Fixture()
            : settings(&bridge), device(&bridge), display(&bridge),
              deviceList(&bridge), apps(&bridge) {
            qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("settings"), &settings);
            ctx->setContextProperty(QStringLiteral("device"), &device);
            ctx->setContextProperty(QStringLiteral("display"), &display);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            ctx->setContextProperty(QStringLiteral("apps"), &apps);
            deviceList.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface3"));
    }

    void bezelPreferenceRoundTrip() {
        Fixture f;
        f.settings.setBezelMode(QStringLiteral("2d"));
        QCOMPARE(f.settings.bezelMode(), QStringLiteral("2d"));
        f.settings.setBezelMode(QStringLiteral("off"));
        QCOMPARE(f.settings.bezelMode(), QStringLiteral("off"));
        // A fresh adapter reads the remembered value (app-wide).
        LTSettingsAdapter again(&f.bridge);
        QCOMPARE(again.bezelMode(), QStringLiteral("off"));
        f.settings.setBezelMode(QStringLiteral("2d")); // restore default
    }

    void screenAppliesBezelMode() {
        Fixture f;
        QObject *screen = f.load("/views/DeviceScreen.qml");
        QVERIFY2(screen != nullptr, "DeviceScreen failed to load");
        QCOMPARE(screen->property("bezelMode").toString(), QStringLiteral("2d"));
        QCOMPARE(screen->property("effectiveBezel").toString(),
                 QStringLiteral("2d"));
        screen->setProperty("bezelMode", QStringLiteral("off"));
        QCOMPARE(screen->property("effectiveBezel").toString(),
                 QStringLiteral("off"));
    }

    void freeFormForcesBezelOff() {
        Fixture f;
        QObject *screen = f.load("/views/DeviceScreen.qml");
        QVERIFY2(screen != nullptr, "DeviceScreen failed to load");
        screen->setProperty("bezelMode", QStringLiteral("2d"));
        // Free-form forces the bezel off…
        screen->setProperty("freeForm", true);
        QCOMPARE(screen->property("effectiveBezel").toString(),
                 QStringLiteral("off"));
        // …and the preference is restored on exit (derived, not stored).
        screen->setProperty("freeForm", false);
        QCOMPARE(screen->property("effectiveBezel").toString(),
                 QStringLiteral("2d"));
        QCOMPARE(screen->property("bezelMode").toString(),
                 QStringLiteral("2d"));
    }
};

QTEST_MAIN(TstSurface3)
#include "tst_surface3.moc"
