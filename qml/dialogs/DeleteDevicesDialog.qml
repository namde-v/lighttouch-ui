// DeleteDevicesDialog.qml — the guarded device-delete question (surface 1).
//
// Mac copy (DeviceLibraryViewController.batchAlert): "Delete “Name”?" /
// "Delete N devices?", informative text names the skipped running/busy rows,
// Delete is destructive, Cancel is the safe default. Shown as a window-modal
// dialog, like the Mac's sheet.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    modal: true
    title: qsTr("Delete device?")
    anchors.centerIn: parent
    width: 420

    property string informativeText: ""
    // Called when the user confirms; set by the asker before open().
    property var onAccept: null

    function askDelete(title, info, acceptFn) {
        root.title = title;
        root.informativeText = info;
        root.onAccept = acceptFn;
        root.open();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: root.informativeText
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Button {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
            Button {
                id: deleteButton
                objectName: "deleteConfirmButton"
                text: qsTr("Delete")
                // Destructive styling: the Mac marks the Delete button
                // hasDestructiveAction.
                highlighted: true
                palette.button: "#c93a3a"
                palette.buttonText: "white"
                onClicked: {
                    root.close();
                    if (root.onAccept)
                        root.onAccept();
                }
            }
        }
    }
}
