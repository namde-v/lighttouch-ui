// TweaksPanel.qml — Device ▸ Tweaks… (surface 4).
//
// Qt counterpart of UI/TweaksPanel.swift: one device's hidden switches by
// section, each row a switch with its caption; rows stay visible but disabled
// on firmware lacking the tweak. The color/clock sub-options appear under an
// enabled tweak (Mac: Picker for Core Animation colors, DatePicker + "Macworld
// 2007" for the clock). Hidden Apps section: spinner while listing, the
// "start the device" hint when stopped, Open buttons per app.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    // The device UUID this panel edits; the adapter re-reads on change.
    property string deviceUuid: tweaks.deviceUuid

    // Sections in first-seen order: [{name, rows:[tweak]}].
    property var sections: {
        const byName = {};
        const order = [];
        for (let i = 0; i < tweaks.tweaks.length; ++i) {
            const t = tweaks.tweaks[i];
            const name = t["section"] || qsTr("General");
            if (!(name in byName)) {
                byName[name] = [];
                order.push(name);
            }
            byName[name].push(t);
        }
        return order.map(function (n) { return { name: n, rows: byName[n] }; });
    }

    // Core Animation color names (CoreAnimationColor.allCases titles).
    readonly property var colorNames: [
        qsTr("Red"), qsTr("Green"), qsTr("Blue"), qsTr("Yellow")
    ]

    function toggleTweak(tweak, on) {
        tweaks.setTweak(tweak["id"], on);
        tweaks.refresh();
    }

    ColumnLayout {
        width: root.width
        spacing: 12

        Repeater {
            model: root.sections
            delegate: GroupBox {
                Layout.fillWidth: true
                title: modelData.name
                ColumnLayout {
                    width: parent.width
                    spacing: 6
                    Repeater {
                        model: modelData.rows
                        delegate: ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Label {
                                        text: modelData["title"] || modelData["id"]
                                        font.bold: true
                                    }
                                    Label {
                                        text: modelData["caption"] || ""
                                        opacity: 0.65
                                        font.pointSize: 9
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                                Switch {
                                    objectName: "tweakSwitch-" + modelData["id"]
                                    checked: modelData["on"] === true
                                    enabled: modelData["available"] === true
                                    onToggled: root.toggleTweak(modelData, checked)
                                }
                            }
                            // Sub-options under an enabled tweak.
                            ComboBox {
                                visible: modelData["id"] === "coreAnimationColors"
                                         && modelData["on"] === true && modelData["available"] === true
                                Layout.fillWidth: true
                                Layout.leftMargin: 16
                                model: root.colorNames
                                onActivated: function (i) { tweaks.setColor(i); }
                            }
                            RowLayout {
                                visible: modelData["id"] === "timeMachine"
                                         && modelData["on"] === true && modelData["available"] === true
                                Layout.fillWidth: true
                                Layout.leftMargin: 16
                                Button {
                                    text: qsTr("Macworld 2007")
                                    onClicked: tweaks.setClock(1167609600) // 2007-01-09
                                }
                                Label {
                                    text: qsTr("Pins the guest clock to the keynote.")
                                    opacity: 0.6
                                    font.pointSize: 9
                                }
                            }
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Hidden Apps")
            ColumnLayout {
                width: parent.width
                spacing: 6
                Label {
                    visible: !tweaks.running
                    text: qsTr("Start the device to open the apps its Home screen hides.")
                    opacity: 0.65
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    visible: tweaks.running && tweaks.hiddenApps.length === 0
                    spacing: 6
                    BusyIndicator { running: true; implicitWidth: 16; implicitHeight: 16 }
                    Label { text: qsTr("Listing…"); opacity: 0.65 }
                }
                Repeater {
                    model: tweaks.hiddenApps
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: modelData["name"] || modelData["bundleId"] || ""
                            Layout.fillWidth: true
                        }
                        Button {
                            text: qsTr("Open")
                            onClicked: tweaks.openApp(modelData["bundleId"] || "")
                        }
                    }
                }
            }
        }

        Label {
            visible: tweaks.message !== ""
            text: tweaks.message
            opacity: 0.65
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Component.onCompleted: {
        tweaks.refresh();
        if (tweaks.running)
            tweaks.listHiddenApps();
    }
}
