/// MockCore.swift — hand-written interesting exports for the LTMockCore stand-in.
///
/// Boundary discipline mirrors the spike (ui/spike/swift): only C types cross,
/// exports are top-level (hence nonisolated), the stored callback global uses
/// nonisolated(unsafe).
///
/// NOTE on lt_free_event_strings: this stand-in never fires events, so it never
/// allocates event strings; the implementation is a documented no-op. The C++
/// mock mode (LTM_BRIDGE_MOCK=1) does not use this library at all.

import Foundation
#if canImport(Darwin)
import Darwin
#elseif canImport(Glibc)
import Glibc
#endif

public typealias LTMockEventCallback =
    @convention(c) (UnsafeRawPointer?, UnsafeMutableRawPointer?) -> Void

// nonisolated(unsafe): single-writer at startup, same as the spike.
nonisolated(unsafe) private var storedCallback: LTMockEventCallback? = nil

@_cdecl("lt_set_event_callback")
public func lt_set_event_callback(_ fn: LTMockEventCallback?,
                                  _ ctx: UnsafeMutableRawPointer?) -> Int32 {
    storedCallback = fn
    return 0  // kLTOk
}

@_cdecl("lt_free_string")
public func lt_free_string(_ s: UnsafePointer<CChar>?) {
    if let s {
        free(UnsafeMutableRawPointer(mutating: s))
    }
}

@_cdecl("lt_free_event_strings")
public func lt_free_event_strings(_ e: UnsafeRawPointer?) {
    // Documented no-op: this stand-in never allocates event strings.
}

@_cdecl("lt_last_error")
public func lt_last_error() -> UnsafePointer<CChar>? {
    UnsafePointer<CChar>(strdup("mock core: no failure recorded on this thread")!)
}

@_cdecl("lt_init")
public func lt_init(_ configJson: UnsafePointer<CChar>?) -> Int32 {
    mockSettingsStore.removeAll()
    return 0  // kLTOk
}

@_cdecl("lt_shutdown")
public func lt_shutdown() -> Int32 {
    0  // kLTOk
}

@_cdecl("lt_abi_version")
public func lt_abi_version() -> Int32 {
    1  // kLTAbiVersionCurrent
}

@_cdecl("lt_version")
public func lt_version(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    out?.pointee = UnsafePointer<CChar>(strdup("0.0.0-mock")!)
    return 0
}

// MARK: - Mock device inventory (handles 1...N; must match Bridge.cpp's)

private struct MockDevice {
    let uuid: String
    let name: String
    let board: String
    let firmware: String
    let status: Int32  // kLTDeviceRowState…
    let statusText: String
    let firmwareNote: String?
}

private var mockDevices: [MockDevice] = [
    MockDevice(uuid: "3f2a1b4c-5d6e-4789-a0b1-c2d3e4f50617",
               name: "iPhone 3GS", board: "n88ap", firmware: "iPhone OS 3.1.3",
               status: 1, statusText: "Off", firmwareNote: nil),
    MockDevice(uuid: "7e8f9a0b-1c2d-4e5f-8091-a2b3c4d5e6f7",
               name: "iPod touch (2nd gen)", board: "n72ap", firmware: "iOS 4.2.1",
               status: 2, statusText: "Running", firmwareNote: "Jailbroken"),
    MockDevice(uuid: "a1b2c3d4-e5f6-4789-abcd-ef0123456789",
               name: "iPad", board: "k48ap", firmware: "iOS 5.1.1",
               status: 4, statusText: "Booting…", firmwareNote: nil),
]

private func mockDevice(handle: UInt64) -> MockDevice? {
    guard handle >= 1, handle <= UInt64(mockDevices.count) else { return nil }
    return mockDevices[Int(handle) - 1]
}

private func rowJson(_ d: MockDevice) -> String {
    let note = d.firmwareNote.map { "\"\($0)\"" } ?? "null"
    return """
        {"id":"\(d.uuid)","name":"\(d.name)","board":"\(d.board)",\
        "firmware":"\(d.firmware)","status":\(d.status),\
        "statusText":"\(d.statusText)","busy":null,"firmwareNote":\(note),\
        "prepared":true}
        """
}

@_cdecl("lt_device_list")
public func lt_device_list(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let json = "[" + mockDevices.map(rowJson).joined(separator: ",") + "]"
    out?.pointee = UnsafePointer<CChar>(strdup(json)!)
    return 0
}

