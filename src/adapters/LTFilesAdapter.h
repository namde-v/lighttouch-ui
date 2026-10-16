// LTFilesAdapter.h — mirrors DeviceServicesClient file APIs + FilesConnection (ABI.md §3.7).
#pragma once

#include "LTAdapterBase.h"

class LTFilesAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceUuid READ deviceUuid WRITE setDeviceUuid NOTIFY deviceUuidChanged)
    Q_PROPERTY(int connectionState READ connectionState NOTIFY connectionChanged)
    Q_PROPERTY(QString connectedDevice READ connectedDevice NOTIFY connectionChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY entriesChanged)
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    Q_PROPERTY(double transferFraction READ transferFraction NOTIFY transferChanged)
    Q_PROPERTY(QString transferFile READ transferFile NOTIFY transferChanged)
    Q_PROPERTY(quint64 transferId READ transferId NOTIFY transferChanged)
    Q_PROPERTY(bool hasTransfer READ hasTransfer NOTIFY transferChanged)

public:
    explicit LTFilesAdapter(Bridge *bridge, QObject *parent = nullptr);

    QString deviceUuid() const { return m_uuid; }
    void setDeviceUuid(const QString &uuid);
    int connectionState() const { return m_connectionState; }
    QString connectedDevice() const { return m_connectedDevice; }
    QString currentPath() const { return m_currentPath; }
    QVariantList entries() const { return m_entries; }
    double transferFraction() const { return m_transferFraction; }
    QString transferFile() const { return m_transferFile; }
    quint64 transferId() const { return m_transferId; }
    bool hasTransfer() const { return m_transferId != 0; }

    Q_INVOKABLE void refreshState();
    Q_INVOKABLE void follow(bool follow);
    Q_INVOKABLE void list(const QString &path);
    Q_INVOKABLE void transfer(const QString &opJson);
    Q_INVOKABLE void cancelTransfer(quint64 transferId);
    // Native pickers (QtWidgets; QtQuick.Dialogs is not shipped on all
    // targets). Empty on cancel.
    Q_INVOKABLE QStringList pickUploadFiles();
    Q_INVOKABLE QString pickDownloadDir();
    Q_INVOKABLE QString pickDownloadFile(const QString &name);

signals:
    void deviceUuidChanged();
    void connectionChanged();
    void entriesChanged();
    void transferChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QString m_uuid;
    int m_connectionState = kLTFilesStateDisconnected;
    QString m_connectedDevice;
    QString m_currentPath;
    QVariantList m_entries;
    double m_transferFraction = -1.0;
    QString m_transferFile;
    quint64 m_pendingRequestId = 0;
    quint64 m_transferId = 0;
    quint64 m_lastDoneId = 0;
};
