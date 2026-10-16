// tst_surface14.cpp — Surface 14 (taskbar progress) tests, offscreen.
//
// Qt counterpart of App/DockProgress.swift:
//   - kLTEventTaskbarProgress → aggregate fraction,
//   - the >= 0.5% redraw rule (smaller changes don't re-emit),
//   - clear-at-zero (fraction < 0 clears the badge),
//   - the jobs table's aggregate feeds the same property.

#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlContext>

#include "Bridge.h"
#include "adapters/LTTaskbarAdapter.h"
#include "adapters/LTJobsAdapter.h"

class TstSurface14 : public QObject {
    Q_OBJECT

    struct Fixture {
        Bridge bridge;
        LTTaskbarAdapter taskbar;
        LTJobsAdapter jobs;
        QQmlEngine engine;
        Fixture() : taskbar(&bridge), jobs(&bridge) {
            QString err;
            QVERIFY2(bridge.initialize(&err), qPrintable(err));
            QQmlContext *ctx = engine.rootContext();
            ctx->setContextProperty(QStringLiteral("bridge"), &bridge);
            ctx->setContextProperty(QStringLiteral("taskbar"), &taskbar);
            ctx->setContextProperty(QStringLiteral("jobs"), &jobs);
            jobs.refresh();
        }
    };

private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
        QCoreApplication::setOrganizationName(QStringLiteral("LightTouchTest"));
        QCoreApplication::setApplicationName(QStringLiteral("tst_surface14"));
    }

    void startsCleared() {
        Fixture f;
        QCOMPARE(f.taskbar.progress(), -1.0);
    }

    void progressEventSetsFraction() {
        Fixture f;
        QSignalSpy changed(&f.taskbar, SIGNAL(progressChanged()));
        QVERIFY(changed.isValid());
        f.bridge.mockFireTaskbarProgress(0.42);
        QTRY_COMPARE_WITH_TIMEOUT(changed.size(), 1, 5000);
        QCOMPARE(f.taskbar.progress(), 0.42);
    }

    void smallChangesDoNotRedraw() {
        Fixture f;
        QSignalSpy changed(&f.taskbar, SIGNAL(progressChanged()));
        f.bridge.mockFireTaskbarProgress(0.42);
        QTRY_COMPARE_WITH_TIMEOUT(changed.size(), 1, 5000);
        // < 0.5% change: the Mac doesn't redraw.
        f.bridge.mockFireTaskbarProgress(0.423);
        QTest::qWait(200);
        QCOMPARE(changed.size(), 1);
        QCOMPARE(f.taskbar.progress(), 0.42);
        // >= 0.5%: redraws.
        f.bridge.mockFireTaskbarProgress(0.43);
        QTRY_COMPARE_WITH_TIMEOUT(changed.size(), 2, 5000);
        QCOMPARE(f.taskbar.progress(), 0.43);
    }

    void negativeFractionClears() {
        Fixture f;
        f.bridge.mockFireTaskbarProgress(0.5);
        QTRY_COMPARE_WITH_TIMEOUT(f.taskbar.progress(), 0.5, 5000);
        f.bridge.mockFireTaskbarProgress(-1.0);
        QTRY_COMPARE_WITH_TIMEOUT(f.taskbar.progress(), -1.0, 5000);
    }

    void jobsAggregateFeedsTaskbar() {
        Fixture f;
        // The core's taskbar event carries the jobs aggregate
        // (FirmwareJob.dockProgress on the Mac); the adapter exposes it.
        f.bridge.mockFireTaskbarProgress(0.42);
        QTRY_COMPARE_WITH_TIMEOUT(f.jobs.aggregateFraction(), 0.42, 5000);
        // The shell feeds that aggregate into the taskbar adapter.
        // (The spy is created after the event, so it only sees the feed.)
        QSignalSpy changed(&f.taskbar, SIGNAL(progressChanged()));
        QVERIFY(changed.isValid());
        f.taskbar.setJobsAggregate(f.jobs.aggregateFraction());
        QCOMPARE(changed.size(), 0); // 0.42 already set by the event
        // A distinct value proves the feed path.
        f.taskbar.setJobsAggregate(0.75);
        QCOMPARE(changed.size(), 1);
        QCOMPARE(f.taskbar.progress(), 0.75);
        f.taskbar.setJobsAggregate(-1.0);
        QCOMPARE(f.taskbar.progress(), -1.0);
    }
};

QTEST_MAIN(TstSurface14)
#include "tst_surface14.moc"
