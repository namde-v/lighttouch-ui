#include "LTCatalogAdapter.h"

#include <QJsonDocument>
#include <QJsonObject>

LTCatalogAdapter::LTCatalogAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTCatalogAdapter::setFilterText(const QString &text) {
    if (text == m_filterText)
        return;
    m_filterText = text;
    emit filterTextChanged();
    refresh();
}

void LTCatalogAdapter::refresh() {
    if (!ready())
        return;
    const QJsonObject filter{
        { QStringLiteral("text"), m_filterText },
        { QStringLiteral("board"), QJsonValue() },
        { QStringLiteral("status"), QJsonValue() },
    };
    const QByteArray filterJson =
        QJsonDocument(filter).toJson(QJsonDocument::Compact);
    const char *raw = nullptr;
    const LTResult r =
        bridge()->api().lt_catalog_list(filterJson.constData(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_catalog_list"))
        return;
    const QVariantList entries = jsonArray(json, "lt_catalog_list");
    if (entries != m_entries) {
        m_entries = entries;
        emit entriesChanged();
    }
}

void LTCatalogAdapter::showDetails(const QString &entryId) {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_catalog_details(entryId.toUtf8().constData(),
                                                          &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_catalog_details"))
        return;
    m_details = jsonObject(json, "lt_catalog_details");
    m_detailsEntryId = entryId;
    emit detailsChanged();
}

void LTCatalogAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                     const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    if (kind == kLTEventCatalogChanged)
        refresh();
}
