// LTCarrierAdapter.h — mirrors CarrierPanelModel (ABI.md §3.5).
#pragma once

#include "LTAdapterBase.h"

class LTCarrierAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString carrierName READ carrierName NOTIFY stateChanged)
    Q_PROPERTY(QString message READ message NOTIFY stateChanged)
    Q_PROPERTY(QString callState READ callState NOTIFY stateChanged)

public:
    explicit LTCarrierAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    bool visible() const { return m_visible; }
    void setVisible(bool v);
    QVariantMap state() const { return m_state; }
    QString carrierName() const { return m_state.value(QStringLiteral("carrierName")).toString(); }
    QString message() const { return m_state.value(QStringLiteral("message")).toString(); }
    QString callState() const { return m_state.value(QStringLiteral("callState")).toString(); }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setField(const QString &field, const QString &value);
    Q_INVOKABLE void applyNetwork();
    Q_INVOKABLE void setRegistered(bool registered);
    Q_INVOKABLE void setSimPresent(bool present);
    Q_INVOKABLE void setBars(int bars);
    Q_INVOKABLE void ring();
    Q_INVOKABLE void answer();
    Q_INVOKABLE void hangUp();
    Q_INVOKABLE void sendSms();
    Q_INVOKABLE void applyLocation();
    Q_INVOKABLE void setLocation(double lat, double lon);
    Q_INVOKABLE void toggleWalk();
    Q_INVOKABLE void notice(const QString &text);

signals:
    void deviceUuidChanged();
    void visibleChanged();
    void stateChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    bool m_visible = false;
    QVariantMap m_state;
};
