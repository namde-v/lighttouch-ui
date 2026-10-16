// ConsoleSplit.qml — the main pane's bottom console (surface 7).
//
// Qt counterpart of UI/ConsoleSplit.swift, after Xcode's debug area: the
// device on top, the console below, and the bar between them is the divider:
// drag it, double-click it, or use its toggle. With the console hidden only
// the 30 pt bar shows, so the device keeps the pane. The bar carries the
// log source picker, filter, and clear (Xcode's console footer); the chosen
// height is saved in QSettings and restored (ConsoleSplitLayout's autosave).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    property bool expanded: true
    property int consoleHeight: 180
    property bool dragging: false
    property int dragStartY: 0
    property int dragStartHeight: 0

    function toggle() {
        root.expanded = !root.expanded;
        settings.setConsoleHeight(root.expanded ? root.consoleHeight : 0);
    }
    function commitHeight() {
        settings.setConsoleHeight(root.expanded ? root.consoleHeight : 0);
    }

    // --- The bar: divider + toggle + console controls ---
    ToolBar {
        id: bar
        objectName: "consoleBar"
        Layout.fillWidth: true
        Layout.preferredHeight: 30
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 8
            spacing: 8

            Button {
                id: toggleButton
                objectName: "consoleToggle"
                flat: !root.expanded
                text: root.expanded ? "▾" : "▴"
                ToolTip.text: root.expanded ? qsTr("Hide Console (Shift+Ctrl+Y)") : qsTr("Show Console (Shift+Ctrl+Y)")
                ToolTip.visible: hovered
                onClicked: root.toggle()
            }
            ComboBox {
                id: sourcePicker
                visible: root.expanded
                model: log.sources
                currentIndex: Math.max(0, log.sources.indexOf(log.currentSource))
                onActivated: function (i) { log.currentSource = log.sources[i]; }
                ToolTip.text: qsTr("Log")
                ToolTip.visible: hovered
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: filterField
                visible: root.expanded
                Layout.preferredWidth: 180
                placeholderText: qsTr("Filter")
                onTextChanged: logView.filter = text
            }
            Button {
                visible: root.expanded
                text: qsTr("Clear")
                onClicked: log.clear()
            }
        }
        // The rest of the strip is the divider's grab area: drag resizes,
        // double-click toggles.
        MouseArea {
            anchors.fill: parent
            z: -1
            cursorShape: Qt.SizeVerCursor
            acceptedButtons: Qt.LeftButton
            onPressed: function (mouse) {
                root.dragging = true;
                root.dragStartY = mouse.y;
                root.dragStartHeight = root.consoleHeight;
            }
            onPositionChanged: function (mouse) {
                if (root.dragging) {
                    root.consoleHeight = Math.max(80, root.dragStartHeight + (root.dragStartY - mouse.y));
                    if (!root.expanded)
                        root.expanded = true;
                }
            }
            onReleased: {
                root.dragging = false;
                root.commitHeight();
            }
            onDoubleClicked: root.toggle()
        }
    }

    // --- The console itself ---
    ScrollView {
        id: consoleScroll
        Layout.fillWidth: true
        Layout.preferredHeight: root.expanded ? root.consoleHeight : 0
        visible: root.expanded
        clip: true
        TextArea {
            id: logView
            objectName: "consoleTextArea"
            property string filter: ""
            readOnly: true
            selectByMouse: true
            font.family: "monospace"
            font.pointSize: 10
            text: {
                const f = logView.filter.toLowerCase();
                if (f === "")
                    return log.lines.join("\n");
                return log.lines.filter(function (l) {
                    return l.toLowerCase().indexOf(f) >= 0;
                }).join("\n");
            }
        }
    }

    Component.onCompleted: {
        const h = settings.consoleHeight();
        if (h >= 0) {
            root.expanded = h > 0;
            if (h > 0)
                root.consoleHeight = h;
        }
        log.refreshSources();
        log.read(0);
        log.following = true;
    }
}
