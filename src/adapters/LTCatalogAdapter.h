// LTCatalogAdapter.h — mirrors FirmwareCatalog / CatalogDetailsModel (ABI.md §3.10).
#pragma once

#include "LTAdapterBase.h"

class LTCatalogAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QVariantMap details READ details NOTIFY detailsChanged)
    Q_PROPERTY(QString detailsEntryId READ detailsEntryId NOTIFY detailsChanged)

public:
    explicit LTCatalogAdapter(Bridge *bridge, QObject *parent = nullptr);

    QVariantList entries() const { return m_entries; }
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);
    QVariantMap details() const { return m_details; }
    QString detailsEntryId() const { return m_detailsEntryId; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void showDetails(const QString &entryId);

signals:
    void entriesChanged();
    void filterTextChanged();
    void detailsChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QVariantList m_entries;
    QString m_filterText;
    QVariantMap m_details;
    QString m_detailsEntryId;
};
