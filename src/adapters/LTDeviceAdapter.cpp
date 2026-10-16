#include "LTDeviceAdapter.h"

#include <QClipboard>
#include <QGuiApplication>

LTDeviceAdapter::LTDeviceAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTDeviceAdapter::setCurrentDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit currentDeviceUuidChanged();
    refresh();
}

void LTDeviceAdapter::refresh() {
    refreshStatus();
    refreshNotice();
    refreshNetwork();
    refreshFingerDots();
}

void LTDeviceAdapter::refreshStatus() {
    if (!ready() || currentDevice() == kLTInvalidDevice) {
        // No device: clear the last device's state (menu validation reads it).
        m_machineState = kLTVMStateUnknown;
        m_running = m_sleeping = m_paused = m_acceptsInput = false;
        emit statusChanged();
        return;
    }
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_device_status(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_device_status"))
        return;
    const QVariantMap s = jsonObject(json, "lt_device_status");
    m_machineState = s.value(QStringLiteral("machine"), kLTVMStateUnknown).toInt();
    m_running = s.value(QStringLiteral("running")).toBool();
    m_sleeping = s.value(QStringLiteral("sleeping")).toBool();
    m_paused = s.value(QStringLiteral("paused")).toBool();
    m_acceptsInput = s.value(QStringLiteral("acceptsInput")).toBool();
    emit statusChanged();
}

void LTDeviceAdapter::refreshNotice() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_notice_get(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_notice_get"))
        return;
    const QVariantMap n = jsonObject(json, "lt_notice_get");
    m_noticeMessage = n.value(QStringLiteral("message")).toString();
    m_noticeOperation = n.value(QStringLiteral("operation")).toString();
    m_noticeOffersErase = n.value(QStringLiteral("offersErase")).toBool();
    emit noticeChanged();
}

void LTDeviceAdapter::refreshNetwork() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_network_status(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_network_status"))
        return;
    const QVariantMap n = jsonObject(json, "lt_network_status");
    m_networkState = n.value(QStringLiteral("state"), kLTNetworkAccessUnknown).toInt();
    emit networkChanged();
}

bool LTDeviceAdapter::networkAnswerUnset() {
    if (!ready())
        return false;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_settings_get_string(
        "networkAccess.answer", &raw);
    const QString answer = bridge()->takeString(raw);
    if (r != kLTOk)
        return false;
    return answer.isEmpty();
}

void LTDeviceAdapter::boot() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    // NetworkAccessPrompt (Mac): first start asks; the answer is saved and
    // the boot goes ahead on the next call.
    if (networkAnswerUnset()) {
        emit networkAccessPromptRequested();
        return;
    }
    check(bridge()->api().lt_device_boot(currentDevice(), "{}"), "lt_device_boot");
}

void LTDeviceAdapter::halt() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_device_halt(currentDevice()), "lt_device_halt");
}

void LTDeviceAdapter::setPaused(bool paused) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_device_pause(currentDevice(), paused ? 1 : 0),
          "lt_device_pause");
}

void LTDeviceAdapter::reset() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_device_reset(currentDevice()), "lt_device_reset");
}

void LTDeviceAdapter::setLocked(bool locked) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_device_lock(currentDevice(), locked ? 1 : 0),
          "lt_device_lock");
}

void LTDeviceAdapter::resolveNotice() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_notice_resolve(currentDevice(),
                                            m_noticeOperation.toUtf8().constData()),
          "lt_notice_resolve");
}

void LTDeviceAdapter::dismissNotice() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_notice_dismiss(currentDevice()), "lt_notice_dismiss");
}

void LTDeviceAdapter::setNetworkAllowed(bool allowed) {
    if (!ready())
        return;
    check(bridge()->api().lt_network_set_allowed(allowed ? 1 : 0),
          "lt_network_set_allowed");
}

QVariantMap LTDeviceAdapter::validateMenuItem(int item, int arg) {    if (!ready() || currentDevice() == kLTInvalidDevice)
        return {};
    const char *raw = nullptr;
    const LTResult r =
        bridge()->api().lt_menu_validate(currentDevice(), item, arg, &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_menu_validate"))
        return {};
    return jsonObject(json, "lt_menu_validate");
}

void LTDeviceAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                    const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventDeviceStatusChanged:
        refreshStatus();
        break;
    case kLTEventBootStage:
        m_bootStageText = payload.value(QStringLiteral("detail")).toString();
        m_bootFraction = payload.value(QStringLiteral("fraction"), -1.0).toDouble();
        emit bootStageChanged();
        break;
    case kLTEventNoticeChanged:
        // Event carries the full payload, but re-query for one source of truth.
        refreshNotice();
        break;
    case kLTEventNetworkAccessChanged:
        refreshNetwork();
        break;
    case kLTEventFingerDotsChanged:
        // The window's finger-dots toggle changed elsewhere; re-query.
        refreshFingerDots();
        break;
    default:
        break;
    }
}

void LTDeviceAdapter::pressButton(int button) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_button(currentDevice(), button), "lt_input_button");
}

void LTDeviceAdapter::shake() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_shake(currentDevice()), "lt_input_shake");
}

void LTDeviceAdapter::setMotionPose(int pose) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_motion_pose(currentDevice(), pose), "lt_input_motion_pose");
}

void LTDeviceAdapter::rotateGuest(int degrees) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_rotate(currentDevice(), degrees), "lt_input_rotate");
}

void LTDeviceAdapter::setCompassHeading(int heading) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_input_compass(currentDevice(), heading), "lt_input_compass");
}

void LTDeviceAdapter::pasteToGuest() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const QString text = QGuiApplication::clipboard()->text();
    if (text.isEmpty())
        return;
    check(bridge()->api().lt_input_text(currentDevice(), text.toUtf8().constData(), 0),
          "lt_input_text");
}

void LTDeviceAdapter::setFingerDots(bool shown) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    if (check(bridge()->api().lt_display_set_finger_dots(currentDevice(), shown ? 1 : 0),
              "lt_display_set_finger_dots")) {
        m_fingerDots = shown;
        emit fingerDotsChanged();
    }
}

void LTDeviceAdapter::refreshFingerDots() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    int32_t out = 0;
    if (check(bridge()->api().lt_display_get_finger_dots(currentDevice(), &out),
              "lt_display_get_finger_dots")) {
        const bool shown = out != 0;
        if (shown != m_fingerDots) {
            m_fingerDots = shown;
            emit fingerDotsChanged();
        }
    }
}
