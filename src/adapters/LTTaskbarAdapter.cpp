#include "LTTaskbarAdapter.h"

#include <QtMath>

LTTaskbarAdapter::LTTaskbarAdapter(Bridge *bridge, QObject *parent)
    : LTAdapterBase(bridge, parent) {}

void LTTaskbarAdapter::setProgress(double fraction) {
    // The Mac redraws at >= 0.5% change and clears the badge at zero/nil.
    const double next = (fraction < 0.0) ? -1.0 : fraction;
    if (m_progress >= 0.0 && next >= 0.0
        && qAbs(m_progress - next) < 0.005)
        return;
    if (qFuzzyCompare(m_progress + 1.0, next + 1.0))
        return;
    m_progress = next;
    emit progressChanged();
    // Per-OS backend hook: the native dock/taskbar badge follows progress
    // here (Windows ITaskbarList3, Linux LauncherEntry DBus) — see header.
}

void LTTaskbarAdapter::setJobsAggregate(double fraction) {
    setProgress(fraction);
}

void LTTaskbarAdapter::onBridgeEvent(int kind, quint64 device, quint64 seq,
                                     const QVariantMap &payload) {
    Q_UNUSED(device);
    Q_UNUSED(seq);
    if (kind != kLTEventTaskbarProgress)
        return;
    setProgress(payload.value(QStringLiteral("fraction")).toDouble());
}
