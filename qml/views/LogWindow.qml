// LogWindow.qml — the Device Logs window (surface 7).
//
// Qt counterpart of UI/LogWindowController.swift: a live, selectable tail
// of one log file in its own window (720×440, title "Device Logs").

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 720
    height: 440
    minimumWidth: 420
    minimumHeight: 250
    title: qsTr("Device Logs")
    visible: false

    LogView {
        anchors.fill: parent
    }
}
