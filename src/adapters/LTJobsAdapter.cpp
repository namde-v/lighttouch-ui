#include "LTJobsAdapter.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>

LTJobsAdapter::LTJobsAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTJobsAdapter::refresh() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_jobs_list(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_jobs_list"))
        return;
    const QVariantList jobs = jsonArray(json, "lt_jobs_list");
    if (jobs != m_jobs) {
        m_jobs = jobs;
        emit jobsChanged();
    }
}

void LTJobsAdapter::refreshCanDownload() {
    if (!ready())
        return;
    int32_t can = 0;
    if (!check(bridge()->api().lt_jobs_can_download(&can), "lt_jobs_can_download"))
        return;
    // ABI.md §3.10: lt_jobs_can_download returns { "can":…, "reason":…|null },
    // but the C signature carries int32_t*; the reason string is documented
    // to arrive via lt_last_error on failure. Keep the int here and surface
    // the reason text when can == 0.
    const bool newCan = (can != 0);
    const QString newReason =
        newCan ? QString() : bridge()->lastErrorString();
    if (newCan != m_canDownload || newReason != m_downloadBlockReason) {
        m_canDownload = newCan;
        m_downloadBlockReason = newReason;
        emit downloadCheckChanged();
    }
}

void LTJobsAdapter::start(const QString &entryId, const QString &optionsJson) {
    if (!ready())
        return;
    check(bridge()->api().lt_job_start(entryId.toUtf8().constData(),
                                       optionsJson.toUtf8().constData()),
          "lt_job_start");
}

void LTJobsAdapter::importIpsws(const QStringList &paths) {
    if (!ready() || paths.isEmpty())
        return;
    const QByteArray pathsJson =
        QJsonDocument(QJsonArray::fromStringList(paths)).toJson(QJsonDocument::Compact);
    check(bridge()->api().lt_job_import_ipsw(pathsJson.constData()),
          "lt_job_import_ipsw");
}

QStringList LTJobsAdapter::pickIpswFiles() {
    return QFileDialog::getOpenFileNames(
        nullptr, tr("Import IPSW"), QString(),
        tr("iPhone software (*.ipsw);;All files (*)"));
}

void LTJobsAdapter::cancel(const QString &entryId) {
    if (!ready())
        return;
    check(bridge()->api().lt_job_cancel(entryId.toUtf8().constData()),
          "lt_job_cancel");
}

void LTJobsAdapter::cancelJobForDevice(const QString &deviceUuid) {
    // "deviceUuid" is a mock-only job field (see Bridge.cpp); the real core
    // will expose the row↔job linkage when it lands.
    for (const QVariant &job : m_jobs) {
        const QVariantMap m = job.toMap();
        if (m.value(QStringLiteral("deviceUuid")).toString() == deviceUuid) {
            cancel(m.value(QStringLiteral("entryId")).toString());
            return;
        }
    }
}

void LTJobsAdapter::cancelAll() {
    if (!ready())
        return;
    check(bridge()->api().lt_job_cancel_all(), "lt_job_cancel_all");
}

void LTJobsAdapter::dismissFailure(const QString &entryId) {
    if (!ready())
        return;
    check(bridge()->api().lt_job_dismiss_failure(entryId.toUtf8().constData()),
          "lt_job_dismiss_failure");
}

void LTJobsAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                  const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    switch (kind) {
    case kLTEventJobDone:
        refresh();
        break;
    case kLTEventJobProgress: {
        // Patch the matching job's progress in place; full refresh on done.
        const QString entryId = payload.value(QStringLiteral("entryId")).toString();
        const double fraction = payload.value(QStringLiteral("fraction")).toDouble();
        bool changed = false;
        for (QVariant &job : m_jobs) {
            QVariantMap m = job.toMap();
            if (m.value(QStringLiteral("entryId")).toString() == entryId) {
                m.insert(QStringLiteral("progress"), fraction);
                QVariantMap preparation = m.value(QStringLiteral("preparation")).toMap();
                preparation.insert(QStringLiteral("fraction"), fraction);
                preparation.insert(QStringLiteral("detail"),
                                   payload.value(QStringLiteral("detail")));
                m.insert(QStringLiteral("preparation"), preparation);
                job = m;
                changed = true;
            }
        }
        if (changed)
            emit jobsChanged();
        break;
    }
    case kLTEventTaskbarProgress: {
        const double fraction =
            payload.value(QStringLiteral("fraction"), -1.0).toDouble();
        if (!qFuzzyCompare(fraction + 1.0, m_aggregateFraction + 1.0)) {
            m_aggregateFraction = fraction;
            emit aggregateChanged();
        }
        break;
    }
    default:
        break;
    }
}
