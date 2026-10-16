#include "LTSettingsAdapter.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QSettings>

LTSettingsAdapter::LTSettingsAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {
    loadLocalPrefs();
}

void LTSettingsAdapter::refresh() {
    if (!ready())
        return;
    loadLocalPrefs();
    emit vibrationSoundChanged();
    emit refresh120HzChanged();
    emit addDeviceShowsExperimentalChanged();
    const QString answer = getString(QStringLiteral("networkAccess.answer"));
    if (answer != m_networkAccessAnswer) {
        m_networkAccessAnswer = answer;
        emit networkAccessAnswerChanged();
    }
    const char *raw = nullptr;
    if (check(bridge()->api().lt_version(&raw), "lt_version")) {
        const QString v = bridge()->takeString(raw);
        if (v != m_version) {
            m_version = v;
            emit versionChanged();
        }
    }
    raw = nullptr;
    if (check(bridge()->api().lt_about_credits(&raw), "lt_about_credits")) {
        const QVariantMap credits =
            jsonObject(bridge()->takeString(raw), "lt_about_credits");
        if (credits != m_aboutCredits) {
            m_aboutCredits = credits;
            emit aboutCreditsChanged();
        }
    }
}

QString LTSettingsAdapter::getString(const QString &key) {
    if (!ready())
        return {};
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_settings_get_string(key.toUtf8().constData(),
                                                              &raw);
    const QString value = bridge()->takeString(raw);
    check(r, "lt_settings_get_string");
    return value;
}

void LTSettingsAdapter::setString(const QString &key, const QString &value) {
    if (!ready())
        return;
    check(bridge()->api().lt_settings_set_string(key.toUtf8().constData(),
                                                 value.toUtf8().constData()),
          "lt_settings_set_string");
}

QVariantList LTSettingsAdapter::helpTopics() {
    if (!ready())
        return {};
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_help_topics(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_help_topics"))
        return {};
    return jsonArray(json, "lt_help_topics");
}

QString LTSettingsAdapter::diagnosticsExport() {
    if (!ready())
        return {};
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_diagnostics_export(&raw);
    const QString path = bridge()->takeString(raw);
    check(r, "lt_diagnostics_export");
    return path;
}

void LTSettingsAdapter::quitRequest() {
    if (!ready())
        return;
    check(bridge()->api().lt_quit_request(), "lt_quit_request");
    // The outcome arrives as kLTEventQuitReady / kLTEventToast.
}

void LTSettingsAdapter::copyBugReportInfo() {
    if (!ready())
        return;
    const char *vraw = nullptr;
    QString version;
    if (bridge()->api().lt_version(&vraw) == kLTOk)
        version = bridge()->takeString(vraw);
    const char *craw = nullptr;
    QString credits;
    if (bridge()->api().lt_about_credits(&craw) == kLTOk)
        credits = bridge()->takeString(craw);
    const QString report = QStringLiteral("LightTouch %1\n%2").arg(version, credits);
    QGuiApplication::clipboard()->setText(report);
}

void LTSettingsAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                      const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    Q_UNUSED(payload);
    if (kind == kLTEventQuitReady)
        emit quitReady();
}

void LTSettingsAdapter::loadLocalPrefs() {
    QSettings s;
    m_vibrationSound = s.value(QStringLiteral("vibrationSound"), true).toBool();
    m_refresh120Hz = s.value(QStringLiteral("refresh120Hz"), false).toBool();
    m_addDeviceShowsExperimental =
        s.value(QStringLiteral("addDeviceShowsExperimental"), false).toBool();
    m_captureScreenOnly = s.value(QStringLiteral("captureMode"), 0).toInt() == 1;
}

void LTSettingsAdapter::setNetworkAccessAnswer(const QString &answer) {
    if (!ready())
        return;
    // kLTSettingsKeyNetworkAccessAnswer (lt_bridge.h §7.11).
    setString(QStringLiteral("networkAccess.answer"), answer);
    if (answer != m_networkAccessAnswer) {
        m_networkAccessAnswer = answer;
        emit networkAccessAnswerChanged();
    }
}

void LTSettingsAdapter::setVibrationSound(bool on) {
    QSettings s;
    s.setValue(QStringLiteral("vibrationSound"), on);
    if (on != m_vibrationSound) {
        m_vibrationSound = on;
        emit vibrationSoundChanged();
    }
}

void LTSettingsAdapter::setRefresh120Hz(bool on) {
    QSettings s;
    s.setValue(QStringLiteral("refresh120Hz"), on);
    if (on != m_refresh120Hz) {
        m_refresh120Hz = on;
        emit refresh120HzChanged();
    }
}

void LTSettingsAdapter::setAddDeviceShowsExperimental(bool on) {
    QSettings s;
    s.setValue(QStringLiteral("addDeviceShowsExperimental"), on);
    if (on != m_addDeviceShowsExperimental) {
        m_addDeviceShowsExperimental = on;
        emit addDeviceShowsExperimentalChanged();
    }
}

void LTSettingsAdapter::setCaptureScreenOnly(bool on) {
    QSettings s;
    s.setValue(QStringLiteral("captureMode"), on ? 1 : 0);
    if (on != m_captureScreenOnly) {
        m_captureScreenOnly = on;
        emit captureScreenOnlyChanged();
    }
}

void LTSettingsAdapter::saveWindowGeometry(int x, int y, int width, int height) {
    QSettings s;
    s.setValue(QStringLiteral("windowGeometry/x"), x);
    s.setValue(QStringLiteral("windowGeometry/y"), y);
    s.setValue(QStringLiteral("windowGeometry/width"), width);
    s.setValue(QStringLiteral("windowGeometry/height"), height);
}

void LTSettingsAdapter::setConsoleHeight(int height) {
    QSettings s;
    s.setValue(QStringLiteral("console/height"), height);
}

int LTSettingsAdapter::consoleHeight() const {
    QSettings s;
    return s.value(QStringLiteral("console/height"), -1).toInt();
}

QString LTSettingsAdapter::bezelMode() const {
    QSettings s;
    // Migrates the old on/off pref (surface 2): on → "2d", off → "off".
    if (s.contains(QStringLiteral("display/bezel"))) {
        return s.value(QStringLiteral("display/bezel")).toString() == QStringLiteral("off")
                   ? QStringLiteral("off") : QStringLiteral("2d");
    }
    return s.value(QStringLiteral("display/bezelMode"), QStringLiteral("2d")).toString();
}

void LTSettingsAdapter::setBezelMode(const QString &mode) {
    QSettings s;
    s.setValue(QStringLiteral("display/bezelMode"), mode);
    s.setValue(QStringLiteral("display/bezel"),
               mode == QStringLiteral("off") ? QStringLiteral("off") : QStringLiteral("on"));
}

QVariantMap LTSettingsAdapter::windowGeometry() const {
    QSettings s;
    QVariantMap m;
    m[QStringLiteral("x")] = s.value(QStringLiteral("windowGeometry/x"), -1);
    m[QStringLiteral("y")] = s.value(QStringLiteral("windowGeometry/y"), -1);
    m[QStringLiteral("width")] = s.value(QStringLiteral("windowGeometry/width"), -1);
    m[QStringLiteral("height")] = s.value(QStringLiteral("windowGeometry/height"), -1);
    return m;
}
