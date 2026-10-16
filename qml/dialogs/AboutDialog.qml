// AboutDialog.qml — About LightTouch (surface 10).
//
// Qt counterpart of App/AboutPanel.swift: the standard panel's credits
// (AboutCredits runs: headings, detail lines, links) with "Show Licenses"
// opening the Licenses window. The Mac's panel opens links itself; here the
// licenses link is a button.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("About LightTouch")
    modal: true
    width: 420
    standardButtons: Dialog.Ok

    property bool hasLicenses: {
        const runs = settings.aboutCredits["credits"] || [];
        for (let i = 0; i < runs.length; ++i)
            if (runs[i]["link"] === "lighttouch:licenses")
                return true;
        return false;
    }

    signal showLicenses()

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        Label {
            text: qsTr("LightTouch %1").arg(settings.version)
            font.pointSize: 14
            font.bold: true
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            clip: true
            ColumnLayout {
                width: parent.width
                spacing: 2
                Repeater {
                    model: settings.aboutCredits["credits"] || []
                    delegate: Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: modelData["text"] || ""
                        font.bold: !!modelData["isHeading"]
                        opacity: modelData["isDetail"] ? 0.7 : 1.0
                        font.pointSize: modelData["isHeading"] ? 11 : 9
                    }
                }
            }
        }
        Button {
            text: qsTr("Show Licenses")
            visible: root.hasLicenses
            onClicked: { root.close(); root.showLicenses(); }
        }
    }
}
