#include "LTCaptureAdapter.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include <QImage>
#include <QUrl>

LTCaptureAdapter::LTCaptureAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTCaptureAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refreshAvailability();
}

void LTCaptureAdapter::refreshAvailability() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_capture_availability(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_capture_availability"))
        return;
    const QVariantMap a = jsonObject(json, "lt_capture_availability");
    m_canTakeScreenshot = a.value(QStringLiteral("canTakeScreenshot")).toBool();
    m_canStartRecording = a.value(QStringLiteral("canStartRecording")).toBool();
    m_canToggleRecording = a.value(QStringLiteral("canToggleRecording")).toBool();
    m_recordingSaving = a.value(QStringLiteral("recordingSaving")).toBool();
    m_recordingCanStop = a.value(QStringLiteral("recordingCanStop")).toBool();
    emit availabilityChanged();
}

void LTCaptureAdapter::screenshot() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    m_pendingShotEffect.clear();
    check(bridge()->api().lt_capture_screenshot(currentDevice()),
          "lt_capture_screenshot");
}

void LTCaptureAdapter::screenshotCopy() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    m_pendingShotEffect = QStringLiteral("copy");
    check(bridge()->api().lt_capture_screenshot(currentDevice()),
          "lt_capture_screenshot");
}

void LTCaptureAdapter::screenshotSaveAs() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    m_pendingShotEffect = QStringLiteral("saveAs");
    check(bridge()->api().lt_capture_screenshot(currentDevice()),
          "lt_capture_screenshot");
}

// Applies the pending Qt-side screenshot effect once the core finished.
void LTCaptureAdapter::applyPendingShotEffect() {
    if (m_pendingShotEffect.isEmpty() || m_lastCapturePath.isEmpty() || !m_lastCaptureOk)
        return;
    const QString effect = m_pendingShotEffect;
    m_pendingShotEffect.clear();
    if (effect == QStringLiteral("copy")) {
        const QImage img(m_lastCapturePath);
        if (!img.isNull()) {
            if (QClipboard *clip = QApplication::clipboard())
                clip->setImage(img);
            emit screenshotCopied();
        }
    } else if (effect == QStringLiteral("saveAs")) {
        const QString dest = QFileDialog::getSaveFileName(
            nullptr, tr("Save Screenshot"),
            m_lastCapturePath.section(QLatin1Char('/'), -1),
            tr("PNG images (*.png)"));
        if (!dest.isEmpty() && dest != m_lastCapturePath) {
            QFile::remove(dest);
            QFile::copy(m_lastCapturePath, dest);
            m_lastCapturePath = dest;
        }
    }
}

void LTCaptureAdapter::startRecording() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_capture_record_start(currentDevice()),
          "lt_capture_record_start");
}

void LTCaptureAdapter::stopRecording(bool discard) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_capture_record_stop(currentDevice(), discard ? 1 : 0),
          "lt_capture_record_stop");
}

void LTCaptureAdapter::saveRecording(const QString &path) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_capture_record_save(currentDevice(),
                                                 path.toUtf8().constData()),
          "lt_capture_record_save");
}

void LTCaptureAdapter::discardRecording() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_capture_record_discard(currentDevice()),
          "lt_capture_record_discard");
}

void LTCaptureAdapter::refreshPrefs() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_capture_prefs_get(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_capture_prefs_get"))
        return;
    m_prefs = jsonObject(json, "lt_capture_prefs_get");
    emit prefsChanged();
}

void LTCaptureAdapter::setPrefs(const QString &prefsJson) {
    if (!ready())
        return;
    if (check(bridge()->api().lt_capture_prefs_set(prefsJson.toUtf8().constData()),
              "lt_capture_prefs_set"))
        refreshPrefs();
}

void LTCaptureAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                     const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventCaptureAvailabilityChanged:
        refreshAvailability();
        break;
    case kLTEventCaptureDone:
        m_lastCaptureKind = payload.value(QStringLiteral("kind")).toInt();
        m_lastCapturePath = payload.value(QStringLiteral("path")).toString();
        m_lastCaptureOk = payload.value(QStringLiteral("ok")).toBool();
        applyPendingShotEffect();
        emit captureDone();
        break;
    case kLTEventRecordingTick:
        m_elapsedSeconds = payload.value(QStringLiteral("elapsedSeconds")).toULongLong();
        emit recordingChanged();
        break;
    case kLTEventRecordingPhaseChanged:
        m_recordingPhase = payload.value(QStringLiteral("phase")).toInt();
        emit recordingChanged();
        break;
    case kLTEventRecordingSaveNeeded:
        emit saveDestinationRequested(
            payload.value(QStringLiteral("suggestedName")).toString());
        break;
    default:
        break;
    }
}

void LTCaptureAdapter::openLastCapture() {
    if (m_lastCapturePath.isEmpty() || !m_lastCaptureOk)
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_lastCapturePath));
}

QString LTCaptureAdapter::pickFolder() {
    const QString dir = QFileDialog::getExistingDirectory(
        nullptr, tr("Choose a save location"), QString());
    return dir;  // empty when the user cancels
}

QString LTCaptureAdapter::pickApplication() {
#ifdef Q_OS_WIN
    const QString filter = tr("Applications (*.exe);;All files (*)");
#else
    const QString filter = tr("All files (*)");
#endif
    const QStringList files = QFileDialog::getOpenFileNames(
        nullptr, tr("Choose an application"), QString(), filter);
    return files.isEmpty() ? QString() : files.first();
}
