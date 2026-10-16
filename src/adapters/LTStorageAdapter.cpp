#include "LTStorageAdapter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

LTStorageAdapter::LTStorageAdapter(Bridge *bridge, QObject *parent)
    : QObject(parent), m_bridge(bridge) {}

void LTStorageAdapter::refresh() {
    if (!m_bridge || !m_bridge->isReady())
        return;
    // Storage is app-wide, not device-scoped; the device argument is
    // accepted for symmetry (the Mac's StorageUsage is also global).
    const char *raw = nullptr;
    const LTResult r = m_bridge->api().lt_storage_get(kLTInvalidDevice, &raw);
    const QString json = m_bridge->takeString(raw);
    if (r != kLTOk) {
        qWarning() << "lt_storage_get failed:" << m_bridge->lastErrorString();
        return;
    }
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "lt_storage_get: bad JSON:" << err.errorString();
        return;
    }
    m_usage = doc.object().toVariantMap();
    emit usageChanged();
}
