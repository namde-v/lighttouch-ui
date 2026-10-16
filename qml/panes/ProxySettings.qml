// ProxySettings.qml — Device ▸ Network ▸ Proxy… (surface 4).
//
// Qt counterpart of UI/ProxySettingsView.swift: per-device web proxy routing
// + CA trust, as a sheet with Cancel / OK (the default). Choices are the
// device's: mode off/direct/archive; archive dates are calendar days
// (yyyy-MM-dd, local). "Other…" isn't needed — the save-location concept
// doesn't apply; the mode + date are the whole configuration.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 12

    // Draft state; the adapter's config is only written on OK.
    property string draftMode: "off"
    property string draftDate: "2026-01-01"

    function loadFromAdapter() {
        const c = proxy.config;
        draftMode = c["mode"] || "off";
        draftDate = c["archiveDate"] || "2026-01-01";
        useProxyBox.checked = draftMode !== "off";
        useArchiveBox.checked = draftMode === "archive";
        dateField.text = draftDate;
    }
    function currentJson() {
        const mode = !useProxyBox.checked ? "off"
                   : useArchiveBox.checked ? "archive" : "direct";
        return JSON.stringify({ mode: mode, archiveDate: dateField.text });
    }

    CheckBox {
        id: useProxyBox
        objectName: "proxyEnabledBox"
        text: qsTr("Use HTTP proxy")
        onToggled: root.draftMode = checked ? "direct" : "off"
    }
    ColumnLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 18
        spacing: 8
        enabled: useProxyBox.checked
        CheckBox {
            id: useArchiveBox
            objectName: "proxyArchiveBox"
            text: qsTr("Browse the Internet Archive")
            onToggled: root.draftMode = checked ? "archive" : "direct"
        }
        RowLayout {
            enabled: useArchiveBox.checked
            Label { text: qsTr("Date:") }
            TextField {
                id: dateField
                objectName: "proxyDateField"
                Layout.preferredWidth: 120
                placeholderText: qsTr("yyyy-MM-dd")
                inputMask: "9999-99-99"
            }
        }
        Label {
            visible: useArchiveBox.checked
            text: qsTr("Uses the closest available capture.")
            opacity: 0.6
            font.pointSize: 9
        }
    }
    Label {
        text: qsTr("Choices are the device's; they apply at the next boot.")
        opacity: 0.6
        font.pointSize: 9
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

    Component.onCompleted: {
        proxy.refresh();
        root.loadFromAdapter();
    }
}
