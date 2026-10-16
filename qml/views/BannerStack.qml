// BannerStack.qml — transient banner stack pinned to the bottom of the
// device scene (surface 1). Qt counterpart of UI/CaptureStatusView.swift.
//
// Banners float over the scene and never inside window captures. Each banner
// carries an optional primary/secondary text button (e.g. Apply/Revert, Show
// Logs, Cancel). Reused by surfaces 1 (device notice, startup toast), 6
// (file-transfer status) and 8 (capture notice).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 8

    // Banner model: array of { id, text, primaryText, secondaryText }.
    property var banners: []

    signal primaryClicked(string bannerId)
    signal secondaryClicked(string bannerId)

    Repeater {
        model: root.banners
        delegate: Frame {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: parent.width - 32
            padding: 10
            // Glass look: translucent dark, like the Mac's glass banner.
            background: Rectangle {
                color: "#2b2f36"
                opacity: 0.92
                radius: 10
                border.color: "#ffffff"
                border.width: 1
            }
            RowLayout {
                spacing: 10
                Label {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 420
                    wrapMode: Text.WordWrap
                    color: "white"
                    text: modelData["text"] || ""
                }
                Button {
                    visible: (modelData["primaryText"] || "") !== ""
                    text: modelData["primaryText"] || ""
                    flat: true
                    palette.buttonText: "#0a84ff"
                    onClicked: root.primaryClicked(modelData["id"])
                }
                Button {
                    visible: (modelData["secondaryText"] || "") !== ""
                    text: modelData["secondaryText"] || ""
                    flat: true
                    onClicked: root.secondaryClicked(modelData["id"])
                }
            }
        }
    }
}
