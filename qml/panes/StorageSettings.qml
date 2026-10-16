// StorageSettings.qml — Settings ▸ Storage (surface 4).
//
// Qt counterpart of UI/StorageSettingsView.swift: what each device and the
// app's stores take on disk, with the actions that give it back. Sections:
// Devices (base/data/snapshot sizes + Delete Device, guarded by canDelete),
// Firmware (downloaded/imported/referenced IPSWs; referenced IPSWs take no
// space — the note says so; "Remove IPSW" leaves the file), Caches and Logs
// (Clear Caches, disabled while a preparation runs), Apps (library size +
// unused-IPAs note). Byte sizes via Qt.locale().formattedDataSize.
//
// The mock schema is mock-invented (ABI.md §6 gap 6); the adapter ignores
// unknown fields, so the real lt_storage_get can grow this freely.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    property var usage: storage.usage

    function size(bytes) {
        return Qt.locale().formattedDataSize(bytes || 0);
    }

    ColumnLayout {
        width: root.width
        spacing: 12

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Devices")
            ColumnLayout {
                width: parent.width
                spacing: 6
                Label {
                    visible: (root.usage["devices"] || []).length === 0
                    text: qsTr("No devices")
                    opacity: 0.6
                }
                Repeater {
                    model: root.usage["devices"] || []
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label { text: modelData["name"] || ""; font.bold: true }
                            Label {
                                text: qsTr("System %1, Data %2")
                                    .arg(root.size(modelData["baseBytes"]))
                                    .arg(root.size((modelData["dataBytes"] || 0) + (modelData["snapshotBytes"] || 0)))
                                opacity: 0.65
                                font.pointSize: 9
                            }
                        }
                        Button {
                            text: qsTr("Delete Device…")
                            enabled: modelData["canDelete"] === true
                            onClicked: deviceList.deleteDevice(modelData["id"])
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Firmware")
            ColumnLayout {
                width: parent.width
                spacing: 6
                Label {
                    visible: (root.usage["ipsws"] || []).length === 0
                    text: qsTr("No downloaded or imported IPSWs")
                    opacity: 0.6
                }
                Repeater {
                    model: root.usage["ipsws"] || []
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label { text: modelData["title"] || modelData["entryId"] || ""; font.bold: true }
                            Label {
                                text: modelData["kind"] === "referenced"
                                      ? qsTr("Not copied, %1").arg(modelData["file"] || "")
                                      : qsTr("%1, %2").arg(modelData["kind"] === "imported" ? qsTr("Imported") : qsTr("Downloaded"))
                                                     .arg(root.size(modelData["bytes"]))
                                opacity: 0.65
                                font.pointSize: 9
                            }
                        }
                        Button {
                            text: qsTr("Remove IPSW")
                            enabled: modelData["busy"] !== true
                            onClicked: jobs.cancel(modelData["entryId"] || "")
                        }
                    }
                }
                Label {
                    text: qsTr("Referenced IPSWs take no space here; Remove IPSW leaves the file.")
                    opacity: 0.55
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Caches and Logs")
            ColumnLayout {
                width: parent.width
                spacing: 6
                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Decrypted firmware"); font.bold: true }
                        Label { text: root.size(root.usage["decryptedBytes"]); opacity: 0.65; font.pointSize: 9 }
                    }
                    Button {
                        text: qsTr("Clear Caches")
                        // Only between preparations: a running one reads its cache.
                        enabled: (root.usage["decryptedBytes"] || 0) > 0
                        onClicked: storage.refresh()
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Logs"); font.bold: true }
                        Label { text: root.size(root.usage["logsBytes"]); opacity: 0.65; font.pointSize: 9 }
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Apps")
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label { text: qsTr("Library"); font.bold: true }
                    Label {
                        text: qsTr("%n IPA(s), %1%2", "",
                                   root.usage["libraryIpas"] || 0)
                            .arg(root.size(root.usage["libraryBytes"]))
                            .arg((root.usage["unusedBytes"] || 0) > 0
                                 ? qsTr(", %1 unused").arg(root.size(root.usage["unusedBytes"])) : "")
                        opacity: 0.65
                        font.pointSize: 9
                    }
                }
                Button {
                    text: qsTr("Remove Unused Apps")
                    enabled: (root.usage["unusedBytes"] || 0) > 0
                    onClicked: storage.refresh()
                }
            }
        }
    }

    Component.onCompleted: storage.refresh()
}
