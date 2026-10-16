// LTDebugPortAdapter.h — mirrors the GDB remote stub settings (ABI.md §3.5).
//
// The Device ▸ Debugging ▸ Debug Port… sheet: the stub's switch, the port
// this boot listens on (null when the boot started without one), and the
// attach commands with per-command Copy buttons.
#pragma once

#include "LTAdapterBase.h"

class LTDebugPortAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)

public:
    explicit LTDebugPortAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    QVariantMap state() const { return m_state; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setEnabled(bool enabled);
    // Copies one attach command to the clipboard (QClipboard, Qt abstraction).
    Q_INVOKABLE void copyCommand(const QString &command);

signals:
    void deviceUuidChanged();
    void stateChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    QVariantMap m_state;
};