@_cdecl("lt_device_row")
public func lt_device_row(_ device: UInt64,
                          _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard let d = mockDevice(handle: device) else { return -2 }  // kLTNoSuchDevice
    out?.pointee = UnsafePointer<CChar>(strdup(rowJson(d))!)
    return 0
}

@_cdecl("lt_device_find")
public func lt_device_find(_ uuid: UnsafePointer<CChar>?,
                           _ out: UnsafeMutablePointer<UInt64>?) -> Int32 {
    guard let uuid,
          let s = String(validatingUTF8: uuid),
          let idx = mockDevices.firstIndex(where: { $0.uuid == s })
    else { return -2 }  // kLTNoSuchDevice
    out?.pointee = UInt64(idx + 1)
    return 0
}

@_cdecl("lt_device_uuid")
public func lt_device_uuid(_ device: UInt64,
                           _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard let d = mockDevice(handle: device) else { return -2 }  // kLTNoSuchDevice
    out?.pointee = UnsafePointer<CChar>(strdup(d.uuid)!)
    return 0
}

@_cdecl("lt_device_status")
public func lt_device_status(_ device: UInt64,
                             _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard let d = mockDevice(handle: device) else { return -2 }  // kLTNoSuchDevice
    let running = d.status == 2 ? 1 : 0
    let machine = d.status == 2 ? 2 : (d.status == 4 ? 5 : 1)  // Running/Starting/Off
    let json = """
        {"machine":\(machine),"running":\(running),"sleeping":0,"paused":0,\
        "shuttingDown":0,"storageFailed":0,"installing":0,\
        "acceptsInput":\(running),"hasCompass":\(running),\
        "hasCellular":\(running),"compassHeading":0,"editingText":0}
        """
    out?.pointee = UnsafePointer<CChar>(strdup(json)!)
    return 0
}

// MARK: - Library mutations (canned stand-in: succeed without mutation)

@_cdecl("lt_device_create")
public func lt_device_create(_ entryId: UnsafePointer<CChar>?,
                             _ name: UnsafePointer<CChar>?,
                             _ out: UnsafeMutablePointer<UInt64>?) -> Int32 {
    guard name != nil else { return -1 }  // kLTBadArgument
    out?.pointee = 99
    return 0
}

@_cdecl("lt_device_delete")
public func lt_device_delete(_ device: UInt64) -> Int32 {
    guard mockDevice(handle: device) != nil else { return -2 }  // kLTNoSuchDevice
    return 0
}

@_cdecl("lt_device_rename")
public func lt_device_rename(_ device: UInt64, _ name: UnsafePointer<CChar>?) -> Int32 {
    guard mockDevice(handle: device) != nil, name != nil else { return -2 }
    return 0
}

@_cdecl("lt_jobs_list")
public func lt_jobs_list(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let json = """
        [{"entryId":"ipsw-k48ap-5.1.1",\
        "deviceUuid":"a1b2c3d4-e5f6-4789-abcd-ef0123456789",\
        "kind":"prepare","progress":0.42,\
        "preparation":{"step":2,"steps":5,"name":"Preparing",\
        "fraction":0.42,"detail":"Decrypting root filesystem",\
        "remaining":"12:34"},"failed":null}]
        """
    out?.pointee = UnsafePointer<CChar>(strdup(json)!)
    return 0
}

@_cdecl("lt_drop_classify")
public func lt_drop_classify(_ path: UnsafePointer<CChar>?,
                             _ out: UnsafeMutablePointer<Int32>?) -> Int32 {
    guard let path, let p = String(validatingUTF8: path) else { return -1 }
    let ext = (p as NSString).pathExtension.lowercased()
    let cls: Int32
    switch ext {
    case "ipsw": cls = 0
    case "ipa": cls = 1
    case "mp3", "m4a", "aac", "wav", "mp4", "m4v", "mov",
         "jpg", "jpeg", "png", "gif", "bmp": cls = 2
    default: cls = 3
    }
    out?.pointee = cls
    return 0
}

@_cdecl("lt_menu_validate")
public func lt_menu_validate(_ device: UInt64, _ item: Int32, _ arg: Int32,
                             _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard let d = mockDevice(handle: device) else { return -2 }  // kLTNoSuchDevice
    let running = d.status == 2
    var title: String? = nil
    var checked: String? = nil
    var enabled = false
    switch item {
    case 1: enabled = running; title = "Lock"
    case 2: enabled = running
    case 3: enabled = running; checked = (arg == 0) ? "true" : "false"
    case 4: enabled = running
    case 5: enabled = running
    case 6: enabled = running; title = "Pause"; checked = "false"
    case 7: enabled = running
    default: break
    }
    let t = title.map { "\"\($0)\"" } ?? "null"
    let c = checked ?? "null"
    let json = "{\"enabled\":\(enabled ? "true" : "false"),\"title\":\(t),\"checked\":\(c)}"
    out?.pointee = UnsafePointer<CChar>(strdup(json)!)
    return 0
}

