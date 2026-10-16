#include "LTNotificationsAdapter.h"

#include <QSystemTrayIcon>
#include <QIcon>

LTNotificationsAdapter::LTNotificationsAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

LTNotificationsAdapter::~LTNotificationsAdapter() = default;

void LTNotificationsAdapter::ensureTray() {
    if (m_tray)
        return;
    // The icon is required for showMessage on some platforms; the bundled
    // device art stands in (a real app icon arrives with the installer).
    m_tray = new QSystemTrayIcon(
        QIcon(QStringLiteral(":/art/device-generic.svg")), this);
    connect(m_tray, &QSystemTrayIcon::messageClicked, this,
            [this]() {
                if (!m_pendingActionId.isEmpty())
                    emit showDeviceRequested(m_pendingActionId);
                m_pendingActionId.clear();
            });
    m_available = QSystemTrayIcon::isSystemTrayAvailable();
    // Headless/offscreen: no tray, but the in-app toast path always works.
    // (Deliberately NOT calling show() — the app must not take a tray slot
    // just to post notifications.)
    emit availableChanged();
}

void LTNotificationsAdapter::notify(const QString &title, const QString &body,
                                    const QString &category,
                                    const QString &actionId) {
    // Permission only on first real news (Mac behavior): we never prompt at
    // startup; the first notify() is the ask.
    if (!m_permissionAsked) {
        m_permissionAsked = true;
        ensureTray();
    }
    emit toastReceived(title, body, category);
    if (!m_available)
        return;
    m_pendingActionId = (category == QStringLiteral("device-ready")) ? actionId
                                                                     : QString();
    m_tray->showMessage(title, body, QSystemTrayIcon::Information, 8000);
}

void LTNotificationsAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                           const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    if (kind != kLTEventToast)
        return;
    const QString title = payload.value(QStringLiteral("title")).toString();
    const QString body = payload.value(QStringLiteral("body")).toString();
    const QString category = payload.value(QStringLiteral("category")).toString();
    // "device-ready" toasts carry the entry id in the body? No: the Mac's
    // CaptureNotificationContent.ready puts the entry in userInfo. The Qt
    // bridge carries it in `category` as "device-ready:<entryId>".
    QString entryId;
    const QString prefix = QStringLiteral("device-ready:");
    if (category.startsWith(prefix))
        entryId = category.mid(prefix.size());
    notify(title, body,
           entryId.isEmpty() ? category : QStringLiteral("device-ready"),
           entryId);
}
