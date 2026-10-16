// LTAdapterBase.h — thin QObject base for all lt_ domain adapters.
//
// Adapters hold NO business logic (ABI.md §1): they forward commands down to
// libLTAppCore through Bridge::api(), and apply event snapshots / query
// results up into Q_PROPERTYs for QML. Every adapter:
//   - is constructed with a Bridge* (must outlive the adapter),
//   - auto-connects Bridge::ltEventReceived to the virtual onBridgeEvent(),
//   - filters device-scoped events by its current device handle when it has one.

#pragma once

#include <QObject>
#include <QVariant>
#include "Bridge.h"

class LTAdapterBase : public QObject {
    Q_OBJECT
public:
    explicit LTAdapterBase(Bridge *bridge, QObject *parent = nullptr);

protected:
    Bridge *bridge() const { return m_bridge; }
    bool ready() const;

    // check(): true when r == kLTOk; otherwise logs what + lt_last_error().
    bool check(LTResult r, const char *what) const;

    // Lenient JSON parsing of query results (unknown fields ignored, §5).
    QVariantList jsonArray(const QString &json, const char *what) const;
    QVariantMap jsonObject(const QString &json, const char *what) const;

    // Resolve a UUID to an lt_device_t handle (kLTInvalidDevice on failure).
    lt_device_t deviceHandle(const QString &uuid) const;

    // Device-scoped event filter: true when the event's device handle is 0
    // (not device-scoped) or matches the adapter's current device.
    bool eventForMe(quint64 eventDevice) const;
    void setCurrentDevice(lt_device_t d) { m_device = d; }
    lt_device_t currentDevice() const { return m_device; }

protected slots:
    virtual void onBridgeEvent(int kind, quint64 device, quint64 seq,
                               const QVariantMap &payload);

private:
    Bridge *m_bridge;
    lt_device_t m_device = kLTInvalidDevice;
};
