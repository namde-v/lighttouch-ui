// DisplayHarness.qml — minimal host for DisplayItem render tests.
import QtQuick
import LightTouch 1.0

Item {
    width: 320
    height: 480
    DisplayItem {
        anchors.fill: parent
        display: display
    }
}
