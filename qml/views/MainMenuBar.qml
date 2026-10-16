// MainMenuBar.qml — the full menu bar (surface 11).
//
// Qt counterpart of App/MainMenu.swift + MainWindowController+MenuValidation.swift.
// Every menu, every shortcut, every validation rule the Mac has:
//
//  - shortcuts map ⌘→Ctrl, ⌥→Alt, ⌃→Meta, ⇧→Shift;
//  - per-state validation runs on each menu open (Menu.aboutToShow), exactly
//    like the Mac's validateMenuItem: titles morph (Start/Shut Down…,
//    Pause/Resume, Show/Hide …, Start/Stop Recording), checkmarks follow
//    state, and inapplicable items are dimmed;
//  - the Device menu's hardware items validate through lt_menu_validate
//    (LTDeviceAdapter.validateMenuItem: kLTMenuItem* 1..7 per lt_bridge.h §6);
//  - items whose surface hasn't landed are listed but dimmed — the Mac's own
//    rule ("what doesn't apply now is dimmed"), never hidden;
//  - Mac-only items are dropped with a note: Services menu, Hide/Show All,
//    Sparkle "Check for Updates…", toolbar customization, "Open Screenshot in
//    Preview" (→ "Open Screenshot" via the OS default app).
//
// Signals marked SURFACE are wired by main.qml today; the rest arrive with
// their surface (noted per item).

import QtQuick
import QtQuick.Controls

