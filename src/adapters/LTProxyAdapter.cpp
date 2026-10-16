#include "LTProxyAdapter.h"

LTProxyAdapter::LTProxyAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTProxyAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refresh();
}

void LTProxyAdapter::refresh() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_proxy_get(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_proxy_get"))
        return;
    m_config = jsonObject(json, "lt_proxy_get");
    emit configChanged();
}

void LTProxyAdapter::apply(const QString &configJson) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    if (check(bridge()->api().lt_proxy_set(currentDevice(), configJson.toUtf8().constData()),
              "lt_proxy_set"))
        refresh();
}

void LTProxyAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                   const QVariantMap &payload) {
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    Q_UNUSED(device);
    // No proxy-specific events: the settings are the device's, read on open.
}
