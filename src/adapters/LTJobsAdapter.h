// LTJobsAdapter.h — mirrors FirmwareJobs / FirmwareJobTable (ABI.md §3.10).
#pragma once

#include "LTAdapterBase.h"

class LTJobsAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QVariantList jobs READ jobs NOTIFY jobsChanged)
    Q_PROPERTY(bool hasJobs READ hasJobs NOTIFY jobsChanged)
    Q_PROPERTY(double aggregateFraction READ aggregateFraction NOTIFY aggregateChanged)
    Q_PROPERTY(bool canDownload READ canDownload NOTIFY downloadCheckChanged)
    Q_PROPERTY(QString downloadBlockReason READ downloadBlockReason NOTIFY downloadCheckChanged)

public:
    explicit LTJobsAdapter(Bridge *bridge, QObject *parent = nullptr);

    QVariantList jobs() const { return m_jobs; }
    bool hasJobs() const { return !m_jobs.isEmpty(); }
    double aggregateFraction() const { return m_aggregateFraction; }
    bool canDownload() const { return m_canDownload; }
    QString downloadBlockReason() const { return m_downloadBlockReason; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshCanDownload();
    Q_INVOKABLE void start(const QString &entryId, const QString &optionsJson);
    Q_INVOKABLE void importIpsws(const QStringList &paths);
    // Native multi-select IPSW picker (QtWidgets QFileDialog; the
    // QtQuick.Dialogs QML module is not shipped on all targets). Returns
    // local paths; empty when the user cancels.
    Q_INVOKABLE QStringList pickIpswFiles();
    Q_INVOKABLE void cancel(const QString &entryId);
    Q_INVOKABLE void cancelAll();
    Q_INVOKABLE void dismissFailure(const QString &entryId);
    // Cancels the firmware job (if any) behind one library device.
    Q_INVOKABLE void cancelJobForDevice(const QString &deviceUuid);

signals:
    void jobsChanged();
    void aggregateChanged();
    void downloadCheckChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    QVariantList m_jobs;
    double m_aggregateFraction = -1.0;
    bool m_canDownload = false;
    QString m_downloadBlockReason;
};
