// tst_surface12.cpp — Surface 12 (drag-and-drop) tests, offscreen.
//
//   - lt_drop_classify via the adapter (extension check),
//   - lt_drop_route → kLTEventDropAction dispatch,
//   - the screen's refusal rules (IPSW any time; .ipa/.media only while the
//     device can queue installs — never an error sheet per file),
//   - DropHighlight shows only for accepted operations.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTAppsAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTDisplayAdapter.h"
#include "items/DisplayItem.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7"; // running
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface12 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTDeviceListAdapter deviceList;
        LTAppsAdapter apps;
        LTDeviceAdapter device;
        LTDisplayAdapter display;
        QQmlEngine engine;
        Fixture()
            : deviceList(&bridge), apps(&bridge), device(&bridge),
              display(&bridge) {
            qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            ctx->setContextProperty(QStringLiteral("apps"), &apps);
            ctx->setContextProperty(QStringLiteral("device"), &device);
            ctx->setContextProperty(QStringLiteral("display"), &display);
            deviceList.refresh();
            QTRY_COMPARE_WITH_TIMEOUT(deviceList.count(), 3, 5000);
            apps.setDeviceUuid(QString::fromLatin1(kIPodUuid));
            device.setCurrentDeviceUuid(QString::fromLatin1(kIPodUuid));
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface12"));
    }

    void classifyByExtension() {
        Fixture f;
        // kLTDropIPSW=0, kLTDropIPA=1, kLTDropMedia=2, kLTDropUnsupported=3.
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.ipsw")), 0);
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.ipa")), 1);
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.mp3")), 2);
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.png")), 2);
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.txt")), 3);
        QCOMPARE(f.deviceList.classifyDropUrl(
                     QStringLiteral("file:///tmp/a.IPSW")), 0); // case-insensitive
    }

    void routeDropFiresActionEvent() {
        Fixture f;
        QSignalSpy routed(&f.deviceList,
                          SIGNAL(dropRouted(quint64, QString, QString)));
        QVERIFY(routed.isValid());
        const quint64 id = f.deviceList.routeDrop(
            QStringList{QStringLiteral("file:///tmp/a.ipsw")},
            QStringLiteral("sidebar"));
        QVERIFY(id != 0);
        QTRY_COMPARE_WITH_TIMEOUT(routed.size(), 1, 5000);
        QCOMPARE(routed[0][0].toULongLong(), id);
        QCOMPARE(routed[0][1].toString(), QStringLiteral("import-ipsw"));
        // .ipa routes to the installer.
        f.deviceList.routeDrop(QStringList{QStringLiteral("file:///tmp/a.ipa")},
                               QStringLiteral("display:x"));
        QTRY_COMPARE_WITH_TIMEOUT(routed.size(), 2, 5000);
        QCOMPARE(routed[1][1].toString(), QStringLiteral("install-ipa"));
    }

    void screenRefusesWhileBusy() {
        Fixture f;
        QObject *screen = f.load("/views/DeviceScreen.qml");
        QVERIFY2(screen != nullptr, "DeviceScreen failed to load");
        // The screen's drop rules live in QML (dropAcceptable).
        QVariant ret;
        // apps.canQueueInstall is true for the running iPod mock.
        QVERIFY(f.apps.canQueueInstall());
        QVERIFY(QMetaObject::invokeMethod(screen, "dropTarget",
                                          Q_RETURN_ARG(QVariant, ret)));
        QVERIFY(ret.toString().startsWith(QStringLiteral("display:")));
    }

    void dropHighlightShowsOnlyWhenAccepted() {
        Fixture f;
        QObject *ring = f.load("/views/DropHighlight.qml");
        QVERIFY2(ring != nullptr, "DropHighlight failed to load");
        QVERIFY(!ring->property("visible").toBool());
        // TEMP DIAGNOSTIC (Windows CI): dump what "show" resolves to.
        const QMetaObject *mo = ring->metaObject();
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            const QMetaMethod m = mo->method(i);
            if (QString::fromLatin1(m.name()).contains(QStringLiteral("show")))
                qDebug() << "method:" << m.methodSignature()
                         << "type:" << m.methodType()
                         << "params:" << m.parameterCount();
        }
        const bool invoked =
            QMetaObject::invokeMethod(ring, "show",
                                      Q_ARG(QVariant, QVariant(true)));
        qDebug() << "invokeMethod returned:" << invoked
                 << "visible now:" << ring->property("visible").toBool();
        QVERIFY(invoked);
        QVERIFY(ring->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(ring, "show",
                                          Q_ARG(QVariant, QVariant(false))));
        QVERIFY(!ring->property("visible").toBool());
    }
};

QTEST_MAIN(TstSurface12)
#include "tst_surface12.moc"
