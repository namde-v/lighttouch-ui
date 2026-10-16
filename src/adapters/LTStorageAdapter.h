// LTStorageAdapter.h — mirrors the Storage settings model (ABI.md §3.5).
//
// Binds lt_storage_get/set. The schema of lt_storage_get is mock-invented
// (ABI.md §6 gap 6: the Mac view's backers live in the app target); the
// adapter exposes the sections StorageSettingsView renders: devices with
// base/data/snapshot sizes + delete, IPSWs (downloaded/imported/referenced),
// decrypted caches, logs, the IPA library.
#pragma once

#include "LTAdapterBase.h"

class LTStorageAdapter : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap usage READ usage NOTIFY usageChanged)

public:
    explicit LTStorageAdapter(Bridge *bridge, QObject *parent = nullptr);

    QVariantMap usage() const { return m_usage; }

    // Re-measures (the Mac re-measures every time the pane is shown).
    Q_INVOKABLE void refresh();

signals:
    void usageChanged();

private:
    Bridge *m_bridge;
    QVariantMap m_usage;
};
