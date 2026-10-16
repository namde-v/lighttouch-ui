#include "LTDeviceListAdapter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>

LTDeviceListAdapter::LTDeviceListAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTDeviceListAdapter::refresh() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_device_list(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_device_list"))
        return;
    const QVariantList devices = jsonArray(json, "lt_device_list");
    if (devices != m_devices) {
        m_devices = devices;
        emit devicesChanged();
    }
    refreshBusy();
}

void LTDeviceListAdapter::refreshBusy() {
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_storage_busy(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_storage_busy"))
        return;
    const QVariantMap busy = jsonObject(json, "lt_storage_busy");
    if (busy != m_busy) {
        m_busy = busy;
        emit busyChanged();
    }
}

void LTDeviceListAdapter::createDevice(const QString &entryId, const QString &name) {
    if (!ready())
        return;
    lt_device_t handle = kLTInvalidDevice;
    check(bridge()->api().lt_device_create(entryId.toUtf8().constData(),
                                           name.toUtf8().constData(), &handle),
          "lt_device_create");
    // The row appears via kLTEventDeviceListChanged; no local mutation.
}

void LTDeviceListAdapter::deleteDevice(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_delete(d), "lt_device_delete");
}

void LTDeviceListAdapter::renameDevice(const QString &uuid, const QString &name) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_rename(d, name.toUtf8().constData()),
              "lt_device_rename");
}

void LTDeviceListAdapter::duplicateDevice(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d == kLTInvalidDevice)
        return;
    lt_device_t out = kLTInvalidDevice;
    check(bridge()->api().lt_device_duplicate(d, &out), "lt_device_duplicate");
}

void LTDeviceListAdapter::eraseDevice(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_erase(d), "lt_device_erase");
}

void LTDeviceListAdapter::bootDevice(const QString &uuid) {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_settings_get_string(
        "networkAccess.answer", &raw);
    const QString answer = bridge()->takeString(raw);
    if (r == kLTOk && answer.isEmpty()) {
        QString name;
        for (const QVariant &row : m_devices) {
            const QVariantMap m = row.toMap();
            if (m.value(QStringLiteral("id")).toString() == uuid) {
                name = m.value(QStringLiteral("name")).toString();
                break;
            }
        }
        emit networkAccessPromptRequested(uuid, name);
        return;
    }
    bootDeviceConfirmed(uuid);
}

void LTDeviceListAdapter::bootDeviceConfirmed(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_boot(d, "{}"), "lt_device_boot");
}

void LTDeviceListAdapter::haltDevice(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_halt(d), "lt_device_halt");
}

void LTDeviceListAdapter::resetDevice(const QString &uuid) {
    const lt_device_t d = deviceHandle(uuid);
    if (d != kLTInvalidDevice)
        check(bridge()->api().lt_device_reset(d), "lt_device_reset");
}

int LTDeviceListAdapter::classifyDropUrl(const QString &url) {
    if (!ready())
        return -1;
    const QString local = QUrl(url).toLocalFile();
    if (local.isEmpty())
        return -1;
    int32_t out = -1;
    if (!check(bridge()->api().lt_drop_classify(local.toUtf8().constData(), &out),
               "lt_drop_classify"))
        return -1;
    return out;
}

quint64 LTDeviceListAdapter::routeDrop(const QStringList &urls, const QString &target) {
    if (!ready() || urls.isEmpty())
        return 0;
    QStringList local;
    for (const QString &u : urls) {
        const QString f = QUrl(u).toLocalFile();
        if (!f.isEmpty())
            local << f;
    }
    if (local.isEmpty())
        return 0;
    const QByteArray pathsJson =
        QJsonDocument(QJsonArray::fromStringList(local)).toJson(QJsonDocument::Compact);
    uint64_t requestId = 0;
    if (!check(bridge()->api().lt_drop_route(pathsJson.constData(),
                                             target.toUtf8().constData(), &requestId),
               "lt_drop_route"))
        return 0;
    return requestId;
}

void LTDeviceListAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                        const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    switch (kind) {
    case kLTEventDeviceListChanged:
    case kLTEventDeviceRowChanged:
        refresh();
        break;
    case kLTEventStorageBusyChanged:
        refreshBusy();
        break;
    case kLTEventDropAction:
        emit dropRouted(payload.value(QStringLiteral("requestId")).toULongLong(),
                        payload.value(QStringLiteral("action")).toString(),
                        payload.value(QStringLiteral("detailJson")).toString());
        break;
    default:
        break;
    }
}
