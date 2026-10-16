// LTProxyAdapter.h — mirrors WebProxyConfiguration (ABI.md §3.5).
//
// The Device ▸ Network ▸ Proxy… sheet: per-device web proxy routing +
// CA trust. The editor binds `config` (mode/archiveDate), then calls
// apply() (OK) or drops the edits (Cancel).
#pragma once

#include "LTAdapterBase.h"

class LTProxyAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged)

public:
    explicit LTProxyAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    QVariantMap config() const { return m_config; }

    Q_INVOKABLE void refresh();
    // Writes the whole configuration (mode + archiveDate) back.
    Q_INVOKABLE void apply(const QString &configJson);

signals:
    void deviceUuidChanged();
    void configChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    QVariantMap m_config;
};
