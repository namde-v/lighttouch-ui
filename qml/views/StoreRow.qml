// StoreRow.qml — one Legacy Store result row (surface 5).
//
// Install button per row; icons warm in row-by-row (content-addressed, never
// stale — here a placeholder until the icon provider lands); double-click
// installs.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 56
    color: "transparent"

    property var app: null
    signal install()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        Rectangle {
            width: 40; height: 40; radius: 9
            color: palette.mid
            Label {
                anchors.centerIn: parent
                text: (root.app && root.app["name"] || "?").charAt(0)
                font.pointSize: 16
                color: palette.highlightedText
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Label {
                text: (root.app && root.app["name"]) || ""
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Label {
                text: (root.app && (root.app["version"] || root.app["subtitle"])) || ""
                opacity: 0.6
                font.pointSize: 9
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Button {
            objectName: "storeInstallButton"
            text: qsTr("Install")
            onClicked: root.install()
        }
    }

    MouseArea {
        anchors.fill: parent
        onDoubleClicked: root.install()
    }
}
