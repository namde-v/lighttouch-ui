// RecordingButton.qml — the capture toolbar button (surface 8).
//
// Qt counterpart of UI/RecordingToolbarButton.swift: idle shows
// "Start Recording (⌘R)"; while recording the label is the live elapsed
// mm:ss with a stop symbol; while saving a spinner + "Saving Recording…".
// No-op updates touch nothing (the button only re-renders on the adapter's
// recordingChanged/availabilityChanged signals).

import QtQuick
import QtQuick.Controls

ToolButton {
    id: root
    objectName: "recordingButton"

    // kLTRecordingPhase… mirror lt_bridge.h §4.
    readonly property int phaseIdle: 0
    readonly property int phaseStarting: 1
    readonly property int phaseRecording: 2
    readonly property int phaseSaving: 3

    property bool active: capture.recordingPhase === phaseRecording
                          || capture.recordingPhase === phaseStarting
    property bool saving: capture.recordingPhase === phaseSaving

    function mmss(totalSeconds) {
        const m = Math.floor(totalSeconds / 60);
        const s = totalSeconds % 60;
        return (m < 10 ? "0" + m : m) + ":" + (s < 10 ? "0" + s : s);
    }

    text: {
        if (root.saving)
            return qsTr("Saving Recording…");
        if (root.active)
            return "⏹ " + root.mmss(capture.elapsedSeconds);
        return qsTr("Start Recording (Ctrl+R)");
    }
    enabled: capture.canStartRecording || capture.recordingCanStop
    ToolTip.text: root.active ? qsTr("Stop Recording") : qsTr("Start Recording")
    ToolTip.visible: hovered

    BusyIndicator {
        anchors.centerIn: parent
        visible: root.saving
        running: root.saving
        implicitWidth: 16
        implicitHeight: 16
    }

    onClicked: {
        if (capture.recordingCanStop)
            capture.stopRecording(false);
        else if (capture.canStartRecording)
            capture.startRecording();
    }
}
