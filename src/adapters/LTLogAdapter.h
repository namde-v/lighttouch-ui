// LTLogAdapter.h — mirrors LogTail (ABI.md §3.9).
#pragma once

#include "LTAdapterBase.h"

class LTLogAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QStringList sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QString currentSource READ currentSource WRITE setCurrentSource
                   NOTIFY currentSourceChanged)
    Q_PROPERTY(bool following READ following WRITE setFollowing NOTIFY followingChanged)
    Q_PROPERTY(QStringList lines READ lines NOTIFY linesChanged)
    Q_PROPERTY(quint64 offset READ offset NOTIFY linesChanged)

public:
    explicit LTLogAdapter(Bridge *bridge, QObject *parent = nullptr);

    QStringList sources() const { return m_sources; }
    QString currentSource() const { return m_source; }
    void setCurrentSource(const QString &source);
    bool following() const { return m_following; }
    void setFollowing(bool follow);
    QStringList lines() const { return m_lines; }
    quint64 offset() const { return m_offset; }

    Q_INVOKABLE void refreshSources();
    Q_INVOKABLE void read(quint64 from = 0);
    Q_INVOKABLE void clear();

signals:
    void sourcesChanged();
    void currentSourceChanged();
    void followingChanged();
    void linesChanged();

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void appendLines(const QString &text, quint64 newOffset);

    QStringList m_sources;
    QString m_source = QStringLiteral("app");
    bool m_following = false;
    QStringList m_lines;
    quint64 m_offset = 0;
    static const int kMaxLines = 5000;
};
