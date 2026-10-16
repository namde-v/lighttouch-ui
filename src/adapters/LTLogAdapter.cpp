#include "LTLogAdapter.h"

LTLogAdapter::LTLogAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTLogAdapter::setCurrentSource(const QString &source) {
    if (source == m_source)
        return;
    if (m_following)
        setFollowing(false);
    m_source = source;
    m_lines.clear();
    m_offset = 0;
    emit currentSourceChanged();
    emit linesChanged();
}

void LTLogAdapter::setFollowing(bool follow) {
    if (follow == m_following)
        return;
    if (ready() && !m_source.isEmpty())
        check(bridge()->api().lt_log_follow(m_source.toUtf8().constData(),
                                            follow ? 1 : 0),
              "lt_log_follow");
    m_following = follow;
    emit followingChanged();
}

void LTLogAdapter::refreshSources() {
    if (!ready())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_log_sources(&raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_log_sources"))
        return;
    const QVariantList items = jsonArray(json, "lt_log_sources");
    QStringList sources;
    sources.reserve(items.size());
    for (const QVariant &v : items)
        sources << v.toString();
    if (sources != m_sources) {
        m_sources = sources;
        emit sourcesChanged();
    }
}

void LTLogAdapter::read(quint64 from) {
    if (!ready() || m_source.isEmpty())
        return;
    const char *raw = nullptr;
    const LTResult r = bridge()->api().lt_log_read(m_source.toUtf8().constData(),
                                                   from, &raw);
    const QString json = bridge()->takeString(raw);
    if (!check(r, "lt_log_read"))
        return;
    const QVariantMap result = jsonObject(json, "lt_log_read");
    if (result.value(QStringLiteral("rotated")).toBool()) {
        m_lines.clear();
        m_offset = 0;
    }
    appendLines(result.value(QStringLiteral("text")).toString(),
                result.value(QStringLiteral("offset")).toULongLong());
}

void LTLogAdapter::clear() {
    if (!ready() || m_source.isEmpty())
        return;
    check(bridge()->api().lt_log_clear(m_source.toUtf8().constData()), "lt_log_clear");
}

void LTLogAdapter::appendLines(const QString &text, quint64 newOffset) {
    if (!text.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const QStringList parts = text.split(u'\n', Qt::SkipEmptyParts);
#else
        const QStringList parts = text.split(QLatin1Char('\n'), QString::SkipEmptyParts);
#endif
        m_lines.append(parts);
        while (m_lines.size() > kMaxLines)
            m_lines.removeFirst();
    }
    m_offset = newOffset;
    emit linesChanged();
}

void LTLogAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                 const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    switch (kind) {
    case kLTEventLogLine: {
        const QString source = payload.value(QStringLiteral("source")).toString();
        if (source != m_source)
            return;
        appendLines(payload.value(QStringLiteral("text")).toString(),
                    payload.value(QStringLiteral("offset")).toULongLong());
        break;
    }
    case kLTEventLogRotated: {
        const QString source = payload.value(QStringLiteral("source")).toString();
        if (source != m_source)
            return;
        m_lines.clear();
        m_offset = 0;
        emit linesChanged();
        break;
    }
    default:
        break;
    }
}
