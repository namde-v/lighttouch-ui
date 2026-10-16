#include "LTSidebarAdapter.h"

#include <QJsonArray>
#include <QJsonDocument>

LTSidebarAdapter::LTSidebarAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTSidebarAdapter::refresh() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_sidebar_list(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_sidebar_list"))
        return;
    // { "ids": […], "names": {…}, "labels": [{ "id", "title", "subtitle" }] }
    const QVariantMap root = jsonObject(json, "lt_sidebar_list");
    const QVariantList entries = root.value(QStringLiteral("labels")).toList();
    if (entries != m_entries) {
        m_entries = entries;
        emit entriesChanged();
    }
}

void LTSidebarAdapter::addEntries(const QStringList &entryIds) {
    if (!ready() || entryIds.isEmpty())
        return;
    const QByteArray idsJson =
        QJsonDocument(QJsonArray::fromStringList(entryIds)).toJson(QJsonDocument::Compact);
    check(bridge()->api().lt_sidebar_add(idsJson.constData()), "lt_sidebar_add");
}

void LTSidebarAdapter::removeEntry(const QString &entryId) {
    if (!ready())
        return;
    check(bridge()->api().lt_sidebar_remove(entryId.toUtf8().constData()),
          "lt_sidebar_remove");
}

void LTSidebarAdapter::renameEntry(const QString &entryId, const QString &name) {
    if (!ready())
        return;
    check(bridge()->api().lt_sidebar_rename(entryId.toUtf8().constData(),
                                            name.toUtf8().constData()),
          "lt_sidebar_rename");
}

void LTSidebarAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                     const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    if (kind == kLTEventSidebarChanged)
        refresh();
}
