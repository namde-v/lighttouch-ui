// LicensesDialog.qml — the Licenses window (surface 10).
//
// Qt counterpart of AboutPanel.swift's LicensesWindow: each bundled
// component's license text (from lt_about_credits), sidebar list on the
// left, selectable monospace text on the right. Non-modal, like the Mac.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    title: qsTr("Licenses")
    width: 860
    height: 560
    minimumWidth: 520
    minimumHeight: 300
    visible: false

    property var licenses: settings.aboutCredits["licenses"] || []
    property int currentIndex: 0

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        ListView {
            id: licenseList
            SplitView.preferredWidth: 220
            SplitView.minimumWidth: 180
            clip: true
            model: root.licenses
            delegate: ItemDelegate {
                width: licenseList.width
                text: modelData["name"] || ""
                highlighted: index === root.currentIndex
                onClicked: root.currentIndex = index
            }
        }

        ScrollView {
            SplitView.fillWidth: true
            clip: true
            TextArea {
                readOnly: true
                selectByMouse: true
                font.family: "Monospace"
                font.pointSize: 9
                wrapMode: Text.WordWrap
                text: {
                    const l = root.licenses[root.currentIndex];
                    return l ? (l["text"] || "") : "";
                }
            }
        }
    }
}
