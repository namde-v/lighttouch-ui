// LTAppsAdapter.h — mirrors DeviceApps / AppInstallPipeline (ABI.md §3.6).
#pragma once

#include "LTAdapterBase.h"

class LTAppsAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(QString bannerText READ bannerText NOTIFY rowsChanged)
    Q_PROPERTY(bool installing READ installing NOTIFY rowsChanged)
    Q_PROPERTY(bool canQueueInstall READ canQueueInstall NOTIFY rowsChanged)

public:
    explicit LTAppsAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    QVariantList rows() const { return m_rows; }
    QString bannerText() const { return m_bannerText; }
    bool installing() const { return m_installing; }
    bool canQueueInstall() const { return m_canQueueInstall; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void install(const QString &ipaPath);
    // Native file pickers (QtWidgets; QtQuick.Dialogs is not shipped on all
    // targets). No-ops when the user cancels.
    Q_INVOKABLE void pickAndInstallApp();
    Q_INVOKABLE void pickAndImportMedia();
    Q_INVOKABLE void remove(const QString &bundleId);
    Q_INVOKABLE void cancelInstall(const QString &bundleId);
    Q_INVOKABLE void restartSpringBoard();
    Q_INVOKABLE void importMedia(const QStringList &paths);
    Q_INVOKABLE QVariantList storeSearch(const QString &query);

signals:
    void deviceUuidChanged();
    void rowsChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    QVariantList m_rows;
    QString m_bannerText;
    bool m_installing = false;
    bool m_canQueueInstall = false;
};
