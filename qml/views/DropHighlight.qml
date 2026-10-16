// DropHighlight.qml — the drop-target cue (surface 12).
//
// Qt counterpart of UI/DropHighlight.swift (HIG p.294): an accent ring just
// inside the view while a drag it will accept is over it. It never takes
// clicks or drags itself.

import QtQuick

Rectangle {
    id: root
    color: "transparent"
    border.color: palette.highlight
    border.width: 3
    radius: 8
    visible: false

    // Shows the ring for an accepted operation (call from the DropArea's
    // onEntered/onPositionChanged/onExited).
    function show(accepted) {
        root.visible = accepted;
    }
}
