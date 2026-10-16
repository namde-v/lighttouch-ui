// FilesBrowser.qml — the device files window (surface 6).
//
// Qt counterpart of UI/DeviceFilesViewController.swift: the device's media
// folder (a jailbroken device's whole filesystem), in its own window.
// Miller columns (NSBrowser) map to a breadcrumb + list; the behaviors are
// the Mac's: files drag in (onto the list) and out, several at once,
// serially; Space previews (download to temp + open in the default app —
// Quick Look has no Qt equivalent, honest flag); Ctrl+Backspace = Delete…
// with confirmation (bare Delete does nothing); the context menu makes,
// renames and deletes; transfer progress with cancel in the status banner;
// "Show Hidden Files" view toggle; Refresh.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../dialogs" // ConfirmDialog

ApplicationWindow {
    id: root
    width: 640
    height: 480
    minimumWidth: 420
    minimumHeight: 300
    title: qsTr("%1 Files").arg(deviceName)

    property string deviceUuid: ""
    property string deviceName: ""
    property string currentPath: ""
    property bool showHidden: false
    property var selectedPaths: []

    // Breadcrumb segments of currentPath.
    property var crumbs: currentPath === "" ? [] : currentPath.split("/")

    function navigate(path) {
        root.currentPath = path;
        root.selectedPaths = [];
        files.list(path);
    }
    function enter(entry) {
        if (entry["isDirectory"] === true)
            navigate(entry["path"]);
    }
    function parentPath() {
        const i = root.currentPath.lastIndexOf("/");
        return i < 0 ? "" : root.currentPath.slice(0, i);
    }
    function visibleEntries() {
        if (!root.showHidden)
            return files.entries.filter(function (e) {
                const n = e["name"] || "";
                return !n.startsWith(".");
            });
        return files.entries;
    }
    function targetDirectory() {
        // New files go into the selected folder, else the current one.
        for (let i = 0; i < root.selectedPaths.length; ++i) {
            const e = entryByPath(root.selectedPaths[i]);
            if (e && e["isDirectory"] === true)
                return e["path"];
        }
        return root.currentPath;
    }
    function entryByPath(path) {
        for (let i = 0; i < files.entries.length; ++i)
            if (files.entries[i]["path"] === path)
                return files.entries[i];
        return null;
    }

    function upload() {
        const local = files.pickUploadFiles();
        if (local.length === 0)
            return;
        const into = root.targetDirectory();
        for (let i = 0; i < local.length; ++i) {
            const name = local[i].split("/").pop();
            files.transfer(JSON.stringify({
                op: "upload", src: local[i],
                dst: into === "" ? name : into + "/" + name
            }));
        }
    }
    function download() {
        const paths = root.selectedPaths.filter(function (p) {
            const e = root.entryByPath(p);
            return e && e["isDirectory"] !== true;
        });
        if (paths.length === 0)
            return;
        if (paths.length === 1) {
            const e = root.entryByPath(paths[0]);
            const dest = files.pickDownloadFile(e["name"] || "file");
            if (dest !== "")
                files.transfer(JSON.stringify({ op: "download", src: e["path"], dst: dest }));
        } else {
            const dir = files.pickDownloadDir();
            if (dir === "")
                return;
            for (let i = 0; i < paths.length; ++i) {
                const e = root.entryByPath(paths[i]);
                files.transfer(JSON.stringify({
                    op: "download", src: e["path"],
                    dst: dir + "/" + (e["name"] || "file")
                }));
            }
        }
    }
    // Space: Quick Look's replacement — download to temp, open in the
    // default app (xdg-open / OS default). A built-in preview pane is the
    // alternative the coverage notes; this is the honest minimum.
    function preview() {
        const paths = root.selectedPaths.filter(function (p) {
            const e = root.entryByPath(p);
            return e && e["isDirectory"] !== true;
        });
        if (paths.length === 0)
            return;
        const e = root.entryByPath(paths[0]);
        const tmp = "/tmp/lighttouch-preview-" + (e["name"] || "file");
        files.transfer(JSON.stringify({ op: "download", src: e["path"], dst: tmp }));
        // The real core's FilesDone carries the finished transfer; the mock
        // completes synchronously, so open right away (best effort).
        Qt.openUrlExternally("file://" + tmp);
    }

    function askDelete() {
        const paths = root.selectedPaths;
        if (paths.length === 0 || files.hasTransfer)
            return;
        const n = paths.length;
        deleteDialog.ask(
            n === 1 ? qsTr("Delete \"%1\"?").arg(paths[0].split("/").pop())
                    : qsTr("Delete %n items?", "", n),
            qsTr("This can't be undone."),
            qsTr("Delete"),
            function () {
                for (let i = 0; i < paths.length; ++i)
                    files.transfer(JSON.stringify({ op: "delete", src: paths[i], dst: "" }));
            });
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            Button {
                text: qsTr("Up")
                enabled: root.currentPath !== ""
                onClicked: root.navigate(root.parentPath())
            }
            Label {
                Layout.fillWidth: true
                elide: Text.ElideMiddle
                text: root.crumbs.length === 0 ? qsTr("Media")
                     : qsTr("Media / %1").arg(root.crumbs.join(" / "))
                font.bold: true
            }
            Button { text: qsTr("Upload…"); enabled: !files.hasTransfer; onClicked: root.upload() }
            Button {
                text: qsTr("Save to Mac…")
                enabled: !files.hasTransfer && root.selectedPaths.length > 0
                onClicked: root.download()
            }
            Button {
                text: qsTr("Refresh")
                enabled: !files.hasTransfer
                onClicked: files.list(root.currentPath)
            }
        }
    }

    // Breadcrumb bar.
    Row {
        id: crumbBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        spacing: 4
        Button {
            text: qsTr("Media")
            flat: true
            onClicked: root.navigate("")
        }
        Repeater {
            model: root.crumbs
            delegate: Row {
                spacing: 4
                Label { text: "/"; opacity: 0.5; anchors.verticalCenter: parent.verticalCenter }
                Button {
                    text: modelData
                    flat: true
                    onClicked: root.navigate(root.crumbs.slice(0, index + 1).join("/"))
                }
            }
        }
    }

    ListView {
        id: entryList
        objectName: "filesEntryList"
        anchors.top: crumbBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        anchors.margins: 8
        clip: true
        model: root.visibleEntries()
        delegate: Rectangle {
            width: entryList.width
            height: 32
            color: root.selectedPaths.indexOf(modelData["path"]) >= 0 ? palette.highlight : "transparent"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 8
                Label {
                    text: modelData["isDirectory"] === true ? "📁" : "📄"
                    font.pointSize: 12
                }
                Label {
                    text: modelData["name"] || ""
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    color: root.selectedPaths.indexOf(modelData["path"]) >= 0
                           ? palette.highlightedText : palette.text
                }
                Label {
                    visible: modelData["isDirectory"] !== true
                    text: Qt.locale().formattedDataSize(modelData["size"] || 0)
                    opacity: 0.6
                    font.pointSize: 9
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: function (mouse) {
                    const p = modelData["path"];
                    if (mouse.button === Qt.RightButton) {
                        if (root.selectedPaths.indexOf(p) < 0)
                            root.selectedPaths = [p];
                        filesMenu.popup();
                    } else if (mouse.modifiers & Qt.ControlModifier) {
                        const s = root.selectedPaths.slice();
                        const i = s.indexOf(p);
                        if (i >= 0) s.splice(i, 1); else s.push(p);
                        root.selectedPaths = s;
                    } else {
                        root.selectedPaths = [p];
                    }
                }
                onDoubleClicked: root.enter(modelData)
            }
        }
        Keys.onPressed: function (event) {
            // Space = preview; Ctrl+Backspace = Delete…; bare Delete = nothing.
            if (event.key === Qt.Key_Space && event.modifiers === Qt.NoModifier) {
                root.preview();
                event.accepted = true;
            } else if (event.key === Qt.Key_Backspace && event.modifiers === Qt.ControlModifier) {
                root.askDelete();
                event.accepted = true;
            }
        }
        focus: true
    }

    // Drop files in (onto the list → the target directory).
    DropArea {
        anchors.fill: entryList
        onDropped: function (drop) {
            if (!drop.hasUrls || files.hasTransfer)
                return;
            const into = root.targetDirectory();
            for (let i = 0; i < drop.urls.length; ++i) {
                const local = drop.urls[i].toString().replace(/^file:\/\//, "");
                const name = local.split("/").pop();
                files.transfer(JSON.stringify({
                    op: "upload", src: local,
                    dst: into === "" ? name : into + "/" + name
                }));
            }
        }
    }

    Menu {
        id: filesMenu
        MenuItem {
            text: qsTr("New Folder…")
            onTriggered: nameDialog.askName(qsTr("New Folder"), "", function (name) {
                const into = root.targetDirectory();
                files.transfer(JSON.stringify({
                    op: "mkdir", src: "", dst: into === "" ? name : into + "/" + name
                }));
            })
        }
        MenuItem {
            text: qsTr("Rename…")
            enabled: root.selectedPaths.length === 1
            onTriggered: {
                const p = root.selectedPaths[0];
                nameDialog.askName(qsTr("Rename"), p.split("/").pop(), function (name) {
                    const dir = p.slice(0, p.lastIndexOf("/"));
                    files.transfer(JSON.stringify({
                        op: "move", src: p,
                        dst: dir === "" ? name : dir + "/" + name
                    }));
                });
            }
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Delete…")
            enabled: root.selectedPaths.length > 0
            onTriggered: root.askDelete()
        }
        MenuSeparator {}
        MenuItem {
            text: root.showHidden ? qsTr("Hide Hidden Files") : qsTr("Show Hidden Files")
            checkable: true
            checked: root.showHidden
            onTriggered: root.showHidden = !root.showHidden
        }
    }

    // Name prompt (New Folder / Rename).
    Dialog {
        id: nameDialog
        modal: true
        anchors.centerIn: parent
        title: qsTr("Name")
        property var onAccept: null
        function askName(title, initial, fn) {
            nameDialog.title = title;
            nameField.text = initial;
            nameDialog.onAccept = fn;
            nameDialog.open();
        }
        ColumnLayout {
            TextField { id: nameField; Layout.preferredWidth: 260 }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button { text: qsTr("Cancel"); onClicked: nameDialog.close() }
                Button {
                    text: qsTr("OK")
                    onClicked: {
                        nameDialog.close();
                        if (nameDialog.onAccept)
                            nameDialog.onAccept(nameField.text);
                    }
                }
            }
        }
    }

    ConfirmDialog { id: deleteDialog }

    // Status banner: transfer progress with cancel; idle shows the device
    // state ("The <device> is disconnected." when not connected).
    footer: ToolBar {
        id: statusBar
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            Label {
                id: statusLabel
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pointSize: 9
                opacity: 0.75
                text: files.hasTransfer
                      ? qsTr("Copying %1… %2%").arg(files.transferFile).arg(Math.round(files.transferFraction * 100))
                      : files.connectionState === 1
                        ? qsTr("Connected")
                        : qsTr("The %1 is disconnected.").arg(root.deviceName)
            }
            ProgressBar {
                visible: files.hasTransfer
                from: 0; to: 1
                value: files.transferFraction
                Layout.preferredWidth: 120
            }
            Button {
                visible: files.hasTransfer
                text: qsTr("Cancel")
                onClicked: files.cancelTransfer(files.transferId)
            }
        }
    }

    Component.onCompleted: {
        files.deviceUuid = root.deviceUuid;
        files.follow(true);
        root.navigate("");
    }
    onClosing: files.follow(false)
}
