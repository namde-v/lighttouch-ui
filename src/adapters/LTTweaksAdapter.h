// LTTweaksAdapter.h — mirrors TweaksPanelModel (ABI.md §3.5).
#pragma once

#include "LTAdapterBase.h"

class LTTweaksAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(QVariantList tweaks READ tweaks NOTIFY tweaksChanged)
    Q_PROPERTY(bool running READ running NOTIFY tweaksChanged)
    Q_PROPERTY(QString message READ message NOTIFY tweaksChanged)
    Q_PROPERTY(QVariantList hiddenApps READ hiddenApps NOTIFY hiddenAppsChanged)

public:
    explicit LTTweaksAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    QVariantList tweaks() const { return m_tweaks; }
    bool running() const { return m_running; }
    QString message() const { return m_message; }
    QVariantList hiddenApps() const { return m_hiddenApps; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setTweak(const QString &tweakId, bool on);
    Q_INVOKABLE void setColor(int color);
    Q_INVOKABLE void setClock(double epoch);
    Q_INVOKABLE void listHiddenApps();
    Q_INVOKABLE void openApp(const QString &bundleId);

signals:
    void deviceUuidChanged();
    void tweaksChanged();
    void hiddenAppsChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    QVariantList m_tweaks;
    bool m_running = false;
    QString m_message;
    QVariantList m_hiddenApps;
};
