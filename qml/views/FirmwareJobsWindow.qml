// FirmwareJobsWindow.qml — firmware catalog & jobs (surface 9).
//
// The Qt home for UI/CatalogDetailsViewController.swift,
// UI/CatalogFilterButton.swift, and Library/FirmwareJobs+App.swift. Two
// tabs: Jobs (download+prepare with progress/cancel, per-job rows) and
// Catalog (browse entries, filter, details, Download and Prepare).
//
// Jobs: each row carries its own progress (percent + detail + step x/y);
// cancel per job; failed jobs show their error + Dismiss. "Download and
// Prepare…" switches to the Catalog tab. canDownload gates starting a job
// (the preparer tool must be present); the reason shows when it can't.
// Ctrl+F (File ▸ Find…) focuses the catalog search.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 560
    height: 480
    minimumWidth: 420
    minimumHeight: 320
    title: qsTr("Firmware")
    visible: false

    property string catalogFilter: ""

    function focusCatalogSearch() {
        root.show();
        root.raise();
        tabBar.currentIndex = 1;
        catalogSearch.forceActiveFocus();
    }

    function startJob(entryId) {
        jobs.refreshCanDownload();
        if (!jobs.canDownload) {
            jobsNote.text = jobs.downloadBlockReason !== ""
                ? jobs.downloadBlockReason
                : qsTr("Downloads are unavailable.");
            return;
        }
        jobs.start(entryId, JSON.stringify({ jailbreak: false, skipSetup: false, keepStock: true }));
        tabBar.currentIndex = 0;
    }

    header: ToolBar {
        TabBar {
            id: tabBar
            anchors.fill: parent
            TabButton { text: qsTr("Jobs") }
            TabButton { text: qsTr("Catalog") }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        // --- Jobs ---
        ColumnLayout {
            spacing: 0
            ListView {
                id: jobsList
                objectName: "firmwareJobsList"
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: jobs.jobs
                delegate: Rectangle {
                    width: jobsList.width
                    height: 64
                    color: "transparent"
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        anchors.topMargin: 6
                        anchors.bottomMargin: 6
                        spacing: 4
                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: modelData["entryId"] || ""
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            Label {
                                visible: modelData["failed"] !== null && modelData["failed"] !== undefined
                                text: modelData["failed"] || ""
                                color: "red"
                                font.pointSize: 9
                            }
                            Button {
                                text: qsTr("Cancel")
                                visible: modelData["failed"] === null || modelData["failed"] === undefined
                                onClicked: jobs.cancel(modelData["entryId"] || "")
                            }
                            Button {
                                text: qsTr("Dismiss")
                                visible: modelData["failed"] !== null && modelData["failed"] !== undefined
                                onClicked: jobs.dismissFailure(modelData["entryId"] || "")
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            ProgressBar {
                                Layout.fillWidth: true
                                from: 0; to: 1
                                value: {
                                    const p = modelData["preparation"];
                                    const f = p ? p["fraction"] : modelData["progress"];
                                    return (f === null || f === undefined || f < 0) ? 0 : f;
                                }
                                indeterminate: {
                                    const p = modelData["preparation"];
                                    const f = p ? p["fraction"] : modelData["progress"];
                                    return f < 0;
                                }
                            }
                            Label {
                                font.pointSize: 9
                                opacity: 0.7
                                text: {
                                    const p = modelData["preparation"] || {};
                                    const f = p["fraction"];
                                    const pct = (f !== null && f !== undefined && f >= 0)
                                                ? Math.round(f * 100) + "%" : "";
                                    const step = p["steps"] ? qsTr("step %1/%2").arg(p["step"] || 0).arg(p["steps"]) : "";
                                    const detail = p["detail"] || "";
                                    return [pct, step, detail].filter(function (x) { return x !== ""; }).join(" · ");
                                }
                            }
                        }
                    }
                }
                Label {
                    anchors.centerIn: parent
                    visible: jobsList.count === 0
                    text: qsTr("No firmware jobs.")
                    opacity: 0.5
                }
            }
            Label {
                id: jobsNote
                visible: text !== ""
                Layout.fillWidth: true
                Layout.margins: 8
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pointSize: 9
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 8
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Import IPSW…")
                    onClicked: {
                        const paths = jobs.pickIpswFiles();
                        if (paths.length > 0)
                            jobs.importIpsws(paths);
                    }
                }
                Button {
                    text: qsTr("Download and Prepare…")
                    onClicked: tabBar.currentIndex = 1
                }
            }
        }

        // --- Catalog ---
        ColumnLayout {
            spacing: 8
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                Layout.topMargin: 8
                TextField {
                    id: catalogSearch
                    objectName: "catalogSearchField"
                    Layout.fillWidth: true
                    placeholderText: qsTr("Filter")
                    text: root.catalogFilter
                    onTextChanged: {
                        root.catalogFilter = text;
                        catalog.filterText = text;
                        catalog.refresh();
                    }
                }
                // CatalogFilterButton: device-kind compatibility filter.
                ComboBox {
                    id: kindFilter
                    objectName: "catalogKindFilter"
                    model: [qsTr("All Devices"), qsTr("iPhone"), qsTr("iPod"), qsTr("iPad")]
                    onActivated: catalog.refresh()
                }
            }
            ListView {
                id: catalogList
                objectName: "firmwareCatalogList"
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: catalog.entries.filter(function (e) {
                    const k = kindFilter.currentIndex;
                    if (k === 0)
                        return true;
                    const name = (e["marketingName"] || "").toLowerCase();
                    return (k === 1 && name.indexOf("iphone") >= 0)
                        || (k === 2 && name.indexOf("ipod") >= 0)
                        || (k === 3 && name.indexOf("ipad") >= 0);
                })
                delegate: Rectangle {
                    width: catalogList.width
                    height: 52
                    color: "transparent"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label {
                                text: "%1 %2".arg(modelData["marketingName"] || "").arg(modelData["version"] || "")
                                font.bold: true
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: [modelData["build"] || "",
                                       modelData["status"] || ""].filter(function (x) { return x !== ""; }).join(" · ")
                                opacity: 0.6
                                font.pointSize: 9
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }
                        Button {
                            text: qsTr("Details…")
                            onClicked: detailsDialog.showEntry(modelData["id"])
                        }
                        Button {
                            objectName: "catalogDownloadButton"
                            text: qsTr("Download and Prepare")
                            enabled: modelData["status"] !== "comingSoon" && jobs.canDownload
                            onClicked: root.startJob(modelData["id"])
                        }
                    }
                }
            }
        }
    }

    CatalogDetailsDialog { id: detailsDialog }

    Component.onCompleted: {
        jobs.refresh();
        jobs.refreshCanDownload();
        catalog.refresh();
    }
}
