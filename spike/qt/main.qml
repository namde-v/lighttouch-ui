import QtQuick
import QtQuick.Window

// Minimal scene: a 320x240 window with a 1:1 Image fed by the
// "spikeframes" QQuickImageProvider, which calls Swift's
// lt_spike_next_frame for each frame index.
Window {
    id: win
    width: 320
    height: 240
    visible: true
    color: "black"
    property int frameIndex: 0

    Image {
        x: 0
        y: 0
        width: 320
        height: 240
        cache: false
        asynchronous: false
        source: "image://spikeframes/" + win.frameIndex
    }
}
