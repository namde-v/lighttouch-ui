// main.cpp — LightTouch Qt shell bootstrap.
//
// Wires: Bridge (dlopen/dlsym or LTM_BRIDGE_MOCK=1) → 12 domain adapters →
// QML context properties → qrc:/qml/main.qml.
//
// Environment:
//   LTM_BRIDGE_MOCK=1  run without libLTAppCore (synthesized devices/events)
//   LTM_APPCORE_LIB=<path>  explicit library path (default: app-dir-relative,
//                        then system search path)

#include <QApplication>
#include <QFileOpenEvent>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

// AppDelegate.swift's app-level hooks, Qt-style:
//  - open-URL (.ipa/.ipsw): QFileOpenEvent → the installer/importer, as the
//    Mac's open-URL handler routes to them;
//  - applicationShouldTerminateAfterLastWindowClosed == NO: closing the last
//    window does not quit (device windows come and go).
class LightTouchApplication : public QApplication {
    Q_OBJECT
public:
    using QApplication::QApplication;

    // Set after the adapters exist; called for every opened file/URL.
    std::function<void(const QStringList &)> openFiles;

protected:
    bool event(QEvent *e) override {
        if (e->type() == QEvent::FileOpen && openFiles) {
            const QString path = static_cast<QFileOpenEvent *>(e)->file();
            if (!path.isEmpty())
                openFiles({path});
            return true;
        }
        return QApplication::event(e);
    }

public:
    std::function<void()> quitRequest;
};

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTSidebarAdapter.h"
#include "adapters/LTDeviceAdapter.h"
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
#include "adapters/LTDisplayAdapter.h"
#include "adapters/LTStorageAdapter.h"
#include "adapters/LTProxyAdapter.h"
#include "adapters/LTDebugPortAdapter.h"
#include "adapters/LTNotificationsAdapter.h"
#include "adapters/LTTaskbarAdapter.h"
#include "items/DisplayItem.h"

int main(int argc, char **argv) {
    // QApplication (not QGuiApplication): LTJobsAdapter.pickIpswFiles() uses
    // a native QFileDialog.
    LightTouchApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("LightTouch"));
    app.setApplicationName(QStringLiteral("LightTouch"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    // The Mac does not quit when its last window closes (device windows).
    app.setQuitOnLastWindowClosed(false);

    Bridge bridge;
    QString bridgeError;
    if (!bridge.initialize(&bridgeError)) {
        qCritical().noquote() << "Cannot start: bridge initialization failed:"
                              << bridgeError;
        return 1;
    }
    qInfo() << "Bridge ready (mock:" << bridge.isMock() << ")";

    // Domain adapters. They hold no business logic: commands go down through
    // the bridge, events come back up as Q_PROPERTY updates.
    LTDeviceListAdapter deviceList(&bridge);
    LTSidebarAdapter sidebar(&bridge);
    LTDeviceAdapter device(&bridge);
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
    LTDisplayAdapter display(&bridge);
    LTStorageAdapter storage(&bridge);
    LTProxyAdapter proxy(&bridge);
    LTDebugPortAdapter debugPort(&bridge);
    LTNotificationsAdapter notifications(&bridge);
    LTTaskbarAdapter taskbar(&bridge);

    // The DisplayItem QML type (surface 2).
    qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");

    // Graceful quit: the core finishes teardown, then we exit.
    QObject::connect(&settings, &LTSettingsAdapter::quitReady,
                     &app, &QCoreApplication::quit, Qt::QueuedConnection);

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
    ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
    ctx->setContextProperty(QStringLiteral("sidebar"), &sidebar);
    ctx->setContextProperty(QStringLiteral("device"), &device);
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
    ctx->setContextProperty(QStringLiteral("display"), &display);
    ctx->setContextProperty(QStringLiteral("storage"), &storage);
    ctx->setContextProperty(QStringLiteral("proxy"), &proxy);
    ctx->setContextProperty(QStringLiteral("debugPort"), &debugPort);
    ctx->setContextProperty(QStringLiteral("notifications"), &notifications);
    ctx->setContextProperty(QStringLiteral("taskbar"), &taskbar);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Cannot start: failed to load qrc:/qml/main.qml";
        return 1;
    }

    // Populate the shell's first paint.
    deviceList.refresh();
    sidebar.refresh();
    settings.refresh();

    // Open-URL routing (AppDelegate): .ipsw → importer, .ipa → installer.
    app.openFiles = [&](const QStringList &paths) {
        QStringList ipsws, ipas;
        for (const QString &p : paths) {
            if (p.endsWith(QStringLiteral(".ipsw"), Qt::CaseInsensitive))
                ipsws << p;
            else if (p.endsWith(QStringLiteral(".ipa"), Qt::CaseInsensitive))
                ipas << p;
        }
        if (!ipsws.isEmpty())
            jobs.importIpsws(ipsws);
        for (const QString &p : ipas)
            apps.install(p);
    };
    app.quitRequest = [&]() { settings.quitRequest(); };
    // System logout/restart: ask the core for graceful shutdown, like
    // applicationShouldTerminate's native reply.
    QObject::connect(&app, &QGuiApplication::commitDataRequest,
                     [&]() { if (app.quitRequest) app.quitRequest(); });

    const int rc = app.exec();
    bridge.shutdown();
    return rc;
}

#include "main.moc"
