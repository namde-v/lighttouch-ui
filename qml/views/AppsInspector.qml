// AppsInspector.qml — the right-hand apps inspector (surface 5).
//
// Qt counterpart of UI/AppsInspectorViewController.swift (+Loading, +Store,
// +Table, +Actions, +ContextMenu): installed apps in home-screen order with
// an Installed/Store segmented switch, add/remove controls, and the Legacy
// Store browser.
//
// Loading contract (the Mac's hardest 1:1): polls for the life of the view;
// a failed read keeps the old list + the stale banner ("kept, no longer
// current") — never blanks; rows rebuild only where changed (the ListView
// keeps selection by identity). The default mode is Store (the suggested
// list); typing in the search flips to Store. Pending install rows sit above
// installed apps with the job's own progress, cancellable from the context
// menu; a finished job renders as an ordinary app row.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../dialogs" // ConfirmDialog

Pane {
    id: root
    padding: 0

    // 0 = Installed, 1 = Store. The Mac defaults to Store.
    property int mode: 1
    property string searchText: ""
    property string selectedIdentity: ""

    // Store results: the suggested list when the search is empty.
    property var storeResults: []

    function refreshStore() {
        // lt_apps_store_search is the portable core of the Store browser;
        // the mock returns the suggested list for an empty query.
        root.storeResults = apps.storeSearch(root.searchText);
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                TabBar {
                    id: modeBar
                    currentIndex: root.mode
                    onCurrentIndexChanged: root.mode = currentIndex
                    TabButton { text: qsTr("Installed"); width: implicitWidth }
                    TabButton { text: qsTr("Store"); width: implicitWidth }
                }
                Item { Layout.fillWidth: true }
                TextField {
                    id: searchField
                    objectName: "appsSearchField"
                    Layout.preferredWidth: 140
                    placeholderText: qsTr("Search")
                    text: root.searchText
                    onTextChanged: {
                        root.searchText = text;
                        if (text !== "" && root.mode !== 1)
                            root.mode = 1;  // typing flips to Store
                        root.refreshStore();
                    }
                }
            }
        }

        // Stale banner: the list is kept but no longer current.
        Label {
            visible: apps.bannerText !== ""
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 6
            text: apps.bannerText
            opacity: 0.65
            font.pointSize: 9
            wrapMode: Text.WordWrap
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.mode

            // --- Installed ---
            ListView {
                id: installedList
                objectName: "installedAppsList"
                clip: true
                model: apps.rows
                spacing: 0
                delegate: AppRow {
                    width: installedList.width
                    row: modelData
                    selected: root.selectedIdentity === modelData["identity"]
                    onClicked: root.selectedIdentity = modelData["identity"]
                    onContextMenu: function (x, y) {
                        root.selectedIdentity = modelData["identity"];
                        root.showRowMenu(modelData, x, y);
                    }
                }
                Label {
                    anchors.centerIn: parent
                    visible: installedList.count === 0
                    text: qsTr("No apps installed.\nInstall one with the + button or by dropping an .ipa.")
                    opacity: 0.5
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // --- Store ---
            ListView {
                id: storeList
                objectName: "storeAppsList"
                clip: true
                model: root.storeResults
                delegate: StoreRow {
                    width: storeList.width
                    app: modelData
                    onInstall: apps.install("store:" + (modelData["id"] || ""))
                }
                Label {
                    anchors.centerIn: parent
                    visible: storeList.count === 0
                    text: qsTr("No results.")
                    opacity: 0.5
                }
            }
        }

        // Add / Remove controls beneath the list.
        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                Button {
                    objectName: "addAppButton"
                    text: "+"
                    ToolTip.text: qsTr("Install App…")
                    ToolTip.visible: hovered
                    enabled: apps.canQueueInstall
                    onClicked: apps.pickAndInstallApp()
                }
                Button {
                    objectName: "removeAppButton"
                    text: "−"
                    ToolTip.text: qsTr("Uninstall")
                    ToolTip.visible: hovered
                    enabled: root.selectedIdentity !== "" && root.mode === 0
                    onClicked: root.askUninstall([root.selectedIdentity])
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Refresh")
                    onClicked: { apps.refresh(); root.refreshStore(); }
                }
            }
        }
    }

    // Row context menu: Install on <other running devices>, cancel/dismiss
    // per pending job, View on Legacy Store. (The AppsMenu model lives in
    // the core; this is the Qt menu over the adapter's rows.)
    Menu {
        id: rowMenu
        property var row: null
        MenuItem {
            text: qsTr("View on Legacy Store")
            visible: rowMenu.row !== null && rowMenu.row["kind"] === "app"
            onTriggered: {
                root.mode = 1;
                root.searchText = rowMenu.row["title"] || "";
                root.refreshStore();
            }
        }
        MenuItem {
            text: qsTr("Cancel Install")
            visible: rowMenu.row !== null && rowMenu.row["kind"] === "install"
            onTriggered: apps.cancelInstall(rowMenu.row["identity"])
        }
        MenuItem {
            text: qsTr("Uninstall")
            visible: rowMenu.row !== null && rowMenu.row["kind"] === "app"
            onTriggered: root.askUninstall([rowMenu.row["identity"]])
        }
    }

    function showRowMenu(row, x, y) {
        rowMenu.row = row;
        rowMenu.popup(x, y);
    }

    // Uninstall asks once per selection (destructive-styled); install
    // accepted while confirming still queues (the adapter queues anyway).
    function askUninstall(identities) {
        if (identities.length === 0)
            return;
        const n = identities.length;
        const title = n === 1 ? qsTr("Uninstall this app?") : qsTr("Uninstall %n apps?", "", n);
        uninstallDialog.ask(
            title,
            qsTr("This removes the app and its data. This cannot be undone."),
            qsTr("Uninstall"),
            function () {
                for (let i = 0; i < identities.length; ++i)
                    apps.remove(identities[i]);
            });
    }

    ConfirmDialog {
        id: uninstallDialog
    }

    Component.onCompleted: {
        apps.refresh();
        root.refreshStore();
    }
}
