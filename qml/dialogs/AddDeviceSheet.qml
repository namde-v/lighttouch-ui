// AddDeviceSheet.qml — the Add Device sheet (surface 10).
//
// Qt counterpart of UI/AddDeviceView.swift + UI/AddDeviceSummary.swift:
// the firmware catalog split by device. Devices on the left (fixed 270 px,
// icon + name + picked-count badge); the shown device's versions on the
// right in catalog order, each with its support status and IPSW state; the
// picks' summary under them.
//
// Mac rules kept: a sheet-like modal dialog; versions already in the sidebar
// are checked and unpickable; "Coming Soon" dimmed and unpickable; only
// stable builds until "Show experimental" (remembered); picks survive device
// switching; the versions list has keyboard focus on open; double-click or
// Return adds the picks; Add is disabled until something is picked.
//
// Documented gaps (see UI_COVERAGE.md surface 10): the "Supported" feature
// grid needs per-entry DeviceFeatures from the core (no ABI export yet), so
// the summary shows facts only; the "already in the sidebar" match is a
// board+firmware-version heuristic (rows carry no catalog entry id); the
// downloaded set is empty (no IPSW-store query in the ABI).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("Add Device")
    modal: true
    width: 800
    height: 620
    standardButtons: Dialog.NoButton

    property var picked: ({})          // entry id -> true; survives device switches
    property string shownBoard: ""
    property bool jailbreak: false
    property bool skipSetup: false

    function isStable(e) {
        return e["status"] === "available"
            || (e["status"] === "userIPSW" && !e["prereleaseBadge"]);
    }
    function statusText(status) {
        switch (status) {
        case "experimental": return qsTr("Experimental");
        case "untested": return qsTr("Untested");
        case "comingSoon": return qsTr("Coming Soon");
        case "userIPSW": return qsTr("Requires an IPSW");
        default: return "";
        }
    }
    function downloadBytes(e) {
        return e["estimates"] ? (e["estimates"]["preparedBytes"] || 0) : 0;
    }
    function ipswState(e) {
        if (e["bundled"])
            return qsTr("Included");
        // No IPSW-store query in the ABI: the downloaded set is empty.
        return formatBytes(downloadBytes(e));
    }
    function formatBytes(n) {
        if (!n)
            return qsTr("None");
        const units = ["B", "KB", "MB", "GB", "TB"];
        let v = n, u = 0;
        while (v >= 1024 && u < units.length - 1) { v /= 1024; ++u; }
        return (u === 0 ? v.toFixed(0) : v.toFixed(1)) + " " + units[u];
    }
    function formatDate(iso) {
        if (!iso)
            return "";
        const d = new Date(iso + "T00:00:00");
        return isNaN(d) ? iso : d.toLocaleDateString(Qt.locale(), Locale.LongFormat);
    }
    // Board+firmware-version heuristic: rows carry no catalog entry id.
    function isAdded(e) {
        for (let i = 0; i < deviceList.devices.length; ++i) {
            const r = deviceList.devices[i];
            if (r["board"] === e["board"]
                    && (r["firmware"] || "").endsWith(e["version"]))
                return true;
        }
        return false;
    }
    function groups() {
        const boards = [];
        const byBoard = {};
        for (let i = 0; i < catalog.entries.length; ++i) {
            const e = catalog.entries[i];
            if (settings.addDeviceShowsExperimental || isStable(e)) {
                if (!byBoard[e["board"]]) {
                    byBoard[e["board"]] = [];
                    boards.push(e["board"]);
                }
                byBoard[e["board"]].push(e);
            }
        }
        return boards.map(function (b) {
            const es = byBoard[b];
            return { id: b, name: es[0]["marketingName"] || es[0]["productType"], entries: es };
        });
    }
    function shownGroup() {
        const gs = groups();
        for (let i = 0; i < gs.length; ++i)
            if (gs[i].id === root.shownBoard)
                return gs[i];
        return gs.length > 0 ? gs[0] : null;
    }
    function pickedIds() {
        const gs = groups();
        const out = [];
        const seen = {};
        for (let g = 0; g < gs.length; ++g) {
            for (let i = 0; i < gs[g].entries.length; ++i) {
                const e = gs[g].entries[i];
                if (e["status"] === "comingSoon" || isAdded(e) || seen[e["id"]])
                    continue;
                seen[e["id"]] = true;
                if (root.picked[e["id"]])
                    out.push(e["id"]);
            }
        }
        return out;
    }
    function pickedEntries() {
        const ids = {};
        const list = pickedIds();
        for (let i = 0; i < list.length; ++i)
            ids[list[i]] = true;
        const out = [];
        const gs = groups();
        for (let g = 0; g < gs.length; ++g)
            for (let i = 0; i < gs[g].entries.length; ++i)
                if (ids[gs[g].entries[i]["id"]])
                    out.push(gs[g].entries[i]);
        return out;
    }
    function groupPickedCount(g) {
        let n = 0;
        for (let i = 0; i < g.entries.length; ++i)
            if (root.picked[g.entries[i]["id"]] && !isAdded(g.entries[i])
                    && g.entries[i]["status"] !== "comingSoon")
                ++n;
        return n;
    }
    function doAdd() {
        const ids = pickedIds();
        if (ids.length === 0)
            return;
        const opts = JSON.stringify({
            jailbreak: root.jailbreak, skipSetup: root.skipSetup, keepStock: false
        });
        for (let i = 0; i < ids.length; ++i)
            jobs.start(ids[i], opts);
        root.picked = {};
        root.close();
    }

    onOpened: {
        catalog.refresh();
        root.shownBoard = "";
        root.picked = {};
        root.jailbreak = false;
        root.skipSetup = false;
        versionsView.forceActiveFocus();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Devices column: fixed, never collapsible in the sheet.
            ListView {
                id: devicesView
                Layout.preferredWidth: 270
                Layout.fillHeight: true
                clip: true
                model: root.groups()
                keyNavigationWraps: true
                delegate: ItemDelegate {
                    width: devicesView.width
                    highlighted: modelData.id === (root.shownGroup() ? root.shownGroup().id : "")
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8
                        Image {
                            source: deviceIcon.iconForBoard(modelData.id)
                            sourceSize.width: 28
                            sourceSize.height: 28
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            fillMode: Image.PreserveAspectFit
                        }
                        Label {
                            text: modelData.name
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label {
                            visible: root.groupPickedCount(modelData) > 0
                            text: root.groupPickedCount(modelData)
                            opacity: 0.6
                        }
                    }
                    onClicked: root.shownBoard = modelData.id
                }
            }

            Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: palette.mid }

            // Versions column + summary.
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                ListView {
                    id: versionsView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    focus: true
                    model: root.shownGroup() ? root.shownGroup().entries : []
                    Keys.onReturnPressed: root.doAdd()
                    delegate: CheckBox {
                        id: rowBox
                        width: versionsView.width
                        property var entry: modelData
                        property bool added: root.isAdded(modelData)
                        property bool soon: modelData["status"] === "comingSoon"
                        enabled: !added && !soon
                        checked: added || !!root.picked[modelData["id"]]
                        opacity: (added || soon) ? 0.55 : 1.0
                        onToggled: {
                            const p = Object.assign({}, root.picked);
                            if (rowBox.checked)
                                p[modelData["id"]] = true;
                            else
                                delete p[modelData["id"]];
                            root.picked = p;
                        }
                        contentItem: RowLayout {
                            spacing: 6
                            Label {
                                text: qsTr("iOS %1").arg(rowBox.entry["version"])
                                font.bold: !rowBox.added && !rowBox.soon
                            }
                            Label {
                                visible: !!rowBox.entry["prereleaseBadge"]
                                text: rowBox.entry["prereleaseBadge"] || ""
                                font.pointSize: 9
                                opacity: 0.7
                            }
                            Label {
                                text: rowBox.entry["build"] || ""
                                opacity: 0.6
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                visible: root.statusText(rowBox.entry["status"]) !== ""
                                text: root.statusText(rowBox.entry["status"])
                                opacity: 0.6
                            }
                            Label {
                                text: root.ipswState(rowBox.entry)
                                opacity: 0.6
                                Layout.minimumWidth: 76
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                        // Double-click adds the picks, as Add does.
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onDoubleClicked: root.doAdd()
                            onClicked: function (mouse) {
                                if (rowBox.enabled)
                                    rowBox.toggle();
                                mouse.accepted = true;
                            }
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: palette.mid }

                // Summary of the picks (AddDeviceSummary: facts only; the
                // Supported feature grid needs per-entry DeviceFeatures, which
                // the ABI does not expose yet).
                ScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    visible: root.pickedEntries().length > 0
                    clip: true
                    ColumnLayout {
                        width: parent.width
                        spacing: 8
                        Repeater {
                            model: root.pickedEntries()
                            delegate: GridLayout {
                                columns: 4
                                Layout.fillWidth: true
                                Layout.leftMargin: 14
                                Layout.rightMargin: 14
                                Layout.topMargin: 6
                                Label { text: modelData["marketingName"]; font.bold: true; Layout.columnSpan: 4 }
                                Label { text: qsTr("iOS %1 (%2)").arg(modelData["version"]).arg(modelData["build"]) }
                                Label { text: root.formatDate(modelData["released"]); opacity: 0.7 }
                                Label { text: root.ipswState(modelData); opacity: 0.7 }
                                Label {
                                    text: root.statusText(modelData["status"]) !== ""
                                          ? root.statusText(modelData["status"])
                                          : qsTr("Supported");
                                    opacity: 0.7
                                }
                            }
                        }
                        Label {
                            Layout.leftMargin: 14
                            opacity: 0.7
                            text: {
                                let bytes = 0, prep = 0;
                                const es = root.pickedEntries();
                                for (let i = 0; i < es.length; ++i) {
                                    bytes += root.downloadBytes(es[i]);
                                    prep += root.downloadBytes(es[i]);
                                }
                                return qsTr("Download: %1   Prepared: %2")
                                    .arg(root.formatBytes(bytes)).arg(root.formatBytes(prep));
                            }
                        }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: palette.mid }

        // Footer: experimental toggle, prepare options, Cancel / Add.
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            spacing: 12
            CheckBox {
                text: qsTr("Show experimental")
                checked: settings.addDeviceShowsExperimental
                onToggled: settings.addDeviceShowsExperimental = checked
            }
            CheckBox {
                text: qsTr("Jailbreak")
                checked: root.jailbreak
                onToggled: root.jailbreak = checked
            }
            CheckBox {
                text: qsTr("Skip setup")
                checked: root.skipSetup
                onToggled: root.skipSetup = checked
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
            Button {
                text: qsTr("Add")
                enabled: root.pickedIds().length > 0
                onClicked: root.doAdd()
            }
        }
    }
}
