// LTDeviceAdapter.h — per-device session lifecycle + status (ABI.md §3.3).
// Drives menu validation, the notice banner, and boot/halt/pause commands.
#pragma once

#include "LTAdapterBase.h"

class LTDeviceAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString currentDeviceUuid READ currentDeviceUuid WRITE setCurrentDeviceUuid
                   NOTIFY currentDeviceUuidChanged)
    Q_PROPERTY(int machineState READ machineState NOTIFY statusChanged)
    Q_PROPERTY(bool running READ running NOTIFY statusChanged)
    Q_PROPERTY(bool sleeping READ sleeping NOTIFY statusChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY statusChanged)
    Q_PROPERTY(bool acceptsInput READ acceptsInput NOTIFY statusChanged)
    Q_PROPERTY(QString bootStageText READ bootStageText NOTIFY bootStageChanged)
    Q_PROPERTY(double bootFraction READ bootFraction NOTIFY bootStageChanged)
    Q_PROPERTY(bool noticeVisible READ noticeVisible NOTIFY noticeChanged)
    Q_PROPERTY(QString noticeMessage READ noticeMessage NOTIFY noticeChanged)
    Q_PROPERTY(QString noticeOperation READ noticeOperation NOTIFY noticeChanged)
    Q_PROPERTY(bool noticeOffersErase READ noticeOffersErase NOTIFY noticeChanged)
    Q_PROPERTY(int networkState READ networkState NOTIFY networkChanged)
    Q_PROPERTY(bool fingerDots READ fingerDots WRITE setFingerDots NOTIFY fingerDotsChanged)
public:
    explicit LTDeviceAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString currentDeviceUuid() const { return m_uuid; }
    void setCurrentDeviceUuid(const QString &uuid);

    int machineState() const { return m_machineState; }
    bool running() const { return m_running; }
    bool sleeping() const { return m_sleeping; }
    bool paused() const { return m_paused; }
    bool acceptsInput() const { return m_acceptsInput; }
    QString bootStageText() const { return m_bootStageText; }
    double bootFraction() const { return m_bootFraction; }
    bool noticeVisible() const { return !m_noticeMessage.isEmpty(); }
    QString noticeMessage() const { return m_noticeMessage; }
    QString noticeOperation() const { return m_noticeOperation; }
    bool noticeOffersErase() const { return m_noticeOffersErase; }
    int networkState() const { return m_networkState; }
    bool fingerDots() const { return m_fingerDots; }
    Q_INVOKABLE void setFingerDots(bool shown);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void boot();
    Q_INVOKABLE void halt();
    Q_INVOKABLE void setPaused(bool paused);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setLocked(bool locked);
    Q_INVOKABLE void resolveNotice();
    Q_INVOKABLE void dismissNotice();
    Q_INVOKABLE void setNetworkAllowed(bool allowed);
    Q_INVOKABLE QVariantMap validateMenuItem(int item, int arg = 0);
    // Device-menu input commands (surface 11; guest commands from the menu).
    Q_INVOKABLE void pressButton(int button);
    Q_INVOKABLE void shake();
    Q_INVOKABLE void setMotionPose(int pose);
    Q_INVOKABLE void rotateGuest(int degrees);
    Q_INVOKABLE void setCompassHeading(int heading);
    Q_INVOKABLE void pasteToGuest();
    Q_INVOKABLE void refreshFingerDots();

signals:
    void currentDeviceUuidChanged();
    void statusChanged();
    void bootStageChanged();
    void noticeChanged();
    void networkChanged();
    void fingerDotsChanged();
    // NetworkAccessPrompt: no saved answer and the device is starting.
    // The UI asks, saves the answer via LTSettingsAdapter, then boots again.
    void networkAccessPromptRequested();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void refreshStatus();
    void refreshNotice();
    void refreshNetwork();
    bool networkAnswerUnset();

    QString m_uuid;
    int m_machineState = kLTVMStateUnknown;
    bool m_running = false;
    bool m_sleeping = false;
    bool m_paused = false;
    bool m_acceptsInput = false;
    QString m_bootStageText;
    double m_bootFraction = -1.0;
    QString m_noticeMessage;
    QString m_noticeOperation;
    bool m_noticeOffersErase = false;
    int m_networkState = kLTNetworkAccessUnknown;
    bool m_fingerDots = false;
};
