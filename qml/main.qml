// main.qml — LightTouch Qt application shell.
//
// Surface 1: three-pane main window — DeviceSidebar | (DevicePane + console
// strip) | apps-inspector placeholder — with window title tracking the
// selected device and its boot-stage line. Full menus (surface 11), the real
// display (surface 2), console (surface 7) and inspector (surface 5) replace
// the placeholders in later milestones. Runs headless under
// QT_QPA_PLATFORM=offscreen.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "views" // DeviceSidebar, DevicePane, DisplayPlaceholder, BannerStack, MainMenuBar
import "dialogs" // AddDeviceSheet, SettingsDialog, AboutDialog, LicensesDialog, HelpWindow, JailbreakDialog, NetworkAccessDialog
import "panes" // CarrierPanel, TweaksPanel, ProxySettings, DebugPort (surface 4)

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: {
        const name = window.currentRowName;
        const sub = device.bootStageText !== "" ? " \u2014 " + device.bootStageText : "";
        return qsTr("LightTouch") + (name !== "" ? " \u2014 " + name + sub : "");
    }

    property string currentRowName: ""
    property var currentSelection: []

    // kLTEventToast == 35 (see src/lt_bridge.h §4). Routed to the footer.
    Connections {
        target: bridge
        function onLtEventReceived(kind, device, seq, payload) {
            if (kind === 35) {
                const title = payload["title"] || "";
                const body = payload["body"] || "";
                toastLabel.text = title !== "" ? title + ": " + body : body;
            }
        }
    }

    function routeSelection(uuids) {
        window.currentSelection = uuids;
        const one = uuids.length === 1 ? uuids[0] : "";
        // The one selected row's name for the window title.
        window.currentRowName = "";
        if (one !== "") {
            for (let i = 0; i < deviceList.devices.length; ++i) {
                if (deviceList.devices[i]["id"] === one) {
                    window.currentRowName = deviceList.devices[i]["name"] || "";
                    break;
                }
            }
        }
        // One selection drives every device-scoped adapter.
        device.currentDeviceUuid = one;
        display.deviceUuid = one;
        tweaks.deviceUuid = one;
        carrier.deviceUuid = one;
        apps.deviceUuid = one;
        files.deviceUuid = one;
        capture.deviceUuid = one;
        devicePane.selectedRow = one !== "" ? sidebarRow(one) : null;
    }
    function sidebarRow(uuid) {
        for (let i = 0; i < deviceList.devices.length; ++i) {
            if (deviceList.devices[i]["id"] === uuid)
                return deviceList.devices[i];
        }
        return null;
    }

    // The selected row can change under us (rename); keep the pane in sync.
    Connections {
        target: deviceList
        function onDevicesChanged() {
            const one = window.currentSelection.length === 1
                ? window.currentSelection[0] : "";
            if (one !== "")
                devicePane.selectedRow = window.sidebarRow(one);
        }
    }

    menuBar: MainMenuBar {
        id: menuBar
        objectName: "menuBar"
        bezelMode: settings.bezelMode()
        onBezelModeChanged: settings.setBezelMode(bezelMode)
        onAddDeviceRequested: addDeviceSheet.open()
        onShowSettingsRequested: settingsDialog.open()
        onShowHelpRequested: helpWindow.show()
        onJailbreakRequested: {
            jailbreakDialog.deviceName = window.currentRowName;
            jailbreakDialog.version = "";
            for (let i = 0; i < deviceList.devices.length; ++i) {
                if (deviceList.devices[i]["id"] === window.currentSelection[0]) {
                    const fw = deviceList.devices[i]["firmware"] || "";
                    const m = fw.match(/(\d+\.\d+(\.\d+)?)/);
                    jailbreakDialog.version = m ? m[1] : "";
                    break;
                }
            }
            jailbreakDialog.open();
        }
        onToggleSidebar: deviceSidebar.visible = !deviceSidebar.visible
        onToggleInspector: inspectorPane.visible = !inspectorPane.visible
        onToggleConsole: consoleSplit.visible = !consoleSplit.visible
        // Surface 4: one panel per device (the adapters are uuid-scoped).
        onShowCarrierRequested: {
            carrier.deviceUuid = window.currentSelection[0] || "";
            carrierDialog.title = qsTr("Carrier — %1").arg(window.currentRowName);
            carrierDialog.open();
        }
        onShowTweaksRequested: {
            tweaks.deviceUuid = window.currentSelection[0] || "";
            tweaksDialog.title = qsTr("Tweaks — %1").arg(window.currentRowName);
            tweaksDialog.open();
        }
        onShowProxyRequested: {
            proxy.deviceUuid = window.currentSelection[0] || "";
            proxyDialog.title = qsTr("Web Proxy — %1").arg(window.currentRowName);
            proxyDialog.open();
        }
        onShowDebugPortRequested: {
            debugPort.deviceUuid = window.currentSelection[0] || "";
            debugPortDialog.title = qsTr("Debug Port — %1").arg(window.currentRowName);
            debugPortDialog.open();
        }
        // Surface 6/7: files browser + log window (one window per device).
        onShowFilesRequested: {
            filesWindow.deviceUuid = window.currentSelection[0] || "";
            filesWindow.deviceName = window.currentRowName;
            filesWindow.show();
            filesWindow.raise();
        }
        onShowLogsRequested: logWindow.show()
        // Surface 9: firmware jobs window; Ctrl+F focuses catalog search.
        onDownloadPrepareRequested: {
            firmwareWindow.show();
            firmwareWindow.raise();
        }
        onFindInCatalogRequested: firmwareWindow.focusCatalogSearch()
        onDiagnosticsExported: function (path) {
            toastLabel.text = qsTr("Diagnostics exported: %1").arg(path);
        }
        sidebarVisible: deviceSidebar.visible
        inspectorVisible: inspectorPane.visible
        consoleVisible: consoleSplit.visible
    }

    // Capture toolbar (surface 8): the recording button lives here, next
    // to the menu bar, like the Mac's unified toolbar.
    header: ToolBar {
        id: captureBar
        visible: device.running
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            Label {
                visible: jobs.hasJobs
                text: qsTr("Jobs: %1").arg(jobs.jobs.length)
                font.pointSize: 9
                opacity: 0.7
            }
            Item { Layout.fillWidth: true }
            RecordingButton { id: recordingButton }
        }
    }

    CaptureSounds { id: captureSounds }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        DeviceSidebar {
            id: deviceSidebar
            SplitView.preferredWidth: 260
            SplitView.minimumWidth: 180
            SplitView.maximumWidth: 320
            onSelectionChanged: function (uuids) { window.routeSelection(uuids); }
            onAddDeviceRequested: addDeviceSheet.open()
        }

        ColumnLayout {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 320
            spacing: 0

            DevicePane {
                id: devicePane
                Layout.fillWidth: true
                Layout.fillHeight: true
                selectedIds: window.currentSelection
                freeForm: menuBar.freeFormActionChecked
                keyboardInputEnabled: menuBar.keyboardInput
                bezelMode: menuBar.bezelMode
                onDropRouted: function (urls, target, id) { window.noteDrop(id, urls); }
                onAddDeviceRequested: addDeviceSheet.open()
            }

            // Console (surface 7): the bar is the divider; drag it,
            // double-click it, or use its toggle.
            ConsoleSplit {
                id: consoleSplit
                Layout.fillWidth: true
            }
        }

        // Apps inspector (surface 5).
        AppsInspector {
            id: inspectorPane
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 280
            SplitView.maximumWidth: 400
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                id: toastLabel
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: deviceList.hasDevices
                      ? qsTr("%n device(s) in library", "", deviceList.count)
                      : qsTr("No devices — add one from the firmware catalog")
            }
        }
    }

    // Surface 10 dialogs.
    AddDeviceSheet {
        id: addDeviceSheet
        anchors.centerIn: parent
    }
    SettingsDialog {
        id: settingsDialog
        anchors.centerIn: parent
    }
    AboutDialog {
        id: aboutDialog
        anchors.centerIn: parent
        onShowLicenses: licensesWindow.show()
    }
    LicensesDialog {
        id: licensesWindow
    }
    HelpWindow {
        id: helpWindow
        deviceName: window.currentRowName
    }
    JailbreakDialog {
        id: jailbreakDialog
        anchors.centerIn: parent
    }
    NetworkAccessDialog {
        id: networkAccessDialog
        anchors.centerIn: parent
    }

    // Surface 8: the recording save destination (two-step handshake).
    Dialog {
        id: saveRecordingDialog
        modal: true
        anchors.centerIn: parent
        title: qsTr("Save Recording")
        property string suggestedName: ""
        function savePath() {
            // "<device> Recording.mov" in the capture folder.
            const folder = (capture.prefs && capture.prefs["saveLocation"]) || "";
            return folder + "/" + saveRecordingDialog.suggestedName;
        }
        ColumnLayout {
            Label {
                text: qsTr("Name the recording:")
                Layout.fillWidth: true
            }
            TextField {
                id: recordingNameField
                Layout.preferredWidth: 320
                text: saveRecordingDialog.suggestedName
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button {
                    text: qsTr("Discard")
                    onClicked: {
                        saveRecordingDialog.close();
                        capture.discardRecording();
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: saveRecordingDialog.close()
                }
                Button {
                    text: qsTr("Save")
                    onClicked: {
                        saveRecordingDialog.close();
                        const folder = (capture.prefs && capture.prefs["saveLocation"]) || "";
                        capture.saveRecording(folder + "/" + recordingNameField.text);
                    }
                }
            }
        }
    }

    // Surface 4: device inspector panels (utility dialogs, one per device —
    // the adapters re-bind on deviceUuid, so one dialog each suffices).
    Dialog {
        id: carrierDialog
        modal: false
        width: 420
        height: 560
        title: qsTr("Carrier")
        CarrierPanel { anchors.fill: parent }
    }
    Dialog {
        id: tweaksDialog
        modal: false
        width: 440
        height: 560
        title: qsTr("Tweaks")
        TweaksPanel { anchors.fill: parent }
    }
    Dialog {
        id: proxyDialog
        modal: true
        anchors.centerIn: parent
        title: qsTr("Web Proxy")
        standardButtons: Dialog.Cancel | Dialog.Ok
        onAccepted: proxy.apply(proxyEditor.currentJson())
        onRejected: proxyEditor.loadFromAdapter()
        ProxySettings {
            id: proxyEditor
            width: 320
        }
    }
    Dialog {
        id: debugPortDialog
        modal: true
        anchors.centerIn: parent
        title: qsTr("Debug Port")
        standardButtons: Dialog.Ok
        DebugPort { width: 480 }
    }

    // Surface 6: the files browser window (one per device).
    FilesBrowser {
        id: filesWindow
    }
    // Surface 7: the device logs window.
    LogWindow {
        id: logWindow
    }
    // Surface 9: firmware catalog & jobs.
    FirmwareJobsWindow {
        id: firmwareWindow
    }

    Connections {
        target: capture
        // Two-step save handshake (ABI.md §3.8).
        function onSaveDestinationRequested(suggestedName) {
            saveRecordingDialog.suggestedName = suggestedName;
            saveRecordingDialog.open();
        }
        // Completion toasts hook into surface 13 (notifications).
        function onCaptureDone() {
            if (!capture.lastCaptureOk)
                return;
            const isRec = capture.lastCaptureKind === 1;
            notifications.notify(
                isRec ? qsTr("Recording saved") : qsTr("Screenshot saved"),
                capture.lastCapturePath,
                isRec ? "capture-recording" : "capture-screenshot");
            captureSounds.playScreenshot();
        }
        function onRecordingChanged() {
            if (capture.recordingPhase === 2)  // recording
                captureSounds.playRecordingStarted();
            else if (capture.recordingPhase === 0)  // idle
                captureSounds.playRecordingStopped();
        }
        function onScreenshotCopied() {
            toastLabel.text = qsTr("Screenshot copied");
        }
    }

    // kLTEventToast → OS notification + in-app toast (surface 13).
    Connections {
        target: notifications
        function onToastReceived(title, body, category) {
            toastLabel.text = title !== "" ? title + ": " + body : body;
        }
        function onShowDeviceRequested(entryId) {
            window.selectDeviceByEntryId(entryId);
        }
    }

    // Surface 12: dispatch routed drops per file.
    property var pendingDrops: ({})
    function noteDrop(requestId, urls) {
        window.pendingDrops[requestId] = urls;
    }
    Connections {
        target: deviceList
        function onDropRouted(requestId, action, detail) {
            const urls = window.pendingDrops[requestId] || [];
            delete window.pendingDrops[requestId];
            const ipsws = [], ipas = [], media = [];
            for (let i = 0; i < urls.length; ++i) {
                const c = deviceList.classifyDropUrl(urls[i]);
                const local = urls[i].replace(/^file:\/\//, "");
                if (c === 0) ipsws.push(local);
                else if (c === 1) ipas.push(local);
                else if (c === 2) media.push(local);
            }
            if (ipsws.length > 0)
                jobs.importIpsws(ipsws);
            for (let i = 0; i < ipas.length; ++i)
                apps.install(ipas[i]);
            if (media.length > 0)
                apps.importMedia(media);
            if (action === "unsupported" && urls.length > 0)
                toastLabel.text = qsTr("Couldn't open %1").arg(urls[0]);
        }
    }

    function selectDeviceByEntryId(entryId) {
        // The Mac selects the prepared device; the core doesn't expose the
        // entry→device mapping yet, so show the firmware window instead.
        firmwareWindow.show();
        firmwareWindow.raise();
    }

    // First-start network question (App/NetworkAccessPrompt.swift): the
    // boot that asked for it goes ahead once the answer is saved.
    Connections {
        target: device
        function onNetworkAccessPromptRequested() {
            networkAccessDialog.deviceName = window.currentRowName;
            networkAccessDialog.onAnswered = function () { device.boot(); };
            networkAccessDialog.open();
        }
    }
    Connections {
        target: deviceList
        function onNetworkAccessPromptRequested(uuid, name) {
            networkAccessDialog.deviceName = name;
            networkAccessDialog.onAnswered = function () {
                deviceList.bootDeviceConfirmed(uuid);
            };
            networkAccessDialog.open();
        }
    }

    // WindowRestorationPolicy: frame autosave/restore (never contents).
    Component.onCompleted: {
        const g = settings.windowGeometry();
        if (g["width"] > 0 && g["height"] > 0) {
            window.x = g["x"];
            window.y = g["y"];
            window.width = g["width"];
            window.height = g["height"];
        }
    }
    onClosing: {
        settings.saveWindowGeometry(window.x, window.y, window.width, window.height);
    }
}
