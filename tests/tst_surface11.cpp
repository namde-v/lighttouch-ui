// tst_surface11.cpp — Surface 11 (menus) tests, offscreen.
//
// Boots the full main.qml against the mock bridge and verifies:
//   - all 8 menus exist with their Mac-mapped shortcuts,
//   - per-open validation dims what doesn't apply (File Start/Delete,
//     future-surface items listed but dimmed, Mac-only Live Text blocked),
//   - morphing titles (Start/Shut Down…, Pause, Show/Hide …),
//   - the Device menu's hardware items validate through lt_menu_validate
//     (mock returns plausible answers from mock device state).

#include <QtTest>
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

namespace {
const char *kIPhoneUuid = "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"; // off
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7";   // running
} // namespace

class TstSurface11 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTDeviceListAdapter deviceList;
        LTSidebarAdapter sidebar;
        LTDeviceAdapter device;
        LTDisplayAdapter display;
        LTTweaksAdapter tweaks;
        LTCarrierAdapter carrier;
        LTAppsAdapter apps;
        LTFilesAdapter files;
        LTCaptureAdapter capture;
        LTLogAdapter log;
        LTCatalogAdapter catalog;
        LTJobsAdapter jobs;
        LTSettingsAdapter settings;
        LTDeviceIconAdapter deviceIcon;
        QQmlApplicationEngine engine;
        QObject *root = nullptr;
        QObject *menuBar = nullptr;
        Fixture()
            : deviceList(&bridge), sidebar(&bridge), device(&bridge),
              tweaks(&bridge), carrier(&bridge), apps(&bridge), files(&bridge),
              capture(&bridge), log(&bridge), catalog(&bridge), jobs(&bridge),
              settings(&bridge), display(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
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
            engine.load(qmlUrl);
            QVERIFY2(!engine.rootObjects().isEmpty(), "main.qml failed to load");
            root = engine.rootObjects().first();
            menuBar = root->findChild<QObject *>(QStringLiteral("menuBar"));
            QVERIFY2(menuBar != nullptr, "MainMenuBar not found");
            deviceList.refresh();
            jobs.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
        }
        QObject *last = nullptr;
        // Finds a menu/action by objectName; fails the test when missing.
        void findAction(const char *name) {
            last = menuBar->findChild<QObject *>(QString::fromUtf8(name));
            QVERIFY2(last != nullptr,
                     qPrintable(QString("menu action %1 not found").arg(name)));
        }
        void findMenu(const char *name) { findAction(name); }
        void refresh() {
            QVERIFY(QMetaObject::invokeMethod(menuBar, "refreshValidations"));
        }
        void selectDevice(const char *uuid) {
            QObject *sb = root->findChild<QObject *>(QStringLiteral("deviceSidebar"));
            QVERIFY(sb != nullptr);
            QVERIFY(QMetaObject::invokeMethod(sb, "setSelection",
                Q_ARG(QVariant, QVariant(QStringList{QString::fromUtf8(uuid)})),
                Q_ARG(QVariant, QVariant(-1))));
            QTRY_COMPARE_WITH_TIMEOUT(device.currentDeviceUuid(),
                                      QString::fromUtf8(uuid), 5000);
        }
        static bool isEnabled(QObject *a) { return a->property("enabled").toBool(); }
        static QString textOf(QObject *a) { return a->property("text").toString(); }
        static bool isChecked(QObject *a) { return a->property("checked").toBool(); }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    void eightMenusExist() {
        Fixture f;
        const char *menus[] = {"fileMenu", "editMenu", "viewMenu", "deviceMenu",
                               "appsMenu", "captureMenu", "windowMenu", "helpMenu"};
        for (const char *m : menus) {
            f.findMenu(m);
            QVERIFY2(f.last != nullptr,
                     qPrintable(QString("menu %1 missing").arg(m)));
        }
    }

    void fileValidationNoSelection() {
        Fixture f;
        f.refresh();
        // Nothing selected: start/delete/cancel dim; future items dim.
        QVERIFY(!Fixture::isEnabled((f.findAction("fileStartAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("deleteAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("cancelDownloadAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("downloadPrepareAction"), f.last))); // surface 9 live
        QVERIFY(Fixture::isEnabled((f.findAction("addDeviceAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("importIpswAction"), f.last)));
    }

    void fileStartMorphsWithPowerState() {
        Fixture f;
        f.selectDevice(kIPodUuid); // running
        f.refresh();
        QCOMPARE(Fixture::textOf((f.findAction("fileStartAction"), f.last)),
                 QStringLiteral("Shut Down…"));
        QVERIFY(Fixture::isEnabled((f.findAction("fileStartAction"), f.last)));
        f.selectDevice(kIPhoneUuid); // off
        f.refresh();
        QCOMPARE(Fixture::textOf((f.findAction("fileStartAction"), f.last)),
                 QStringLiteral("Start"));
        QVERIFY(Fixture::isEnabled((f.findAction("fileStartAction"), f.last)));
        QCOMPARE(Fixture::textOf((f.findAction("deleteAction"), f.last)),
                 QStringLiteral("Delete Device…"));
    }

    void deviceValidationThroughMenuValidate() {
        Fixture f;
        f.selectDevice(kIPodUuid); // running: guest accepts input
        f.refresh();
        QVERIFY(Fixture::isEnabled((f.findAction("homeAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("lockAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("rotateLeftAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("pauseAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("restartAction"), f.last)));
        QVERIFY(Fixture::isEnabled((f.findAction("northAction"), f.last)));
        QVERIFY(Fixture::isChecked((f.findAction("northAction"), f.last))); // mock: heading 0
        QVERIFY(!Fixture::isChecked((f.findAction("eastAction"), f.last)));
        QCOMPARE(Fixture::textOf((f.findAction("lockAction"), f.last)), QStringLiteral("Lock"));
        QCOMPARE(Fixture::textOf((f.findAction("pauseAction"), f.last)), QStringLiteral("Pause"));

        f.selectDevice(kIPhoneUuid); // off: input items dim
        f.refresh();
        QVERIFY(!Fixture::isEnabled((f.findAction("homeAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("lockAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("rotateLeftAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("pauseAction"), f.last)));
        QVERIFY(!Fixture::isEnabled((f.findAction("restartAction"), f.last)));
    }

    void validateMenuItemAdapterSeam() {
        Fixture f;
        f.device.setCurrentDeviceUuid(QString::fromUtf8(kIPodUuid));
        QVariantMap m = f.device.validateMenuItem(5 /* input */, 0);
        QVERIFY(m.value(QStringLiteral("enabled")).toBool());
        f.device.setCurrentDeviceUuid(QString::fromUtf8(kIPhoneUuid));
        m = f.device.validateMenuItem(5, 0);
        QVERIFY(!m.value(QStringLiteral("enabled")).toBool());
        // Unknown item: dim, never crashes.
        m = f.device.validateMenuItem(99, 0);
        QVERIFY(!m.value(QStringLiteral("enabled")).toBool());
    }

    void editMenuTextAndBlockedItems() {
        Fixture f;
        f.refresh();
        QVERIFY(Fixture::isEnabled((f.findAction("findAction"), f.last)));      // surface 9 live
        QVERIFY(!Fixture::isEnabled((f.findAction("liveTextAction"), f.last))); // blocked: no Qt Vision
        QCOMPARE(Fixture::textOf((f.findAction("liveTextAction"), f.last)),
                 QStringLiteral("Select Text on Screen"));
    }

    void shortcutsMapMacToQt() {
        Fixture f;
        QCOMPARE((f.findAction("recordAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+R"));
        QCOMPARE((f.findAction("saveScreenshotAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+S"));
        QCOMPARE((f.findAction("discardRecordingAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Escape"));
        QCOMPARE((f.findAction("installAppAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+Shift+I"));
        QCOMPARE((f.findAction("homeAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+Shift+H"));
        QCOMPARE((f.findAction("lockAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+L"));
        QCOMPARE((f.findAction("pasteToGuestAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+Meta+V"));
        QCOMPARE((f.findAction("quitAction"), f.last)->property("shortcut").toString(),
                 QStringLiteral("Ctrl+Q"));
    }

    void futureSurfaceItemsListedButDimmed() {
        Fixture f;
        f.selectDevice(kIPodUuid);
        f.refresh();
        // Every item whose surface hasn't landed is present and dimmed.
        // (The zoom items left this list when surface 2 landed.)
        const char *dim[] = {
            "bezel3dAction",
            "nativeSizeAction", "showHiddenAction",
            "connectInternetAction", "copyLldbAction",
            "jailbreakAction",
        };
        for (const char *n : dim) {
            f.findAction(n);
            QVERIFY2(!Fixture::isEnabled(f.last),
                     qPrintable(QString("%1 should be dimmed").arg(n)));
        }
    }

    void zoomItemsEnabledWhenRunning() {
        Fixture f;
        // Surface 2: zoom + free-form are live while a device runs.
        f.selectDevice(kIPodUuid); // running in the mock
        f.refresh();
        const char *live[] = {
            "physicalAction", "pixelAction", "fitAction",
            "zoomInAction", "zoomOutAction", "freeFormAction",
        };
        for (const char *n : live) {
            f.findAction(n);
            QVERIFY2(Fixture::isEnabled(f.last),
                     qPrintable(QString("%1 should be enabled (running)").arg(n)));
        }
        // ...and dimmed with nothing selected.
        f.selectDevice("");
        f.refresh();
        for (const char *n : live) {
            f.findAction(n);
            QVERIFY2(!Fixture::isEnabled(f.last),
                     qPrintable(QString("%1 should be dimmed (no device)").arg(n)));
        }
    }

    void fingerDotsToggleRoundTrip() {
        Fixture f;
        f.selectDevice(kIPodUuid);
        f.refresh();
        QObject *dots = (f.findAction("fingerDotsAction"), f.last);
        QCOMPARE(Fixture::textOf(dots), QStringLiteral("Show Finger Dots"));
        QVERIFY(!Fixture::isChecked(dots));
        QVERIFY(QMetaObject::invokeMethod(dots, "trigger"));
        QTRY_VERIFY_WITH_TIMEOUT(f.device.fingerDots(), 5000);
        f.refresh();
        QCOMPARE(Fixture::textOf(dots), QStringLiteral("Hide Finger Dots"));
        QVERIFY(Fixture::isChecked(dots));
    }

    void sidebarVisibilityMorph() {
        Fixture f;
        QObject *sb = f.root->findChild<QObject *>(QStringLiteral("deviceSidebar"));
        QVERIFY(sb != nullptr);
        f.refresh();
        QCOMPARE(Fixture::textOf((f.findAction("sidebarAction"), f.last)),
                 QStringLiteral("Hide Sidebar"));
        sb->setProperty("visible", false);
        f.refresh();
        QCOMPARE(Fixture::textOf((f.findAction("sidebarAction"), f.last)),
                 QStringLiteral("Show Sidebar"));
        QVERIFY(Fixture::isEnabled((f.findAction("sidebarAction"), f.last)));
    }
};

QTEST_MAIN(TstSurface11)
#include "tst_surface11.moc"
