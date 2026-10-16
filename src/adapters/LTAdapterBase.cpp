#include "LTAdapterBase.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

LTAdapterBase::LTAdapterBase(Bridge *bridge, QObject *parent)
    : QObject(parent), m_bridge(bridge) {
    connect(bridge, &Bridge::ltEventReceived, this, &LTAdapterBase::onBridgeEvent);
}

bool LTAdapterBase::ready() const {
    return m_bridge && m_bridge->isReady();
}

bool LTAdapterBase::check(LTResult r, const char *what) const {
    if (r == kLTOk)
        return true;
    qWarning().noquote() << "LTAdapter" << what << "failed:"
                         << r << m_bridge->lastErrorString();
    return false;
}

QVariantList LTAdapterBase::jsonArray(const QString &json, const char *what) const {
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        qWarning() << "LTAdapter" << what << ": expected JSON array:" << err.errorString();
        return {};
    }
    return doc.array().toVariantList();
}

QVariantMap LTAdapterBase::jsonObject(const QString &json, const char *what) const {
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "LTAdapter" << what << ": expected JSON object:" << err.errorString();
        return {};
    }
    return doc.object().toVariantMap();
}

lt_device_t LTAdapterBase::deviceHandle(const QString &uuid) const {
    if (!ready() || uuid.isEmpty())
        return kLTInvalidDevice;
    lt_device_t h = kLTInvalidDevice;
    const QByteArray u = uuid.toUtf8();
    if (m_bridge->api().lt_device_find(u.constData(), &h) != kLTOk)
        return kLTInvalidDevice;
    return h;
}

bool LTAdapterBase::eventForMe(quint64 eventDevice) const {
    return eventDevice == 0 || m_device == kLTInvalidDevice || eventDevice == m_device;
}

void LTAdapterBase::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                  const QVariantMap &payload) {
    Q_UNUSED(kind);
    Q_UNUSED(device);
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    // Base does nothing; subclasses override for the kinds they own.
}
