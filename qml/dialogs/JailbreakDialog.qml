// JailbreakDialog.qml — Device ▸ Jailbreak…'s question (surface 10).
//
// Qt counterpart of UI/JailbreakAlert.swift: what the jailbreak adds to the
// device (Cydia + Substrate on iOS 3–6, afc2) and that Erase removes it.
// The copy varies by iOS major version, exactly like the Mac.
//
// NOTE: the confirm path is blocked — DeviceFilesystemEdits.perform(.jailbreak)
// has no lt_ export in the ABI yet. The dialog is built and tested; wiring
// the Jailbreak button is one line when the core lands the export.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    modal: true
    width: 440
    standardButtons: Dialog.NoButton

    property string deviceName: ""
    property string version: ""   // e.g. "5.1.1"

    // Mirrors JailbreakAlert.make's major-version copy rule.
    function informativeText() {
        const major = parseInt((root.version.split(".")[0] || "0"), 10) || 0;
        const adds = (major >= 3 && major <= 6)
            ? qsTr("Adds Cydia, Substrate, and afc2, which shows the whole file system in Files.")
            : qsTr("Adds Cydia and afc2, which shows the whole file system in Files.");
        return adds + " " + qsTr("Erase All Content and Settings removes them.");
    }

    signal jailbreakConfirmed()

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.bold: true
            font.pointSize: 12
            text: qsTr("Jailbreak %1?").arg(root.deviceName)
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: root.informativeText()
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Button {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
            Button {
                text: qsTr("Jailbreak")
                enabled: false   // blocked: no lt_device_jailbreak in the ABI
                onClicked: { root.close(); root.jailbreakConfirmed(); }
            }
        }
    }
}