private var mockSettingsStore: [String: String] = [:]

@_cdecl("lt_settings_get_string")
public func lt_settings_get_string(_ key: UnsafePointer<CChar>?,
                                    _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let v = mockSettingsStore[String(cString: key!)] ?? ""
    out?.pointee = UnsafePointer<CChar>(strdup(v)!)
    return 0
}

@_cdecl("lt_settings_set_string")
public func lt_settings_set_string(_ key: UnsafePointer<CChar>?,
                                    _ value: UnsafePointer<CChar>?) -> Int32 {
    mockSettingsStore[String(cString: key!)] = String(cString: value!)
    return 0
}

@_cdecl("lt_catalog_list")
public func lt_catalog_list(_ filterJson: UnsafePointer<CChar>?,
                             _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let entries: [[String: Any]] = [
        ["id":"n88ap-3.1.3","board":"n88ap","productType":"iPhone2,1","marketingName":"iPhone 3GS",
         "version":"3.1.3","build":"7E18","status":"available","statusNote":NSNull(),
         "prereleaseBadge":NSNull(),"media":[] as [String],"released":"2010-02-02",
         "managesApps":true,"estimates":["preparedBytes":2147483648]],
        ["id":"n72ap-4.2.1","board":"n72ap","productType":"iPod2,1","marketingName":"iPod touch (2nd gen)",
         "version":"4.2.1","build":"8C148","status":"available","statusNote":NSNull(),
         "prereleaseBadge":NSNull(),"media":[] as [String],"released":"2010-11-22",
         "managesApps":true,"estimates":["preparedBytes":1073741824]],
        ["id":"k48ap-5.1.1","board":"k48ap","productType":"iPad1,1","marketingName":"iPad",
         "version":"5.1.1","build":"9B206","status":"available","statusNote":NSNull(),
         "prereleaseBadge":NSNull(),"media":[] as [String],"released":"2012-05-07",
         "managesApps":true,"estimates":["preparedBytes":3221225472]],
        ["id":"k48ap-6.0","board":"k48ap","productType":"iPad1,1","marketingName":"iPad",
         "version":"6.0","build":"10A403","status":"comingSoon","statusNote":"Not yet prepared",
         "prereleaseBadge":NSNull(),"media":[] as [String],"released":NSNull(),
         "managesApps":true,"estimates":["preparedBytes":3221225472]],
        ["id":"k48ap-5.0","board":"k48ap","productType":"iPad1,1","marketingName":"iPad",
         "version":"5.0","build":"9A334","status":"userIPSW","statusNote":NSNull(),
         "prereleaseBadge":NSNull(),"media":[] as [String],"released":"2011-10-12",
         "managesApps":true,"estimates":["preparedBytes":3221225472]],
    ]
    let data = try! JSONSerialization.data(withJSONObject: entries)
    out?.pointee = UnsafePointer<CChar>(strdup(String(data: data, encoding: .utf8)!)!)
    return 0
}

@_cdecl("lt_about_credits")
public func lt_about_credits(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let json = "{\"credits\":[{\"text\":\"LightTouch\",\"isHeading\":true,\"isDetail\":false}," +
        "{\"text\":\"Classic iPhone, iPod and iPad emulation.\",\"isHeading\":false,\"isDetail\":true}," +
        "{\"text\":\"Show Licenses\",\"isHeading\":false,\"isDetail\":false,\"link\":\"lighttouch:licenses\"}]," +
        "\"licenses\":[{\"name\":\"MockCore\",\"text\":\"Mock license text.\"}]}"
    out?.pointee = UnsafePointer<CChar>(strdup(json)!)
    return 0
}

@_cdecl("lt_help_topics")
public func lt_help_topics(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    let topics: [[String: String]] = [
        ["title":"Getting Started","body":"Add a device from the firmware catalog, then start it. Your [Device] appears in the sidebar."],
        ["title":"Installing Apps","body":"Drag a decrypted .ipa onto your [Device], or use Apps > Install App."],
    ]
    let data = try! JSONSerialization.data(withJSONObject: topics)
    out?.pointee = UnsafePointer<CChar>(strdup(String(data: data, encoding: .utf8)!)!)
    return 0
}

