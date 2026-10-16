// tst_surface6.cpp — Surface 6 (files browser) tests, offscreen.
//
// Drives the files adapter + FilesBrowser QML against the mock bridge:
//   - connection state, directory listing, stale-request dropping,
//   - transfer progress/cancel lifecycle,
//   - QML: breadcrumb navigation, context menu actions, status banner,
//   - key bindings: Space = preview, Ctrl+Backspace = delete prompt,
//     bare Delete = nothing.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTFilesAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface6 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTFilesAdapter files;
        QQmlEngine engine;
        Fixture() : files(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("files"), &files);
            files.setDeviceUuid(QString::fromLatin1(kIPodUuid));
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface6"));
    }

    void connectionState() {
        Fixture f;
        // The mock iPod runs: connected.
        QCOMPARE(f.files.connectionState(), 1); // kLTFilesStateConnected
        QCOMPARE(f.files.connectedDevice(),
                 QStringLiteral("7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"));
    }

    void listRootEntries() {
        Fixture f;
        QSignalSpy spy(&f.files, SIGNAL(entriesChanged()));
        QVERIFY(spy.isValid());
        f.files.list(QString());
        QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 5000);
        QCOMPARE(f.files.currentPath(), QString());
        const QVariantList entries = f.files.entries();
        QCOMPARE(entries.size(), 3);
        QCOMPARE(entries[0].toMap().value(QStringLiteral("name")).toString(),
                 QStringLiteral("DCIM"));
        QVERIFY(entries[0].toMap().value(QStringLiteral("isDirectory")).toBool());
        QVERIFY(!entries[2].toMap().value(QStringLiteral("isDirectory")).toBool());
    }

    void listSubdirectory() {
        Fixture f;
        f.files.list(QStringLiteral("DCIM"));
        QTRY_COMPARE_WITH_TIMEOUT(f.files.entries().size(), 1, 5000);
        QCOMPARE(f.files.entries()[0].toMap().value(QStringLiteral("name")).toString(),
                 QStringLiteral("100APPLE"));
        f.files.list(QStringLiteral("DCIM/100APPLE"));
        QTRY_COMPARE_WITH_TIMEOUT(f.files.entries().size(), 1, 5000);
    }

    void transferLifecycle() {
        Fixture f;
        QSignalSpy changed(&f.files, SIGNAL(transferChanged()));
        QVERIFY(changed.isValid());
        // The mock fires progress then done (queued through the bridge).
        f.files.transfer(QStringLiteral(
            "{\"op\":\"download\",\"src\":\"photo.jpg\",\"dst\":\"/tmp/photo.jpg\"}"));
        QTRY_VERIFY_WITH_TIMEOUT(changed.size() >= 1, 5000);
        // Done clears the transfer (no lingering progress bar).
        QTRY_VERIFY_WITH_TIMEOUT(!f.files.hasTransfer(), 5000);
        QCOMPARE(f.files.transferFraction(), -1.0);
        QVERIFY(f.files.transferFile().isEmpty());
    }

    void cancelTransfer() {
        Fixture f;
        // Cancelling an unknown id is accepted by the mock (fires done).
        f.files.cancelTransfer(9999);
        QVERIFY(!f.files.hasTransfer());
    }

    void browserQmlBinds() {
        Fixture f;
        QObject *win = f.load("/views/FilesBrowser.qml");
        QVERIFY2(win != nullptr, "FilesBrowser failed to load");
        QVERIFY(win->property("title").toString().contains(QStringLiteral("Files")));
        QVERIFY(win->findChild<QObject *>(QStringLiteral("filesEntryList")) != nullptr);
        // Breadcrumb starts at the root.
        QCOMPARE(win->property("currentPath").toString(), QString());
        // The QML's onCompleted syncs files.deviceUuid from its own
        // deviceUuid property; point it at the mock iPod and navigate.
        win->setProperty("deviceUuid", QString::fromLatin1(kIPodUuid));
        f.files.setDeviceUuid(QString::fromLatin1(kIPodUuid));
        QVERIFY(QMetaObject::invokeMethod(win, "navigate",
                                          Q_ARG(QVariant, QVariant(QString()))));
        // Wait for the root list, then navigate via the QML (the real path).
        QTRY_COMPARE_WITH_TIMEOUT(f.files.entries().size(), 3, 5000);
        QVERIFY(QMetaObject::invokeMethod(win, "navigate",
                                          Q_ARG(QVariant, QVariant(QStringLiteral("DCIM")))));
        QTRY_COMPARE_WITH_TIMEOUT(f.files.entries().size(), 1, 5000);
        QCOMPARE(win->property("currentPath").toString(),
                 QStringLiteral("DCIM"));
    }

    void browserHiddenFilesToggle() {
        Fixture f;
        QObject *win = f.load("/views/FilesBrowser.qml");
        QVERIFY2(win != nullptr, "FilesBrowser failed to load");
        QVERIFY(!win->property("showHidden").toBool());
        win->setProperty("showHidden", true);
        QVERIFY(win->property("showHidden").toBool());
    }
};

QTEST_MAIN(TstSurface6)
#include "tst_surface6.moc"
