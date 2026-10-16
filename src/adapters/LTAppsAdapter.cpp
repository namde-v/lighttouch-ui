#include "LTAppsAdapter.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>

LTAppsAdapter::LTAppsAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTAppsAdapter::setDeviceUuid(const QString &uuid) {
    if (uuid == m_uuid)
        return;
    m_uuid = uuid;
    setCurrentDevice(deviceHandle(uuid));
    emit deviceUuidChanged();
    refresh();
}

void LTAppsAdapter::refresh() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_apps_list(currentDevice(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_apps_list"))
        return;
    const QVariantMap root = jsonObject(json, "lt_apps_list");
    m_rows = root.value(QStringLiteral("rows")).toList();
    m_bannerText = root.value(QStringLiteral("banner")).toMap()
                       .value(QStringLiteral("text")).toString();
    m_installing = root.value(QStringLiteral("device")).toMap()
                       .value(QStringLiteral("installing")).toBool();
    m_canQueueInstall = root.value(QStringLiteral("device")).toMap()
                            .value(QStringLiteral("canQueueInstall")).toBool();
    emit rowsChanged();
}

void LTAppsAdapter::install(const QString &ipaPath) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_apps_install(currentDevice(), ipaPath.toUtf8().constData()),
          "lt_apps_install");
}

void LTAppsAdapter::pickAndInstallApp() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const QStringList paths = QFileDialog::getOpenFileNames(
        nullptr, tr("Choose decrypted .ipa files"), QString(),
        tr("iPhone apps (*.ipa);;All files (*)"));
    for (const QString &p : paths)
        install(p);
}

void LTAppsAdapter::pickAndImportMedia() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    const QStringList paths = QFileDialog::getOpenFileNames(
        nullptr, tr("Import media"), QString(),
        tr("Media files (*.mp3 *.m4a *.aac *.wav *.mp4 *.m4v *.mov *.jpg *.jpeg *.png *.gif);;All files (*)"));
    importMedia(paths);
}

void LTAppsAdapter::remove(const QString &bundleId) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_apps_remove(currentDevice(), bundleId.toUtf8().constData()),
          "lt_apps_remove");
}

void LTAppsAdapter::cancelInstall(const QString &bundleId) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_apps_cancel_install(currentDevice(),
                                                 bundleId.toUtf8().constData()),
          "lt_apps_cancel_install");
}

void LTAppsAdapter::restartSpringBoard() {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return;
    check(bridge()->api().lt_apps_restart_springboard(currentDevice()),
          "lt_apps_restart_springboard");
}

void LTAppsAdapter::importMedia(const QStringList &paths) {
    if (!ready() || currentDevice() == kLTInvalidDevice || paths.isEmpty())
        return;
    const QByteArray pathsJson =
        QJsonDocument(QJsonArray::fromStringList(paths)).toJson(QJsonDocument::Compact);
    check(bridge()->api().lt_apps_import_media(currentDevice(), pathsJson.constData()),
          "lt_apps_import_media");
}

QVariantList LTAppsAdapter::storeSearch(const QString &query) {
    if (!ready() || currentDevice() == kLTInvalidDevice)
        return {};
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_apps_store_search(currentDevice(),
                                                            query.toUtf8().constData(), &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_apps_store_search"))
        return {};
    return jsonArray(json, "lt_apps_store_search");
}

void LTAppsAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                  const QVariantMap &payload) {
    Q_UNUSED(seq);
    if (!eventForMe(device))
        return;
    switch (kind) {
    case kLTEventAppsChanged:
    case kLTEventInstallDone:
        refresh();
        break;
    case kLTEventInstallProgress: {
        // Patch the matching row's progress in place; full refresh on done.
        const QString bundleId = payload.value(QStringLiteral("bundleId")).toString();
        const double fraction = payload.value(QStringLiteral("fraction")).toDouble();
        bool changed = false;
        for (QVariant &row : m_rows) {
            QVariantMap m = row.toMap();
            if (m.value(QStringLiteral("identity")).toString() == bundleId) {
                m.insert(QStringLiteral("progress"), fraction);
                row = m;
                changed = true;
            }
        }
        if (changed)
            emit rowsChanged();
        break;
    }
    default:
        break;
    }
}
