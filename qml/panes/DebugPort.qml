// DebugPort.qml — Device ▸ Debugging ▸ Debug Port… (surface 4).
//
// Qt counterpart of UI/DebugPortView.swift: the emulator's GDB remote stub
// as a sheet — its switch, where it is (the port this boot listens on, null
// when it started without one), what it is, and the attach commands, each
// with a Copy button. Done is the default action.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 12

    property var st: debugPort.state
    property bool stubEnabled: st["enabled"] === true
    // Nullable int: -1 when the boot started without a port.
    property int port: st["port"] === null || st["port"] === undefined ? -1 : st["port"]
    property string shortName: st["shortName"] || ""

    // The attach commands (DebugPortText.commands): plain lldb + kernel
    // symbols when available.
    property var commands: {
        const cmds = [];
        if (root.port >= 0) {
            cmds.push({
                title: qsTr("Attach with lldb"),
                command: "lldb -o 'gdb-remote localhost:%1'".arg(root.port)
            });
            if (root.st["lldbWithSymbols"])
                cmds.push({
                    title: qsTr("Attach with kernel symbols"),
                    command: root.st["lldbWithSymbols"]
                });
        }
        return cmds;
    }

    Switch {
        id: enableSwitch
        objectName: "debugPortSwitch"
        text: qsTr("Debug Port")
        font.bold: true
        checked: root.stubEnabled
        onToggled: debugPort.setEnabled(checked)
    }
    Label {
        text: root.port >= 0
              ? qsTr("Listening on localhost:%1.").arg(root.port)
              : qsTr("The device did not start with a debug port; enable it and restart.")
        opacity: 0.65
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }
    Label {
        text: qsTr("The emulator's GDB remote stub. A debugger attached to it runs the %1's CPU: it pauses the whole device and reads or changes its kernel and apps. Only this computer can reach it, and it has no password.").arg(root.shortName)
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }
    Repeater {
        model: root.commands
        delegate: ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label {
                text: modelData.title
                opacity: 0.65
                font.pointSize: 9
            }
            RowLayout {
                Layout.fillWidth: true
                TextArea {
                    Layout.fillWidth: true
                    text: modelData.command
                    readOnly: true
                    selectByMouse: true
                    font.family: "monospace"
                    wrapMode: Text.WordWrap
                }
                Button {
                    objectName: "copyDebugCommand-" + index
                    text: qsTr("Copy")
                    onClicked: debugPort.copyCommand(modelData.command)
                }
            }
        }
    }

    Component.onCompleted: debugPort.refresh()
}
