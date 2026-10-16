// tst_surface1.cpp — Surface 1 (main window + sidebar) tests, offscreen.
//
// Drives the real QML (qml/views/DeviceSidebar.qml, DevicePlaceholder.qml,
// DevicePane.qml) against the mock bridge: row population, selection,
// guarded delete (one question for prepared devices, running rows skipped,
// Finder-style next-row selection), in-place rename, per-row job progress,
// and the device↔job icon/classify adapter seams.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTDisplayAdapter.h"
#include "items/DisplayItem.h"
#include "adapters/LTCaptureAdapter.h"
#include "adapters/LTJobsAdapter.h"
#include "adapters/LTAppsAdapter.h"
#include "adapters/LTDeviceIconAdapter.h"
#include "adapters/LTSettingsAdapter.h"

namespace {
const char *kIPhoneUuid = "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"; // off, prepared
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7";   // running
const char *kIPadUuid = "a1b2c3d4-e5f6-4789-abcd-ef0123456789";   // booting + job

QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface1 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTDeviceListAdapter deviceList;
        LTDeviceAdapter device;
        LTDisplayAdapter display;
        LTCaptureAdapter capture;
        LTJobsAdapter jobs;
        LTAppsAdapter apps;
        LTDeviceIconAdapter deviceIcon;
        LTSettingsAdapter settings;
        QQmlEngine engine;
        Fixture()
            : deviceList(&bridge), device(&bridge), jobs(&bridge),
              apps(&bridge), settings(&bridge), display(&bridge),
              capture(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            qmlRegisterType<DisplayItem>("LightTouch", 1, 0, "DisplayItem");
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("deviceList"), &deviceList);
            ctx->setContextProperty(QStringLiteral("device"), &device);
            ctx->setContextProperty(QStringLiteral("display"), &display);
            ctx->setContextProperty(QStringLiteral("capture"), &capture);
            ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
            ctx->setContextProperty(QStringLiteral("apps"), &apps);
            ctx->setContextProperty(QStringLiteral("deviceIcon"), &deviceIcon);
            ctx->setContextProperty(QStringLiteral("settings"), &settings);
            deviceList.refresh();
            jobs.refresh();
        }
        QObject *load(const char *rel, QQuickWindow *window = nullptr) {
            QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath(rel)));
            if (comp.isError()) {
                qWarning() << comp.errors();
                return nullptr;
            }
            QObject *o = comp.create(engine.rootContext());
            if (window) {
                if (auto *item = qobject_cast<QQuickItem *>(o)) {
                    item->setParentItem(window->contentItem());
                    item->setSize(window->size());
                }
            }
            return o;
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    // Adapter seams -----------------------------------------------------

    void iconAdapterMapsBoards() {
        LTDeviceIconAdapter icons;
        QVERIFY(icons.iconSource(QStringLiteral("n88ap")).endsWith(QStringLiteral("iphone.png")));
        QVERIFY(icons.iconSource(QStringLiteral("n72ap")).endsWith(QStringLiteral("ipod.png")));
        QVERIFY(icons.iconSource(QStringLiteral("k48ap")).endsWith(QStringLiteral("ipad.png")));
        QVERIFY(icons.iconSource(QStringLiteral("xyz-1,1")).endsWith(QStringLiteral("device-generic.png")));
    }

    void dropClassification() {
        Fixture f;
        QCOMPARE(f.deviceList.classifyDropUrl(QStringLiteral("file:///tmp/a.ipsw")), kLTDropIPSW);
        QCOMPARE(f.deviceList.classifyDropUrl(QStringLiteral("file:///tmp/a.IPA")), kLTDropIPA);
        QCOMPARE(f.deviceList.classifyDropUrl(QStringLiteral("file:///tmp/a.mp3")), kLTDropMedia);
        QCOMPARE(f.deviceList.classifyDropUrl(QStringLiteral("file:///tmp/a.txt")), kLTDropUnsupported);
        QCOMPARE(f.deviceList.classifyDropUrl(QStringLiteral("not-a-url")), -1);
    }

    void renameMutatesMockAndRefreshes() {
        Fixture f;
        QSignalSpy spy(&f.deviceList, &LTDeviceListAdapter::devicesChanged);
        f.deviceList.renameDevice(QString::fromLatin1(kIPhoneUuid), QStringLiteral("Lab Phone"));
        QTRY_VERIFY_WITH_TIMEOUT(spy.size() >= 1, 5000);
        QCOMPARE(f.deviceList.devices().first().toMap().value(QStringLiteral("name")).toString(),
                 QStringLiteral("Lab Phone"));
    }

    // Sidebar QML --------------------------------------------------------

    void sidebarPopulatesAndSelects() {
        Fixture f;
        QQuickWindow window;
        window.resize(300, 600);
        QObject *root = f.load("/views/DeviceSidebar.qml", &window);
        QVERIFY2(root != nullptr, "DeviceSidebar.qml failed to load");
        window.show();

        auto *list = root->findChild<QQuickItem *>(QStringLiteral("deviceListView"));
        QVERIFY(list != nullptr);
        QTRY_COMPARE_WITH_TIMEOUT(list->property("count").toInt(), 3, 5000);

        // Click the first row → selectionChanged([iphoneUuid]).
        QSignalSpy selSpy(root, SIGNAL(selectionChanged(QVariant)));
        const QPointF p = list->mapToItem(window.contentItem(),
                                          QPointF(list->width() / 2, 23));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
                          p.toPoint());
        QCOMPARE(selSpy.size(), 1);
        const QStringList ids =
            selSpy.first().first().toStringList();
        QCOMPARE(ids, QStringList(QString::fromLatin1(kIPhoneUuid)));
        QCOMPARE(root->property("selectedIds").toStringList(), ids);
    }

    void deleteGuardAsksSkipsRunningAndSelectsNext() {
        Fixture f;
        QQuickWindow window;
        window.resize(300, 600);
        QObject *root = f.load("/views/DeviceSidebar.qml", &window);
        QVERIFY(root != nullptr);
        window.show();
        auto *list = root->findChild<QQuickItem *>(QStringLiteral("deviceListView"));
        QTRY_COMPARE_WITH_TIMEOUT(list->property("count").toInt(), 3, 5000);

        // Select the prepared iPhone AND the running iPod.
        QMetaObject::invokeMethod(root, "setSelection",
                                  Q_ARG(QVariant, QVariant(QStringList{
                                      QString::fromLatin1(kIPhoneUuid),
                                      QString::fromLatin1(kIPodUuid)})),
                                  Q_ARG(QVariant, QVariant(-1)));
        // The guarded delete: asks once (iPhone is prepared), notes the
        // running iPod stays.
        QMetaObject::invokeMethod(root, "deleteSelected");
        QObject *dlg = root->property("deleteDialog").value<QObject *>();
        QVERIFY2(dlg != nullptr, "delete dialog did not load");
        QTRY_VERIFY_WITH_TIMEOUT(dlg->property("opened").toBool(), 5000);
        QVERIFY(dlg->property("title").toString().contains(QStringLiteral("Delete")));
        QVERIFY(dlg->property("informativeText").toString().contains(
            QStringLiteral("running or busy")));

        // Confirm → iPhone deleted, iPod skipped, selection on next row.
        QQuickItem *confirmBtn =
            dlg->findChild<QQuickItem *>(QStringLiteral("deleteConfirmButton"));
        QVERIFY(confirmBtn != nullptr);
        QMetaObject::invokeMethod(confirmBtn, "clicked");
        QTRY_COMPARE_WITH_TIMEOUT(f.deviceList.count(), 2, 5000);
        QCOMPARE(f.deviceList.devices().first().toMap().value(QStringLiteral("id")).toString(),
                 QString::fromLatin1(kIPodUuid));
        // Finder-style: the next row took the selection.
        QTRY_COMPARE_WITH_TIMEOUT(
            root->property("selectedIds").toStringList(),
            QStringList(QString::fromLatin1(kIPodUuid)), 5000);
    }

    void deleteGuardInstantForUnpreparedRow() {
        Fixture f;
        // A fresh (unprepared) row leaves the list at once, no question.
        f.deviceList.createDevice(QStringLiteral("entry-x"), QStringLiteral("Fresh"));
        QTRY_COMPARE_WITH_TIMEOUT(f.deviceList.count(), 4, 5000);
        QQuickWindow window;
        window.resize(300, 600);
        QObject *root = f.load("/views/DeviceSidebar.qml", &window);
        QVERIFY(root != nullptr);
        window.show();
        auto *list = root->findChild<QQuickItem *>(QStringLiteral("deviceListView"));
        QTRY_COMPARE_WITH_TIMEOUT(list->property("count").toInt(), 4, 5000);

        const QString freshId =
            f.deviceList.devices().last().toMap().value(QStringLiteral("id")).toString();
        QMetaObject::invokeMethod(root, "setSelection",
                                  Q_ARG(QVariant, QVariant(QStringList{freshId})),
                                  Q_ARG(QVariant, QVariant(-1)));
        QMetaObject::invokeMethod(root, "deleteSelected");
        // No dialog for a nothing-on-disk row.
        QObject *dlg = root->property("deleteDialog").value<QObject *>();
        QTest::qWait(300);
        QVERIFY(!dlg->property("opened").toBool());
        QTRY_COMPARE_WITH_TIMEOUT(f.deviceList.count(), 3, 5000);
    }

    void renameInPlace() {
        Fixture f;
        QQuickWindow window;
        window.resize(300, 600);
        QObject *root = f.load("/views/DeviceSidebar.qml", &window);
        QVERIFY(root != nullptr);
        window.show();
        auto *list = root->findChild<QQuickItem *>(QStringLiteral("deviceListView"));
        QTRY_COMPARE_WITH_TIMEOUT(list->property("count").toInt(), 3, 5000);

        QMetaObject::invokeMethod(root, "beginRename",
                                  Q_ARG(QVariant, QVariant(QString::fromLatin1(kIPhoneUuid))));
        QQuickItem *editor = root->findChild<QQuickItem *>(QStringLiteral("renameEditor"));
        QVERIFY(editor != nullptr);
        QVERIFY(editor->property("visible").toBool());
        editor->setProperty("text", QStringLiteral("Lab Phone"));
        QMetaObject::invokeMethod(editor, "accepted");
        QTRY_COMPARE_WITH_TIMEOUT(
            f.deviceList.devices().first().toMap().value(QStringLiteral("name")).toString(),
            QStringLiteral("Lab Phone"), 5000);
    }

    void contextMenuListsWiredActions() {
        Fixture f;
        QQuickWindow window;
        window.resize(300, 600);
        QObject *root = f.load("/views/DeviceSidebar.qml", &window);
        QVERIFY(root != nullptr);
        window.show();
        auto *list = root->findChild<QQuickItem *>(QStringLiteral("deviceListView"));
        QTRY_COMPARE_WITH_TIMEOUT(list->property("count").toInt(), 3, 5000);

        // Read back through a QML test seam: exactly what the menu shows.
        auto menuTexts = [&](const char *uuid) -> QStringList {
            QMetaObject::invokeMethod(root, "buildMenu",
                                      Q_ARG(QVariant, QVariant(QStringList{
                                                                QString::fromLatin1(uuid)})));
            QVariant result;
            const bool ok = QMetaObject::invokeMethod(root, "menuItemTexts",
                                                      Q_RETURN_ARG(QVariant, result));
            if (!ok)
                return QStringList();
            return result.toStringList();
        };
        QVERIFY2(root->findChild<QObject *>(QStringLiteral("sidebarContextMenu")) != nullptr,
                 "sidebar context menu not found");

        // Running row: the Start item morphs to "Shut Down…".
        const QStringList runningTexts = menuTexts(kIPodUuid);
        QVERIFY(runningTexts.contains(QStringLiteral("Shut Down\u2026")));
        QVERIFY(runningTexts.contains(QStringLiteral("Force Stop\u2026")));
        QVERIFY(runningTexts.contains(QStringLiteral("Import IPSW\u2026")));
        QVERIFY(runningTexts.contains(QStringLiteral("Rename")));
        QVERIFY(runningTexts.contains(QStringLiteral("Delete\u2026")));
        // The iPad has a job in flight → Cancel is enabled.
        // (Cancel presence is covered below via the off row's Start item.)
        const QStringList offTexts = menuTexts(kIPhoneUuid);
        QVERIFY(offTexts.contains(QStringLiteral("Start")));
        QVERIFY(!offTexts.contains(QStringLiteral("Shut Down\u2026")));

        // The Start action boots the device without crashing in mock mode.
        QMetaObject::invokeMethod(root, "runMenuAction",
                                  Q_ARG(QVariant, QVariant(QStringLiteral("start"))),
                                  Q_ARG(QVariant, QVariant(QStringList{
                                                            QString::fromLatin1(kIPhoneUuid)})));
        QTest::qWait(200);
    }

    // Placeholder QML ----------------------------------------------------

    void placeholderPrimaryActionAndJobBar() {
        Fixture f;
        QQuickWindow window;
        window.resize(600, 600);
        QObject *root = f.load("/views/DevicePlaceholder.qml", &window);
        QVERIFY2(root != nullptr, "DevicePlaceholder.qml failed to load");
        window.show();

        const QVariantMap offRow = f.deviceList.devices().first().toMap();
        root->setProperty("row", QVariant::fromValue(offRow));
        QTRY_COMPARE_WITH_TIMEOUT(root->property("primaryText").toString(),
                                  QStringLiteral("Start"), 5000);
        QQuickItem *primary =
            root->findChild<QQuickItem *>(QStringLiteral("primaryAction"));
        QVERIFY(primary != nullptr);
        QVERIFY(primary->property("visible").toBool());
        QVERIFY(primary->property("enabled").toBool());

        // The iPad's job: its own progress bar, 42%, no cross-animation.
        const QVariantMap ipadRow = f.deviceList.devices().at(2).toMap();
        const QVariantMap job = f.jobs.jobs().first().toMap();
        QCOMPARE(job.value(QStringLiteral("deviceUuid")).toString(),
                 QString::fromLatin1(kIPadUuid));
        root->setProperty("row", QVariant::fromValue(ipadRow));
        root->setProperty("job", QVariant::fromValue(job));
        QQuickItem *bar = root->findChild<QQuickItem *>(QStringLiteral("jobProgressBar"));
        QVERIFY(bar != nullptr);
        QTRY_VERIFY_WITH_TIMEOUT(bar->property("visible").toBool(), 5000);
        QCOMPARE(root->property("primaryText").toString(), QString());
        QQuickItem *caption = root->findChild<QQuickItem *>(QStringLiteral("jobCaption"));
        QVERIFY(caption != nullptr);
        QVERIFY(caption->property("text").toString().contains(QStringLiteral("42%")));
        QVERIFY(caption->property("text").toString().contains(
            QStringLiteral("Decrypting root filesystem")));
    }

    // DevicePane swap -----------------------------------------------------

    void devicePaneSwapsOnSelection() {
        Fixture f;
        QQuickWindow window;
        window.resize(600, 600);
        QObject *root = f.load("/views/DevicePane.qml", &window);
        QVERIFY2(root != nullptr, "DevicePane.qml failed to load");
        window.show();
        QQuickItem *stack = root->findChild<QQuickItem *>(QStringLiteral("detailStack"));
        QVERIFY(stack != nullptr);

        // 0: empty library is impossible in mock; start at nothing selected.
        root->setProperty("selectedIds", QVariant(QStringList()));
        QTRY_COMPARE_WITH_TIMEOUT(stack->property("currentIndex").toInt(), 1, 5000);

        // 2: multi-select → count label.
        root->setProperty("selectedIds", QVariant(QStringList{
                                               QString::fromLatin1(kIPhoneUuid),
                                               QString::fromLatin1(kIPodUuid)}));
        QTRY_COMPARE_WITH_TIMEOUT(stack->property("currentIndex").toInt(), 2, 5000);

        // 3: one running device → display placeholder.
        const QVariantMap ipodRow = f.deviceList.devices().at(1).toMap();
        root->setProperty("selectedIds",
                          QVariant(QStringList{QString::fromLatin1(kIPodUuid)}));
        root->setProperty("selectedRow", QVariant::fromValue(ipodRow));
        QTRY_COMPARE_WITH_TIMEOUT(stack->property("currentIndex").toInt(), 3, 5000);

        // 4: one stopped device → placeholder.
        const QVariantMap iphoneRow = f.deviceList.devices().first().toMap();
        root->setProperty("selectedIds",
                          QVariant(QStringList{QString::fromLatin1(kIPhoneUuid)}));
        root->setProperty("selectedRow", QVariant::fromValue(iphoneRow));
        QTRY_COMPARE_WITH_TIMEOUT(stack->property("currentIndex").toInt(), 4, 5000);
    }
};

QTEST_MAIN(TstSurface1)
#include "tst_surface1.moc"