// Hand-written (in LT_MOCK_EXCLUDE_CSV): the C++ mock drives the synthetic
// frame ring; the Swift mock just flips the mock device state.
@_cdecl("lt_device_boot")
public func lt_device_boot(_ device: UInt64, _ optionsJson: UnsafePointer<CChar>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    mockDevices[Int(device) - 1] = MockDevice(
        uuid: mockDevices[Int(device) - 1].uuid,
        name: mockDevices[Int(device) - 1].name,
        board: mockDevices[Int(device) - 1].board,
        firmware: mockDevices[Int(device) - 1].firmware,
        status: 2, statusText: "Running", firmwareNote: nil)
    return 1  // kLTAccepted
}

@_cdecl("lt_device_halt")
public func lt_device_halt(_ device: UInt64) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    let d = mockDevices[Int(device) - 1]
    mockDevices[Int(device) - 1] = MockDevice(
        uuid: d.uuid, name: d.name, board: d.board, firmware: d.firmware,
        status: 1, statusText: "Off", firmwareNote: nil)
    return 1
}

// ------------------------------------------------------------------
// Surfaces 4–9, 12: hand-written Swift stand-ins (LT_MOCK_EXCLUDE_CSV).
// Simple canned snapshots; the C++ mock carries the scripted event
// behavior used by the UI tests.
// ------------------------------------------------------------------

private func swiftMockString(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?, _ s: String) {
    out?.pointee = UnsafePointer<CChar>(strdup(s)!)
}

@_cdecl("lt_tweaks_get")
public func lt_tweaks_get(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, "{\"running\":true,\"tweaks\":[{\"id\":\"fingerDots\",\"title\":\"Show Touches\",\"caption\":\"View Show Finger Dots\",\"on\":false,\"available\":true}],\"color\":2,\"clock\":null,\"status\":null,\"message\":null,\"hiddenApps\":null}")
    return 0
}

@_cdecl("lt_carrier_get")
public func lt_carrier_get(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, "{\"carrierName\":\"Light Touch\",\"mcc\":\"001\",\"mnc\":\"01\",\"settings\":{\"registered\":true,\"simPresent\":true,\"bars\":4},\"status\":null,\"sent\":[],\"message\":null,\"networkValid\":true,\"networkEdited\":false,\"applyingNetwork\":false,\"callNumber\":\"\",\"smsNumber\":\"\",\"smsText\":\"\",\"callState\":\"idle\",\"canRing\":true,\"canAnswer\":false,\"canHangUp\":false,\"latitude\":\"37.3349\",\"longitude\":\"-122.0090\",\"walking\":false,\"reportedLocation\":null}")
    return 0
}

@_cdecl("lt_storage_get")
public func lt_storage_get(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, "{\"devices\":[],\"ipsws\":[],\"decryptedBytes\":0,\"logsBytes\":0,\"libraryIpas\":0,\"libraryBytes\":0,\"unusedBytes\":0}")
    return 0
}

private var swiftMockProxyJson = "{\"mode\":\"off\",\"archiveDate\":\"2026-01-01\"}"

@_cdecl("lt_proxy_get")
public func lt_proxy_get(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, swiftMockProxyJson)
    return 0
}

@_cdecl("lt_proxy_set")
public func lt_proxy_set(_ device: UInt64, _ json: UnsafePointer<CChar>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    if let json { swiftMockProxyJson = String(cString: json) }
    return 0
}

private var swiftMockDebugPortEnabled = false

@_cdecl("lt_debug_port_get")
public func lt_debug_port_get(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, "{\"enabled\":\(swiftMockDebugPortEnabled),\"port\":null,\"shortName\":\"device\",\"lldbWithSymbols\":null}")
    return 0
}

@_cdecl("lt_debug_port_set")
public func lt_debug_port_set(_ device: UInt64, _ json: UnsafePointer<CChar>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockDebugPortEnabled = (json.map { String(cString: $0) } ?? "").contains("\"enabled\":true")
    return 0
}

@_cdecl("lt_apps_list")
public func lt_apps_list(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, "{\"banner\":{\"text\":null},\"device\":{\"canQueueInstall\":true,\"canReachDevice\":true,\"installing\":false,\"preparingDevice\":false,\"hasFileTransfer\":true,\"reconnecting\":false,\"transfersPaused\":false},\"rows\":[]}")
    return 0
}

