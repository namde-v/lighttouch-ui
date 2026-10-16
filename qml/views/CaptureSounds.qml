// CaptureSounds.qml — capture sound effects (surface 8).
//
// Qt counterpart of Features/CaptureSound.swift. The Mac plays iOS system
// sounds (AudioServicesPlaySystemSound IDs); those IDs have no Qt
// equivalent, so this plays short bundled WAVs (qml/sounds/) through
// QSoundEffect. Honors the Capture preferences' "Play sound effects".

import QtQuick
import QtMultimedia

Item {
    id: root
    visible: false

    function enabled() {
        return !!(capture.prefs && capture.prefs["soundEffectsEnabled"]);
    }

    function playScreenshot() {
        if (root.enabled())
            shutter.play();
    }
    function playRecordingStarted() {
        if (root.enabled())
            recStart.play();
    }
    function playRecordingStopped() {
        if (root.enabled())
            recStop.play();
    }

    SoundEffect {
        id: shutter
        source: Qt.resolvedUrl("../sounds/shutter.wav")
    }
    SoundEffect {
        id: recStart
        source: Qt.resolvedUrl("../sounds/record-start.wav")
    }
    SoundEffect {
        id: recStop
        source: Qt.resolvedUrl("../sounds/record-stop.wav")
    }
}
