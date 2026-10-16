// NetworkAccessDialog.qml — first-start network question (surface 10).
//
// Qt counterpart of App/NetworkAccessPrompt.swift: "Connect your <device> to
// the internet?" Asked once (no saved answer); the answer is saved and the
// pending boot goes ahead. Changeable later in Device ▸ Network.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("Network Access")
    modal: true
    width: 440
    standardButtons: Dialog.NoButton

    property string deviceName: ""
    // Set by the opener: called after the answer is saved.
    property var onAnswered: null

    function answer(connect) {
        settings.networkAccessAnswer = connect ? "always" : "never";
        root.close();
        if (root.onAnswered)
            root.onAnswered(connect);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.bold: true
            font.pointSize: 12
            text: qsTr("Connect your %1 to the internet?").arg(
                root.deviceName !== "" ? root.deviceName : qsTr("device"))
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Your %1 can use this computer's internet connection. You can change this later in the Device menu.")
                .arg(root.deviceName !== "" ? root.deviceName : qsTr("device"))
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Button {
                text: qsTr("Use Offline")
                onClicked: root.answer(false)
            }
            Button {
                text: qsTr("Connect")
                onClicked: root.answer(true)
            }
        }
    }
}
