// tst_adapters.cpp — adapter unit tests against the mock bridge.
//
// Verifies the shell's core contract: queries populate Q_PROPERTYs, and
// events flowing through Bridge::ltEventReceived update those properties
// (event → Q_PROPERTY round-trip), with no business logic in the adapters.

#include <QtTest>
#include <QSignalSpy>

#include "Bridge.h"
#include "adapters/LTDeviceListAdapter.h"
#include "adapters/LTDeviceAdapter.h"
#include "adapters/LTTweaksAdapter.h"
#include "adapters/LTCaptureAdapter.h"
#include "adapters/LTJobsAdapter.h"
#include "adapters/LTSettingsAdapter.h"

namespace {
const char *kIPodUuid = "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7";  // running
const char *kIPhoneUuid = "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617"; // off
}

class TstAdapters : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        qputenv("LTM_BRIDGE_MOCK", "1");
        qputenv("LTM_BRIDGE_MOCK_QUIET", "1");
        qunsetenv("LTM_APPCORE_LIB");
    }

    void deviceListPopulates() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTDeviceListAdapter list(&b);

        QSignalSpy spy(&list, &LTDeviceListAdapter::devicesChanged);
        list.refresh();
        QCOMPARE(spy.size(), 1);
        QCOMPARE(list.count(), 3);
        QVERIFY(list.hasDevices());

        const QVariantMap first = list.devices().first().toMap();
        QCOMPARE(first.value(QStringLiteral("name")).toString(),
                 QStringLiteral("iPhone 3GS"));
        QCOMPARE(first.value(QStringLiteral("id")).toString(),
                 QString::fromLatin1(kIPhoneUuid));
        QCOMPARE(first.value(QStringLiteral("status")).toInt(), kLTDeviceRowStateOff);
        b.shutdown();
    }

    void deviceListEventRoundTrip() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTDeviceListAdapter list(&b);

        // kLTEventDeviceListChanged → adapter re-queries → devicesChanged.
        // (No prior refresh(): the adapter starts empty, so the event-driven
        // refresh is guaranteed to change the property and emit.)
        QSignalSpy spy(&list, &LTDeviceListAdapter::devicesChanged);
        b.mockFireDeviceListChanged();
        QVERIFY2(spy.wait(3000), "devicesChanged never emitted");
        QCOMPARE(list.count(), 3);
        b.shutdown();
    }

    void deviceAdapterStatusAndEvents() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTDeviceAdapter dev(&b);

        dev.setCurrentDeviceUuid(QString::fromLatin1(kIPodUuid));
        QVERIFY(dev.running());
        QCOMPARE(dev.machineState(), kLTVMStateRunning);
        QVERIFY(dev.acceptsInput());

        dev.setCurrentDeviceUuid(QString::fromLatin1(kIPhoneUuid));
        QVERIFY(!dev.running());
        QCOMPARE(dev.machineState(), kLTVMStateOff);

        // Event for OUR device (handle 1 = iPhone) triggers a re-query…
        QSignalSpy spy(&dev, &LTDeviceAdapter::statusChanged);
        b.mockFireDeviceStatus(1, kLTVMStateRunning, true, false);
        QVERIFY2(spy.wait(3000), "statusChanged never emitted");
        // …and the properties reflect the re-queried snapshot. The event is
        // only a hint (ABI.md §4.3); the mock's query truth for handle 1
        // is "off", so the adapter converges there.
        QVERIFY(!dev.running());
        QCOMPARE(dev.machineState(), kLTVMStateOff);

        // …while an event for another device leaves this adapter alone.
        dev.setCurrentDeviceUuid(QString::fromLatin1(kIPodUuid));  // handle 2
        QVERIFY(dev.running());
        spy.clear();
        b.mockFireDeviceStatus(3, kLTVMStateStarting, false, false);
        QTest::qWait(300);
        QCOMPARE(spy.size(), 0);
        QVERIFY(dev.running());
        b.shutdown();
    }

    void bootStageEventUpdatesProperties() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTDeviceAdapter dev(&b);
        dev.setCurrentDeviceUuid(QString::fromLatin1(kIPhoneUuid));

        QSignalSpy spy(&dev, &LTDeviceAdapter::bootStageChanged);
        b.mockFireBootStage(1, kLTBootStageKernel, QStringLiteral("Loading kernel…"), 0.42);
        QVERIFY2(spy.wait(3000), "bootStageChanged never emitted");
        QCOMPARE(dev.bootStageText(), QStringLiteral("Loading kernel…"));
        QCOMPARE(dev.bootFraction(), 0.42);
        b.shutdown();
    }

    void tweaksQueryParses() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTTweaksAdapter tweaks(&b);
        tweaks.setDeviceUuid(QString::fromLatin1(kIPodUuid));
        // Hand-written mock (MOCK-ONLY): three tweaks with sections.
        QCOMPARE(tweaks.tweaks().size(), 3);
        QCOMPARE(tweaks.tweaks()[0].toMap().value(QStringLiteral("id")).toString(),
                 QStringLiteral("fingerDots"));
        QVERIFY(tweaks.running());
        b.shutdown();
    }

    void jobsProgressEventPatchesInPlace() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTJobsAdapter jobs(&b);
        // The mock ships one prepare job (iPad) at 42% so the placeholder's
        // per-job progress bar has something to bind.
        jobs.refresh();
        QVERIFY(jobs.hasJobs());
        QCOMPARE(jobs.jobs().first().toMap().value(QStringLiteral("entryId")).toString(),
                 QStringLiteral("ipsw-k48ap-5.1.1"));
        QCOMPARE(jobs.jobs().first().toMap().value(QStringLiteral("progress")).toDouble(), 0.42);

        // A progress event patches the matching job in place (no full
        // refresh, no cross-animation between rows).
        QSignalSpy spy(&jobs, &LTJobsAdapter::jobsChanged);
        b.mockFireJobProgress(QStringLiteral("ipsw-k48ap-5.1.1"), 0.9);
        QVERIFY2(spy.wait(3000), "jobsChanged never emitted");
        QCOMPARE(jobs.jobs().first().toMap().value(QStringLiteral("progress")).toDouble(), 0.9);

        // Taskbar aggregate event updates independently of the job list.
        QSignalSpy aggSpy(&jobs, &LTJobsAdapter::aggregateChanged);
        LTEvent e{};
        e.kind = kLTEventTaskbarProgress;
        e.u.taskbarProgress.fraction = 0.5;
        b.postEvent(&e);
        QVERIFY2(aggSpy.wait(3000), "aggregateChanged never emitted");
        QCOMPARE(jobs.aggregateFraction(), 0.5);
        b.shutdown();
    }

    void captureAvailabilityQuery() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTCaptureAdapter capture(&b);
        capture.setDeviceUuid(QString::fromLatin1(kIPodUuid));
        capture.refreshAvailability();  // the iPod mock row runs: can capture
        QVERIFY(capture.canTakeScreenshot());
        QVERIFY(capture.canStartRecording());
        b.shutdown();
    }

    void settingsVersion() {
        Bridge b;
        QString err;
        QVERIFY(b.initialize(&err));
        LTSettingsAdapter settings(&b);

        QSignalSpy spy(&settings, &LTSettingsAdapter::versionChanged);
        settings.refresh();
        QVERIFY(spy.size() >= 1);
        QCOMPARE(settings.version(), QStringLiteral("0.0.0-mock"));
        b.shutdown();
    }
};

QTEST_MAIN(TstAdapters)
#include "tst_adapters.moc"
