#include "LTDebugPortAdapter.h"

#include <QApplication>
#include <QClipboard>

LTDebugPortAdapter::LTDebugPortAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTDebugPortAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refresh();
}

void LTDebugPortAdapter::refresh() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_debug_port_get(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_debug_port_get"))
        return;
    m_state = jsonObject(json, "lt_debug_port_get");
    emit stateChanged();
}

void LTDebugPortAdapter::setEnabled(bool enabled) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const QString json = QStringLiteral("{\"enabled\":%1}")
                             .arg(enabled ? QStringLiteral("true")
                                          : QStringLiteral("false"));
    if (check(bridge()->api().lt_debug_port_set(currentDevice(), json.toUtf8().constData()),
              "lt_debug_port_set"))
        refresh();
}

void LTDebugPortAdapter::copyCommand(const QString &command) {
    QClipboard *clip = QApplication::clipboard();
    if (clip)
        clip->setText(command);
}

void LTDebugPortAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                       const QVariantMap &payload) {
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    Q_UNUSED(device);
    // No debug-port events: the port is per-boot state, read on open.
}
