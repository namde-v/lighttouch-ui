// DeviceScreen.qml — the running-device display (surface 2).
//
// Qt counterpart of UI/DisplayView.swift (+Frames, +Input, +Keyboard, +Tilt,
// +Zoom, +FreeForm, +Layout): the flat bezel shell (surface 3 phase 1) with
// the guest screen, power badges, home button, finger-dot overlay, and all
// input routing.
//
// Frames: DisplayItem presents the shm frame ring (DisplayLinkReady → map;
// FrameAvailable → acquire on the UI thread → upload on the render thread).
// In mock mode the ring is synthetic (animated test pattern, MOCK-ONLY).
//
// Input rules (from DisplayView+Input.swift):
//  - press/drag/release on the screen (or within the 14pt edge margin) is a
//    guest touch via lt_input_touch; the margin is scaled to item pixels.
//  - wheel over the screen is one finger flick (begin/move/end).
//  - pinch over the screen is a GUEST pinch (two touchIds around the anchor,
//    spread tracked continuously) — never host zoom.
//  - keyboard: text → lt_input_text; special keys → lt_input_key with a
//    best-effort Qt→HID map (ABI.md §6 gap 8).
//  - Alt+drag tilts (lt_input_tilt); the attitude indicator shows pitch/roll
//    and clicks to level.
// Zoom: Fit / Physical / Pixel-Accurate + ⌘+/− ladder (lt_zoom_stops_json /
// lt_zoom_step), remembered per board. Physical falls back to Fit: Qt reports
// no physical size here (ABI.md §6 gap 9).
// Free-form: edge/corner handles; Return applies via lt_display_set_panel_size
// (restarts the device when running), Esc reverts.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LightTouch 1.0

