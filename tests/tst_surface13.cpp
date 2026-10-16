// tst_surface13.cpp — Surface 13 (notifications) tests, offscreen.
//
// Qt counterpart of Features/CaptureNotifications.swift:
//   - kLTEventToast → notify() → toastReceived (in-app) + OS notification,
//   - permission asked only on first real news (never at startup),
//   - "device-ready" click → showDeviceRequested(entryId),
//   - capture completion toasts (surface 8's hook).

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>

#include "Bridge.h"
#include "adapters/LTNotificationsAdapter.h"

namespace {
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface13 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTNotificationsAdapter notifications;
        QQmlEngine engine;
        Fixture() : notifications(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("notifications"),
                                    &notifications);
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface13"));
    }

    void toastReachesInAppSurface() {
        Fixture f;
        QSignalSpy toast(&f.notifications,
                         SIGNAL(toastReceived(QString, QString, QString)));
        QVERIFY(toast.isValid());
        f.bridge.mockFireToast(QStringLiteral("Ready"),
                               QStringLiteral("iPad is ready to use"),
                               QStringLiteral("device-ready:n88ap-3.1.3"));
        QTRY_COMPARE_WITH_TIMEOUT(toast.size(), 1, 5000);
        QCOMPARE(toast[0][0].toString(), QStringLiteral("Ready"));
        QCOMPARE(toast[0][1].toString(),
                 QStringLiteral("iPad is ready to use"));
    }

    void notifyDirect() {
        Fixture f;
        QSignalSpy toast(&f.notifications,
                         SIGNAL(toastReceived(QString, QString, QString)));
        // The first notify() is the permission ask (Mac: only on first news);
        // constructing the adapter alone never prompts.
        f.notifications.notify(QStringLiteral("Screenshot saved"),
                               QStringLiteral("/tmp/a.png"),
                               QStringLiteral("capture-screenshot"));
        QCOMPARE(toast.size(), 1);
        QCOMPARE(toast[0][2].toString(),
                 QStringLiteral("capture-screenshot"));
    }

    void deviceReadyCategoryCarriesEntryId() {
        Fixture f;
        QSignalSpy toast(&f.notifications,
                         SIGNAL(toastReceived(QString, QString, QString)));
        QVERIFY(toast.isValid());
        // The core's "device-ready:<entryId>" category splits: the in-app
        // surface sees "device-ready", the click handler keeps the entry id
        // for showDeviceRequested.
        f.bridge.mockFireToast(QStringLiteral("Ready"),
                               QStringLiteral("iPad is ready to use"),
                               QStringLiteral("device-ready:k48ap-5.1.1"));
        QTRY_COMPARE_WITH_TIMEOUT(toast.size(), 1, 5000);
        QCOMPARE(toast[0][2].toString(), QStringLiteral("device-ready"));
        // A plain toast keeps its category untouched.
        f.bridge.mockFireToast(QStringLiteral("T"), QStringLiteral("B"),
                               QStringLiteral("mock"));
        QTRY_COMPARE_WITH_TIMEOUT(toast.size(), 2, 5000);
        QCOMPARE(toast[1][2].toString(), QStringLiteral("mock"));
    }
};

QTEST_MAIN(TstSurface13)
#include "tst_surface13.moc"