@_cdecl("lt_apps_install")
public func lt_apps_install(_ device: UInt64, _ ipaPath: UnsafePointer<CChar>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

@_cdecl("lt_files_state")
public func lt_files_state(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, "{\"state\":\"disconnected\",\"device\":null}")
    return 0
}

@_cdecl("lt_files_list")
public func lt_files_list(_ device: UInt64, _ path: UnsafePointer<CChar>?, _ requestIdOut: UnsafeMutablePointer<UInt64>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    requestIdOut?.pointee = 1
    return 0
}

@_cdecl("lt_files_transfer")
public func lt_files_transfer(_ device: UInt64, _ opJson: UnsafePointer<CChar>?, _ transferIdOut: UnsafeMutablePointer<UInt64>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    transferIdOut?.pointee = 1
    return 0
}

@_cdecl("lt_files_cancel")
public func lt_files_cancel(_ transferId: UInt64) -> Int32 {
    return 0
}

@_cdecl("lt_log_sources")
public func lt_log_sources(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, "[\"app\"]")
    return 0
}

@_cdecl("lt_log_read")
public func lt_log_read(_ source: UnsafePointer<CChar>?, _ from: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, "{\"text\":\"\",\"rotated\":false,\"offset\":0}")
    return 0
}

@_cdecl("lt_capture_availability")
public func lt_capture_availability(_ device: UInt64, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    swiftMockString(out, "{\"canTakeScreenshot\":false,\"canStartRecording\":false,\"canToggleRecording\":false,\"screenshotBusy\":false,\"recordingSaving\":false,\"recordingCanStop\":false}")
    return 0
}

@_cdecl("lt_capture_screenshot")
public func lt_capture_screenshot(_ device: UInt64) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

@_cdecl("lt_capture_record_start")
public func lt_capture_record_start(_ device: UInt64) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

@_cdecl("lt_capture_record_stop")
public func lt_capture_record_stop(_ device: UInt64, _ discard: Int32) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

@_cdecl("lt_capture_record_save")
public func lt_capture_record_save(_ device: UInt64, _ path: UnsafePointer<CChar>?) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

@_cdecl("lt_capture_record_discard")
public func lt_capture_record_discard(_ device: UInt64) -> Int32 {
    guard device >= 1, device <= UInt64(mockDevices.count) else { return -2 }
    return 0
}

private var swiftMockCapturePrefs = "{\"saveLocation\":\"~/Pictures\",\"openFinderAfterCapture\":true,\"copyOnCapture\":false,\"soundEffectsEnabled\":true,\"recordBezel\":false}"

@_cdecl("lt_capture_prefs_get")
public func lt_capture_prefs_get(_ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, swiftMockCapturePrefs)
    return 0
}

@_cdecl("lt_capture_prefs_set")
public func lt_capture_prefs_set(_ json: UnsafePointer<CChar>?) -> Int32 {
    if let json { swiftMockCapturePrefs = String(cString: json) }
    return 0
}

@_cdecl("lt_catalog_details")
public func lt_catalog_details(_ entryId: UnsafePointer<CChar>?, _ out: UnsafeMutablePointer<UnsafePointer<CChar>?>?) -> Int32 {
    swiftMockString(out, "{\"id\":\"n88ap-3.1.3\",\"requirements\":[],\"keysState\":\"bundled\",\"recipeSteps\":[],\"emulatorNotes\":\"\",\"installedVersion\":null,\"offersJailbreak\":true,\"offersSkipSetup\":true}")
    return 0
}

@_cdecl("lt_job_start")
public func lt_job_start(_ entryId: UnsafePointer<CChar>?, _ optionsJson: UnsafePointer<CChar>?) -> Int32 {
    return 0
}

@_cdecl("lt_job_cancel")
public func lt_job_cancel(_ entryId: UnsafePointer<CChar>?) -> Int32 {
    return 0
}

@_cdecl("lt_jobs_can_download")
public func lt_jobs_can_download(_ out: UnsafeMutablePointer<Int32>?) -> Int32 {
    out?.pointee = 1
    return 0
}

@_cdecl("lt_drop_route")
public func lt_drop_route(_ pathsJson: UnsafePointer<CChar>?, _ target: UnsafePointer<CChar>?, _ requestIdOut: UnsafeMutablePointer<UInt64>?) -> Int32 {
    requestIdOut?.pointee = 1
    return 0
}
