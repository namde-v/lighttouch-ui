// DisplayPlaceholder.qml — device pane placeholder (surface 2 comes later:
// the DisplayItem QQuickItem mapping the shm frame ring will live here).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    padding: 0

    Rectangle {
        anchors.fill: parent
        color: palette.base

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 80, 480)
            spacing: 12

            Label {
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 18
                text: device.currentDeviceUuid !== ""
                      ? qsTr("Device pane")
                      : qsTr("No device selected")
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.6
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                text: device.currentDeviceUuid !== ""
                      ? qsTr("DisplayItem (shm frame ring) lands here in the surface-2 milestone.\nBoot state: %1")
                        .arg(device.bootStageText !== "" ? device.bootStageText : qsTr("idle"))
                      : qsTr("Select a device in the sidebar.")
            }

            // Notice banner (DeviceNotices) — the real banner stack (surface 4)
            // replaces this placeholder.
            Frame {
                Layout.fillWidth: true
                visible: device.noticeVisible
                padding: 8
                ColumnLayout {
                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: device.noticeMessage
                    }
                    RowLayout {
                        Button {
                            text: qsTr("Resolve")
                            visible: device.noticeOperation !== ""
                            onClicked: device.resolveNotice()
                        }
                        Button {
                            text: qsTr("Dismiss")
                            onClicked: device.dismissNotice()
                        }
                    }
                }
            }

            ProgressBar {
                Layout.fillWidth: true
                visible: device.bootFraction >= 0
                from: 0
                to: 1
                value: device.bootFraction
            }
        }
    }
}
