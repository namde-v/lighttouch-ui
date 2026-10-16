#include "LTCarrierAdapter.h"

LTCarrierAdapter::LTCarrierAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTCarrierAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refresh();
}

void LTCarrierAdapter::setVisible(bool v) {
    if (v == m_visible)
        return;
    m_visible = v;
    emit visibleChanged();
    if (ready() && currentDevice() != kLTInvalidDevice)
        check(bridge()->api().lt_carrier_set_visible(currentDevice(), v ? 1 : 0),
              "lt_carrier_set_visible");
}

void LTCarrierAdapter::refresh() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_carrier_get(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_carrier_get"))
        return;
    const QVariantMap state = jsonObject(json, "lt_carrier_get");
    if (state != m_state) {
        m_state = state;
        emit stateChanged();
    }
}

void LTCarrierAdapter::setField(const QString &field, const QString &value) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_set_field(currentDevice(),
                                               field.toUtf8().constData(),
                                               value.toUtf8().constData()),
          "lt_carrier_set_field");
}

void LTCarrierAdapter::applyNetwork() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_apply_network(currentDevice()),
          "lt_carrier_apply_network");
}

void LTCarrierAdapter::setRegistered(bool registered) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_set_registered(currentDevice(), registered ? 1 : 0),
          "lt_carrier_set_registered");
}

void LTCarrierAdapter::setSimPresent(bool present) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_set_sim(currentDevice(), present ? 1 : 0),
          "lt_carrier_set_sim");
}

void LTCarrierAdapter::setBars(int bars) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_set_bars(currentDevice(), bars),
          "lt_carrier_set_bars");
}

void LTCarrierAdapter::ring() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_ring(currentDevice()), "lt_carrier_ring");
}

void LTCarrierAdapter::answer() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_answer(currentDevice()), "lt_carrier_answer");
}

void LTCarrierAdapter::hangUp() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_hangup(currentDevice()), "lt_carrier_hangup");
}

void LTCarrierAdapter::sendSms() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_send_sms(currentDevice()), "lt_carrier_send_sms");
}

void LTCarrierAdapter::applyLocation() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_apply_location(currentDevice()),
          "lt_carrier_apply_location");
}

void LTCarrierAdapter::setLocation(double lat, double lon) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_set_location(currentDevice(), lat, lon),
          "lt_carrier_set_location");
}

void LTCarrierAdapter::toggleWalk() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_toggle_walk(currentDevice()),
          "lt_carrier_toggle_walk");
}

void LTCarrierAdapter::notice(const QString &text) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_carrier_notice(currentDevice(), text.toUtf8().constData()),
          "lt_carrier_notice");
}

void LTCarrierAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                    const QVariantMap &payload) {
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    if (!eventForMe(device))
        return;
    if (kind == kLTEventCarrierChanged)
        refresh();
}
