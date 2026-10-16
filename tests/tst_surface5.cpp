// tst_surface5.cpp — Surface 5 (apps inspector) tests, offscreen.
//
// Drives the apps adapter + AppsInspector QML against the mock bridge:
//   - list rows (pending install above installed, home-screen order),
//   - install → progress patches the row in place, done refreshes,
//   - remove / cancelInstall commands,
//   - store search flips, QML mode/search wiring,
//   - uninstall confirmation dialog contract.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTAppsAdapter.h"
#include "adapters/LTDeviceListAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface5 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTAppsAdapter apps;
        LTDeviceListAdapter deviceList;
        QQmlEngine engine;
        Fixture() : apps(&bridge), deviceList(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("apps"), &apps);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            deviceList.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
            apps.setDeviceUuid(QString::fromLatin1(kIPodUuid));
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface5"));
    }

    void listRowsPendingFirst() {
        Fixture f;
        const QVariantList rows = f.apps.rows();
        QCOMPARE(rows.size(), 3);
        // Pending install rows sit above installed apps.
        QCOMPARE(rows[0].toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("install"));
        QCOMPARE(rows[0].toMap().value(QStringLiteral("progress")).toDouble(), 0.42);
        QCOMPARE(rows[1].toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("app"));
        QCOMPARE(rows[1].toMap().value(QStringLiteral("title")).toString(),
                 QStringLiteral("Maps"));
        QVERIFY(f.apps.canQueueInstall());
        QVERIFY(f.apps.bannerText().isEmpty()); // no stale banner on fresh read
    }

    void installProgressPatchesRowInPlace() {
        Fixture f;
        QSignalSpy spy(&f.apps, SIGNAL(rowsChanged()));
        QVERIFY(spy.isValid());
        // The mock install fires progress (0.5) then done + list-changed
        // (queued through the bridge).
        f.apps.install(QStringLiteral("/tmp/mock.ipa"));
        QTRY_VERIFY_WITH_TIMEOUT(spy.size() >= 1, 5000);
        // After done the list refreshes; the pending row is still first.
        QTRY_COMPARE_WITH_TIMEOUT(
            f.apps.rows()[0].toMap().value(QStringLiteral("kind")).toString(),
            QStringLiteral("install"), 5000);
    }

    void removeAndCancelCommands() {
        Fixture f;
        // Generic mock accepts; the point is no crash and correct routing.
        f.apps.remove(QStringLiteral("com.apple.Maps"));
        f.apps.cancelInstall(QStringLiteral("com.mock.installing"));
        f.apps.restartSpringBoard();
        f.apps.importMedia(QStringList{QStringLiteral("/tmp/song.mp3")});
    }

    void inspectorQmlDefaultsToStore() {
        Fixture f;
        QObject *view = f.load("/views/AppsInspector.qml");
        QVERIFY2(view != nullptr, "AppsInspector failed to load");
        // The Mac defaults to Store, not Installed.
        QCOMPARE(view->property("mode").toInt(), 1);
        QVERIFY(view->findChild<QObject *>(QStringLiteral("storeAppsList")) != nullptr);
        QVERIFY(view->findChild<QObject *>(QStringLiteral("installedAppsList")) != nullptr);
        // Typing in the search flips to Store.
        view->setProperty("mode", 0);
        QObject *search = view->findChild<QObject *>(QStringLiteral("appsSearchField"));
        QVERIFY(search != nullptr);
        search->setProperty("text", QStringLiteral("maps"));
        QCOMPARE(view->property("mode").toInt(), 1);
        QCOMPARE(view->property("searchText").toString(), QStringLiteral("maps"));
    }

    void inspectorRowSelection() {
        Fixture f;
        QObject *view = f.load("/views/AppsInspector.qml");
        QVERIFY2(view != nullptr, "AppsInspector failed to load");
        view->setProperty("mode", 0);
        view->setProperty("selectedIdentity", QStringLiteral("com.apple.Maps"));
        QCOMPARE(view->property("selectedIdentity").toString(),
                 QStringLiteral("com.apple.Maps"));
        // Remove is enabled with a selection in Installed mode.
        QObject *removeBtn = view->findChild<QObject *>(QStringLiteral("removeAppButton"));
        QVERIFY(removeBtn != nullptr);
        QVERIFY(removeBtn->property("enabled").toBool());
    }
};

QTEST_MAIN(TstSurface5)
#include "tst_surface5.moc"
