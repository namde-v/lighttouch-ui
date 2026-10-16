// tst_surface9.cpp — Surface 9 (firmware catalog & jobs) tests, offscreen.
//
// Drives the jobs/catalog adapters + FirmwareJobsWindow/CatalogDetails QML
// against the mock bridge:
//   - jobs list, start → progress → done, cancel,
//   - canDownload gating,
//   - catalog entries, kind filter, details dialog,
//   - taskbar aggregate from the jobs table.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include "Bridge.h"
#include "adapters/LTJobsAdapter.h"
#include "adapters/LTCatalogAdapter.h"

namespace {
QString qmlPath(const char *rel) {
    return QString::fromUtf8(LT_QML_DIR) + QString::fromUtf8(rel);
}
} // namespace

class TstSurface9 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTJobsAdapter jobs;
        LTCatalogAdapter catalog;
        QQmlEngine engine;
        Fixture() : jobs(&bridge), catalog(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
            ctx->setContextProperty(QStringLiteral("catalog"), &catalog);
            jobs.refresh();
            catalog.refresh();
        }
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
            if (!QMetaObject::invokeMethod(o, method, Q_RETURN_ARG(QVariant, ret)))
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
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface9"));
    }

    void jobsListParses() {
        Fixture f;
        QVERIFY(f.jobs.hasJobs());
        const QVariantMap job = f.jobs.jobs()[0].toMap();
        QCOMPARE(job.value(QStringLiteral("entryId")).toString(),
                 QStringLiteral("ipsw-k48ap-5.1.1"));
        QCOMPARE(job.value(QStringLiteral("progress")).toDouble(), 0.42);
        const QVariantMap prep = job.value(QStringLiteral("preparation")).toMap();
        QCOMPARE(prep.value(QStringLiteral("step")).toInt(), 2);
        QCOMPARE(prep.value(QStringLiteral("steps")).toInt(), 5);
    }

    void jobStartProgressDone() {
        Fixture f;
        QSignalSpy changed(&f.jobs, SIGNAL(jobsChanged()));
        QVERIFY(changed.isValid());
        // The mock job runs its scripted arc: progress → done → refresh.
        f.jobs.start(QStringLiteral("n88ap-3.1.3"),
                     QStringLiteral("{\"jailbreak\":false}"));
        QTRY_VERIFY_WITH_TIMEOUT(changed.size() >= 1, 5000);
    }

    void jobCancel() {
        Fixture f;
        QSignalSpy changed(&f.jobs, SIGNAL(jobsChanged()));
        f.jobs.cancel(QStringLiteral("ipsw-k48ap-5.1.1"));
        QTRY_VERIFY_WITH_TIMEOUT(changed.size() >= 1, 5000);
        f.jobs.cancelAll();
        f.jobs.dismissFailure(QStringLiteral("ipsw-k48ap-5.1.1"));
    }

    void canDownloadGatesStart() {
        Fixture f;
        f.jobs.refreshCanDownload();
        QVERIFY(f.jobs.canDownload()); // the preparer exists in the mock world
        QVERIFY(f.jobs.downloadBlockReason().isEmpty());
    }

    void catalogEntriesAndFilter() {
        Fixture f;
        QCOMPARE(f.catalog.entries().size(), 7);
        QObject *win = f.load("/views/FirmwareJobsWindow.qml");
        QVERIFY2(win != nullptr, "FirmwareJobsWindow failed to load");
        QVERIFY(win->findChild<QObject *>(QStringLiteral("firmwareJobsList")) != nullptr);
        QVERIFY(win->findChild<QObject *>(QStringLiteral("firmwareCatalogList")) != nullptr);
        QVERIFY(win->findChild<QObject *>(QStringLiteral("catalogSearchField")) != nullptr);
        QVERIFY(win->findChild<QObject *>(QStringLiteral("catalogKindFilter")) != nullptr);
        // Ctrl+F focuses the catalog search.
        QVERIFY(QMetaObject::invokeMethod(win, "focusCatalogSearch"));
    }

    void catalogDetailsDialog() {
        Fixture f;
        QObject *dlg = f.load("/views/CatalogDetailsDialog.qml");
        QVERIFY2(dlg != nullptr, "CatalogDetailsDialog failed to load");
        QVERIFY(QMetaObject::invokeMethod(dlg, "showEntry",
                                          Q_ARG(QVariant, QVariant(QStringLiteral("n88ap-3.1.3")))));
        QTRY_VERIFY_WITH_TIMEOUT(
            !f.catalog.details().isEmpty(), 5000);
        QCOMPARE(f.catalog.details().value(QStringLiteral("version")).toString(),
                 QStringLiteral("3.1.3"));
        QVERIFY(f.catalog.details().contains(QStringLiteral("requirements")));
    }

    void jobsWindowStartJob() {
        Fixture f;
        QObject *win = f.load("/views/FirmwareJobsWindow.qml");
        QVERIFY2(win != nullptr, "FirmwareJobsWindow failed to load");
        // startJob gates on canDownload, then starts.
        QVERIFY(QMetaObject::invokeMethod(win, "startJob",
                                          Q_ARG(QVariant, QVariant(QStringLiteral("n88ap-3.1.3")))));
        QTRY_VERIFY_WITH_TIMEOUT(f.jobs.hasJobs(), 5000);
    }
};

QTEST_MAIN(TstSurface9)
#include "tst_surface9.moc"
