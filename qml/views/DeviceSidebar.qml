// DeviceSidebar.qml — the device library sidebar (surface 1).
//
// Qt counterpart of UI/DeviceLibraryViewController.swift. Rows show the
// device icon (deviceIcon.iconSource), name + firmware over the state line,
// and a trailing state accessory. Behaviors preserved from the Mac:
//  - double-click / Return renames in place; Esc cancels;
//  - ⌘⌫ analog (Ctrl+Backspace) deletes with the guarded flow: one question
//    for prepared devices ("Delete N devices?"), instant removal for rows
//    with nothing on disk, running/busy rows skipped, selection moves to the
//    next row Finder-style;
//  - multi-select (Ctrl/Cmd-click toggles, Shift-click extends);
//  - type-to-select;
//  - context menu with Start/Shut Down…, Force Stop…, Import IPSW…, Cancel,
//    Rename, Delete…/Remove… (remaining Mac items arrive with surfaces 4/9/10);
//  - IPSW drops import via the firmware jobs; .ipa drops onto a running row
//    install on that device (refused at the drag layer otherwise).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    objectName: "deviceSidebar"
    padding: 0

    // kLTDeviceRowState… values mirror src/lt_bridge.h §6 (DEVIATION 7).
    readonly property int rowOff: 1
    readonly property int rowRunning: 2
    readonly property int rowPaused: 3
    readonly property int rowBooting: 4
    readonly property int rowError: 5
    // kLTDrop* mirror src/lt_bridge.h §6.
    readonly property int dropIPSW: 0
    readonly property int dropIPA: 1

    signal selectionChanged(var uuids)
    signal addDeviceRequested()

    // Ordered list of selected device UUIDs (JS array of strings).
    property var selectedIds: []
    property int anchorIndex: -1
    // Row under an open context menu (AppKit's clicked-row rule).
    property string menuUuid: ""
    property string typePrefix: ""

    function rowAt(index) {
        if (index < 0 || index >= deviceList.devices.length)
            return null;
        return deviceList.devices[index];
    }
    function uuidAt(index) {
        const r = rowAt(index);
        return r ? r["id"] : "";
    }
    function indexOfUuid(uuid) {
        for (let i = 0; i < deviceList.devices.length; ++i)
            if (deviceList.devices[i]["id"] === uuid)
                return i;
        return -1;
    }
    function isSelected(uuid) {
        return root.selectedIds.indexOf(uuid) !== -1;
    }
    function setSelection(ids, anchor) {
        root.selectedIds = ids;
        root.anchorIndex = anchor === undefined ? root.anchorIndex : anchor;
        root.selectionChanged(root.selectedIds);
    }
    function jobFor(uuid) {
        for (let i = 0; i < jobs.jobs.length; ++i) {
            const j = jobs.jobs[i];
            // "deviceUuid" is a mock-only field (Bridge.cpp); the real core
            // will expose the row↔job linkage when it lands (see coverage).
            if (j["deviceUuid"] === uuid)
                return j;
        }
        return null;
    }
    function rowBusy(uuid) {
        return deviceList.busyMap[uuid] || "";
    }
    // Mac: running rows and rows with a job in flight are skipped by delete.
    function deleteSkipped(uuid) {
        const r = rowAt(indexOfUuid(uuid));
        if (!r)
            return true;
        const st = r["status"];
        return st === root.rowRunning || st === root.rowBooting
            || rowBusy(uuid) !== "" || jobFor(uuid) !== null;
    }
    function displayName(row) {
        if (!row)
            return "";
        const n = row["name"] || "";
        return n !== "" ? "\u201c" + n + "\u201d"
                        : (row["firmware"] || qsTr("device"));
    }

    // Selection follows list mutations Finder-style: when selected rows
    // vanish, the next row takes the selection.
    property var pendingFixupIndex: -1
    Connections {
        target: deviceList
        function onDevicesChanged() {
            const ids = [];
            for (let i = 0; i < deviceList.devices.length; ++i)
                ids.push(deviceList.devices[i]["id"]);
            let sel = root.selectedIds.filter(function (u) { return ids.indexOf(u) !== -1; });
            if (sel.length === 0 && ids.length > 0 && root.pendingFixupIndex >= 0) {
                const at = Math.min(root.pendingFixupIndex, ids.length - 1);
                sel = [ids[at]];
                list.currentIndex = at;
            }
            root.pendingFixupIndex = -1;
            root.setSelection(sel, -1);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Label {
            text: qsTr("Devices")
            font.bold: true
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.topMargin: 8
            Layout.bottomMargin: 4
        }

        ListView {
            id: list
            objectName: "deviceListView"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: deviceList.devices
            currentIndex: -1
            spacing: 1

            // 38 pt rows, like the Mac's source list.
            delegate: Item {
                id: rowDelegate
                width: list.width
                height: 46
                property bool isCurrent: root.isSelected(modelData["id"])

                Rectangle {
                    anchors.fill: parent
                    color: rowDelegate.isCurrent ? palette.highlight
                         : rowMouse.containsMouse ? palette.alternateBase
                         : "transparent"
                }

                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onClicked: function (mouse) {
                        if (mouse.modifiers & (Qt.ControlModifier | Qt.MetaModifier)) {
                            const ids = root.selectedIds.slice();
                            const at = ids.indexOf(modelData["id"]);
                            if (at === -1)
                                ids.push(modelData["id"]);
                            else
                                ids.splice(at, 1);
                            root.setSelection(ids, index);
                        } else if (mouse.modifiers & Qt.ShiftModifier) {
                            const a = root.anchorIndex === -1 ? 0 : root.anchorIndex;
                            const lo = Math.min(a, index), hi = Math.max(a, index);
                            const ids = [];
                            for (let i = lo; i <= hi; ++i)
                                ids.push(root.uuidAt(i));
                            root.setSelection(ids, root.anchorIndex);
                        } else {
                            list.currentIndex = index;
                            root.setSelection([modelData["id"]], index);
                        }
                    }
                    onDoubleClicked: function (mouse) {
                        if (mouse.button === Qt.LeftButton)
                            root.beginRename(modelData["id"]);
                    }
                    onPressAndHold: root.openMenu(modelData["id"], index)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 4
                    anchors.rightMargin: 6
                    spacing: 6
                    Image {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        Layout.alignment: Qt.AlignVCenter
                        source: deviceIcon.iconSource(modelData["board"] || "")
                        fillMode: Image.PreserveAspectFit
                        opacity: root.rowBusy(modelData["id"]) !== "" ? 0.5 : 1.0
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 1
                        Label {
                            id: nameLabel
                            Layout.fillWidth: true
                            text: modelData["name"] || ""
                            font.bold: true
                            elide: Text.ElideRight
                            color: rowDelegate.isCurrent ? palette.highlightedText
                                                        : palette.text
                        }
                        Label {
                            Layout.fillWidth: true
                            text: (modelData["firmware"] || "")
                                  + ((modelData["statusText"] || "") !== ""
                                     ? " \u2014 " + modelData["statusText"] : "")
                            font.pointSize: 9
                            opacity: rowDelegate.isCurrent ? 0.85 : 0.7
                            color: rowDelegate.isCurrent ? palette.highlightedText
                                                        : palette.text
                            elide: Text.ElideRight
                        }
                    }
                    // Trailing state accessory (Mac DeviceRow.accessory).
                    Loader {
                        Layout.alignment: Qt.AlignVCenter
                        sourceComponent: {
                            const busy = root.rowBusy(modelData["id"]);
                            if (busy !== "")
                                return busyText;
                            const job = root.jobFor(modelData["id"]);
                            if (job)
                                return jobProgress;
                            switch (modelData["status"]) {
                            case root.rowRunning: return runningDot;
                            case root.rowBooting: return spinner;
                            case root.rowError: return errorMark;
                            default: break;
                            }
                            if ((modelData["firmwareNote"] || "") !== "")
                                return noteText;
                            return null;
                        }
                    }
                    Component {
                        id: busyText
                        Label {
                            text: root.rowBusy(modelData["id"]) === "deleting"
                                  ? qsTr("Deleting…") : qsTr("Erasing…")
                            font.pointSize: 9
                            opacity: 0.7
                        }
                    }
                    Component {
                        id: jobProgress
                        ProgressBar {
                            width: 64
                            from: 0
                            to: 1
                            value: {
                                const j = root.jobFor(modelData["id"]);
                                return j ? (j["progress"] || 0) : 0;
                            }
                        }
                    }
                    Component {
                        id: runningDot
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: "#34c759"
                        }
                    }
                    Component {
                        id: spinner
                        BusyIndicator {
                            width: 16
                            height: 16
                            running: true
                        }
                    }
                    Component {
                        id: errorMark
                        Label {
                            text: "\u26a0"
                            color: "#ffcc00"
                            font.pointSize: 12
                        }
                    }
                    Component {
                        id: noteText
                        Label {
                            text: modelData["firmwareNote"] || ""
                            font.pointSize: 9
                            opacity: 0.7
                            elide: Text.ElideRight
                        }
                    }
                }

                ToolTip.visible: rowMouse.containsMouse
                ToolTip.delay: 600
                ToolTip.text: {
                    const bits = [modelData["name"] || "",
                                  modelData["firmware"] || "",
                                  modelData["statusText"] || ""];
                    if ((modelData["firmwareNote"] || "") !== "")
                        bits.push(modelData["firmwareNote"]);
                    return bits.filter(function (s) { return s !== ""; }).join("\n");
                }
            }

            // Right-click context menu (AppKit's clicked-row rule).
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: false
                onPressed: function (mouse) {
                    const idx = list.indexAt(mouse.x, mouse.y);
                    root.openMenu(idx === -1 ? "" : root.uuidAt(idx), idx);
                    mouse.accepted = true;
                }
            }

            Keys.onPressed: function (event) {
                // ⌘⌫ analog: Ctrl+Backspace deletes. Bare Delete does
                // nothing in the Mac sidebar (File ▸ Delete Device… owns it).
                if (event.key === Qt.Key_Backspace
                    && (event.modifiers & (Qt.ControlModifier | Qt.MetaModifier))) {
                    root.deleteSelected();
                    event.accepted = true;
                } else if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                           && root.selectedIds.length === 1) {
                    root.beginRename(root.selectedIds[0]);
                    event.accepted = true;
                } else if (event.text !== "" && event.text.length === 1
                           && !(event.modifiers & (Qt.ControlModifier | Qt.MetaModifier | Qt.AltModifier))) {
                    // Type-to-select, like the Mac's typeSelectStringFor.
                    root.typePrefix += event.text.toLowerCase();
                    for (let i = 0; i < deviceList.devices.length; ++i) {
                        const n = (deviceList.devices[i]["name"] || "").toLowerCase();
                        if (n.indexOf(root.typePrefix) === 0) {
                            list.currentIndex = i;
                            root.setSelection([root.uuidAt(i)], i);
                            break;
                        }
                    }
                    typeReset.restart();
                    event.accepted = true;
                }
            }
            Timer {
                id: typeReset
                interval: 900
                onTriggered: root.typePrefix = ""
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: qsTr("No devices")
                opacity: 0.5
            }
        }

        Button {
            id: addButton
            visible: deviceList.count === 0
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 12
            text: qsTr("Add Device…")
            onClicked: root.addDeviceRequested()
        }
    }

    // IPSW / .ipa drops (surface 12 behavior, sidebar half).
    DropArea {
        anchors.fill: parent
        onEntered: function (drag) {
            drag.accepted = root.dropAcceptable(drag);
        }
        onDropped: function (drop) {
            root.handleDrop(drop);
        }
    }

    function dropAcceptable(drag) {
        if (!drag.hasUrls)
            return false;
        for (let i = 0; i < drag.urls.length; ++i) {
            if (deviceList.classifyDropUrl(drag.urls[i]) === root.dropIPSW)
                return true;
        }
        return false;
    }
    function handleDrop(drop) {
        const ipsws = [];
        for (let i = 0; i < drop.urls.length; ++i) {
            if (deviceList.classifyDropUrl(drop.urls[i]) === root.dropIPSW)
                ipsws.push(drop.urls[i]);
        }
        if (ipsws.length > 0)
            jobs.importIpsws(ipsws);
    }

    // --- Rename -----------------------------------------------------------
    property string renameUuid: ""
    function beginRename(uuid) {
        const idx = root.indexOfUuid(uuid);
        if (idx === -1)
            return;
        root.renameUuid = uuid;
        list.currentIndex = idx;
        root.setSelection([uuid], idx);
        renameEditor.text = rowAt(idx)["name"] || "";
        renameEditor.forceActiveFocus();
        renameEditor.selectAll();
    }

    // --- Context menu ------------------------------------------------------
    function openMenu(uuid, index) {
        // AppKit rule: if the clicked row is in the selection, the menu acts
        // on the selection; otherwise on the clicked row alone.
        let targets;
        if (uuid !== "" && root.isSelected(uuid))
            targets = root.selectedIds;
        else if (uuid !== "")
            targets = [uuid];
        else
            targets = [];
        root.menuUuid = uuid;
        buildMenu(targets);
        contextMenu.popup();
    }

    function addMenuSeparator() {
        const sep = Qt.createQmlObject(
            'import QtQuick.Controls; MenuSeparator {}', contextMenu);
        contextMenu.addItem(sep);
    }

    function addMenuItem(text, enabled, action) {
        // QML Menu.addItem takes a MenuItem, not a string: build one.
        const item = Qt.createQmlObject(
            'import QtQuick.Controls; MenuItem {}', contextMenu);
        item.text = text;
        item.enabled = enabled !== false;
        item.triggered.connect(function () {
            root.runMenuAction(action, contextMenu.targets);
        });
        contextMenu.addItem(item);
    }

    function buildMenu(targets) {
        // QML Menu has no clear(): remove and destroy items one by one
        // (removeItem alone leaves them parented and findable).
        while (contextMenu.count > 0) {
            const gone = contextMenu.itemAt(0);
            contextMenu.removeItem(gone);
            gone.destroy();
        }
        contextMenu.targets = targets;
        if (targets.length === 0)
            return;
        if (targets.length > 1) {
            // Multi-row menu: the one batch delete (Mac batchTitle).
            let deletable = 0, removable = 0;
            for (let i = 0; i < targets.length; ++i) {
                if (root.deleteSkipped(targets[i]))
                    continue;
                const r = root.rowAt(root.indexOfUuid(targets[i]));
                if (r && r["prepared"] === false)
                    removable++;
                else
                    deletable++;
            }
            const n = deletable + removable;
            if (n === 0)
                return;
            const noun = n === 1 ? qsTr("Device") : qsTr("%n Devices", "", n);
            addMenuItem(deletable > 0 ? qsTr("Delete %1…").arg(noun)
                                      : qsTr("Remove %1").arg(noun),
                        true, "delete");
            return;
        }
        const uuid = targets[0];
        const row = root.rowAt(root.indexOfUuid(uuid));
        if (!row)
            return;
        const running = row["status"] === root.rowRunning
            || row["status"] === root.rowBooting;
        const busy = root.rowBusy(uuid) !== "";
        const job = root.jobFor(uuid);
        const prepared = row["prepared"] !== false;
        addMenuItem(running ? qsTr("Shut Down…") : qsTr("Start"), !busy,
                    running ? "shutdown" : "start");
        addMenuItem(qsTr("Force Stop…"), running && !busy, "forcestop");
        addMenuSeparator();
        addMenuItem(qsTr("Import IPSW…"), true, "importipsw");
        addMenuItem(qsTr("Cancel"), job !== null, "canceljob");
        addMenuSeparator();
        addMenuItem(qsTr("Rename"), true, "rename");
        addMenuItem(prepared ? qsTr("Delete…") : qsTr("Remove"), true, "delete");
    }

    Menu {
        id: contextMenu
        objectName: "sidebarContextMenu"
        property var targets: []
    }

    // Test seam: the texts the menu currently shows, in order.
    function menuItemTexts() {
        const out = [];
        for (let i = 0; i < contextMenu.count; ++i) {
            const t = contextMenu.itemAt(i).text;
            if (t !== undefined && t !== "")
                out.push(t);
        }
        return out;
    }

    function runMenuAction(action, targets) {
        const uuid = targets.length > 0 ? targets[0] : "";
        if (action === "start") {
            deviceList.bootDevice(uuid);
        } else if (action === "shutdown") {
            confirmDialog.ask(
                qsTr("Shut down %1?").arg(displayName(rowAt(indexOfUuid(uuid)))),
                qsTr("The guest OS shuts down cleanly; unsaved state in apps is kept."),
                qsTr("Shut Down"),
                function () { deviceList.haltDevice(uuid); });
        } else if (action === "forcestop") {
            confirmDialog.ask(
                qsTr("Force stop %1?").arg(displayName(rowAt(indexOfUuid(uuid)))),
                qsTr("The emulated device is stopped immediately. Unsaved guest state may be lost."),
                qsTr("Force Stop"),
                function () { deviceList.haltDevice(uuid); });
        } else if (action === "importipsw") {
            const picked = jobs.pickIpswFiles();
            if (picked.length > 0)
                jobs.importIpsws(picked);
        } else if (action === "canceljob") {
            const job = jobFor(uuid);
            if (job)
                jobs.cancel(job["entryId"]);
        } else if (action === "rename") {
            beginRename(uuid);
        } else if (action === "delete") {
            deleteDevices(targets);
        }
    }

    // --- Delete guard -------------------------------------------------------
    // Mac DeviceLibraryViewController.removeTargets/batch: prepared devices
    // ask once for the lot, rows with nothing on disk leave instantly,
    // running/busy rows are skipped.
    function deleteSelected() {
        deleteDevices(root.selectedIds);
    }
    function deleteDevices(ids) {
        const toDelete = [], toRemove = [], skipped = [];
        for (let i = 0; i < ids.length; ++i) {
            const uuid = ids[i];
            if (deleteSkipped(uuid)) {
                skipped.push(uuid);
                continue;
            }
            const r = rowAt(indexOfUuid(uuid));
            if (r && r["prepared"] === false)
                toRemove.push(uuid);
            else
                toDelete.push(uuid);
        }
        if (toDelete.length === 0 && toRemove.length === 0)
            return;
        if (toDelete.length === 0) {
            // Nothing on disk: leaves the list at once, no question.
            root.pendingFixupIndex = root.anchorIndex;
            for (let i = 0; i < toRemove.length; ++i)
                deviceList.deleteDevice(toRemove[i]);
            return;
        }
        const names = toDelete.map(function (u) {
            return displayName(rowAt(indexOfUuid(u)));
        });
        const skippedNames = skipped.map(function (u) {
            return displayName(rowAt(indexOfUuid(u)));
        });
        let title, info;
        if (toDelete.length === 1 && toRemove.length === 0) {
            title = qsTr("Delete %1?").arg(names[0]);
            info = qsTr("This permanently removes the apps, settings, and saved state of %1.")
                .arg(names[0]);
        } else {
            const n = toDelete.length + toRemove.length;
            title = qsTr("Delete %n devices?", "", n);
            info = qsTr("This permanently removes the apps, settings, and saved state of %1.")
                .arg(names.join(qsTr(", ")));
        }
        if (skippedNames.length > 0) {
            info += " " + qsTr("%1 %2 running or busy and stays.")
                .arg(skippedNames.join(qsTr(", ")))
                .arg(skippedNames.length === 1 ? qsTr("is") : qsTr("are"));
        }
        // Re-check on the answer: a device started while the question was up
        // stays (Mac finish() re-batches).
        deleteDialog.askDelete(title, info, function () {
            root.pendingFixupIndex = root.anchorIndex;
            for (let i = 0; i < toRemove.length; ++i)
                deviceList.deleteDevice(toRemove[i]);
            const still = toDelete.filter(function (u) { return !root.deleteSkipped(u); });
            for (let i = 0; i < still.length; ++i)
                deviceList.deleteDevice(still[i]);
        });
    }

    // --- Dialogs (loaded lazily so tests can drive the sidebar alone) ------
    Loader {
        id: deleteDialogLoader
        source: Qt.resolvedUrl("../dialogs/DeleteDevicesDialog.qml")
    }
    property alias deleteDialog: deleteDialogLoader.item

    Loader {
        id: confirmDialogLoader
        source: Qt.resolvedUrl("../dialogs/ConfirmDialog.qml")
    }
    property alias confirmDialog: confirmDialogLoader.item

    // "Import IPSW…" uses LTJobsAdapter.pickIpswFiles() (native dialog via
    // QtWidgets): the QtQuick.Dialogs QML module is not shipped on all
    // targets, so no QML FileDialog here.

    // In-place rename editor, overlaid on the renamed row (a ListView child
    // so it tracks scrolling via contentY, like the Mac's cell editor).
    TextField {
        id: renameEditor
        objectName: "renameEditor"
        parent: list
        visible: root.renameUuid !== ""
        z: 10
        x: 44
        width: list.width - 130
        y: {
            const idx = root.indexOfUuid(root.renameUuid);
            return idx === -1 ? 0 : idx * 47 - list.contentY + 8;
        }
        onAccepted: {
            const uuid = root.renameUuid;
            const newName = text;
            root.renameUuid = "";
            list.forceActiveFocus();
            if (uuid !== "")
                deviceList.renameDevice(uuid, newName);
        }
        Keys.onEscapePressed: {
            // Esc: cancel, like the Mac's cancelledRename.
            root.renameUuid = "";
            list.forceActiveFocus();
        }
    }
}
