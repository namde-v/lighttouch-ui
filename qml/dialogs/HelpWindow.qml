// HelpWindow.qml — "Light Touch Help" (surface 10).
//
// Qt counterpart of UI/HelpWindowController.swift: Help.txt's task topics in
// a sidebar, the chosen topic beside it, 780×560. "[Device]" placeholders
// re-render with the current device name, keeping the topic; the text has a
// find field.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    title: qsTr("Light Touch Help")
    width: 780
    height: 560
    minimumWidth: 520
    minimumHeight: 300
    visible: false

    property string deviceName: ""
    property var topics: settings.helpTopics()
    property int currentIndex: 0
    property string findText: ""

    function shownBody(topic) {
        const name = root.deviceName !== "" ? root.deviceName : qsTr("device");
        return (topic["body"] || "").split("[Device]").join(name);
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        ListView {
            id: topicList
            SplitView.preferredWidth: 230
            SplitView.minimumWidth: 180
            SplitView.maximumWidth: 300
            clip: true
            model: root.topics
            delegate: ItemDelegate {
                width: topicList.width
                text: modelData["title"] || ""
                highlighted: index === root.currentIndex
                onClicked: root.currentIndex = index
            }
        }

        ColumnLayout {
            SplitView.fillWidth: true
            spacing: 0

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                TextArea {
                    id: bodyText
                    readOnly: true
                    selectByMouse: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                    font.pointSize: 11
                    leftPadding: 24
                    rightPadding: 24
                    topPadding: 20
                    text: {
                        const t = root.topics[root.currentIndex];
                        return t ? (t["title"] || "") + "\n\n" + root.shownBody(t) : "";
                    }
                    // Jump to the first find hit.
                    onTextChanged: {
                        if (root.findText !== "") {
                            const i = text.toLowerCase().indexOf(root.findText.toLowerCase());
                            if (i >= 0)
                                bodyText.select(i, i + root.findText.length);
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Label { text: qsTr("Find:") ; opacity: 0.7 }
                TextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search this topic")
                    onTextChanged: root.findText = text
                }
            }
        }
    }
}
