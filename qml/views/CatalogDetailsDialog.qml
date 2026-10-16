// CatalogDetailsDialog.qml — version details for a catalog entry (surface 9).
//
// Qt counterpart of UI/CatalogDetailsViewController.swift: versions,
// compatibility, install. Sizes to content; shows the installed version;
// install eligibility is rechecked by the emulator endpoint at choose time
// (the Install button calls jobs.start, which revalidates).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    modal: true
    anchors.centerIn: parent
    width: 420
    title: qsTr("Firmware Details")

    property string entryId: ""
    property var details: catalog.details

    function showEntry(id) {
        root.entryId = id;
        catalog.showDetails(id);
        root.open();
    }

    ColumnLayout {
        width: parent.width
        spacing: 10

        Label {
            Layout.fillWidth: true
            font.bold: true
            font.pointSize: 13
            wrapMode: Text.WordWrap
            text: "%1 %2 (%3)".arg(root.details["marketingName"] || "")
                              .arg(root.details["version"] || "")
                              .arg(root.details["build"] || "")
        }
        Label {
            visible: (root.details["installedVersion"] || "") !== ""
            Layout.fillWidth: true
            text: qsTr("Installed: %1").arg(root.details["installedVersion"] || "")
            opacity: 0.7
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Requirements")
            ColumnLayout {
                width: parent.width
                Repeater {
                    model: root.details["requirements"] || []
                    delegate: Label {
                        Layout.fillWidth: true
                        text: "• " + modelData
                        wrapMode: Text.WordWrap
                    }
                }
                Label {
                    visible: (root.details["requirements"] || []).length === 0
                    text: qsTr("None.")
                    opacity: 0.6
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Prepare Steps")
            ColumnLayout {
                width: parent.width
                Repeater {
                    model: root.details["recipeSteps"] || []
                    delegate: Label {
                        Layout.fillWidth: true
                        text: "%1. %2".arg(index + 1).arg(modelData)
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Label {
            visible: (root.details["emulatorNotes"] || "") !== ""
            Layout.fillWidth: true
            text: root.details["emulatorNotes"] || ""
            opacity: 0.65
            wrapMode: Text.WordWrap
            font.pointSize: 9
        }
        Label {
            visible: (root.details["keysState"] || "") !== ""
            Layout.fillWidth: true
            text: qsTr("Keys: %1").arg(root.details["keysState"] || "")
            opacity: 0.65
            font.pointSize: 9
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            Button {
                text: qsTr("Close")
                onClicked: root.close()
            }
            Button {
                objectName: "catalogInstallButton"
                text: qsTr("Download and Prepare")
                enabled: jobs.canDownload
                onClicked: {
                    root.close();
                    jobs.start(root.entryId,
                               JSON.stringify({ jailbreak: false, skipSetup: false, keepStock: true }));
                }
            }
        }
    }
}
