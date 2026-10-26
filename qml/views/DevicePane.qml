// DevicePane.qml — the main column's detail area (surface 1).
//
// Selection-driven swap (MainWindowController+Library.swift):
//  - no devices → empty-library prompt (+ Add Device…)
//  - nothing selected → "Select a device"
//  - several selected → "%n devices selected" (nothing starts)
//  - one running device → the display placeholder (surface 2's DisplayItem)
//  - one stopped device → DevicePlaceholder (state line + primary action)
// The swap never resets the console strip (a sibling in main.qml).
// A BannerStack floats over the scene's bottom edge: the device notice and
// the startup toast.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    padding: 0

    // Alias for the context "display" property (LTDisplayAdapter). A bare
    // `display` reference inside DeviceScreen resolves to DeviceScreen's OWN
    // display property first (self-binding + "Binding loop detected" +
    // display null), never to the context property. At Pane scope there is
    // no display property, so this alias resolves to the context property.
    property var displayAdapter: display

    readonly property int rowRunning: 2 // lt_bridge.h kLTDeviceRowStateRunning

    // Ordered selection from the sidebar.
    property var selectedIds: []
    // The single selected row's DeviceRow map, or null.
    property var selectedRow: null
    // From the menu bar (surface 11): free-form screen, keyboard input.
    // Surface 3: the bezel mode preference.
    property bool freeForm: false
    property bool keyboardInputEnabled: true
    property string bezelMode: "2d"

    signal addDeviceRequested()
    signal dropRouted(var urls, string target, double requestId)

    function jobFor(uuid) {
        for (let i = 0; i < jobs.jobs.length; ++i) {
            if (jobs.jobs[i]["deviceUuid"] === uuid)
                return jobs.jobs[i];
        }
        return null;
    }

    StackLayout {
        id: stack
        objectName: "detailStack"
        anchors.fill: parent
        currentIndex: {
            if (deviceList.count === 0)
                return 0;
            if (root.selectedIds.length === 0)
                return 1;
            if (root.selectedIds.length > 1)
                return 2;
            const r = root.selectedRow;
            if (r && r["status"] === root.rowRunning)
                return 3;
            return 4;
        }

        // 0: empty library
        Pane {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    opacity: 0.6
                    text: qsTr("No devices")
                }
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Add Device…")
                    onClicked: root.addDeviceRequested()
                }
            }
        }
        // 1: nothing selected
        Pane {
            Label {
                anchors.centerIn: parent
                opacity: 0.5
                text: qsTr("Select a device")
            }
        }
        // 2: multi-select
        Pane {
            Label {
                anchors.centerIn: parent
                opacity: 0.6
                text: qsTr("%n devices selected", "", root.selectedIds.length)
            }
        }
        // 3: running device → the display (surface 2: DisplayItem + chrome)
        DeviceScreen {
            row: root.selectedRow
            display: root.displayAdapter
            freeForm: root.freeForm
            keyboardInputEnabled: root.keyboardInputEnabled
            bezelMode: root.bezelMode
            onDropRouted: function (urls, target, id) { root.dropRouted(urls, target, id); }
        }
        // 4: stopped device → placeholder
        DevicePlaceholder {
            row: root.selectedRow
            job: root.selectedRow ? root.jobFor(root.selectedRow["id"]) : null
            onAddDeviceRequested: root.addDeviceRequested()
        }
    }

    // Transient banner stack over the scene's bottom edge (CaptureStatusView).
    BannerStack {
        id: banners
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        banners: {
            const out = [];
            // Device notice (DeviceNotices.message).
            if (device.noticeVisible) {
                out.push({
                    id: "notice",
                    text: device.noticeMessage,
                    primaryText: device.noticeOperation !== "" ? device.noticeOperation : "",
                    secondaryText: qsTr("Dismiss")
                });
            }
            // Startup toast: the boot-stage line while booting.
            if (device.bootStageText !== "" && device.machineState === 5) {
                out.push({
                    id: "boot",
                    text: device.bootStageText,
                    primaryText: "",
                    secondaryText: ""
                });
            }
            return out;
        }
        onPrimaryClicked: function (bannerId) {
            if (bannerId === "notice")
                device.resolveNotice();
        }
        onSecondaryClicked: function (bannerId) {
            if (bannerId === "notice")
                device.dismissNotice();
        }
    }
}
