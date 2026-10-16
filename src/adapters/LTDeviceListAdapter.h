// LTDeviceListAdapter.h — mirrors the DeviceLibrary/DeviceRow surface (ABI.md §3.2).
#pragma once

#include "LTAdapterBase.h"

class LTDeviceListAdapter : public LTAdapterBase {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(int count READ count NOTIFY devicesChanged)
    Q_PROPERTY(bool hasDevices READ hasDevices NOTIFY devicesChanged)
    Q_PROPERTY(QVariantMap busyMap READ busyMap NOTIFY busyChanged)

public:
    explicit LTDeviceListAdapter(Bridge *bridge, QObject *parent = nullptr);

    QVariantList devices() const { return m_devices; }
    int count() const { return m_devices.size(); }
    bool hasDevices() const { return !m_devices.isEmpty(); }
    QVariantMap busyMap() const { return m_busy; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void createDevice(const QString &entryId, const QString &name);
    Q_INVOKABLE void deleteDevice(const QString &uuid);
    Q_INVOKABLE void renameDevice(const QString &uuid, const QString &name);
    Q_INVOKABLE void duplicateDevice(const QString &uuid);
    Q_INVOKABLE void eraseDevice(const QString &uuid);
    // UUID-scoped session commands for context-menu actions on rows that are
    // not the current selection (MainWindowController+Commands.swift).
    Q_INVOKABLE void bootDevice(const QString &uuid);
    // After the network-access prompt saved its answer: boot without asking.
    Q_INVOKABLE void bootDeviceConfirmed(const QString &uuid);
    Q_INVOKABLE void haltDevice(const QString &uuid);
    Q_INVOKABLE void resetDevice(const QString &uuid);
    // lt_drop_classify for a file:// URL; returns kLTDrop* or -1 on error.
    Q_INVOKABLE int classifyDropUrl(const QString &url);
    // Routes dropped files (surface 12): target is "display:<uuid>" |
    // "sidebar" | "apps:<uuid>" | "window". The result arrives as
    // kLTEventDropAction and is dispatched to the importer/installer.
    Q_INVOKABLE quint64 routeDrop(const QStringList &urls, const QString &target);

signals:
    void devicesChanged();
    // A drop was routed (for the shell's status feedback).
    void dropRouted(quint64 requestId, const QString &action,
                    const QString &detailJson);
    void busyChanged();
    // NetworkAccessPrompt: no saved answer and a row is starting.
    void networkAccessPromptRequested(const QString &uuid, const QString &name);

protected:
    void onBridgeEvent(int kind, quint64 device, quint64 seq,
                       const QVariantMap &payload) override;

private:
    void refreshBusy();

    QVariantList m_devices;
    QVariantMap m_busy;
};
