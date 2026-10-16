// CaptureSettings.qml — Settings ▸ Capture (surfaces 4 + 8).
//
// Qt counterpart of UI/CaptureOptionsView.swift: capture-specific choices,
// shared with the toolbar's Save/Open actions. Save location and
// open-in-app pop-ups end with "Other…" asking for a folder/app (native
// file dialog; the QtQuick.Dialogs module isn't shipped on all targets).
// Any change writes the prefs back through the adapter; the shell's
// "Open Screenshot" action follows the chosen app.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    property var prefs: capture.prefs

    // Draft edits; written back on change (the Mac's onChange retitles the
    // toolbar's "Open Screenshot" item — here the menu action reads prefs).
    function save() {
        capture.setPrefs(JSON.stringify({
            saveLocation: saveLocField.text,
            openInApplication: openAppBox.currentText === qsTr("Default app") ? "" : openAppField.text,
            openFinderAfterCapture: openFinderBox.checked,
            copyOnCapture: copyBox.checked,
            soundEffectsEnabled: soundBox.checked,
            recordBezel: bezelBox.checked
        }));
    }
    function chooseFolder() {
        const path = capture.pickFolder();
        if (path !== "")
            saveLocField.text = path;
        root.save();
    }

    ColumnLayout {
        width: root.width
        spacing: 10

        GroupBox {
            title: qsTr("Screenshots")
            Layout.fillWidth: true
            ColumnLayout {
                width: parent.width
                spacing: 8
                Label { text: qsTr("Save location") }
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: saveLocField
                        objectName: "saveLocationField"
                        Layout.fillWidth: true
                        text: (root.prefs && root.prefs["saveLocation"]) || ""
                        onEditingFinished: root.save()
                    }
                    Button {
                        text: qsTr("Choose…")
                        onClicked: root.chooseFolder()
                    }
                }
                Label { text: qsTr("Open screenshots in") }
                RowLayout {
                    Layout.fillWidth: true
                    ComboBox {
                        id: openAppBox
                        objectName: "openAppBox"
                        Layout.fillWidth: true
                        model: [qsTr("Default app"), qsTr("Other…")]
                        currentIndex: (root.prefs && root.prefs["openInApplication"]) ? 1 : 0
                        onActivated: function (i) {
                            if (i === 1) {
                                const app = capture.pickApplication();
                                if (app !== "") {
                                    openAppField.text = app;
                                    root.save();
                                } else {
                                    currentIndex = 0;
                                }
                            } else {
                                openAppField.text = "";
                                root.save();
                            }
                        }
                    }
                    TextField {
                        id: openAppField
                        visible: openAppBox.currentIndex === 1
                        Layout.preferredWidth: 160
                        text: (root.prefs && root.prefs["openInApplication"]) || ""
                        onEditingFinished: root.save()
                    }
                }
                CheckBox {
                    id: openFinderBox
                    text: qsTr("Show captures in Finder")
                    checked: !!(root.prefs && root.prefs["openFinderAfterCapture"])
                    onToggled: root.save()
                }
                CheckBox {
                    id: copyBox
                    text: qsTr("Copy screenshots to the clipboard")
                    checked: !!(root.prefs && root.prefs["copyOnCapture"])
                    onToggled: root.save()
                }
            }
        }
        GroupBox {
            title: qsTr("Recording")
            Layout.fillWidth: true
            ColumnLayout {
                CheckBox {
                    id: bezelBox
                    text: qsTr("Record device bezel")
                    checked: !!(root.prefs && root.prefs["recordBezel"])
                    onToggled: root.save()
                }
                CheckBox {
                    id: soundBox
                    text: qsTr("Play sound effects")
                    checked: !!(root.prefs && root.prefs["soundEffectsEnabled"])
                    onToggled: root.save()
                }
            }
        }
        Item { Layout.fillHeight: true }
    }

    Component.onCompleted: capture.refreshPrefs()
}
