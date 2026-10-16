// tst_surface10.cpp — Surface 10 (dialogs & sheets) tests, offscreen.
//
// Drives the real QML dialogs against the mock bridge:
//   - settings prefs round-trip (networkAccessAnswer via the mock settings
//     store; vibrationSound/refresh120/experimental via QSettings),
//   - about credits + licenses parse,
//   - Help window [Device] substitution,
//   - Add Device sheet: board grouping, stable/experimental filter,
//     already-in-sidebar + Coming Soon unpickable, pick → Add,
//   - Jailbreak dialog copy by iOS major version,
//   - first-start network prompt gating boot,
//   - window geometry save/restore.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTCatalogAdapter.h"
#include "adapters/LTJobsAdapter.h"
#include "adapters/LTSettingsAdapter.h"
#include "adapters/LTCaptureAdapter.h"
#include "adapters/LTDeviceIconAdapter.h"

namespace {
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface10 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTDeviceListAdapter deviceList;
        LTDeviceAdapter device;
        LTCatalogAdapter catalog;
        LTJobsAdapter jobs;
        LTSettingsAdapter settings;
        LTCaptureAdapter capture;
        LTDeviceIconAdapter deviceIcon;
        QQmlEngine engine;
        Fixture()
            : deviceList(&bridge), device(&bridge), catalog(&bridge),
              jobs(&bridge), settings(&bridge), capture(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            ctx->setContextProperty(QStringLiteral("device"), &device);
            ctx->setContextProperty(QStringLiteral("catalog"), &catalog);
            ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
            ctx->setContextProperty(QStringLiteral("settings"), &settings);
            ctx->setContextProperty(QStringLiteral("capture"), &capture);
            ctx->setContextProperty(QStringLiteral("deviceIcon"), &deviceIcon);
            deviceList.refresh();
            settings.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
        }
        // Returns null on failure; callers QVERIFY the result.
        QObject *load(const char *rel) {
            QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath(rel)));
            if (comp.isError()) {
                qWarning() << comp.errors();
                return nullptr;
            }
            return comp.create(engine.rootContext());
        }
        QVariant call(QObject *o, const char *method) {
            QVariant ret;
            if (!QMetaObject::invokeMethod(o, method,
                                           Q_RETURN_ARG(QVariant, ret)))
                qWarning() << "invokeMethod failed:" << method;
            return ret;
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface10"));
    }

    void settingsPrefsRoundTrip() {
        Fixture f;
        // Unset at start (mock store clears on lt_init).
        QCOMPARE(f.settings.networkAccessAnswer(), QString());
        f.settings.setNetworkAccessAnswer(QStringLiteral("always"));
        QCOMPARE(f.settings.networkAccessAnswer(), QStringLiteral("always"));
        // Raw key round-trips through the mock store.
        f.settings.setString(QStringLiteral("x.test"), QStringLiteral("v"));
        QCOMPARE(f.settings.getString(QStringLiteral("x.test")), QStringLiteral("v"));
        // Qt-native prefs (were @AppStorage).
        f.settings.setVibrationSound(false);
        QVERIFY(!f.settings.vibrationSound());
        f.settings.setRefresh120Hz(true);
        QVERIFY(f.settings.refresh120Hz());
        f.settings.setAddDeviceShowsExperimental(true);
        QVERIFY(f.settings.addDeviceShowsExperimental());
    }

    void aboutCreditsParse() {
        Fixture f;
        f.settings.refresh();
        const QVariantMap credits = f.settings.aboutCredits();
        const QVariantList runs = credits.value(QStringLiteral("credits")).toList();
        QVERIFY(runs.size() >= 2);
        QCOMPARE(runs[0].toMap().value(QStringLiteral("text")).toString(),
                 QStringLiteral("LightTouch"));
        QVERIFY(runs[0].toMap().value(QStringLiteral("isHeading")).toBool());
        bool hasLicensesLink = false;
        for (const QVariant &r : runs)
            if (r.toMap().value(QStringLiteral("link")).toString()
                == QStringLiteral("lighttouch:licenses"))
                hasLicensesLink = true;
        QVERIFY(hasLicensesLink);
        const QVariantList licenses =
            credits.value(QStringLiteral("licenses")).toList();
        QVERIFY(!licenses.isEmpty());
        QVERIFY(!licenses[0].toMap().value(QStringLiteral("name")).toString().isEmpty());
        QVERIFY(!licenses[0].toMap().value(QStringLiteral("text")).toString().isEmpty());
    }

    void helpTopicsDeviceSubstitution() {
        Fixture f;
        QObject *help = f.load("/dialogs/HelpWindow.qml");
        QVERIFY2(help != nullptr, "dialog failed to load");
        const QVariantList topics = f.settings.helpTopics();
        QVERIFY(!topics.isEmpty());
        help->setProperty("deviceName", QStringLiteral("iPod touch"));
        QVariant ret;
        QVERIFY(QMetaObject::invokeMethod(
            help, "shownBody", Q_RETURN_ARG(QVariant, ret),
            Q_ARG(QVariant, topics[0].toMap())));
        QVERIFY(!ret.toString().contains(QStringLiteral("[Device]")));
        QVERIFY(ret.toString().contains(QStringLiteral("iPod touch")));
    }

    void addDeviceSheetGrouping() {
        Fixture f;
        f.settings.setAddDeviceShowsExperimental(false); // (QSettings persists)
        QObject *sheet = f.load("/dialogs/AddDeviceSheet.qml");
        QVERIFY2(sheet != nullptr, "dialog failed to load");
        f.catalog.refresh(); // (the sheet does this in onOpened)
        QVERIFY(f.catalog.entries().size() == 7);
        QVariant groups = f.call(sheet, "groups");
        QCOMPARE(groups.toList().size(), 3); // n88ap, n72ap, k48ap
        // Stable-only by default: n72ap's experimental beta is hidden.
        for (const QVariant &g : groups.toList()) {
            const QVariantMap gm = g.toMap();
            const QVariantList es = gm.value(QStringLiteral("entries")).toList();
            if (gm.value(QStringLiteral("id")).toString() == QStringLiteral("n72ap"))
                QCOMPARE(es.size(), 1);
            if (gm.value(QStringLiteral("id")).toString() == QStringLiteral("n88ap"))
                QCOMPARE(es.size(), 2);
        }
        f.settings.setAddDeviceShowsExperimental(true);
        groups = f.call(sheet, "groups");
        for (const QVariant &g : groups.toList()) {
            const QVariantMap gm = g.toMap();
            if (gm.value(QStringLiteral("id")).toString() == QStringLiteral("n72ap"))
                QCOMPARE(gm.value(QStringLiteral("entries")).toList().size(), 2);
        }
    }

    void addDeviceSheetAddedAndComingSoon() {
        Fixture f;
        QObject *sheet = f.load("/dialogs/AddDeviceSheet.qml");
        QVERIFY2(sheet != nullptr, "dialog failed to load");
        f.catalog.refresh();
        QVERIFY(f.catalog.entries().size() == 7);
        // The mock iPhone 3GS row (n88ap, "iPhone OS 3.1.3") matches the
        // n88ap-3.1.3 entry: checked and unpickable.
        QVariantMap entry;
        for (const QVariant &e : f.catalog.entries()) {
            if (e.toMap().value(QStringLiteral("id")).toString()
                == QStringLiteral("n88ap-3.1.3"))
                entry = e.toMap();
        }
        QVERIFY(!entry.isEmpty());
        QVariant ret;
        QVERIFY(QMetaObject::invokeMethod(sheet, "isAdded",
                                         Q_RETURN_ARG(QVariant, ret),
                                         Q_ARG(QVariant, entry)));
        QVERIFY(ret.toBool());
        // Picking the added entry anyway must not surface in pickedIds().
        QVariantMap picked;
        picked[QStringLiteral("n88ap-3.1.3")] = true;
        picked[QStringLiteral("k48ap-5.0")] = true; // userIPSW, not in sidebar
        picked[QStringLiteral("k48ap-6.0")] = true; // comingSoon
        sheet->setProperty("picked", picked);
        const QVariantList ids = f.call(sheet, "pickedIds").toList();
        QCOMPARE(ids.size(), 1);
        QCOMPARE(ids[0].toString(), QStringLiteral("k48ap-5.0"));
    }

    void addDeviceSheetPickAndAdd() {
        Fixture f;
        QObject *sheet = f.load("/dialogs/AddDeviceSheet.qml");
        QVERIFY2(sheet != nullptr, "dialog failed to load");
        f.catalog.refresh();
        QVERIFY(f.catalog.entries().size() == 7);
        QVariantMap picked;
        picked[QStringLiteral("k48ap-5.0")] = true; // userIPSW, not in sidebar
        sheet->setProperty("picked", picked);
        sheet->setProperty("jailbreak", true);
        sheet->setProperty("skipSetup", true);
        // doAdd starts one job per pick with the prepare options, clears picks.
        QVERIFY(QMetaObject::invokeMethod(sheet, "doAdd"));
        QCOMPARE(sheet->property("picked").toMap().size(), 0);
    }

    void jailbreakCopyByVersion() {
        Fixture f;
        QObject *dlg = f.load("/dialogs/JailbreakDialog.qml");
        QVERIFY2(dlg != nullptr, "dialog failed to load");
        dlg->setProperty("deviceName", QStringLiteral("iPad"));
        dlg->setProperty("version", QStringLiteral("5.1.1"));
        QVariant ret;
        QVERIFY(QMetaObject::invokeMethod(dlg, "informativeText",
                                         Q_RETURN_ARG(QVariant, ret)));
        QVERIFY(ret.toString().contains(QStringLiteral("Substrate")));
        dlg->setProperty("version", QStringLiteral("7.0"));
        QVERIFY(QMetaObject::invokeMethod(dlg, "informativeText",
                                         Q_RETURN_ARG(QVariant, ret)));
        QVERIFY(!ret.toString().contains(QStringLiteral("Substrate")));
        QVERIFY(ret.toString().contains(QStringLiteral("afc2")));
    }

    void networkPromptGatesBoot() {
        Fixture f;
        // No saved answer: boot() asks instead of booting.
        QSignalSpy prompt(&f.device,
                         SIGNAL(networkAccessPromptRequested()));
        QVERIFY(prompt.isValid());
        f.device.setCurrentDeviceUuid(
            QStringLiteral("3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"));
        f.device.boot();
        QCOMPARE(prompt.size(), 1);
        // With an answer saved, boot() goes straight through (no prompt).
        f.settings.setNetworkAccessAnswer(QStringLiteral("never"));
        f.device.boot();
        QCOMPARE(prompt.size(), 1);
        // Same gate on the row-scoped path.
        QSignalSpy rowPrompt(
            &f.deviceList,
            SIGNAL(networkAccessPromptRequested(QString, QString)));
        QVERIFY(rowPrompt.isValid());
        f.settings.setNetworkAccessAnswer(QString());
        f.deviceList.bootDevice(
            QStringLiteral("3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"));
        QCOMPARE(rowPrompt.size(), 1);
        QCOMPARE(rowPrompt[0][1].toString(), QStringLiteral("iPhone 3GS"));
    }

    void settingsDialogPanes() {
        Fixture f;
        QObject *dlg = f.load("/dialogs/SettingsDialog.qml");
        QVERIFY2(dlg != nullptr, "dialog failed to load");
        QVERIFY(dlg->property("title").toString().contains(QStringLiteral("General")));
        QObject *bar = dlg->findChild<QObject *>(QStringLiteral("settingsTabBar"));
        QVERIFY(bar != nullptr);
        bar->setProperty("currentIndex", 1);
        QVERIFY(dlg->property("title").toString().contains(QStringLiteral("Capture")));
        bar->setProperty("currentIndex", 2);
        QVERIFY(dlg->property("title").toString().contains(QStringLiteral("Storage")));
    }

    void windowGeometryRoundTrip() {
        Fixture f;
        f.settings.saveWindowGeometry(10, 20, 300, 400);
        const QVariantMap g = f.settings.windowGeometry();
        QCOMPARE(g.value(QStringLiteral("x")).toInt(), 10);
        QCOMPARE(g.value(QStringLiteral("y")).toInt(), 20);
        QCOMPARE(g.value(QStringLiteral("width")).toInt(), 300);
        QCOMPARE(g.value(QStringLiteral("height")).toInt(), 400);
    }
};

QTEST_MAIN(TstSurface10)
#include "tst_surface10.moc"