MenuBar {
    id: root

    // kLTMenuItem* constants mirror src/lt_bridge.h §6 (DEVIATION 5).
    readonly property int menuLock: 1
    readonly property int menuRotate: 2
    readonly property int menuCompass: 3
    readonly property int menuCarrier: 4
    readonly property int menuInput: 5
    readonly property int menuPause: 6
    readonly property int menuRestart: 7

    // kLTButton* mirror lt_bridge.h §6.
    readonly property int buttonHome: 1
    readonly property int buttonVolumeUp: 3
    readonly property int buttonVolumeDown: 4

    // Driven by main.qml (SplitView pane visibility).
    property bool sidebarVisible: true
    property bool inspectorVisible: true
    property bool consoleVisible: true
    property string bezelMode: "2d"      // surface 3 applies it (phase 1: flat)
    property int motionPose: 0           // 0 = upright, 1 = flat
    property bool rotateAutomatically: false  // persisted by surface 10
    property bool keyboardInput: true         // surface 2 owns the state
    property bool hardwareKeyboard: false     // surface 2 owns the state
    property bool freeFormActionChecked: freeFormAction.checked

    signal addDeviceRequested()
    signal showSettingsRequested()
    signal showHelpRequested()
    signal jailbreakRequested()
    signal toggleSidebar()
    signal toggleInspector()
    signal toggleConsole()
    signal diagnosticsExported(string path)
    // Surface 4: device inspector panels.
    signal showCarrierRequested()
    signal showTweaksRequested()
    signal showProxyRequested()
    signal showDebugPortRequested()
    // Surface 6/7: files browser, log window.
    signal showFilesRequested()
    signal showLogsRequested()
    // Surface 9: firmware catalog.
    signal downloadPrepareRequested()
    signal findInCatalogRequested()

    // The Mac validates on every menu open; so do we.
    function v(item, arg) {
        const m = device.validateMenuItem(item, arg || 0);
        return {
            enabled: m["enabled"] === true,
            title: m["title"] || "",
            checked: m["checked"] === true
        };
    }
    function hasSelection() { return window.currentSelection.length > 0; }
    function selectedUuid() {
        return window.currentSelection.length === 1 ? window.currentSelection[0] : "";
    }
    function jobForSelected() {
        const uuid = selectedUuid();
        if (uuid === "")
            return null;
        for (let i = 0; i < jobs.jobs.length; ++i) {
            if (jobs.jobs[i]["deviceUuid"] === uuid)
                return jobs.jobs[i];
        }
        return null;
    }
    function activeTextItem() {
        const f = window.activeFocusItem;
        return (f && (f instanceof TextField || f instanceof TextArea || f instanceof TextInput)) ? f : null;
    }
    function askConfirm(title, info, confirmText, fn) {
        confirmLoader.item.ask(title, info, confirmText, fn);
    }

    function refreshValidations() {
        // Morphing titles.
        const running = device.running;
        fileStartAction.text = running ? qsTr("Shut Down…") : qsTr("Start");
        deleteAction.text = window.currentSelection.length > 1
            ? qsTr("Delete %n Devices…", "", window.currentSelection.length)
            : qsTr("Delete Device…");
        pauseAction.text = v(root.menuPause, 0).title || (device.paused ? qsTr("Resume") : qsTr("Pause"));
        lockAction.text = v(root.menuLock, 0).title || qsTr("Lock");
        recordAction.text = capture.recordingCanStop ? qsTr("Stop Recording") : qsTr("Start Recording");
        fingerDotsAction.text = device.fingerDots ? qsTr("Hide Finger Dots") : qsTr("Show Finger Dots");
        sidebarAction.text = root.sidebarVisible ? qsTr("Hide Sidebar") : qsTr("Show Sidebar");
        inspectorAction.text = root.inspectorVisible ? qsTr("Hide Inspector") : qsTr("Show Inspector");
        consoleAction.text = root.consoleVisible ? qsTr("Hide Console") : qsTr("Show Console");
        fullscreenAction.text = window.visibility === Window.FullScreen
            ? qsTr("Exit Full Screen") : qsTr("Enter Full Screen");
        // Checkmarks.
        bezel2dAction.checked = root.bezelMode === "2d";
        bezelOffAction.checked = root.bezelMode === "off";
        fingerDotsAction.checked = device.fingerDots;
        uprightAction.checked = root.motionPose === 0;
        flatAction.checked = root.motionPose === 1;
        rotateAutoAction.checked = root.rotateAutomatically;
        keyboardInputAction.checked = root.keyboardInput;
        hardwareKeyboardAction.checked = root.hardwareKeyboard;
        attachNetworkAction.checked = device.networkState === 1; // kLTNetworkAccessGranted
        // Enablement.
        const sel = root.hasSelection();
        const one = root.selectedUuid() !== "";
        carrierAction.enabled = one;
        tweaksAction.enabled = one;
        proxyAction.enabled = one;
        debugPortAction.enabled = one;
        showFilesAction.enabled = one;
        deviceLogsAction.enabled = true;
        addDeviceAction.enabled = true;
        importIpswAction.enabled = true;
        downloadPrepareAction.enabled = jobs.canDownload;
        cancelDownloadAction.enabled = root.jobForSelected() !== null;
        fileStartAction.enabled = sel;
        deleteAction.enabled = sel;
        closeAction.enabled = true;
        settingsAction.enabled = true;
        quitAction.enabled = true;
        const ti = root.activeTextItem();
        undoAction.enabled = ti !== null;
        redoAction.enabled = ti !== null;
        cutAction.enabled = ti !== null;
        copyAction.enabled = ti !== null;
        pasteAction.enabled = ti !== null;
        deleteTextAction.enabled = ti !== null;
        selectAllAction.enabled = ti !== null;
        pasteToGuestAction.enabled = device.acceptsInput;
        saveScreenshotAction.enabled = capture.canTakeScreenshot;
        saveScreenshotAsAction.enabled = capture.canTakeScreenshot;
        copyScreenshotAction.enabled = capture.canTakeScreenshot;
        openScreenshotAction.enabled = capture.lastCaptureOk;
        recordAction.enabled = capture.canStartRecording || capture.recordingCanStop;
        discardRecordingAction.enabled = capture.recordingCanStop;
        screenOnlyAction.enabled = !capture.recordingCanStop;
        recordBezelAction.enabled = !capture.recordingCanStop;
        findAction.enabled = true;
        liveTextAction.enabled = false;                              // blocked: no Qt Vision equivalent
        sidebarAction.enabled = true;
        inspectorAction.enabled = true;
        consoleAction.enabled = true;
        physicalAction.enabled = running;
        pixelAction.enabled = running;
        fitAction.enabled = running;
        zoomInAction.enabled = running;
        zoomOutAction.enabled = running;
        bezel3dAction.enabled = false;                               // surface 3 phase 2
        freeFormAction.enabled = running;
        nativeSizeAction.enabled = false;                            // not in Mac zoom behavior; stays off
        fingerDotsAction.enabled = device.currentDeviceUuid !== "";
        showHiddenAction.enabled = false;                            // surface 6
        fullscreenAction.enabled = true;
        const inV = root.v(root.menuInput, 0).enabled;
        homeAction.enabled = inV;
        lockAction.enabled = root.v(root.menuLock, 0).enabled;
        rotateLeftAction.enabled = root.v(root.menuRotate, 0).enabled;
        rotateRightAction.enabled = rotateLeftAction.enabled;
        rotateAutoAction.enabled = device.currentDeviceUuid !== "";
        uprightAction.enabled = inV;
        flatAction.enabled = inV;
        resetTiltAction.enabled = inV;
        shakeAction.enabled = inV;
        compassMenu.enabled = root.v(root.menuCompass, 0).enabled;
        northAction.enabled = compassMenu.enabled;
        eastAction.enabled = compassMenu.enabled;
        southAction.enabled = compassMenu.enabled;
        westAction.enabled = compassMenu.enabled;
        northAction.checked = root.v(root.menuCompass, 0).checked;
        eastAction.checked = root.v(root.menuCompass, 90).checked;
        southAction.checked = root.v(root.menuCompass, 180).checked;
        westAction.checked = root.v(root.menuCompass, 270).checked;
        volumeUpAction.enabled = inV;
        volumeDownAction.enabled = inV;
        keyboardInputAction.enabled = device.currentDeviceUuid !== "";
        hardwareKeyboardAction.enabled = device.currentDeviceUuid !== "";
        connectInternetAction.enabled = false;                       // no ABI
        attachNetworkAction.enabled = device.currentDeviceUuid !== "";
        proxyAction.enabled = false;                                 // surface 4
        carrierAction.enabled = false;                               // surface 4
        tweaksAction.enabled = false;                                // surface 4
        debugPortAction.enabled = false;                             // surface 4
        copyLldbAction.enabled = false;                              // surface 4
        pauseAction.enabled = root.v(root.menuPause, 0).enabled;
        restartAction.enabled = root.v(root.menuRestart, 0).enabled;
        shutdownAction.enabled = device.running;
        forceStopAction.enabled = device.running;
        jailbreakAction.enabled = false; // blocked: no lt_device_jailbreak in the ABI
        eraseAction.enabled = sel;
        installAppAction.enabled = apps.canQueueInstall;
        importMediaAction.enabled = apps.canQueueInstall;
        refreshAppsAction.enabled = device.currentDeviceUuid !== "";
        saveScreenshotAction.enabled = capture.canTakeScreenshot;
        saveScreenshotAsAction.enabled = false;                      // needs core save-as
        copyScreenshotAction.enabled = false;                        // surface 2 frame path
        openScreenshotAction.enabled = capture.lastCaptureOk;
        recordAction.enabled = capture.canToggleRecording;
        discardRecordingAction.enabled = capture.recordingCanStop;
        screenOnlyAction.enabled = false;                            // surface 8 prefs
        recordBezelAction.enabled = false;                           // surface 8 prefs
        minimizeAction.enabled = true;
        zoomWindowAction.enabled = true;
        showDeviceAction.enabled = true;
        showFilesAction.enabled = false;                             // surface 6
        deviceLogsAction.enabled = false;                            // surface 7
        bringAllAction.enabled = true;
        helpAction.enabled = true;
        exportDiagnosticsAction.enabled = true;
        copyBugReportAction.enabled = true;
        aboutAction.enabled = true;
    }

    // --- File -----------------------------------------------------------
    Menu {
        id: fileMenu
        objectName: "fileMenu"
        title: qsTr("&File")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "addDeviceAction"
            id: addDeviceAction
            text: qsTr("&Add Device…")
            shortcut: "Ctrl+N"
            onTriggered: root.addDeviceRequested()
        }
        MenuSeparator {}
        Action {
            objectName: "importIpswAction"
            id: importIpswAction
            text: qsTr("&Import IPSW…")
            onTriggered: {
                const picked = jobs.pickIpswFiles();
                if (picked.length > 0)
                    jobs.importIpsws(picked);
            }
        }
        Action {
            objectName: "downloadPrepareAction"
            id: downloadPrepareAction
            text: qsTr("&Download and Prepare")
            onTriggered: root.downloadPrepareRequested()
        }
        Action {
            objectName: "cancelDownloadAction"
            id: cancelDownloadAction
            text: qsTr("&Cancel Download")
            onTriggered: {
                const job = root.jobForSelected();
                if (job)
                    jobs.cancel(job["entryId"]);
            }
        }
        MenuSeparator {}
        Action {
            objectName: "fileStartAction"
            id: fileStartAction
            text: qsTr("&Start")
            onTriggered: {
                if (device.running)
                    root.askConfirm(qsTr("Shut down this device?"),
                                    qsTr("The guest OS shuts down cleanly; unsaved state in apps is kept."),
                                    qsTr("Shut Down"), function () { device.halt(); });
                else
                    device.boot();
            }
        }
        MenuSeparator {}
        Action {
            objectName: "deleteAction"
            id: deleteAction
            text: qsTr("&Delete Device…")
            shortcut: "Ctrl+Backspace"
            onTriggered: deviceSidebar.deleteSelected()
        }
        MenuSeparator {}
        Action {
            objectName: "settingsAction"
            id: settingsAction
            text: qsTr("&Settings…")
            shortcut: "Ctrl+,"
            onTriggered: root.showSettingsRequested()
        }
        Action {
            objectName: "closeAction"
            id: closeAction
            text: qsTr("&Close")
            shortcut: "Ctrl+W"
            onTriggered: window.close()
        }
        MenuSeparator {}
        Action {
            objectName: "quitAction"
            id: quitAction
            text: qsTr("&Quit")
            shortcut: "Ctrl+Q"
            onTriggered: settings.quitRequest()
        }
    }

    // --- Edit -----------------------------------------------------------
    Menu {
        id: editMenu
        objectName: "editMenu"
        title: qsTr("&Edit")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "undoAction"
            id: undoAction
            text: qsTr("&Undo")
            shortcut: "Ctrl+Z"
            onTriggered: { const f = root.activeTextItem(); if (f) f.undo(); }
        }
        Action {
            objectName: "redoAction"
            id: redoAction
            text: qsTr("&Redo")
            shortcut: "Ctrl+Shift+Z"
            onTriggered: { const f = root.activeTextItem(); if (f) f.redo(); }
        }
        MenuSeparator {}
        Action {
            objectName: "cutAction"
            id: cutAction
            text: qsTr("Cu&t")
            shortcut: "Ctrl+X"
            onTriggered: { const f = root.activeTextItem(); if (f) f.cut(); }
        }
        Action {
            objectName: "copyAction"
            id: copyAction
            text: qsTr("&Copy")
            shortcut: "Ctrl+C"
            onTriggered: { const f = root.activeTextItem(); if (f) f.copy(); }
        }
        Action {
            objectName: "pasteAction"
            id: pasteAction
            text: qsTr("&Paste")
            shortcut: "Ctrl+V"
            onTriggered: { const f = root.activeTextItem(); if (f) f.paste(); }
        }
        Action {
            objectName: "deleteTextAction"
            id: deleteTextAction
            text: qsTr("&Delete")
            onTriggered: {
                const f = root.activeTextItem();
                if (f)
                    f.text = f.text.slice(0, f.selectionStart) + f.text.slice(f.selectionEnd);
            }
        }
        Action {
            objectName: "selectAllAction"
            id: selectAllAction
            text: qsTr("Select &All")
            shortcut: "Ctrl+A"
            onTriggered: { const f = root.activeTextItem(); if (f) f.selectAll(); }
        }
        MenuSeparator {}
        Action {
            objectName: "pasteToGuestAction"
            id: pasteToGuestAction
            text: qsTr("Paste Text to Device")
            shortcut: "Ctrl+Meta+V"
            onTriggered: device.pasteToGuest()
        }
        MenuSeparator {}
        Action {
            objectName: "findAction"
            id: findAction
            text: qsTr("&Find…")
            shortcut: "Ctrl+F"
            onTriggered: root.findInCatalogRequested()
        }
        Action {
            objectName: "liveTextAction"
            id: liveTextAction
            text: qsTr("Select Text on Screen")
        }
    }

    // --- View -----------------------------------------------------------
    Menu {
        id: viewMenu
        objectName: "viewMenu"
        title: qsTr("&View")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "sidebarAction"
            id: sidebarAction
            text: qsTr("Show Sidebar")
            shortcut: "Ctrl+Meta+S"
            onTriggered: root.toggleSidebar()
        }
        Action {
            objectName: "inspectorAction"
            id: inspectorAction
            text: qsTr("Show Inspector")
            shortcut: "Ctrl+Alt+I"
            onTriggered: root.toggleInspector()
        }
        Action {
            objectName: "consoleAction"
            id: consoleAction
            text: qsTr("Show Console")
            shortcut: "Ctrl+Shift+Y"
            onTriggered: root.toggleConsole()
        }
        MenuSeparator {}
        Action {
            objectName: "physicalAction"
            id: physicalAction
            text: qsTr("Physical Size")
            shortcut: "Ctrl+8"
            onTriggered: display.setZoomModeExplicit("physical")
        }
        Action {
            objectName: "pixelAction"
            id: pixelAction
            text: qsTr("Pixel Accurate")
            shortcut: "Ctrl+9"
            onTriggered: display.setZoomModeExplicit("pixel")
        }
        Action {
            objectName: "fitAction"
            id: fitAction
            text: qsTr("Zoom to Fit")
            shortcut: "Ctrl+0"
            onTriggered: display.setZoomModeExplicit("fit")
        }
        Action {
            objectName: "zoomInAction"
            id: zoomInAction
            text: qsTr("Zoom &In")
            shortcut: "Ctrl+Plus"
            onTriggered: display.zoomIn()
        }
        Action {
            objectName: "zoomOutAction"
            id: zoomOutAction
            text: qsTr("Zoom &Out")
            shortcut: "Ctrl+Minus"
            onTriggered: display.zoomOut()
        }
        MenuSeparator {}
        Menu {
            title: qsTr("Device Bezels")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "bezel3dAction"
                id: bezel3dAction
                text: qsTr("3D")
                checkable: true
            }
            Action {
                objectName: "bezel2dAction"
                id: bezel2dAction
                text: qsTr("2D")
                checkable: true
                onTriggered: root.bezelMode = "2d"
            }
            Action {
                objectName: "bezelOffAction"
                id: bezelOffAction
                text: qsTr("Off")
                checkable: true
                onTriggered: root.bezelMode = "off"
            }
        }
        Action {
            objectName: "freeFormAction"
            id: freeFormAction
            text: qsTr("Free-Form Screen")
            checkable: true
        }
        Action {
            objectName: "nativeSizeAction"
            id: nativeSizeAction
            text: qsTr("Native Size")
        }
        Action {
            objectName: "fingerDotsAction"
            id: fingerDotsAction
            text: qsTr("Show Finger Dots")
            checkable: true
            onTriggered: device.setFingerDots(!device.fingerDots)
        }
        Action {
            objectName: "showHiddenAction"
            id: showHiddenAction
            text: qsTr("Show Hidden Files")
            checkable: true
        }
        MenuSeparator {}
        Action {
            objectName: "fullscreenAction"
            id: fullscreenAction
            text: qsTr("Enter Full Screen")
            shortcut: "Ctrl+Meta+F"
            onTriggered: {
                window.visibility = window.visibility === Window.FullScreen
                    ? Window.Windowed : Window.FullScreen;
            }
        }
    }

    // --- Device ---------------------------------------------------------
    Menu {
        id: deviceMenu
        objectName: "deviceMenu"
        title: qsTr("&Device")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "homeAction"
            id: homeAction
            text: qsTr("Home Screen")
            shortcut: "Ctrl+Shift+H"
            onTriggered: device.pressButton(root.buttonHome)
        }
        Action {
            objectName: "lockAction"
            id: lockAction
            text: qsTr("Lock")
            shortcut: "Ctrl+L"
            onTriggered: device.setLocked(true)
        }
        MenuSeparator {}
        Action {
            objectName: "rotateLeftAction"
            id: rotateLeftAction
            text: qsTr("Rotate Left")
            shortcut: "Ctrl+Left"
            onTriggered: device.rotateGuest(-90)
        }
        Action {
            objectName: "rotateRightAction"
            id: rotateRightAction
            text: qsTr("Rotate Right")
            shortcut: "Ctrl+Right"
            onTriggered: device.rotateGuest(90)
        }
        Action {
            objectName: "rotateAutoAction"
            id: rotateAutoAction
            text: qsTr("Rotate Automatically")
            checkable: true
            onTriggered: root.rotateAutomatically = !root.rotateAutomatically
        }
        MenuSeparator {}
        Menu {
            title: qsTr("Motion")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "uprightAction"
                id: uprightAction
                text: qsTr("Upright")
                checkable: true
                onTriggered: { root.motionPose = 0; device.setMotionPose(0); }
            }
            Action {
                objectName: "flatAction"
                id: flatAction
                text: qsTr("Flat")
                checkable: true
                onTriggered: { root.motionPose = 1; device.setMotionPose(1); }
            }
            MenuSeparator {}
            Action {
                objectName: "resetTiltAction"
                id: resetTiltAction
                text: qsTr("Reset Tilt")
                onTriggered: { root.motionPose = 0; device.setMotionPose(0); }
            }
            Action {
                objectName: "shakeAction"
                id: shakeAction
                text: qsTr("Shake")
                onTriggered: device.shake()
            }
        }
        Menu {
            objectName: "compassMenu"
            id: compassMenu
            title: qsTr("Compass Heading")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "northAction"
                id: northAction
                text: qsTr("North")
                checkable: true
                onTriggered: device.setCompassHeading(0)
            }
            Action {
                objectName: "eastAction"
                id: eastAction
                text: qsTr("East")
                checkable: true
                onTriggered: device.setCompassHeading(90)
            }
            Action {
                objectName: "southAction"
                id: southAction
                text: qsTr("South")
                checkable: true
                onTriggered: device.setCompassHeading(180)
            }
            Action {
                objectName: "westAction"
                id: westAction
                text: qsTr("West")
                checkable: true
                onTriggered: device.setCompassHeading(270)
            }
        }
        Menu {
            title: qsTr("Input")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "volumeUpAction"
                id: volumeUpAction
                text: qsTr("Volume Up")
                shortcut: "Ctrl+Alt+Up"
                onTriggered: device.pressButton(root.buttonVolumeUp)
            }
            Action {
                objectName: "volumeDownAction"
                id: volumeDownAction
                text: qsTr("Volume Down")
                shortcut: "Ctrl+Alt+Down"
                onTriggered: device.pressButton(root.buttonVolumeDown)
            }
            MenuSeparator {}
            Action {
                objectName: "keyboardInputAction"
                id: keyboardInputAction
                text: qsTr("Send Keyboard Input")
                checkable: true
                onTriggered: root.keyboardInput = !root.keyboardInput
            }
            Action {
                objectName: "hardwareKeyboardAction"
                id: hardwareKeyboardAction
                text: qsTr("Connect Hardware Keyboard")
                shortcut: "Ctrl+Shift+K"
                checkable: true
                onTriggered: root.hardwareKeyboard = !root.hardwareKeyboard
            }
        }
        Menu {
            title: qsTr("Network")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "connectInternetAction"
                id: connectInternetAction
                text: qsTr("Connect to the Internet")
            }
            Action {
                objectName: "attachNetworkAction"
                id: attachNetworkAction
                text: qsTr("Attach to Local Network")
                checkable: true
                onTriggered: device.setNetworkAllowed(!attachNetworkAction.checked)
            }
            MenuSeparator {}
            Action {
                objectName: "proxyAction"
                id: proxyAction
                text: qsTr("Proxy…")
                onTriggered: root.showProxyRequested()
            }
        }
        Action {
            objectName: "carrierAction"
            id: carrierAction
            text: qsTr("Carrier…")
            onTriggered: root.showCarrierRequested()
        }
        Action {
            objectName: "tweaksAction"
            id: tweaksAction
            text: qsTr("Tweaks…")
            onTriggered: root.showTweaksRequested()
        }
        Menu {
            title: qsTr("Debugging")
            onAboutToShow: root.refreshValidations()
            Action {
                objectName: "debugPortAction"
                id: debugPortAction
                text: qsTr("Debug Port…")
                onTriggered: root.showDebugPortRequested()
            }
            Action {
                objectName: "copyLldbAction"
                id: copyLldbAction
                text: qsTr("Copy lldb Command")
            }
        }
        MenuSeparator {}
        Action {
            objectName: "pauseAction"
            id: pauseAction
            text: qsTr("Pause")
            onTriggered: device.setPaused(!device.paused)
        }
        MenuSeparator {}
        Action {
            objectName: "restartAction"
            id: restartAction
            text: qsTr("Restart…")
            onTriggered: {
                root.askConfirm(qsTr("Restart this device?"),
                                qsTr("The guest reboots. Unsaved state in apps may be lost."),
                                qsTr("Restart"),
                                function () { deviceList.resetDevice(root.selectedUuid()); });
            }
        }
        Action {
            objectName: "shutdownAction"
            id: shutdownAction
            text: qsTr("Shut Down…")
            shortcut: "Ctrl+."
            onTriggered: {
                root.askConfirm(qsTr("Shut down this device?"),
                                qsTr("The guest OS shuts down cleanly; unsaved state in apps is kept."),
                                qsTr("Shut Down"), function () { device.halt(); });
            }
        }
        Action {
            objectName: "forceStopAction"
            id: forceStopAction
            text: qsTr("Force Stop…")
            onTriggered: {
                root.askConfirm(qsTr("Force stop this device?"),
                                qsTr("The emulated device is stopped immediately. Unsaved guest state may be lost."),
                                qsTr("Force Stop"), function () { device.halt(); });
            }
        }
        MenuSeparator {}
        Action {
            objectName: "jailbreakAction"
            id: jailbreakAction
            text: qsTr("Jailbreak…")
            onTriggered: root.jailbreakRequested()
        }
        Action {
            objectName: "eraseAction"
            id: eraseAction
            text: qsTr("Erase All Content and Settings…")
            onTriggered: {
                root.askConfirm(qsTr("Erase this device?"),
                                qsTr("All apps, settings and saved state are removed. This cannot be undone."),
                                qsTr("Erase"),
                                function () { deviceList.eraseDevice(root.selectedUuid()); });
            }
        }
    }

    // --- Apps -----------------------------------------------------------
    Menu {
        id: appsMenu
        objectName: "appsMenu"
        title: qsTr("&Apps")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "installAppAction"
            id: installAppAction
            text: qsTr("Install App…")
            shortcut: "Ctrl+Shift+I"
            onTriggered: apps.pickAndInstallApp()
        }
        Action {
            objectName: "importMediaAction"
            id: importMediaAction
            text: qsTr("Import Media…")
            onTriggered: apps.pickAndImportMedia()
        }
        MenuSeparator {}
        Action {
            objectName: "refreshAppsAction"
            id: refreshAppsAction
            text: qsTr("Refresh Apps")
            onTriggered: apps.refresh()
        }
    }

    // --- Capture --------------------------------------------------------
    Menu {
        id: captureMenu
        objectName: "captureMenu"
        title: qsTr("&Capture")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "saveScreenshotAction"
            id: saveScreenshotAction
            text: qsTr("Save Screenshot")
            shortcut: "Ctrl+S"
            onTriggered: capture.screenshot()
        }
        Action {
            objectName: "saveScreenshotAsAction"
            id: saveScreenshotAsAction
            text: qsTr("Save Screenshot As…")
            shortcut: "Ctrl+Shift+S"
            onTriggered: capture.screenshotSaveAs()
        }
        Action {
            objectName: "copyScreenshotAction"
            id: copyScreenshotAction
            text: qsTr("Copy Screenshot")
            onTriggered: capture.screenshotCopy()
        }
        Action {
            objectName: "openScreenshotAction"
            id: openScreenshotAction
            text: qsTr("Open Screenshot")
            onTriggered: capture.openLastCapture()
        }
        MenuSeparator {}
        Action {
            objectName: "recordAction"
            id: recordAction
            text: qsTr("Start Recording")
            shortcut: "Ctrl+R"
            onTriggered: {
                if (capture.recordingCanStop)
                    capture.stopRecording(false);
                else
                    capture.startRecording();
            }
        }
        Action {
            objectName: "discardRecordingAction"
            id: discardRecordingAction
            text: qsTr("Discard Recording…")
            shortcut: "Escape"
            onTriggered: {
                root.askConfirm(qsTr("Discard the recording?"),
                                qsTr("The in-progress recording is discarded."),
                                qsTr("Discard"),
                                function () { capture.stopRecording(true); });
            }
        }
        MenuSeparator {}
        Action {
            objectName: "screenOnlyAction"
            id: screenOnlyAction
            text: qsTr("Capture Screen Only")
            checkable: true
            checked: settings.captureScreenOnly
            onTriggered: settings.captureScreenOnly = !settings.captureScreenOnly
        }
        Action {
            objectName: "recordBezelAction"
            id: recordBezelAction
            text: qsTr("Record Device Bezel")
            checkable: true
            checked: !!(capture.prefs && capture.prefs["recordBezel"])
            onTriggered: {
                const p = Object.assign({}, capture.prefs || {});
                p["recordBezel"] = !p["recordBezel"];
                capture.setPrefs(JSON.stringify(p));
            }
        }
    }

    // --- Window ---------------------------------------------------------
    Menu {
        id: windowMenu
        objectName: "windowMenu"
        title: qsTr("&Window")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "minimizeAction"
            id: minimizeAction
            text: qsTr("Minimize")
            shortcut: "Ctrl+M"
            onTriggered: window.showMinimized()
        }
        Action {
            objectName: "zoomWindowAction"
            id: zoomWindowAction
            text: qsTr("Zoom")
            onTriggered: {
                if (window.visibility === Window.Maximized)
                    window.showNormal();
                else
                    window.showMaximized();
            }
        }
        MenuSeparator {}
        Action {
            objectName: "showDeviceAction"
            id: showDeviceAction
            text: qsTr("Show Device")
            shortcut: "Ctrl+1"
            onTriggered: { window.raise(); window.requestActivate(); }
        }
        Action {
            objectName: "showFilesAction"
            id: showFilesAction
            text: qsTr("Show Files")
            shortcut: "Ctrl+2"
            onTriggered: root.showFilesRequested()
        }
        Action {
            objectName: "deviceLogsAction"
            id: deviceLogsAction
            text: qsTr("Device Logs")
            onTriggered: root.showLogsRequested()
        }
        MenuSeparator {}
        Action {
            objectName: "bringAllAction"
            id: bringAllAction
            text: qsTr("Bring All to Front")
            onTriggered: { window.raise(); window.requestActivate(); }
        }
    }

    // --- Help -----------------------------------------------------------
    Menu {
        id: helpMenu
        objectName: "helpMenu"
        title: qsTr("&Help")
        onAboutToShow: root.refreshValidations()
        Action {
            objectName: "helpAction"
            id: helpAction
            text: qsTr("LightTouch Help")
            shortcut: "Ctrl+?"
            onTriggered: root.showHelpRequested()
        }
        MenuSeparator {}
        Action {
            objectName: "exportDiagnosticsAction"
            id: exportDiagnosticsAction
            text: qsTr("Export Diagnostics…")
            onTriggered: {
                const path = settings.diagnosticsExport();
                if (path !== "")
                    root.diagnosticsExported(path);
            }
        }
        Action {
            objectName: "copyBugReportAction"
            id: copyBugReportAction
            text: qsTr("Copy Bug Report Info")
            onTriggered: settings.copyBugReportInfo()
        }
        MenuSeparator {}
        Action {
            objectName: "aboutAction"
            id: aboutAction
            text: qsTr("About LightTouch")
            onTriggered: aboutDialog.open()
        }
    }

    // Shared confirmation dialog (guarded device commands).
    Loader {
        id: confirmLoader
        source: Qt.resolvedUrl("../dialogs/ConfirmDialog.qml")
    }
}
