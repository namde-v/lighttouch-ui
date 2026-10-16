#include "LTTweaksAdapter.h"

LTTweaksAdapter::LTTweaksAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTTweaksAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refresh();
}

void LTTweaksAdapter::refresh() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_tweaks_get(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_tweaks_get"))
        return;
    const QVariantMap root = jsonObject(json, "lt_tweaks_get");
    m_tweaks = root.value(QStringLiteral("tweaks")).toList();
    m_running = root.value(QStringLiteral("running")).toBool();
    m_message = root.value(QStringLiteral("message")).toString();
    const QVariantList hidden = root.value(QStringLiteral("hiddenApps")).toList();
    if (hidden != m_hiddenApps) {
        m_hiddenApps = hidden;
        emit hiddenAppsChanged();
    }
    emit tweaksChanged();
}

void LTTweaksAdapter::setTweak(const QString &tweakId, bool on) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_tweaks_set(currentDevice(), tweakId.toUtf8().constData(),
                                        on ? 1 : 0),
          "lt_tweaks_set");
}

void LTTweaksAdapter::setColor(int color) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_tweaks_set_color(currentDevice(), color),
          "lt_tweaks_set_color");
}

void LTTweaksAdapter::setClock(double epoch) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_tweaks_set_clock(currentDevice(), epoch),
          "lt_tweaks_set_clock");
}

void LTTweaksAdapter::listHiddenApps() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_tweaks_list_hidden_apps(currentDevice()),
          "lt_tweaks_list_hidden_apps");
}

void LTTweaksAdapter::openApp(const QString &bundleId) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_tweaks_open_app(currentDevice(),
                                             bundleId.toUtf8().constData()),
          "lt_tweaks_open_app");
}

void LTTweaksAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                    const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventTweaksChanged:
        refresh();
        break;
    case kLTEventHiddenAppsListed: {
        const QString appsJson = payload.value(QStringLiteral("appsJson")).toString();
        const QVariantList apps = jsonArray(appsJson, "kLTEventHiddenAppsListed");
        if (apps != m_hiddenApps) {
            m_hiddenApps = apps;
            emit hiddenAppsChanged();
        }
        break;
    }
    default:
        break;
    }
}
