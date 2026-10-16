// AppRow.qml — one installed-app / install-job row (surface 5).
//
// The Mac draws these (AppRowCells.swift) over AppsInspectorRows; all
// text/progress comes from the core model. 56 pt rows, install rows carry
// the job's own progress bar, right-click opens the row menu.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 56
    color: selected ? palette.highlight : "transparent"

    property var row: null
    property bool selected: false
    signal clicked()
    signal contextMenu(int x, int y)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        // Icon placeholder (AppIconAdapter content-addresses real icons in
        // the full build; the mock has none).
        Rectangle {
            width: 40; height: 40; radius: 9
            color: palette.mid
            Label {
                anchors.centerIn: parent
                text: (root.row && root.row["title"] || "?").charAt(0)
                font.pointSize: 16
                color: palette.highlightedText
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Label {
                text: (root.row && root.row["title"]) || ""
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
                color: root.selected ? palette.highlightedText : palette.text
            }
            Label {
                text: (root.row && root.row["subtitle"]) || ""
                opacity: 0.6
                font.pointSize: 9
                elide: Text.ElideRight
                Layout.fillWidth: true
                color: root.selected ? palette.highlightedText : palette.text
            }
            ProgressBar {
                visible: root.row && root.row["kind"] === "install"
                Layout.fillWidth: true
                from: 0; to: 1
                value: (root.row && root.row["progress"]) || 0
            }
        }

        Label {
            visible: root.row && root.row["kind"] === "install"
            text: qsTr("%1%").arg(Math.round(((root.row && root.row["progress"]) || 0) * 100))
            opacity: 0.7
            font.pointSize: 9
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: function (mouse) {
            if (mouse.button === Qt.RightButton)
                root.contextMenu(mouse.x, mouse.y);
            else
                root.clicked();
        }
        onPressAndHold: function (mouse) { root.contextMenu(mouse.x, mouse.y); }
    }
}
