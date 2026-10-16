// LTCaptureAdapter.h — mirrors ScreenRecordingSession / CapturePreferences (ABI.md §3.8).
#pragma once

#include "LTAdapterBase.h"

class LTCaptureAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(bool canTakeScreenshot READ canTakeScreenshot NOTIFY availabilityChanged)
    Q_PROPERTY(bool canStartRecording READ canStartRecording NOTIFY availabilityChanged)
    Q_PROPERTY(bool canToggleRecording READ canToggleRecording NOTIFY availabilityChanged)
    Q_PROPERTY(bool recordingSaving READ recordingSaving NOTIFY availabilityChanged)
    Q_PROPERTY(bool recordingCanStop READ recordingCanStop NOTIFY availabilityChanged)
    Q_PROPERTY(int recordingPhase READ recordingPhase NOTIFY recordingChanged)
    Q_PROPERTY(quint64 elapsedSeconds READ elapsedSeconds NOTIFY recordingChanged)
    Q_PROPERTY(QString lastCapturePath READ lastCapturePath NOTIFY captureDone)
    Q_PROPERTY(bool lastCaptureOk READ lastCaptureOk NOTIFY captureDone)
    Q_PROPERTY(int lastCaptureKind READ lastCaptureKind NOTIFY captureDone)
    Q_PROPERTY(QVariantMap prefs READ prefs NOTIFY prefsChanged)

public:
    explicit LTCaptureAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    bool canTakeScreenshot() const { return m_canTakeScreenshot; }
    bool canStartRecording() const { return m_canStartRecording; }
    bool canToggleRecording() const { return m_canToggleRecording; }
    bool recordingSaving() const { return m_recordingSaving; }
    bool recordingCanStop() const { return m_recordingCanStop; }
    int recordingPhase() const { return m_recordingPhase; }
    quint64 elapsedSeconds() const { return m_elapsedSeconds; }
    QString lastCapturePath() const { return m_lastCapturePath; }
    bool lastCaptureOk() const { return m_lastCaptureOk; }
    // kLTCaptureScreenshot (0) | kLTCaptureRecording (1).
    int lastCaptureKind() const { return m_lastCaptureKind; }
    QVariantMap prefs() const { return m_prefs; }

    Q_INVOKABLE void refreshAvailability();
    Q_INVOKABLE void screenshot();
    // Qt-side screenshot variants (the ABI's lt_capture_screenshot follows
    // CapturePreferences; copy/save-as are Qt effects on the CaptureDone
    // event + prefs snapshot, per ABI.md §3.8).
    Q_INVOKABLE void screenshotCopy();
    Q_INVOKABLE void screenshotSaveAs();
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording(bool discard);
    Q_INVOKABLE void saveRecording(const QString &path);
    Q_INVOKABLE void discardRecording();
    Q_INVOKABLE void refreshPrefs();
    Q_INVOKABLE void setPrefs(const QString &prefsJson);
    // Opens the last capture in the OS default app (QDesktopServices).
    Q_INVOKABLE void openLastCapture();
    // Native pickers for the Capture settings pane (QtWidgets; the
    // QtQuick.Dialogs QML module is not shipped on all targets). Empty on
    // cancel. "Other…" targets for the save-location / open-in-app pop-ups.
    Q_INVOKABLE QString pickFolder();
    Q_INVOKABLE QString pickApplication();

signals:
    void deviceUuidChanged();
    void availabilityChanged();
    void recordingChanged();
    void captureDone();
    void prefsChanged();
    // Two-step save handshake: Swift needs a destination; QML shows the dialog
    // and calls saveRecording()/discardRecording().
    void saveDestinationRequested(const QString &suggestedName);
    // Brief confirmation after a copy-to-clipboard screenshot (Mac: 1.5 s).
    void screenshotCopied();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    bool m_canTakeScreenshot = false;
    bool m_canStartRecording = false;
    bool m_canToggleRecording = false;
    bool m_recordingSaving = false;
    bool m_recordingCanStop = false;
    int m_recordingPhase = kLTRecordingPhaseIdle;
    void applyPendingShotEffect();

    quint64 m_elapsedSeconds = 0;
    QString m_lastCapturePath;
    // Pending Qt-side screenshot effect for the next CaptureDone.
    QString m_pendingShotEffect; // "" | "copy" | "saveAs"
    bool m_lastCaptureOk = false;
    int m_lastCaptureKind = kLTCaptureScreenshot;
    QVariantMap m_prefs;
};
