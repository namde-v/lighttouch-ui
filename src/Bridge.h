// Bridge.h — dlopen/dlsym binding of libLTAppCore + event re-post to the Qt UI thread.
//
// Threading (ABI.md §4):
//  - The Bridge lives on the Qt UI thread. All lt_ calls are made on the UI
//    thread unless the function is [pure] (adapters only call those too, so in
//    practice everything here is UI-thread).
//  - Swift invokes the event trampoline from arbitrary threads. The trampoline
//    deep-copies the LTEvent, releases the original's strings via
//    lt_free_event_strings, and re-posts the copy with Qt::QueuedConnection.
//    No Qt work happens on the Swift thread.
//  - Mock mode (LTM_BRIDGE_MOCK=1): no library is loaded; a set of static
//    mock_* functions backs the same Api struct, and a timer synthesizes
//    plausible events so UI work proceeds before libLTAppCore exists.
//
// Library resolution order:
//  1. $LTM_APPCORE_LIB (explicit override — also how tests point at a build)
//  2. <applicationDir>/libLTAppCore.so (LTAppCore.dll on Windows)
//  3. system search path ("libLTAppCore.so" / "LTAppCore.dll")
// QLibrary is used instead of dlopen directly so the same sources compile on
// Windows (LoadLibrary). No Linux-only APIs in this file's code paths.

#pragma once

#include <QObject>
#include <QLibrary>
#include <QTimer>
#include <QString>
#include <QVariant>

#include "lt_bridge.h"

class Bridge : public QObject {
    Q_OBJECT
public:
    explicit Bridge(QObject *parent = nullptr);
    ~Bridge() override;

    // Binds the library (or activates mock mode), checks the ABI version,
    // calls lt_init and registers the event callback. Returns false with a
    // human-readable message on any failure — never a silent half-init.
    bool initialize(QString *errorMessage = nullptr);
    void shutdown();

    bool isReady() const { return m_ready; }
    bool isMock() const { return m_mock; }

    // Every lt_ export, bound at initialize() time. Adapters call through
    // this struct; in mock mode the pointers refer to the mock_* functions.
    struct Api {
#include "lt_api_members.inc"  // generated at configure time from lt_bridge.h
    };
    const Api &api() const { return m_api; }

    // --- String helpers (ABI.md §1.3) ----------------------------------
    // Wrap a core-allocated UTF-8 string into a QString and release it with
    // lt_free_string. NULL-safe (returns null QString).
    QString takeString(const char *s);
    // lt_last_error() wrapped + freed. Only meaningful right after a < 0 return.
    QString lastErrorString();

    // Called by the C event trampoline on an arbitrary Swift thread.
    // Deep-copies e, frees e's strings, queues dispatchEvent() on the UI thread.
    void postEvent(const LTEvent *e);

    // --- Mock-mode event synthesis (no-op unless mock mode) -------------
    // These build an LTEvent with owned strings and push it through the same
    // postEvent() path the real trampoline uses, so tests exercise the full
    // copy → queued-dispatch → adapter-update chain.
    void mockFireToast(const QString &title, const QString &body,
                       const QString &category = QString());
    void mockFireDeviceListChanged();
    void mockFireDeviceStatus(lt_device_t device, int machine, bool running, bool sleeping);
    void mockFireBootStage(lt_device_t device, int stage, const QString &detail, double fraction);
    void mockFireJobProgress(const QString &entryId, double fraction);
    // Surfaces 4–9, 12–14 (mock-only event synthesis for tests/UI work).
    void mockFireTweaksChanged();
    void mockFireCarrierChanged();
    void mockFireCaptureAvailability();
    void mockFireCaptureDone(int kind, const QString &path, bool ok, const QString &message);
    void mockFireRecordingTick(quint64 elapsedSeconds);
    void mockFireRecordingPhase(int phase);
    void mockFireRecordingSaveNeeded(const QString &suggestedName);
    void mockFireAppsChanged();
    void mockFireInstallProgress(const QString &bundleId, double fraction, const QString &phase);
    void mockFireInstallDone(const QString &bundleId, bool ok, const QString &message);
    void mockFireFilesListed(quint64 requestId, const QString &entriesJson);
    void mockFireFilesProgress(quint64 transferId, double fraction, const QString &currentFile,
                               qint64 bytesDone, qint64 bytesTotal);
    void mockFireFilesDone(quint64 transferId, bool ok, bool cancelled, const QString &message);
    void mockFireFilesConnection(int state);
    void mockFireLogLine(const QString &source, const QString &text, quint64 offset);
    void mockFireLogRotated(const QString &source);
    void mockFireJobDone(const QString &entryId, bool ok, bool cancelled, const QString &message);
    void mockFireTaskbarProgress(double fraction);
    void mockFireDropAction(quint64 requestId, const QString &action, const QString &detailJson);
    void mockFireCatalogChanged();

signals:
    // Emitted on the UI thread. payload holds the active union member's
    // fields as a QVariantMap (strings as QString); unknown kinds arrive
    // with an empty payload and must be tolerated (ABI.md §5).
    void ltEventReceived(int kind, quint64 device, quint64 seq, const QVariantMap &payload);
    void readyChanged(bool ready);

private:
    void dispatchEvent(LTEvent *copy);  // UI thread: map payload, emit, free
    bool resolveLibraryPath(QString &pathOut, QString *errorMessage);
    bool bindApi(QString *errorMessage);
    void fillMockApi();
    void startMockEventLoop();

    bool m_ready = false;
    bool m_mock = false;
    QLibrary m_lib;
    Api m_api{};
    QTimer *m_mockTimer = nullptr;
    QTimer *m_mockFrameTimer = nullptr;
    int m_mockTick = 0;

public:
    // Synthetic display link (mock-only).
    void mockStartDisplayLink(lt_device_t device);
    void mockStopDisplayLink(lt_device_t device = 0);

private:
};
