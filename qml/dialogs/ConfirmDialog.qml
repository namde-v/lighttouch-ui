// ConfirmDialog.qml — generic window-modal confirmation (surface 1).
// Used for Shut Down… / Force Stop… and other guarded device commands.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    modal: true
    anchors.centerIn: parent
    width: 400

    property string informativeText: ""
    property string confirmText: qsTr("OK")
    property var onAccept: null

    function ask(title, info, confirm, acceptFn) {
        root.title = title;
        root.informativeText = info;
        root.confirmText = confirm;
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
                objectName: "confirmOkButton"
                text: root.confirmText
                highlighted: true
                onClicked: {
                    root.close();
                    if (root.onAccept)
                        root.onAccept();
                }
            }
        }
    }
}
