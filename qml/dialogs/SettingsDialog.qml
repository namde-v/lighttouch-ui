// SettingsDialog.qml — Settings… (surface 10).
//
// Qt counterpart of UI/SettingsWindowController.swift: one dialog, a tab per
// pane, titled after the pane, sized to the pane. General: internet
// preference (Always/Never/Ask — the NetworkAccessPreference behind the
// first-start prompt), vibration sound, 120 Hz refresh. Capture: the
// CapturePreferences JSON (save location, copy-on-capture, record-bezel,
// open-after, sound). Storage: per-device rows with delete actions.
//
// Documented gaps: Storage re-measures on show on the Mac; the ABI exposes
// no storage-usage export yet, so the pane lists devices (deletable) without
// measured sizes.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../panes" // CaptureSettings, StorageSettings (surface 4)

Dialog {
    id: root
    modal: true
    width: 560
    title: {
        switch (bar.currentIndex) {
        case 0: return qsTr("Settings — General");
        case 1: return qsTr("Settings — Capture");
        default: return qsTr("Settings — Storage");
        }
    }
    standardButtons: Dialog.Close

    onOpened: {
        // Storage re-measures every time its pane is shown (Mac); the
        // device list is live anyway.
        deviceList.refresh();
        capture.refreshPrefs();
    }

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            id: bar
            objectName: "settingsTabBar"
            Layout.fillWidth: true
            TabButton { text: qsTr("General") }
            TabButton { text: qsTr("Capture") }
            TabButton { text: qsTr("Storage") }
        }
        StackLayout {
            currentIndex: bar.currentIndex
            Layout.fillWidth: true
            Layout.preferredHeight: 380

            // --- General ---
            ScrollView {
                clip: true
                ColumnLayout {
                    width: parent.width
                    spacing: 10
                    GroupBox {
                        title: qsTr("Network")
                        Layout.fillWidth: true
                        ColumnLayout {
                            Label { text: qsTr("Connect to the internet…"); wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            ComboBox {
                                id: internetBox
                                Layout.fillWidth: true
                                model: [qsTr("Always"), qsTr("Never"), qsTr("Ask When a Device Starts")]
                                currentIndex: settings.networkAccessAnswer === "always" ? 0
                                            : settings.networkAccessAnswer === "never" ? 1 : 2
                                onActivated: function (i) {
                                    settings.networkAccessAnswer = i === 0 ? "always" : i === 1 ? "never" : "";
                                }
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Devices")
                        Layout.fillWidth: true
                        ColumnLayout {
                            CheckBox {
                                text: qsTr("Play vibration sound")
                                checked: settings.vibrationSound
                                onToggled: settings.vibrationSound = checked
                            }
                            CheckBox {
                                text: qsTr("120 Hz refresh (experimental)")
                                checked: settings.refresh120Hz
                                onToggled: settings.refresh120Hz = checked
                            }
                            Label {
                                text: qsTr("Takes effect when a device next starts. Games that count frames run too fast.")
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                opacity: 0.6
                                font.pointSize: 9
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            // --- Capture (surface 4/8 pane) ---
            CaptureSettings {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            // --- Storage (surface 4 pane; re-measures on show, like the Mac) ---
            StorageSettings {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
