#include "LTFilesAdapter.h"

#include <QFileDialog>

LTFilesAdapter::LTFilesAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTFilesAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refreshState();
}

void LTFilesAdapter::refreshState() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_files_state(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_files_state"))
        return;
    const QVariantMap s = jsonObject(json, "lt_files_state");
    const QString stateStr = s.value(QStringLiteral("state")).toString();
    m_connectionState = (stateStr == QStringLiteral("transferring")) ? kLTFilesStateTransferring
                      : (stateStr == QStringLiteral("connected"))    ? kLTFilesStateConnected
                                                                    : kLTFilesStateDisconnected;
    m_connectedDevice = s.value(QStringLiteral("device")).toString();
    emit connectionChanged();
}

void LTFilesAdapter::follow(bool follow) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_files_follow(currentDevice(), follow ? 1 : 0),
          "lt_files_follow");
}

void LTFilesAdapter::list(const QString &path) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    uint64_t requestId = 0;
    if (!check(bridge()->api().lt_files_list(currentDevice(), path.toUtf8().constData(),
                                             &requestId),
               "lt_files_list"))
        return;
    m_pendingRequestId = requestId;
    m_currentPath = path;
}

void LTFilesAdapter::transfer(const QString &opJson) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    uint64_t transferId = 0;
    check(bridge()->api().lt_files_transfer(currentDevice(), opJson.toUtf8().constData(),
                                            &transferId),
          "lt_files_transfer");
}

void LTFilesAdapter::cancelTransfer(quint64 transferId) {
    if (!ready())
        return;
    check(bridge()->api().lt_files_cancel(transferId), "lt_files_cancel");
}

QStringList LTFilesAdapter::pickUploadFiles() {
    return QFileDialog::getOpenFileNames(
        nullptr, tr("Copy to device"), QString(), tr("All files (*)"));
}

QString LTFilesAdapter::pickDownloadDir() {
    return QFileDialog::getExistingDirectory(
        nullptr, tr("Choose where to save"), QString());
}

QString LTFilesAdapter::pickDownloadFile(const QString &name) {
    return QFileDialog::getSaveFileName(
        nullptr, tr("Save to Mac"), name, tr("All files (*)"));
}

void LTFilesAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                   const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventFilesListed: {
        // Drop stale responses (Qt navigated away since the request).
        const quint64 requestId = payload.value(QStringLiteral("requestId")).toULongLong();
        if (requestId != m_pendingRequestId)
            return;
        m_entries = jsonArray(payload.value(QStringLiteral("entriesJson")).toString(),
                                  "kLTEventFilesListed");
        emit entriesChanged();
        break;
    }
    case kLTEventFilesProgress:
        m_transferId = payload.value(QStringLiteral("transferId")).toULongLong();
        m_transferFraction = payload.value(QStringLiteral("fraction")).toDouble();
        m_transferFile = payload.value(QStringLiteral("currentFile")).toString();
        emit transferChanged();
        break;
    case kLTEventFilesDone:
        m_lastDoneId = payload.value(QStringLiteral("transferId")).toULongLong();
        m_transferFraction = -1.0;
        m_transferFile.clear();
        m_transferId = 0;
        emit transferChanged();
        refreshState();
        // Re-list the current directory: the transfer changed it.
        if (!m_currentPath.isNull())
            list(m_currentPath);
        break;
    case kLTEventFilesConnectionChanged:
        refreshState();
        break;
    default:
        break;
    }
}
