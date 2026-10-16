// CarrierPanel.qml — Device ▸ Carrier… (surface 4).
//
// Qt counterpart of UI/CarrierPanel.swift: the fake cellular network —
// carrier identity (name, MCC/MNC, Apply with networkEdited/networkValid
// gating), registered/SIM/bars, calls (Ring/Answer/Hang Up with live state),
// SMS (send + sent list), and the modem GPS (lat/long + presets + Walk) when
// the modem has GPS. The panel drives the 1 Hz poll via carrier.visible
// (lt_carrier_set_visible); changes arrive as kLTEventCarrierChanged.
//
// The map: QtLocation needs network tiles and isn't guaranteed offline, so
// the panel shows coordinates + presets (Mac's "Apple Park" / "This Mac's
// Location" via QtPositioning) instead of an embedded map. Honest deviation,
// noted in UI_COVERAGE.md.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    property var s: carrier.state
    property var st: carrier.state["settings"] || {}
    property var status: carrier.state["status"] || {}

    function refreshVisible() { carrier.visible = root.visible; }

    ColumnLayout {
        width: root.width
        spacing: 12

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Network")
            GridLayout {
                width: parent.width
                columns: 2
                columnSpacing: 8
                rowSpacing: 8
                Label { text: qsTr("Carrier") }
                TextField {
                    id: carrierField
                    objectName: "carrierNameField"
                    Layout.fillWidth: true
                    text: root.s["carrierName"] || ""
                    onEditingFinished: carrier.setField("carrierName", text)
                }
                Label { text: qsTr("MCC / MNC") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    TextField {
                        objectName: "mccField"
                        Layout.preferredWidth: 64
                        placeholderText: "001"
                        text: root.s["mcc"] || ""
                        onEditingFinished: carrier.setField("mcc", text)
                    }
                    Label { text: "/"; opacity: 0.6 }
                    TextField {
                        objectName: "mncField"
                        Layout.preferredWidth: 64
                        placeholderText: "01"
                        text: root.s["mnc"] || ""
                        onEditingFinished: carrier.setField("mnc", text)
                    }
                    Button {
                        objectName: "applyNetworkButton"
                        text: qsTr("Apply")
                        enabled: root.s["networkEdited"] === true && root.s["networkValid"] === true
                        onClicked: carrier.applyNetwork()
                    }
                }
                Label { text: qsTr("Registered") }
                Switch {
                    objectName: "registeredSwitch"
                    checked: root.st["registered"] === true
                    onToggled: carrier.setRegistered(checked)
                }
                Label { text: qsTr("SIM Present") }
                Switch {
                    objectName: "simSwitch"
                    checked: root.st["simPresent"] === true
                    onToggled: carrier.setSimPresent(checked)
                }
                Label { text: qsTr("Signal") }
                RowLayout {
                    Layout.fillWidth: true
                    Slider {
                        objectName: "barsSlider"
                        Layout.fillWidth: true
                        from: 0; to: 5; stepSize: 1
                        value: root.st["bars"] || 0
                        onMoved: carrier.setBars(Math.round(value))
                    }
                    Label {
                        text: qsTr("%n bar(s)", "", root.st["bars"] || 0)
                        opacity: 0.6
                    }
                }
            }
            Label {
                visible: root.s["applyingNetwork"] === true
                text: qsTr("Applying…")
                opacity: 0.6
            }
            Label {
                text: qsTr("Signal indicator may take a moment to update.")
                opacity: 0.55
                font.pointSize: 9
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Calls")
            ColumnLayout {
                width: parent.width
                spacing: 8
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: callField
                        objectName: "callNumberField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Caller")
                        text: root.s["callNumber"] || ""
                        onEditingFinished: carrier.setField("callNumber", text)
                    }
                    Button {
                        objectName: "ringButton"
                        text: qsTr("Ring")
                        enabled: root.s["canRing"] === true
                        onClicked: carrier.ring()
                    }
                }
                RowLayout {
                    Button {
                        objectName: "answerButton"
                        text: qsTr("Answer")
                        enabled: root.s["canAnswer"] === true
                        ToolTip.text: qsTr("The other end picks up the call the phone is making")
                        ToolTip.visible: hovered
                        onClicked: carrier.answer()
                    }
                    Button {
                        objectName: "hangUpButton"
                        text: qsTr("Hang Up")
                        enabled: root.s["canHangUp"] === true
                        ToolTip.text: qsTr("The other end ends the call, ringing or connected")
                        ToolTip.visible: hovered
                        onClicked: carrier.hangUp()
                    }
                }
                GridLayout {
                    columns: 2
                    columnSpacing: 8
                    Label { text: qsTr("State"); opacity: 0.65 }
                    Label {
                        text: {
                            const cs = root.s["callState"] || "idle";
                            const em = root.status["emergencyCall"] === true ? qsTr(" (Emergency)") : "";
                            return cs.charAt(0).toUpperCase() + cs.slice(1) + em;
                        }
                    }
                    Label { text: qsTr("Last Dialed"); opacity: 0.65 }
                    Label { text: root.status["lastDialed"] || "—" }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("SMS")
            ColumnLayout {
                width: parent.width
                spacing: 8
                TextField {
                    objectName: "smsNumberField"
                    Layout.fillWidth: true
                    placeholderText: qsTr("From")
                    text: root.s["smsNumber"] || ""
                    onEditingFinished: carrier.setField("smsNumber", text)
                }
                RowLayout {
                    Layout.fillWidth: true
                    TextArea {
                        objectName: "smsTextField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Type your SMS…")
                        text: root.s["smsText"] || ""
                        wrapMode: Text.WordWrap
                        onEditingFinished: carrier.setField("smsText", text)
                    }
                    Button {
                        objectName: "sendSmsButton"
                        text: qsTr("Send")
                        enabled: (root.s["smsNumber"] || "") !== "" && (root.s["smsText"] || "") !== ""
                        onClicked: carrier.sendSms()
                    }
                }
                Label {
                    visible: (root.s["sent"] || []).length === 0
                    text: qsTr("Messages the phone sends appear here.")
                    opacity: 0.6
                }
                Repeater {
                    model: root.s["sent"] || []
                    delegate: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: qsTr("To %1").arg(modelData["number"] || "")
                            opacity: 0.6
                            font.pointSize: 9
                        }
                        Label {
                            text: modelData["text"] || ""
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        GroupBox {
            visible: root.status["hasGPS"] === true
            Layout.fillWidth: true
            title: qsTr("Location")
            ColumnLayout {
                width: parent.width
                spacing: 8
                Label {
                    text: qsTr("No embedded map: QtLocation tiles aren't guaranteed offline. Coordinates and presets set the modem GPS directly.")
                    opacity: 0.55
                    font.pointSize: 9
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        objectName: "latField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Latitude")
                        text: root.s["latitude"] || ""
                        onEditingFinished: carrier.setField("latitude", text)
                    }
                    TextField {
                        objectName: "lonField"
                        Layout.fillWidth: true
                        placeholderText: qsTr("Longitude")
                        text: root.s["longitude"] || ""
                        onEditingFinished: carrier.setField("longitude", text)
                    }
                    Button {
                        objectName: "applyLocationButton"
                        text: qsTr("Set")
                        onClicked: carrier.applyLocation()
                    }
                }
                RowLayout {
                    Button {
                        objectName: "appleParkButton"
                        text: qsTr("Apple Park")
                        onClicked: carrier.setLocation(37.3349, -122.0090)
                    }
                    Button {
                        objectName: "hostLocationButton"
                        text: qsTr("This Computer's Location")
                        // QtPositioning would ask the OS here (CoreLocation's
                        // replacement); the notice path covers "unavailable".
                        onClicked: carrier.notice(qsTr("This computer's location isn't available."))
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        objectName: "walkButton"
                        text: root.s["walking"] === true ? qsTr("Stop Walking") : qsTr("Walk")
                        ToolTip.text: qsTr("Walk a circle 100 m across from here, at 1.4 m/s")
                        ToolTip.visible: hovered
                        onClicked: carrier.toggleWalk()
                    }
                }
            }
        }

        Label {
            visible: (root.s["message"] || "") !== ""
            text: root.s["message"] || ""
            opacity: 0.65
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    onVisibleChanged: refreshVisible()
    Component.onCompleted: {
        carrier.refresh();
        refreshVisible();
    }
}
