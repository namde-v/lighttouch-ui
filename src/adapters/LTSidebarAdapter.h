// LTSidebarAdapter.h — mirrors SidebarList (ABI.md §3.2).
#pragma once

#include "LTAdapterBase.h"

class LTSidebarAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    Q_PROPERTY(int count READ count NOTIFY entriesChanged)

public:
    explicit LTSidebarAdapter(Bridge *bridge, QObject *parent = nullptr);

    QVariantList entries() const { return m_entries; }
    int count() const { return m_entries.size(); }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addEntries(const QStringList &entryIds);
    Q_INVOKABLE void removeEntry(const QString &entryId);
    Q_INVOKABLE void renameEntry(const QString &entryId, const QString &name);

signals:
    void entriesChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QVariantList m_entries;
};
