// LTTaskbarAdapter.h — taskbar/dock progress (surface 14).
//
// Qt counterpart of App/DockProgress.swift: one aggregate bar while any
// firmware job runs (FirmwareJob.dockProgress), cleared when jobs end, and
// the system kept awake meanwhile.
//
// Backend notes (no Qt equivalent for NSApp.dockTile):
//  - Windows: ITaskbarList3::SetProgressValue on the app window (per-OS
//    backend behind this adapter's setProgress; not wired in this build).
//  - Linux: the com.canonical.Unity.LauncherEntry progress signal over DBus
//    (works on GNOME via the same interface in most docks; absent elsewhere
//    — degrades silently). Not wired in this build.
//  - Idle-sleep hold: Windows SetThreadExecutionState, Linux
//    org.freedesktop.ScreenSaver inhibit. Not wired in this build.
//
// What IS wired and tested: the aggregate fraction (kLTEventTaskbarProgress
// plus the jobs table's own aggregate), the >= 0.5% redraw rule, and the
// clear-at-zero rule — i.e. everything the OS backend consumes.
#pragma once

#include "LTAdapterBase.h"

class LTTaskbarAdapter : public LTAdapterBase {
    Q_OBJECT
    // Aggregate job fraction, or -1 when no job runs (badge cleared).
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)

public:
    explicit LTTaskbarAdapter(Bridge *bridge, QObject *parent = nullptr);

    double progress() const { return m_progress; }

    // Lets the shell feed the jobs table's aggregate (the Mac derives the
    // dock bar from FirmwareJobs, not only from the event).
    Q_INVOKABLE void setJobsAggregate(double fraction);

signals:
    void progressChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void setProgress(double fraction);

    double m_progress = -1.0;
};
