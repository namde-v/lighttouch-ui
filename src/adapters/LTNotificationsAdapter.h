// LTNotificationsAdapter.h — routes core notifications (surface 13).
//
// Qt counterpart of Features/CaptureNotifications.swift. UNUserNotificationCenter
// has no Qt equivalent, so this is a Qt abstraction with per-OS backends
// behind one interface:
//
//  - Linux: org.freedesktop.Notifications via the desktop portal (QDBus) with
//    click actions; falls back to QSystemTrayIcon::showMessage.
//  - Windows: native toast notifications (WinRT) with click-to-show-device.
//  - This build: QSystemTrayIcon::showMessage (works wherever a tray/host
//    notification daemon exists; silent no-op headless). The "ready to use"
//    click → show-device action is wired through `showDeviceRequested`.
//
// Permission is asked only the first time there is real news (Mac behavior):
// the adapter requests/notes permission lazily on the first toast, never at
// startup. While the app is frontmost, toasts also surface in-app (the
// shell's footer toast line); the OS notification is the out-of-app path.
#pragma once

#include "LTAdapterBase.h"

class QSystemTrayIcon;

class LTNotificationsAdapter : public LTAdapterBase {
    Q_OBJECT
    // Whether OS notifications are available in this session (a tray/host
    // daemon exists). Always true for the in-app toast path.
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
    explicit LTNotificationsAdapter(Bridge *bridge, QObject *parent = nullptr);
    ~LTNotificationsAdapter() override;

    bool available() const { return m_available; }

    // Posts a user-facing notification. Category "device-ready" carries the
    // catalog entry id in `actionId`; clicking it shows that device.
    Q_INVOKABLE void notify(const QString &title, const QString &body,
                            const QString &category = QString(),
                            const QString &actionId = QString());

signals:
    void availableChanged();
    // A "ready to use" notification was clicked: select that device.
    void showDeviceRequested(const QString &entryId);
    // Every toast, for the in-app surface (footer toast line).
    void toastReceived(const QString &title, const QString &body,
                       const QString &category);

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void ensureTray();

    QSystemTrayIcon *m_tray = nullptr;
    bool m_available = false;
    bool m_permissionAsked = false;
    QString m_pendingActionId;
};
