// LTSettingsAdapter.h — Swift-owned scalar prefs + misc app functions (ABI.md §3.11).
#pragma once

#include "LTAdapterBase.h"

class LTSettingsAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    Q_PROPERTY(QVariantMap aboutCredits READ aboutCredits NOTIFY aboutCreditsChanged)
    // NetworkAccessPreference (kLTSettingsKeyNetworkAccessAnswer): "always",
    // "never", or "" when the device should ask at start.
    Q_PROPERTY(QString networkAccessAnswer READ networkAccessAnswer
               WRITE setNetworkAccessAnswer NOTIFY networkAccessAnswerChanged)
    // Qt-native prefs (were @AppStorage on the Mac; no ABI key).
    Q_PROPERTY(bool vibrationSound READ vibrationSound WRITE setVibrationSound
               NOTIFY vibrationSoundChanged)
    Q_PROPERTY(bool refresh120Hz READ refresh120Hz WRITE setRefresh120Hz
               NOTIFY refresh120HzChanged)
    Q_PROPERTY(bool addDeviceShowsExperimental READ addDeviceShowsExperimental
               WRITE setAddDeviceShowsExperimental
               NOTIFY addDeviceShowsExperimentalChanged)
    // Capture mode (surface 8): screen-only vs canvas-with-bezel screenshots.
    // The Mac's "captureMode" UserDefaults (1 = screen only).
    Q_PROPERTY(bool captureScreenOnly READ captureScreenOnly WRITE setCaptureScreenOnly
               NOTIFY captureScreenOnlyChanged)

public:
    explicit LTSettingsAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString version() const { return m_version; }
    QVariantMap aboutCredits() const { return m_aboutCredits; }
    QString networkAccessAnswer() const { return m_networkAccessAnswer; }
    void setNetworkAccessAnswer(const QString &answer);
    bool vibrationSound() const { return m_vibrationSound; }
    void setVibrationSound(bool on);
    bool refresh120Hz() const { return m_refresh120Hz; }
    void setRefresh120Hz(bool on);
    bool addDeviceShowsExperimental() const { return m_addDeviceShowsExperimental; }
    void setAddDeviceShowsExperimental(bool on);
    bool captureScreenOnly() const { return m_captureScreenOnly; }
    void setCaptureScreenOnly(bool on);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString getString(const QString &key);
    Q_INVOKABLE void setString(const QString &key, const QString &value);
    Q_INVOKABLE QVariantList helpTopics();
    Q_INVOKABLE QString diagnosticsExport();
    Q_INVOKABLE void quitRequest();
    // "Copy Bug Report Info" (Help menu): version + pins on the clipboard.
    Q_INVOKABLE void copyBugReportInfo();
    // Main-window geometry persistence (WindowRestorationPolicy: frame only,
    // never contents).
    Q_INVOKABLE void saveWindowGeometry(int x, int y, int width, int height);
    Q_INVOKABLE QVariantMap windowGeometry() const;
    // Console split height persistence (ConsoleSplitLayout's autosave):
    // -1 = never saved, 0 = collapsed.
    Q_INVOKABLE void setConsoleHeight(int height);
    Q_INVOKABLE int consoleHeight() const;
    // Bezel mode preference (surface 3: "2d" | "off"), app-wide, remembered.
    Q_INVOKABLE QString bezelMode() const;
    Q_INVOKABLE void setBezelMode(const QString &mode);

signals:
    void versionChanged();
    void aboutCreditsChanged();
    void networkAccessAnswerChanged();
    void vibrationSoundChanged();
    void refresh120HzChanged();
    void addDeviceShowsExperimentalChanged();
    void captureScreenOnlyChanged();
    // Emitted when the core finished graceful shutdown (kLTEventQuitReady):
    // the app may now exit.
    void quitReady();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void loadLocalPrefs();

    QString m_version;
    QVariantMap m_aboutCredits;
    QString m_networkAccessAnswer;
    bool m_vibrationSound = true;
    bool m_refresh120Hz = false;
    bool m_addDeviceShowsExperimental = false;
    bool m_captureScreenOnly = false;
};
