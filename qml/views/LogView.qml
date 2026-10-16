// LogView.qml — live, selectable tail of one log file (surface 7).
//
// Qt counterpart of LogWindowController.swift's LogTextView: the Device Logs
// window's view and the main window's console share this. Shows only the
// lines containing the filter (case-insensitive), like Xcode's console
// filter; selecting text holds updates (the selection check); Clear hides
// what the file held so far.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    property alias filter: filterField.text
    property bool paused: pauseBox.checked

    // Applies the Xcode-style filter over the adapter's lines.
    function shownText() {
        const f = filterField.text.toLowerCase();
        if (f === "")
            return log.lines.join("\n");
        return log.lines.filter(function (l) {
            return l.toLowerCase().indexOf(f) >= 0;
        }).join("\n");
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8
        Layout.topMargin: 4
        Layout.bottomMargin: 4
        spacing: 8

        ComboBox {
            id: sourcePicker
            objectName: "logSourcePicker"
            model: log.sources
            currentIndex: Math.max(0, log.sources.indexOf(log.currentSource))
            onActivated: function (i) { log.currentSource = log.sources[i]; }
            ToolTip.text: qsTr("Log file")
            ToolTip.visible: hovered
        }
        CheckBox {
            id: pauseBox
            objectName: "logPauseBox"
            text: qsTr("Pause updates")
            checked: !log.following
            onToggled: log.following = !checked
        }
        Item { Layout.fillWidth: true }
        TextField {
            id: filterField
            objectName: "logFilterField"
            Layout.preferredWidth: 180
            placeholderText: qsTr("Filter")
        }
        Button {
            objectName: "logClearButton"
            text: qsTr("Clear")
            onClicked: log.clear()
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        TextArea {
            id: logText
            objectName: "logTextArea"
            readOnly: true
            selectByMouse: true
            font.family: "monospace"
            font.pointSize: 10
            text: root.shownText()
            // Selecting text holds updates (Mac: selectedRange().length > 0
            // pauses the poll); follow the end otherwise.
            onTextChanged: {
                if (logText.selectedText === "")
                    logText.cursorPosition = logText.length;
            }
        }
    }

    Component.onCompleted: {
        log.refreshSources();
        log.read(0);
        log.following = true;
    }
    Component.onDestruction: {
        log.following = false;
    }
}
