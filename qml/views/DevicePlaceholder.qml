// DevicePlaceholder.qml — the detail area for a device not running in this
// window (surface 1). Qt counterpart of UI/DevicePlaceholderViewController.swift.
//
// Dimmed device art, a state line, and one primary action. Each job keeps its
// own progress bar (no cross-animation when switching rows): the bar binds to
// the job matching this row's deviceUuid. Bar caption = percent + time-left +
// detail. Prepare options (Skip Setup / Jailbreak) live in the Add Device
// sheet; rows already here were prepared with their choices baked in.
// The whole pane is an IPSW drop target; the info button pops the
// catalog note.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    padding: 0

    // kLTDeviceRowState… mirrors src/lt_bridge.h §6.
    readonly property int rowOff: 1
    readonly property int rowRunning: 2
    readonly property int rowPaused: 3
    readonly property int rowBooting: 4
    readonly property int rowError: 5

    // The selected row's DeviceRow map (id/name/board/firmware/status/…).
    property var row: null
    // The firmware job for this device, if any (jobs adapter row map).
    property var job: null

    signal addDeviceRequested()

    // One primary action (Mac DevicePlaceholderViewController).
    readonly property string primaryText: {
        if (!root.row)
            return "";
        if (root.job)
            return ""; // the job's own bar + Cancel are the UI
        const st = root.row["status"];
        if (st === root.rowOff || st === root.rowPaused)
            return qsTr("Start");
        if (root.row["prepared"] === false)
            return qsTr("Download and Prepare");
        return "";
    }
    readonly property bool primaryEnabled: {
        if (!root.row || root.job)
            return false;
        if (root.row["prepared"] === false)
            return false; // needs the catalog (surface 9)
        return true;
    }
    readonly property string primaryTip: {
        if (root.row && root.row["prepared"] === false && !root.job)
            return qsTr("The firmware catalog arrives with surface 9");
        return "";
    }
    function runPrimary() {
        if (root.primaryText === qsTr("Start") && root.row)
            deviceList.bootDevice(root.row["id"]);
    }

    DropArea {
        anchors.fill: parent
        onEntered: function (drag) {
            drag.accepted = root.hasIpsw(drag);
        }
        onDropped: function (drop) {
            const ipsws = [];
            for (let i = 0; i < drop.urls.length; ++i) {
                if (deviceList.classifyDropUrl(drop.urls[i]) === 0)
                    ipsws.push(drop.urls[i]);
            }
            if (ipsws.length > 0)
                jobs.importIpsws(ipsws);
        }
    }
    function hasIpsw(drag) {
        if (!drag.hasUrls)
            return false;
        for (let i = 0; i < drag.urls.length; ++i) {
            if (deviceList.classifyDropUrl(drag.urls[i]) === 0)
                return true;
        }
        return false;
    }

    Rectangle {
        anchors.fill: parent
        color: palette.base

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 120, 420)
            spacing: 10

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 96
                Layout.preferredHeight: 96
                source: root.row ? deviceIcon.iconSource(root.row["board"] || "") : ""
                fillMode: Image.PreserveAspectFit
                opacity: 0.35 // dimmed art, like the Mac's placeholder
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: 16
                font.bold: true
                elide: Text.ElideRight
                text: root.row ? (root.row["name"] || "") : ""
            }

            Label {
                id: stateLine
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.7
                text: root.row ? (root.row["statusText"] || "") : ""
            }

            // The job's own progress bar (Mac: percent + time-left + speed;
            // never animates between values when switching rows — each row's
            // bar binds to its own job object).
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.job !== null && !(root.job && root.job["failed"])
                spacing: 4
                ProgressBar {
                    id: jobBar
                    objectName: "jobProgressBar"
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: {
                        const p = root.job && root.job["preparation"];
                        const f = p ? p["fraction"] : (root.job ? root.job["progress"] : 0);
                        return f === undefined || f === null || f < 0 ? 0 : f;
                    }
                }
                Label {
                    id: jobCaption
                    objectName: "jobCaption"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 9
                    opacity: 0.7
                    text: {
                        if (!root.job)
                            return "";
                        const p = root.job["preparation"] || {};
                        const f = p["fraction"];
                        const pct = (f === undefined || f === null || f < 0)
                            ? "" : Math.round(f * 100) + "%";
                        const eta = p["remaining"] ? " \u2014 " + p["remaining"] + qsTr(" left") : "";
                        const detail = p["detail"] ? " \u00b7 " + p["detail"] : "";
                        return (pct + eta + detail).trim();
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Cancel")
                    onClicked: jobs.cancel(root.job["entryId"])
                }
            }

            // Failed job: the error stays listed on its row (Mac adoptOwned).
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.job !== null && root.job !== undefined
                         && !!(root.job && root.job["failed"])
                spacing: 4
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    color: "#c93a3a"
                    text: root.job && root.job["failed"] ? root.job["failed"] : ""
                }
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Dismiss")
                    onClicked: jobs.dismissFailure(root.job["entryId"])
                }
            }

            // One primary action (Mac DevicePlaceholderViewController).
            Button {
                id: primaryAction
                objectName: "primaryAction"
                Layout.alignment: Qt.AlignHCenter
                highlighted: true
                visible: root.primaryText !== ""
                text: root.primaryText
                enabled: root.primaryEnabled
                ToolTip.visible: hovered && root.primaryTip !== ""
                ToolTip.text: root.primaryTip
                onClicked: root.runPrimary()
            }

            // Catalog note popover (Mac info popover for catalog notes).
            Button {
                Layout.alignment: Qt.AlignHCenter
                visible: root.row !== null && (root.row["firmwareNote"] || "") !== ""
                text: qsTr("Info")
                flat: true
                onClicked: notePopup.open()
                Popup {
                    id: notePopup
                    anchors.centerIn: parent
                    width: 300
                    modal: true
                    Label {
                        anchors.fill: parent
                        anchors.margins: 8
                        wrapMode: Text.WordWrap
                        text: root.row ? (root.row["firmwareNote"] || "") : ""
                    }
                }
            }
        }
    }
}