Item {
    id: root

    // Driven by the pane / menu bar.
    property var row: null
    property var display: null // LTDisplayAdapter, passed through from the context
    property bool freeForm: false
    property bool keyboardInputEnabled: true
    // Surface 3 (phase 1: flat bezel only): "2d" shows the flat shell chrome,
    // "off" shows the screen alone. Free-form forces the bezel off and the
    // preference is restored on exit (derived, not stored).
    property string bezelMode: "2d"
    readonly property string effectiveBezel: root.freeForm ? "off" : root.bezelMode

    // Tilt state (local; the core keeps the authoritative accelerometer).
    property double tiltAngle: 0
    property double tiltPitch: 0
    property bool tilting: false

    // Active guest touches, for the finger-dot overlay: [{x, y} item coords].
    property var activeTouches: []

    readonly property bool acceptsInput: device.acceptsInput && root.row !== null

    // The screen item, sized at guest pixels × zoom points.
    DisplayItem {
        id: screen
        display: root.display
        anchors.centerIn: parent
        width: (display ? display.frameWidth : 0) * (display ? display.zoomPoints : 1)
        height: (display ? display.frameHeight : 0) * (display ? display.zoomPoints : 1)
        rotation: display ? display.rotationDegrees : 0
        drawsNearest: false
        visible: display ? display.linkReady : false
    }

    // Fit context for the zoom ladder (Mac: 16pt inset, not a fraction).
    function updateZoomContext() {
        const fit = Math.min(Math.max(root.width - 32, 1) / Math.max(display.frameWidth, 1),
                              Math.max(root.height - 32, 1) / Math.max(display.frameHeight, 1));
        // No physical-size data from Qt here (ABI.md §6 gap 9): -1 → Fit.
        display.setZoomContext(fit, -1, Screen.devicePixelRatio || 1);
    }
    onWidthChanged: updateZoomContext()
    onHeightChanged: updateZoomContext()
    Component.onCompleted: updateZoomContext()

    // Power dim + badge (Mac: sleeping 0.45 / off 0.25; screen hidden when off).
    // The placeholder owns the off state; here only sleeping dims.
    Rectangle {
        anchors.fill: screen
        color: "black"
        opacity: device.sleeping ? 0.55 : 0
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }
    Button {
        anchors.centerIn: screen
        visible: device.sleeping
        text: qsTr("Wake")
        onClicked: device.setLocked(false)
    }

    // Home button overlay (physical home button boards): the flat shell
    // chrome (surface 3 phase 1). Hidden with the bezel off.
    Rectangle {
        id: homeButton
        width: 44; height: 44; radius: 22
        color: "transparent"
        border.color: palette.mid
        border.width: 2
        anchors.top: screen.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: screen.horizontalCenter
        visible: screen.visible && root.effectiveBezel === "2d"
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: device.pressButton(1) // kLTButtonHome
        }
    }

    // Finger-dot overlay (View ▸ Show Finger Dots; Qt owns the rendering).
    Repeater {
        model: device.fingerDots ? root.activeTouches : []
        delegate: Rectangle {
            width: 28; height: 28; radius: 14
            color: "transparent"
            border.color: "white"
            border.width: 2
            opacity: 0.8
            x: modelData.x - 14
            y: modelData.y - 14
        }
    }

    // Attitude indicator (horizon widget): live pitch/roll, click to level.
    Rectangle {
        id: attitude
        width: 120; height: 44; radius: 6
        color: palette.window
        border.color: palette.mid
        visible: root.tilting || root.tiltAngle !== 0 || root.tiltPitch !== 0
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        RowLayout {
            anchors.fill: parent
            anchors.margins: 6
            spacing: 6
            Label { text: qsTr("Tilt"); opacity: 0.7; font.pointSize: 9 }
            Rectangle {
                Layout.fillWidth: true
                height: 6
                color: palette.mid
                Rectangle {
                    width: 6; height: 6; radius: 3
                    color: palette.highlight
                    x: (parent.width - 6) / 2 + root.tiltAngle * 40
                    y: 0
                }
            }
            Label {
                font.pointSize: 9
                text: qsTr("P %1° R %2°")
                    .arg(Math.round(root.tiltPitch * 57.3))
                    .arg(Math.round(root.tiltAngle * 57.3))
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: root.levelTilt()
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Click to level")
        }
    }

    function levelTilt() {
        root.tiltAngle = 0;
        root.tiltPitch = 0;
        root.tilting = false;
        display.sendTilt(0, 0);
    }

    // --- Input -----------------------------------------------------------

    // Normalized 0..1 (y-down) with the 14pt edge margin, scaled to item px.
    function normalized(px, py) {
        if (!root.acceptsInput)
            return null;
        const w = screen.width, h = screen.height;
        if (w <= 0 || h <= 0)
            return null;
        if (px >= 0 && px <= w && py >= 0 && py <= h)
            return [px / w, py / h];
        const m = 14 * (w / Math.max(display.frameWidth, 1));
        if (px >= -m && px <= w + m && py >= -m && py <= h + m)
            return [Math.min(Math.max(px / w, 0), 1),
                    Math.min(Math.max(py / h, 0), 1)];
        return null;
    }

    function setTouch(i, p) {
        const t = root.activeTouches.slice();
        t[i] = { x: screen.x + p[0] * screen.width, y: screen.y + p[1] * screen.height };
        root.activeTouches = t;
    }
    function clearTouches() { root.activeTouches = []; }

    MouseArea {
        id: mouse
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        focus: true

        property var pressPoint: null
        property bool altTilt: false

        onPressed: function (e) {
            forceActiveFocus();
            if (e.button === Qt.RightButton) {
                contextMenu.popup();
                return;
            }
            const lp = mapToItem(screen, e.x, e.y);
            if ((e.modifiers & Qt.AltModifier) && root.acceptsInput) {
                // Alt+drag tilts the chassis (DisplayView+Tilt).
                altTilt = true;
                root.tilting = true;
                pressPoint = [e.x, e.y];
                e.accepted = true;
                return;
            }
            const p = root.normalized(lp.x, lp.y);
            if (p) {
                pressPoint = p;
                display.sendTouch(0, p[0], p[1], 0); // kLTTouchBegan
                root.setTouch(0, p);
            }
        }
        onPositionChanged: function (e) {
            if (altTilt && pressPoint) {
                root.tiltAngle = (e.x - pressPoint[0]) / 200;
                root.tiltPitch = -(e.y - pressPoint[1]) / 200;
                display.sendTilt(root.tiltAngle, root.tiltPitch);
                return;
            }
            if (pressPoint && (e.buttons & Qt.LeftButton)) {
                const lp = mapToItem(screen, e.x, e.y);
                // Clamped: a drag that drifts off the edge keeps tracking.
                const w = screen.width, h = screen.height;
                const p = [Math.min(Math.max(lp.x / w, 0), 1),
                           Math.min(Math.max(lp.y / h, 0), 1)];
                pressPoint = p;
                display.sendTouch(1, p[0], p[1], 0); // kLTTouchMoved
                root.setTouch(0, p);
            }
        }
        onReleased: function (e) {
            if (altTilt) {
                root.levelTilt();
                altTilt = false;
                pressPoint = null;
                return;
            }
            if (pressPoint) {
                display.sendTouch(2, pressPoint[0], pressPoint[1], 0); // kLTTouchEnded
                pressPoint = null;
                root.clearTouches();
            }
        }
        onWheel: function (e) {
            // Over the screen a wheel is one finger flick (Mac: wheelFlick).
            const lp = mapToItem(screen, e.x, e.y);
            const p = root.normalized(lp.x, lp.y);
            if (!p || !root.acceptsInput)
                return;
            const s = Math.max(screen.width / Math.max(display.frameWidth, 1), 0.01);
            const dx = e.angleDelta.x / 8 / s / Math.max(display.frameWidth, 1);
            const dy = e.angleDelta.y / 8 / s / Math.max(display.frameHeight, 1);
            display.sendTouch(0, p[0], p[1], 0);
            const q = [Math.min(Math.max(p[0] + dx, 0), 1),
                       Math.min(Math.max(p[1] + dy, 0), 1)];
            display.sendTouch(1, q[0], q[1], 0);
            display.sendTouch(2, q[0], q[1], 0);
            e.accepted = true;
        }
    }

    // Pinch over the screen is the GUEST's pinch (two touchIds around the
    // anchor, spread tracked continuously) — never host zoom.
    PinchHandler {
        id: pinch
        target: null
        property var anchor: null
        property double spread: 0.12
        property double lastScale: 1
        onActiveChanged: {
            if (active) {
                const c = pinch.centroid.position;
                const lp = root.mapToItem(screen, c.x, c.y);
                anchor = root.normalized(lp.x, lp.y) || [0.5, 0.5];
                spread = 0.12;
                lastScale = 1;
                sendPinch(0);
            } else if (anchor) {
                sendPinch(2);
                anchor = null;
                root.clearTouches();
            }
        }
        onScaleChanged: {
            if (!active || !anchor)
                return;
            spread = Math.min(Math.max(spread * (scale / lastScale), 0.01), 0.6);
            lastScale = scale;
            sendPinch(1);
        }
        function sendPinch(phase) {
            if (!root.acceptsInput)
                return;
            const x1 = Math.min(Math.max(anchor[0] - spread, 0), 1);
            const x2 = Math.min(Math.max(anchor[0] + spread, 0), 1);
            display.sendTouch(phase, x1, anchor[1], 0);
            display.sendTouch(phase, x2, anchor[1], 1);
            root.setTouch(0, [x1, anchor[1]]);
            const t = root.activeTouches.slice();
            t[1] = { x: screen.x + x2 * screen.width, y: screen.y + anchor[1] * screen.height };
            root.activeTouches = t;
        }
    }

    // Keyboard → guest (DisplayView+Keyboard, minus the exotic modes).
    Keys.onPressed: function (e) {
        if (!root.acceptsInput || !root.keyboardInputEnabled)
            return;
        if (e.text !== "") {
            display.typeText(e.text);
        } else {
            display.sendKey(e.key, 1, e.modifiers);
        }
        e.accepted = true;
    }
    Keys.onReleased: function (e) {
        if (!root.acceptsInput || !root.keyboardInputEnabled)
            return;
        if (e.text === "")
            display.sendKey(e.key, 0, e.modifiers);
        e.accepted = true;
    }

    // Right-click menu (Mac: Home Screen, Lock, Rotate L/R, Shake, Copy/Save).
    Menu {
        id: contextMenu
        MenuItem {
            text: qsTr("Home Screen")
            onTriggered: device.pressButton(1)
        }
        MenuItem {
            text: qsTr("Lock")
            onTriggered: device.setLocked(true)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Rotate Left")
            onTriggered: device.rotateGuest(-90)
        }
        MenuItem {
            text: qsTr("Rotate Right")
            onTriggered: device.rotateGuest(90)
        }
        MenuItem {
            text: qsTr("Shake")
            onTriggered: device.shake()
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Save Screenshot")
            enabled: capture.canTakeScreenshot
            onTriggered: capture.screenshot()
        }
    }

    // --- Free-form resize ------------------------------------------------
    property var freeFormStart: null   // {w, h} guest px at drag start

    function freeFormHandles() { return root.freeForm && display.linkReady; }

    // 8 handles; corners do both axes. ⌥ (Alt) resizes symmetrically.
    Repeater {
        model: root.freeFormHandles() ? 8 : 0
        delegate: Rectangle {
            width: 12; height: 12; radius: 2
            color: palette.highlight
            visible: root.freeFormHandles()
            // 3×3 grid minus the center, in screen-item coords.
            property int handleCol: 1
            property int handleRow: 1
            Component.onCompleted: {
                const idx = index >= 4 ? index + 1 : index;
                handleCol = idx % 3;
                handleRow = Math.floor(idx / 3);
                x = screen.x + (screen.width * handleCol) / 2 - 6;
                y = screen.y + (screen.height * handleRow) / 2 - 6;
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeAllCursor
                property var dragStart: null
                onPressed: function (e) {
                    dragStart = [e.x, e.y, display.frameWidth, display.frameHeight];
                }
                onPositionChanged: function (e) {
                    if (!dragStart)
                        return;
                    const ppp = display.zoomPoints || 1;
                    let dw = (e.x - dragStart[0]) / ppp;
                    let dh = (e.y - dragStart[1]) / ppp;
                    const sym = (e.modifiers & Qt.AltModifier) ? 2 : 1;
                    if (parent.handleCol === 0) dw = -dw * sym;
                    else if (parent.handleCol === 1) dw = 0;
                    else dw *= sym;
                    if (parent.handleRow === 0) dh = -dh * sym;
                    else if (parent.handleRow === 1) dh = 0;
                    else dh *= sym;
                    const nw = Math.max(160, Math.round(dragStart[2] + dw));
                    const nh = Math.max(160, Math.round(dragStart[3] + dh));
                    root.freeFormSize = [nw, nh];
                }
            }
        }
    }
    property var freeFormSize: null
    // Live readout while dragging.
    Label {
        visible: root.freeFormSize !== null
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 10
        padding: 6
        background: Rectangle { color: palette.window; border.color: palette.mid; radius: 4 }
        text: root.freeFormSize ? qsTr("%1 × %2 — Return applies, Esc reverts")
              .arg(root.freeFormSize[0]).arg(root.freeFormSize[1]) : ""
    }
    Keys.onReturnPressed: {
        if (root.freeForm && root.freeFormSize) {
            const w = root.freeFormSize[0], h = root.freeFormSize[1];
            display.setPanelSize(w, h);
            if (device.running)
                device.reset(); // UIKit takes panel size at boot only
            root.freeFormSize = null;
        }
    }
    Keys.onEscapePressed: {
        if (root.freeForm)
            root.freeFormSize = null;
    }

    // --- Drag and drop (surface 12) ------------------------------------
    //
    // The Mac's refusal rules (DisplayView+DragAndDrop.swift), reimplemented
    // at the drag layer: an IPSW is for the library, not this device — it
    // routes to the library even mid-boot, from outside drags. .ipa/.media
    // install only while the device can queue installs; the drop is refused
    // (no accept, no per-file error sheets) while it can't. Local drags
    // (installed rows dragged out as .ipa) never install back in — the
    // apps list doesn't offer an internal drag yet, so there is nothing to
    // refuse today; the rule is recorded here for when it does.

    // kLTDrop* mirror lt_bridge.h (DEVIATION: lt_drop_classify values).
    readonly property int dropIPSW: 0
    readonly property int dropIPA: 1
    readonly property int dropMedia: 2

    // Emitted after lt_drop_route accepts; the shell dispatches per file.
    signal dropRouted(var urls, string target, double requestId)

    function dropTarget() {
        const id = root.row && root.row["id"] ? root.row["id"] : "";
        return id !== "" ? "display:" + id : "display:";
    }
    function dropKinds(drag) {
        const kinds = [];
        if (!drag.hasUrls)
            return kinds;
        for (let i = 0; i < drag.urls.length; ++i) {
            const c = deviceList.classifyDropUrl(drag.urls[i]);
            if (kinds.indexOf(c) < 0)
                kinds.push(c);
        }
        return kinds;
    }
    function dropAcceptable(drag) {
        const kinds = root.dropKinds(drag);
        if (kinds.length === 0)
            return false;
        // IPSW: to the library, any time.
        if (kinds.indexOf(root.dropIPSW) >= 0)
            return true;
        // .ipa/.media: only while the device can queue installs. Never one
        // error sheet per file: refuse at the drag layer instead.
        if (!apps.canQueueInstall)
            return false;
        return kinds.indexOf(root.dropIPA) >= 0 || kinds.indexOf(root.dropMedia) >= 0;
    }

    DropArea {
        id: screenDrop
        anchors.fill: parent
        onEntered: function (drag) {
            drag.accepted = root.dropAcceptable(drag);
            dropRing.show(drag.accepted);
        }
        onPositionChanged: function (drag) {
            const ok = root.dropAcceptable(drag);
            drag.accepted = ok;
            dropRing.show(ok);
        }
        onExited: dropRing.show(false)
        onDropped: function (drop) {
            dropRing.show(false);
            if (!root.dropAcceptable(drop))
                return;
            const urls = [];
            for (let i = 0; i < drop.urls.length; ++i)
                urls.push(drop.urls[i].toString());
            const id = deviceList.routeDrop(urls, root.dropTarget());
            if (id > 0)
                root.dropRouted(urls, root.dropTarget(), id);
        }
    }

    DropHighlight {
        id: dropRing
        anchors.fill: screen
        anchors.margins: -6
    }
}
