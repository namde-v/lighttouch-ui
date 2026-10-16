// tst_applaunch.cpp — app-launch smoke test.
//
// Boots the same object graph as src/main.cpp (Bridge + all 12 adapters +
// QQmlApplicationEngine), loads qml/main.qml offscreen, and verifies:
//   - the window creates without crashing,
//   - the sidebar populates with the mock devices,
//   - the auto-select wiring sets a current device on LTDeviceAdapter.

#include <QtTest>
#include <QFile>
#include <QSignalSpy>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTSidebarAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTDisplayAdapter.h"
#include "items/DisplayItem.h"
#include "adapters/LTTweaksAdapter.h"
#include "adapters/LTCarrierAdapter.h"
#include "adapters/LTAppsAdapter.h"
#include "adapters/LTFilesAdapter.h"
#include "adapters/LTCaptureAdapter.h"
#include "adapters/LTLogAdapter.h"
#include "adapters/LTCatalogAdapter.h"
#include "adapters/LTJobsAdapter.h"
#include "adapters/LTSettingsAdapter.h"
#include "adapters/LTDeviceIconAdapter.h"

class TstAppLaunch : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    void launch() {
        Bridge bridge;
        QString err;
        QVERIFY2(bridge.initialize(&err), qPrintable(err));
            qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");

        LTDeviceListAdapter deviceList(&bridge);
        LTSidebarAdapter sidebar(&bridge);
        LTDeviceAdapter device(&bridge);
        LTDisplayAdapter display(&bridge);
        LTTweaksAdapter tweaks(&bridge);
        LTCarrierAdapter carrier(&bridge);
        LTAppsAdapter apps(&bridge);
        LTFilesAdapter files(&bridge);
        LTCaptureAdapter capture(&bridge);
        LTLogAdapter log(&bridge);
        LTCatalogAdapter catalog(&bridge);
        LTJobsAdapter jobs(&bridge);
        LTSettingsAdapter settings(&bridge);
        LTDeviceIconAdapter deviceIcon;

        QQmlApplicationEngine engine;
        QQmlContext *ctx = engine.rootContext();
        ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
        ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
        ctx->setContextProperty(QStringLiteral("sidebar"), &sidebar);
        ctx->setContextProperty(QStringLiteral("device"), &device);
        ctx->setContextProperty(QStringLiteral("display"), &display);
        ctx->setContextProperty(QStringLiteral("tweaks"), &tweaks);
        ctx->setContextProperty(QStringLiteral("carrier"), &carrier);
        ctx->setContextProperty(QStringLiteral("apps"), &apps);
        ctx->setContextProperty(QStringLiteral("files"), &files);
        ctx->setContextProperty(QStringLiteral("capture"), &capture);
        ctx->setContextProperty(QStringLiteral("log"), &log);
        ctx->setContextProperty(QStringLiteral("catalog"), &catalog);
        ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
        ctx->setContextProperty(QStringLiteral("settings"), &settings);
        ctx->setContextProperty(QStringLiteral("deviceIcon"), &deviceIcon);

        const QUrl qmlUrl = QUrl::fromLocalFile(
            QString::fromUtf8(LT_QML_DIR) + QStringLiteral("/main.qml"));
        QVERIFY2(QFile::exists(qmlUrl.toLocalFile()),
                 qPrintable(qmlUrl.toLocalFile()));
        engine.load(qmlUrl);
        QVERIFY2(!engine.rootObjects().isEmpty(), "main.qml failed to load");

        QQuickWindow *window =
            qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QVERIFY(window != nullptr);
        window->show();
        QTRY_VERIFY_WITH_TIMEOUT(window->isExposed(), 5000);

        // The shell populates itself: main.cpp calls refresh(); the mock's
        // startup timer also fires kLTEventDeviceListChanged.
        deviceList.refresh();
        QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);

        // Nothing is auto-selected (Mac-faithful): selecting the first row
        // drives every device-scoped adapter.
        QVERIFY(device.currentDeviceUuid().isEmpty());
        QObject *sidebarView = engine.rootObjects().first()->findChild<QObject *>(
            QStringLiteral("deviceSidebar"));
        QVERIFY2(sidebarView != nullptr, "DeviceSidebar not found in main.qml");
        QMetaObject::invokeMethod(sidebarView, "setSelection",
                                  Q_ARG(QVariant, QVariant(QStringList{
                                      QStringLiteral("3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617")})),
                                  Q_ARG(QVariant, QVariant(-1)));
        QTRY_VERIFY_WITH_TIMEOUT(!device.currentDeviceUuid().isEmpty(), 5000);
        QCOMPARE(device.currentDeviceUuid(),
                 QStringLiteral("3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"));

        // Window survives a few event-loop turns with the mock event timer
        // firing toasts/job progress in the background.
        QTest::qWait(2500);
        QVERIFY(window->isVisible());

        bridge.shutdown();
    }
};

QTEST_MAIN(TstAppLaunch)
#include "tst_applaunch.moc"
