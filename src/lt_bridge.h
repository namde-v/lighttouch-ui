/* lt_bridge.h — C ABI contract between libLTAppCore (Swift) and the Qt 6 UI.
 *
 * This header is the single source of truth both sides compile against.
 * Translated from ui/ABI.md (2026-10-16, Phase 2 design). Where ABI.md was
 * silent and a choice was required, this header chooses and marks the choice
 * with "DEVIATION" below so the Swift side can reconcile when it lands.
 *
 * DEVIATIONS from ABI.md (all are Qt-side placeholders pending the real
 * libLTAppCore; none change the shape of the contract):
 *  1. LTEventKind numeric values: ABI.md lists kinds but assigns no numbers.
 *     Values 1..39 assigned here in ABI.md §2.3 document order;
 *     kLTEventReservedStart = 0x1000 per ABI.md.
 *  2. kLTVMState* values: ABI.md names the family ("kLTVMState…") but no
 *     members. kLTVMStateUnknown/Off/Running/Paused/Sleeping/Starting/Stopping
 *     = 0..6 defined here as placeholders.
 *  3. kLTBootStage* values: same — Unknown/Prepare/LoadFirmware/BootRom/
 *     Kernel/SpringBoard/Ready = 0..6 placeholders.
 *  4. kLTButton* values: ABI.md says "DeviceInput.Button raw values as
 *     kLTButton…" without listing them. kLTButtonNone=0, Home=1, Menu=2,
 *     VolumeUp=3, VolumeDown=4, Power=5, Mute=6 placeholders.
 *  5. kLTMenuItem* values: ABI.md names Lock/Rotate/CompassHeading/Carrier/
 *     Input/Pause/Restart. Assigned 1..7 here.
 *  6. kLTNetworkAccess* values: Unknown=0, Granted=1, Denied=2,
 *     NotDetermined=3 placeholders.
 *  7. kLTDeviceRowState* values: the "status" field of lt_device_list rows is
 *     typed "kLTDeviceRowState…" in ABI.md. Unknown=0, Off=1, Running=2,
 *     Paused=3, Booting=4, Error=5 placeholders.
 *  8. KeyModifiers bitmask: ABI.md references the mask without values.
 *     Shift=0x01, Control=0x02, Alt=0x04, Command=0x08, CapsLock=0x10,
 *     Function=0x20 placeholders.
 *  9. kLTPixelFormat* values: Unknown=0, BGRA8888=1, RGBA8888=2 placeholders.
 * 10. lt_carrier_set_registered/sim/bars and lt_carrier_ring/answer/hangup:
 *     ABI.md uses slash shorthand; expanded here to six separate functions
 *     with explicit signatures.
 * 11. lt_proxy_get/set, lt_debug_port_get/set: ABI.md signatures were "…";
 *     defined here as JSON snapshot in/out (query/command pair).
 * 12. lt_capture_record_save / lt_capture_record_discard: named in ABI.md
 *     §3.8 prose (two-step save handshake) but absent from the table;
 *     declared here.
 * 13. kLTDropAction* string constants ("import-ipsw" etc.): ABI.md §3.10
 *     gives the wire strings inside the kLTEventDropAction payload; hoisted
 *     here as named constants.
 * 14. lt_jobs_can_download: ABI.md §3.10 describes the result as JSON
 *     { "can":…, "reason":…|null } but the C signature carries only
 *     int32_t*. Kept as declared; the Qt side reads *out as 0/1 and takes
 *     the reason from lt_last_error() when can == 0. Swift side to confirm.
 *
 * Rules (normative, from ABI.md §1):
 *  - Only C types cross. <stdint.h> fixed-width types exclusively.
 *  - String ownership: callee allocates, caller frees with lt_free_string().
 *    Strings passed IN are borrowed. Event strings are owned by the callback
 *    invocation and released with lt_free_event_strings().
 *  - No exceptions, no re-entrancy (never call lt_* from inside the event
 *    callback), commands never block, queries never wait on @MainActor.
 */

#ifndef LT_BRIDGE_H
#define LT_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* 1. ABI version                                                      */
/* ------------------------------------------------------------------ */

/* The ABI version this header describes. The Qt side requires the loaded
 * library's lt_abi_version() >= kLTAbiVersionMin; newer libraries are
 * backward compatible (ABI.md §5). */
#define kLTAbiVersionCurrent 1
#define kLTAbiVersionMin     1

/* ------------------------------------------------------------------ */
/* 2. Result codes (ABI.md §1.4)                                       */
/* ------------------------------------------------------------------ */

typedef int32_t LTResult;
#define kLTOk           0  /* accepted / succeeded */
#define kLTAccepted     0  /* alias: command enqueued; outcome arrives as LTEvent */
#define kLTBadArgument -1  /* null pointer, malformed UTF-8/JSON/UUID, out-of-range */
#define kLTNoSuchDevice -2 /* unknown lt_device_t or UUID */
#define kLTNotReady    -3  /* core not initialized, or session not in a usable state */
#define kLTBusy        -4  /* an exclusive operation is already running */
#define kLTFailed      -5  /* anything else; detail in lt_last_error() */

/* ------------------------------------------------------------------ */
/* 3. Handles                                                          */
/* ------------------------------------------------------------------ */

/* Opaque per-device cookie minted by Swift's app controller at startup,
 * stable for the process lifetime. 0 = invalid. */
typedef uint64_t lt_device_t;
#define kLTInvalidDevice ((lt_device_t)0)

/* ------------------------------------------------------------------ */
/* 4. Event kinds (ABI.md §2.3)                                        */
/* ------------------------------------------------------------------ */

typedef int32_t LTEventKind;

#define kLTEventDeviceListChanged          1
#define kLTEventDeviceRowChanged           2
#define kLTEventSidebarChanged             3
#define kLTEventDeviceStatusChanged        4
#define kLTEventBootStage                  5
#define kLTEventNoticeChanged              6
#define kLTEventStorageBusyChanged         7
#define kLTEventDisplayLinkReady           8
#define kLTEventDisplayLinkClosed           9
#define kLTEventFrameAvailable            10
#define kLTEventRotationChanged           11
#define kLTEventZoomChanged               12
#define kLTEventFingerDotsChanged         13
#define kLTEventTweaksChanged             14
#define kLTEventHiddenAppsListed          15
#define kLTEventCarrierChanged            16
#define kLTEventAppsChanged               17
#define kLTEventInstallProgress           18
#define kLTEventInstallDone               19
#define kLTEventFilesListed               20
#define kLTEventFilesProgress             21
#define kLTEventFilesDone                 22
#define kLTEventFilesConnectionChanged    23
#define kLTEventLogLine                   24
#define kLTEventLogRotated                25
#define kLTEventCaptureAvailabilityChanged 26
#define kLTEventCaptureDone               27
#define kLTEventRecordingTick             28
#define kLTEventRecordingPhaseChanged     29
#define kLTEventRecordingSaveNeeded       30
#define kLTEventCatalogChanged            31
#define kLTEventJobProgress               32
#define kLTEventJobDone                   33
#define kLTEventTaskbarProgress           34
#define kLTEventToast                     35
#define kLTEventDropAction                36
#define kLTEventNetworkAccessChanged      37
#define kLTEventQuitReady                 38
#define kLTEventShutdown                  39

/* First value available to future extension without renumbering. */
#define kLTEventReservedStart 0x1000

/* ------------------------------------------------------------------ */
/* 5. Event payload structs and the LTEvent tagged union               */
/* ------------------------------------------------------------------ */

typedef struct LTEventDeviceRowChanged {
    lt_device_t device;
} LTEventDeviceRowChanged;

typedef struct LTEventDeviceStatusChanged {
    int32_t machine;   /* kLTVMState… */
    int32_t running;   /* 0/1 */
    int32_t sleeping;  /* 0/1 */
} LTEventDeviceStatusChanged;

typedef struct LTEventBootStage {
    int32_t stage;        /* kLTBootStage… */
    const char* detail;   /* may be NULL */
    double fraction;      /* < 0 = indeterminate */
} LTEventBootStage;

typedef struct LTEventNoticeChanged {
    const char* message;    /* NULL = cleared */
    const char* operation;  /* may be NULL */
    int32_t offersErase;    /* 0/1 */
} LTEventNoticeChanged;

typedef struct LTEventDisplayLinkReady {
    int32_t fd;              /* POSIX fd (Linux/macOS); named file mapping handle value (Windows) */
    int32_t width;
    int32_t height;
    int32_t stride;          /* bytes per row */
    int32_t format;          /* kLTPixelFormat… */
    uint64_t ringGeneration; /* invalidates any previous mapping */
} LTEventDisplayLinkReady;

typedef struct LTEventFrameAvailable {
    uint64_t frameSeq;
    int32_t width;
    int32_t height;
} LTEventFrameAvailable;

typedef struct LTEventRotationChanged {
    int32_t degrees;
    int32_t width;   /* upright guest pixels */
    int32_t height;
} LTEventRotationChanged;

typedef struct LTEventZoomChanged {
    double points;        /* effective points-per-pixel */
    const char* mode;     /* "fit" | "physical" | "pixel" | "p:<n>" */
} LTEventZoomChanged;

typedef struct LTEventFingerDotsChanged {
    int32_t shown;  /* 0/1 */
} LTEventFingerDotsChanged;

typedef struct LTEventHiddenAppsListed {
    const char* appsJson;  /* NULL on failure */
} LTEventHiddenAppsListed;

typedef struct LTEventInstallProgress {
    const char* bundleId;
    double fraction;
    const char* phase;  /* may be NULL */
} LTEventInstallProgress;

typedef struct LTEventInstallDone {
    const char* bundleId;
    int32_t ok;             /* 0/1 */
    const char* message;    /* NULL on success */
} LTEventInstallDone;

typedef struct LTEventFilesListed {
    uint64_t requestId;
    const char* entriesJson;  /* NULL on failure */
} LTEventFilesListed;

typedef struct LTEventFilesProgress {
    uint64_t transferId;
    double fraction;
    const char* currentFile;  /* may be NULL */
    int64_t bytesDone;
    int64_t bytesTotal;
} LTEventFilesProgress;

typedef struct LTEventFilesDone {
    uint64_t transferId;
    int32_t ok;         /* 0/1 */
    int32_t cancelled;  /* 0/1 */
    const char* message; /* may be NULL */
} LTEventFilesDone;

typedef struct LTEventFilesConnectionChanged {
    int32_t state;  /* kLTFilesState… */
} LTEventFilesConnectionChanged;

typedef struct LTEventLogLine {
    const char* source;  /* "app" | "helper:<uuid>" */
    const char* text;    /* whole lines, capped per post */
    uint64_t offset;     /* byte offset after this batch */
} LTEventLogLine;

typedef struct LTEventLogRotated {
    const char* source;
} LTEventLogRotated;

typedef struct LTEventCaptureDone {
    int32_t kind;          /* kLTCaptureScreenshot | kLTCaptureRecording */
    const char* path;      /* "" = ask the user (recording save handshake) */
    int32_t ok;            /* 0/1 */
    const char* message;   /* may be NULL */
} LTEventCaptureDone;

typedef struct LTEventRecordingTick {
    uint64_t elapsedSeconds;
} LTEventRecordingTick;

typedef struct LTEventRecordingPhaseChanged {
    int32_t phase;  /* kLTRecordingPhase… */
} LTEventRecordingPhaseChanged;

typedef struct LTEventRecordingSaveNeeded {
    const char* suggestedName;  /* may be NULL */
} LTEventRecordingSaveNeeded;

typedef struct LTEventJobProgress {
    const char* entryId;
    double fraction;      /* -1 = indeterminate */
    const char* detail;   /* may be NULL */
    double etaSeconds;    /* -1 = unknown */
    int32_t step;
    int32_t steps;
} LTEventJobProgress;

typedef struct LTEventJobDone {
    const char* entryId;
    int32_t ok;         /* 0/1 */
    int32_t cancelled;  /* 0/1 */
    const char* message; /* may be NULL */
} LTEventJobDone;

typedef struct LTEventTaskbarProgress {
    double fraction;  /* < 0 = clear badge */
} LTEventTaskbarProgress;

typedef struct LTEventToast {
    const char* title;     /* may be NULL */
    const char* body;      /* may be NULL */
    const char* category;  /* may be NULL */
} LTEventToast;

typedef struct LTEventDropAction {
    uint64_t requestId;
    const char* action;      /* kLTDropAction* string */
    const char* detailJson;  /* may be NULL */
} LTEventDropAction;

typedef struct LTEventNetworkAccessChanged {
    int32_t state;  /* kLTNetworkAccess… */
} LTEventNetworkAccessChanged;

typedef struct LTEvent {
    LTEventKind kind;
    lt_device_t device;  /* 0 when the event is not device-scoped */
    uint64_t seq;        /* per-kind monotonic sequence; 0 when unused */
    union {
        LTEventDeviceRowChanged       rowChanged;
        LTEventDeviceStatusChanged    statusChanged;
        LTEventBootStage              bootStage;
        LTEventNoticeChanged          noticeChanged;
        LTEventDisplayLinkReady       displayLinkReady;
        LTEventFrameAvailable         frameAvailable;
        LTEventRotationChanged        rotationChanged;
        LTEventZoomChanged            zoomChanged;
        LTEventFingerDotsChanged      fingerDotsChanged;
        LTEventHiddenAppsListed       hiddenAppsListed;
        LTEventInstallProgress        installProgress;
        LTEventInstallDone            installDone;
        LTEventFilesListed            filesListed;
        LTEventFilesProgress          filesProgress;
        LTEventFilesDone              filesDone;
        LTEventFilesConnectionChanged filesConnectionChanged;
        LTEventLogLine                logLine;
        LTEventLogRotated             logRotated;
        LTEventCaptureDone            captureDone;
        LTEventRecordingTick          recordingTick;
        LTEventRecordingPhaseChanged  recordingPhaseChanged;
        LTEventRecordingSaveNeeded    recordingSaveNeeded;
        LTEventJobProgress            jobProgress;
        LTEventJobDone                jobDone;
        LTEventTaskbarProgress        taskbarProgress;
        LTEventToast                  toast;
        LTEventDropAction             dropAction;
        LTEventNetworkAccessChanged   networkAccessChanged;
    } u;
} LTEvent;

/* The event callback. Invoked synchronously from whatever Swift thread
 * produced the event; the LTEvent is valid only for the call duration.
 * The callback must be a leaf: copy, queue, return — never call lt_*
 * functions from inside it, never do Qt work in it. */
typedef void (*lt_event_fn)(const LTEvent* event, void* ctx);

/* ------------------------------------------------------------------ */
/* 6. Enum constants                                                   */
/*                                                                     */
/* Unknown values from a newer Swift core must be tolerated by Qt       */
/* (ABI.md §5): adapters map unrecognized values to a documented        */
/* "unknown/other" fallback, never crash.                              */
/* ------------------------------------------------------------------ */

/* Machine/session states (kLTVMState…; DEVIATION 2: values assigned here). */
#define kLTVMStateUnknown  0
#define kLTVMStateOff      1
#define kLTVMStateRunning  2
#define kLTVMStatePaused   3
#define kLTVMStateSleeping 4
#define kLTVMStateStarting 5
#define kLTVMStateStopping 6

/* Boot phases for kLTEventBootStage (DEVIATION 3). */
#define kLTBootStageUnknown      0
#define kLTBootStagePrepare      1
#define kLTBootStageLoadFirmware 2
#define kLTBootStageBootRom      3
#define kLTBootStageKernel       4
#define kLTBootStageSpringBoard  5
#define kLTBootStageReady        6

/* Pixel formats for kLTEventDisplayLinkReady.format (DEVIATION 9). */
#define kLTPixelFormatUnknown  0
#define kLTPixelFormatBGRA8888 1  /* little-endian UInt32 == QImage::Format_ARGB32 */
#define kLTPixelFormatRGBA8888 2

/* Touch phases for lt_input_touch (ABI.md §3.4). */
#define kLTTouchBegan     0
#define kLTTouchMoved     1
#define kLTTouchEnded     2
#define kLTTouchCancelled 3

/* Physical buttons for lt_input_button (DEVIATION 4). */
#define kLTButtonNone      0
#define kLTButtonHome      1
#define kLTButtonMenu      2
#define kLTButtonVolumeUp  3
#define kLTButtonVolumeDown 4
#define kLTButtonPower    5
#define kLTButtonMute     6

/* Motion poses for lt_input_motion_pose (ABI.md §3.4). */
#define kLTMotionPoseUpright 0
#define kLTMotionPoseFlat    1

/* Files connection states (ABI.md §2.3/§3.7). */
#define kLTFilesStateDisconnected 0
#define kLTFilesStateConnected    1
#define kLTFilesStateTransferring 2

/* Capture kinds for kLTEventCaptureDone.kind (ABI.md §2.3). */
#define kLTCaptureScreenshot 0
#define kLTCaptureRecording  1

/* Recording phases for kLTEventRecordingPhaseChanged (ABI.md §2.3). */
#define kLTRecordingPhaseIdle      0
#define kLTRecordingPhaseStarting  1
#define kLTRecordingPhaseRecording 2
#define kLTRecordingPhaseSaving    3

/* Local-network permission states (DEVIATION 6). */
#define kLTNetworkAccessUnknown      0
#define kLTNetworkAccessGranted      1
#define kLTNetworkAccessDenied       2
#define kLTNetworkAccessNotDetermined 3

/* Drop classification for lt_drop_classify (ABI.md §3.10). */
#define kLTDropIPSW        0
#define kLTDropIPA         1
#define kLTDropMedia       2
#define kLTDropUnsupported 3

/* Drop-action wire strings for kLTEventDropAction.action (DEVIATION 13). */
#define kLTDropActionImportIPSW  "import-ipsw"
#define kLTDropActionInstallIPA  "install-ipa"
#define kLTDropActionImportMedia "import-media"
#define kLTDropActionUnsupported "unsupported"

/* Menu items for lt_menu_validate (DEVIATION 5). */
#define kLTMenuItemLock           1
#define kLTMenuItemRotate         2
#define kLTMenuItemCompassHeading 3
#define kLTMenuItemCarrier        4
#define kLTMenuItemInput          5
#define kLTMenuItemPause          6
#define kLTMenuItemRestart        7

/* Key-modifier bitmask for lt_input_key (DEVIATION 8). */
#define kLTKeyModifierShift    0x01
#define kLTKeyModifierControl  0x02
#define kLTKeyModifierAlt      0x04
#define kLTKeyModifierCommand  0x08
#define kLTKeyModifierCapsLock 0x10
#define kLTKeyModifierFunction 0x20

/* Device-row "status" field values for lt_device_list/row (DEVIATION 7). */
#define kLTDeviceRowStateUnknown 0
#define kLTDeviceRowStateOff     1
#define kLTDeviceRowStateRunning 2
#define kLTDeviceRowStatePaused  3
#define kLTDeviceRowStateBooting  4
#define kLTDeviceRowStateError    5

/* Zoom mode strings (ABI.md §3.4). Free-form zoom is "p:<points>". */
#define kLTZoomModeFit      "fit"
#define kLTZoomModePhysical "physical"
#define kLTZoomModePixel    "pixel"

/* Keys for lt_settings_get/set_string (ABI.md §3.11). */
#define kLTSettingsKeyNetworkAccessAnswer "networkAccess.answer"

/* ------------------------------------------------------------------ */
/* 7. Exported functions (ABI.md §3)                                   */
/*                                                                     */
/* Each entry is labeled [command], [query], or [pure] (ABI.md §1.5).  */
/* JSON schemas are normative for the fields listed; Swift may ADD     */
/* fields (Qt ignores unknown fields), never remove or retype them.    */
/* ------------------------------------------------------------------ */

/* ---- 7.1 Lifecycle ---- */

/* [command] Initialize the app controller (storage, library, catalog,
 * jobs, event pump). configJson: { "appSupportDir":…, "logDir":…,
 * "resourceDir":… }. Call once, on the UI thread, before any other lt_ call. */
LTResult lt_init(const char* configJson);

/* [command] Graceful teardown; posts kLTEventShutdown. After it returns
 * only lt_init (fresh process state) is legal. */
LTResult lt_shutdown(void);

/* [command] Register the §2 event callback. Idempotent; fn == NULL
 * unregisters. */
LTResult lt_set_event_callback(lt_event_fn fn, void* ctx);

/* [pure] The ABI version this library was built against (§5). */
int32_t lt_abi_version(void);

/* [query] App version string (AboutCredits). Caller frees with lt_free_string. */
LTResult lt_version(const char** out);

/* [pure] Deallocator for every returned string (§1.3). NULL-safe. */
void lt_free_string(const char* s);

/* [pure] Deep-frees an event's string payloads (§1.3). */
void lt_free_event_strings(const LTEvent* e);

/* [command] QuitCoordinator: begins graceful shutdown. Result arrives as
 * kLTEventQuitReady or kLTEventToast. */
LTResult lt_quit_request(void);

/* [query] JSON { "credits": […], "licenses": […] } from AboutCredits. */
LTResult lt_about_credits(const char** out);

/* ---- 7.2 Device library & rows ---- */

/* [query] JSON array of DeviceRow snapshots:
 * [{ "id": uuid, "name":…, "board":…, "firmware":…,
 *    "status": kLTDeviceRowState…, "statusText":…,
 *    "busy": "deleting"|"erasing"|null, "firmwareNote":…|null }] */
LTResult lt_device_list(const char** out);

/* [query] UUID → handle. */
LTResult lt_device_find(const char* uuid, lt_device_t* out);

/* [query] Handle → UUID string. */
LTResult lt_device_uuid(lt_device_t device, const char** out);

/* [query] One row's JSON (same schema as list entries). */
LTResult lt_device_row(lt_device_t device, const char** out);

/* [command] Create a library device from a catalog entry. The row exists
 * immediately (handle in out); storage creation is enqueued.
 * Emits kLTEventDeviceListChanged. */
LTResult lt_device_create(const char* entryId, const char* name, lt_device_t* out);

/* [command] Guarded delete. Emits kLTEventDeviceListChanged / failure toast. */
LTResult lt_device_delete(lt_device_t device);

/* [command] Rename. Emits kLTEventDeviceListChanged. */
LTResult lt_device_rename(lt_device_t device, const char* name);

/* [command] Duplicate device; new handle in out. */
LTResult lt_device_duplicate(lt_device_t device, lt_device_t* out);

/* [command] Erase storage (guest reset). Emits kLTEventStorageBusyChanged +
 * kLTEventDeviceStatusChanged. */
LTResult lt_device_erase(lt_device_t device);

/* [query] JSON { "<uuid>": "deleting"|"erasing" } busy map. */
LTResult lt_storage_busy(const char** out);

/* [query] JSON { "ids": […], "names": {…},
 * "labels": [{ "id":…, "title":…, "subtitle":… }] } */
LTResult lt_sidebar_list(const char** out);

/* [command] SidebarList.add + save; emits kLTEventSidebarChanged. */
LTResult lt_sidebar_add(const char* idsJson);

/* [command] SidebarList.remove + save. */
LTResult lt_sidebar_remove(const char* entryId);

/* [command] SidebarList.rename + save. */
LTResult lt_sidebar_rename(const char* entryId, const char* name);

/* ---- 7.3 Session lifecycle & status ---- */

/* [command] BootCycle.begin() with BootSettings from optionsJson
 * ({ "panel":…, "network":…, "debugPort":…, "bootArgs":… }).
 * Progress via kLTEventBootStage; readiness via kLTEventDeviceStatusChanged. */
LTResult lt_device_boot(lt_device_t device, const char* optionsJson);

/* [command] ShutdownLadder graceful halt. */
LTResult lt_device_halt(lt_device_t device);

/* [command] Pause/resume machine. */
LTResult lt_device_pause(lt_device_t device, int32_t paused);

/* [command] BootCycle.reset() (in-place restart). */
LTResult lt_device_reset(lt_device_t device);

/* [command] Lock/Wake/Start. */
LTResult lt_device_lock(lt_device_t device, int32_t locked);

/* [query] JSON snapshot merging DeviceMenuState + CaptureAvailability:
 * { "machine": kLTVMState…, "running":…, "sleeping":…, "paused":…,
 *   "shuttingDown":…, "storageFailed":…, "installing":…, "acceptsInput":…,
 *   "hasCompass":…, "hasCellular":…, "compassHeading":…, "editingText":… } */
LTResult lt_device_status(lt_device_t device, const char** out);

/* [query] DeviceMenuState.validate for one item (kLTMenuItem*; arg = heading
 * for compass). Returns { "enabled":…, "title":…|null, "checked":…|null }. */
LTResult lt_menu_validate(lt_device_t device, int32_t item, int32_t arg, const char** out);

/* [query] CaptureAvailability as JSON:
 * { "canTakeScreenshot":…, "canStartRecording":…, "canToggleRecording":…,
 *   "screenshotBusy":…, "recordingSaving":…, "recordingCanStop":… } */
LTResult lt_capture_availability(lt_device_t device, const char** out);

/* [query] DeviceNotices.message snapshot:
 * { "message":…|null, "operation":…|null, "offersErase":… } */
LTResult lt_notice_get(lt_device_t device, const char** out);

/* [command] DeviceNotices.resolve(_:). */
LTResult lt_notice_resolve(lt_device_t device, const char* operation);

/* [command] DeviceNotices.dismiss(). */
LTResult lt_notice_dismiss(lt_device_t device);

/* [query] LocalNetworkAccess state JSON:
 * { "state": kLTNetworkAccess…, "explanation":… } */
LTResult lt_network_status(const char** out);

/* [command] Record the NetworkAccessPrompt answer; emits
 * kLTEventNetworkAccessChanged. */
LTResult lt_network_set_allowed(int32_t allowed);

/* ---- 7.4 Display & input ---- */
/* The frame path bypasses the bridge (Qt maps the shm ring directly).
 * Everything below is the control plane. All lt_input_* are [command]. */

/* Normalized touch (0–1, y-down). phase: kLTTouchBegan/Moved/Ended/Cancelled.
 * Multi-touch: one call per touch id. */
LTResult lt_input_touch(lt_device_t d, int32_t phase, double x, double y, int32_t touchId);

/* Key event. code = USB HID usage the guest expects; modifiers =
 * kLTKeyModifier* bitmask. */
LTResult lt_input_key(lt_device_t d, uint16_t code, int32_t down, int32_t modifiers);

/* Text that doesn't map to key codes (IME, dead keys, paste). */
LTResult lt_input_text(lt_device_t d, const char* text, int32_t shiftHeld);

/* Physical button tap (kLTButton…). */
LTResult lt_input_button(lt_device_t d, int32_t button);

/* Shake gesture. */
LTResult lt_input_shake(lt_device_t d);

/* Tilt: accelerometer angle/pitch. */
LTResult lt_input_tilt(lt_device_t d, double angle, double pitch);

/* Motion pose (kLTMotionPoseUpright/Flat). */
LTResult lt_input_motion_pose(lt_device_t d, int32_t pose);

/* Guest rotation request; geometry arrives as kLTEventRotationChanged. */
LTResult lt_input_rotate(lt_device_t d, int32_t degrees);

/* Compass heading override. */
LTResult lt_input_compass(lt_device_t d, int32_t heading);

/* Keyboard-as-mouse drive mode key. */
LTResult lt_input_keyboard_pointer_key(lt_device_t d, uint16_t code, int32_t down, int32_t modifiers);

/* [command] Set ZoomMode ("fit"|"physical"|"pixel"|"p:<n>"); persists per
 * board. Emits kLTEventZoomChanged. */
LTResult lt_display_set_zoom(lt_device_t d, const char* mode);

/* [query] { "mode": "p:1.5", "points": 2.0, "shown": "fit" } — mode,
 * effective points-per-pixel, and the display name. */
LTResult lt_display_get_zoom(lt_device_t d, const char** out);

/* [pure] ZoomContext.stops as JSON [{ "mode":…, "points":… }] for the
 * zoom ladder. fit/backing computed Qt-side from the pane; physical = -1
 * when unknown (falls back to Fit). */
LTResult lt_zoom_stops_json(double fit, double physical, double backing, const char** out);

/* [pure] ZoomContext.step(from:direction:). Returns -1 past either end. */
double lt_zoom_step(double fit, double physical, double backing, double p, int32_t direction);

/* [pure] ZoomContext.drawsNearest — texture filtering choice. */
int32_t lt_zoom_draws_nearest(double p, double backing);

/* [pure] FreeFormResize drag math → upright guest pixels:
 * { "width":…, "height":…, "shiftX":…, "shiftY":…, "limit":…|null }. */
LTResult lt_resize_drag(const char* board, double startW, double startH, double edgeX, double edgeY, double deltaX, double deltaY, double pointsPerPixel, int32_t symmetric, int32_t quarterTurned, const char** out);

/* [command] Apply a free-form size to the running session. Emits
 * kLTEventRotationChanged (geometry) + kLTEventZoomChanged. */
LTResult lt_display_set_panel_size(lt_device_t d, int32_t width, int32_t height);

/* [command] Window finger-dots toggle (Qt owns the overlay rendering). */
LTResult lt_display_set_finger_dots(lt_device_t d, int32_t shown);

/* [query] Current finger-dots toggle state. */
LTResult lt_display_get_finger_dots(lt_device_t d, int32_t* out);

/* ---- 7.5 Inspector panels ---- */

/* Tweaks (fronts TweaksPanelModel): */

/* [query] JSON { "running":…, "tweaks": [{ "id":…, "caption":…, "on":…,
 * "available":… }], "color":…, "clock": epoch|null, "status":…|null,
 * "message":…|null, "hiddenApps": […]|null } */
LTResult lt_tweaks_get(lt_device_t d, const char** out);

/* [command] set(_:on:); emits kLTEventTweaksChanged. */
LTResult lt_tweaks_set(lt_device_t d, const char* tweakId, int32_t on);

/* [command] set(coreAnimationColor:) (CoreAnimationColor raw value). */
LTResult lt_tweaks_set_color(lt_device_t d, int32_t color);

/* [command] set(clock:); epoch = Unix seconds, UTC. */
LTResult lt_tweaks_set_clock(lt_device_t d, double epoch);

/* [command] Async listHiddenApps(); result in kLTEventHiddenAppsListed. */
LTResult lt_tweaks_list_hidden_apps(lt_device_t d);

/* [command] open(_:); failures surface via kLTEventTweaksChanged message. */
LTResult lt_tweaks_open_app(lt_device_t d, const char* bundleId);

/* Carrier (fronts CarrierPanelModel). The poll loop is Swift-side;
 * lt_carrier_set_visible starts/stops it. Changes emit kLTEventCarrierChanged. */

/* [command] Start/stop polling + (re)bind the session backend. */
LTResult lt_carrier_set_visible(lt_device_t d, int32_t visible);

/* [query] JSON { "carrierName":…, "mcc":…, "mnc":…, "settings": {…},
 * "status": {…}|null, "sent": [{ "id":…, "number":…, "text":… }],
 * "message":…|null, "networkValid":…, "networkEdited":…,
 * "applyingNetwork":…, "callNumber":…, "smsNumber":…, "smsText":…,
 * "callState":…, "canRing":…, "canAnswer":…, "canHangUp":…,
 * "latitude":…, "longitude":…, "walking":…, "reportedLocation": {…} } */
LTResult lt_carrier_get(lt_device_t d, const char** out);

/* [command] Typed-but-unapplied edits. field: "carrierName"|"mcc"|"mnc"|
 * "callNumber"|"smsNumber"|"smsText"|"latitude"|"longitude". */
LTResult lt_carrier_set_field(lt_device_t d, const char* field, const char* value);

/* [command] applyNetwork(). */
LTResult lt_carrier_apply_network(lt_device_t d);

/* [command] set(registered:) / set(simPresent:) / set(bars:).
 * (DEVIATION 10: ABI.md's slash shorthand expanded to three functions.) */
LTResult lt_carrier_set_registered(lt_device_t d, int32_t registered);
LTResult lt_carrier_set_sim(lt_device_t d, int32_t present);
LTResult lt_carrier_set_bars(lt_device_t d, int32_t bars);

/* [command] ring() / answer() / hangUp(). (DEVIATION 10.) */
LTResult lt_carrier_ring(lt_device_t d);
LTResult lt_carrier_answer(lt_device_t d);
LTResult lt_carrier_hangup(lt_device_t d);

/* [command] sendSMS() (number/text set via lt_carrier_set_field). */
LTResult lt_carrier_send_sms(lt_device_t d);

/* [command] applyLocation(). */
LTResult lt_carrier_apply_location(lt_device_t d);

/* [command] set(location:) — presets, map clicks. */
LTResult lt_carrier_set_location(lt_device_t d, double lat, double lon);

/* [command] toggleWalk(). */
LTResult lt_carrier_toggle_walk(lt_device_t d);

/* [command] notice(_:) — e.g. host location unavailable. */
LTResult lt_carrier_notice(lt_device_t d, const char* text);

/* Storage / proxy / debug port.
 * (DEVIATION 11: signatures were "…" in ABI.md; JSON snapshot in/out.) */

/* [query] Storage settings snapshot JSON (schema fixed when the Swift model
 * lands — ABI.md §6 gap 6). */
LTResult lt_storage_get(lt_device_t d, const char** out);

/* [command] Apply storage settings JSON. */
LTResult lt_storage_set(lt_device_t d, const char* json);

/* [query] WebProxyConfiguration as JSON. */
LTResult lt_proxy_get(lt_device_t d, const char** out);

/* [command] Apply proxy configuration JSON. */
LTResult lt_proxy_set(lt_device_t d, const char* json);

/* [query] DebugPort (HostRuntime) as JSON. */
LTResult lt_debug_port_get(lt_device_t d, const char** out);

/* [command] Apply debug-port JSON. */
LTResult lt_debug_port_set(lt_device_t d, const char* json);

/* ---- 7.6 Apps inspector ---- */

/* [query] JSON { "banner": { "text":…|null },
 * "device": { "canQueueInstall":…, "canReachDevice":…, "installing":…,
 *   "preparingDevice":…, "hasFileTransfer":…, "reconnecting":…,
 *   "transfersPaused":… },
 * "rows": [{ "identity":…, "title":…, "subtitle":…,
 *   "kind": "app"|"install"|"open"|"progress"|"failed"|"resume",
 *   "progress":…|null, "enabled":… }] } */
LTResult lt_apps_list(lt_device_t d, const char** out);

/* [command] Re-read the device; emits kLTEventAppsChanged. */
LTResult lt_apps_refresh(lt_device_t d);

/* [command] Queue an install; progress via kLTEventInstallProgress,
 * outcome via kLTEventInstallDone. */
LTResult lt_apps_install(lt_device_t d, const char* ipaPath);

/* [command] Uninstall. */
LTResult lt_apps_remove(lt_device_t d, const char* bundleId);

/* [command] Dequeue a queued install. */
LTResult lt_apps_cancel_install(lt_device_t d, const char* bundleId);

/* [command] restartSpringBoard(). */
LTResult lt_apps_restart_springboard(lt_device_t d);

/* [command] DeviceApps.importMedia (pathsJson = JSON array of paths). */
LTResult lt_apps_import_media(lt_device_t d, const char* pathsJson);

/* [query] Store browser search (portable core; see ABI.md §6 gap 10). */
LTResult lt_apps_store_search(lt_device_t d, const char* query, const char** out);

/* ---- 7.7 Files browser ---- */

/* [command] Drive FilesConnection follow for the app's one Files window:
 * follow=1 binds to this device's boot endpoint, 0 releases.
 * Emits kLTEventFilesConnectionChanged. */
LTResult lt_files_follow(lt_device_t d, int32_t follow);

/* [query] { "state": "disconnected"|"connected"|"transferring",
 * "device": uuid|null } */
LTResult lt_files_state(const char** out);

/* [command] Async files(in:); result in kLTEventFilesListed with requestId.
 * Stale responses are dropped by requestId. */
LTResult lt_files_list(lt_device_t d, const char* path, uint64_t* requestIdOut);

/* [command] Start upload/download/move/mkdir/delete.
 * opJson = { "op": "upload"|"download"|"delete"|"move"|"mkdir",
 *            "src":…, "dst":… }.
 * Progress → kLTEventFilesProgress; end → kLTEventFilesDone. */
LTResult lt_files_transfer(lt_device_t d, const char* opJson, uint64_t* transferIdOut);

/* [command] Cancel a transfer. */
LTResult lt_files_cancel(uint64_t transferId);

/* ---- 7.8 Capture ---- */
/* Frame sourcing: Swift maps the same shm ring the Qt DisplayItem maps.
 * Screenshots/recordings are produced Swift-side; Qt never ships pixels
 * across the boundary. */

/* [command] Capture the latest frame; result in kLTEventCaptureDone.
 * Clipboard/open-app are Qt-side effects driven by the event + prefs. */
LTResult lt_capture_screenshot(lt_device_t d);

/* [command] ScreenRecordingSession.start with the shm-ring frame supplier.
 * Phases → kLTEventRecordingPhaseChanged; ticks → kLTEventRecordingTick. */
LTResult lt_capture_record_start(lt_device_t d);

/* [command] stop(discard:). If the session needs a save destination, Swift
 * emits kLTEventRecordingSaveNeeded instead of finishing; Qt shows the save
 * dialog then calls lt_capture_record_save / lt_capture_record_discard. */
LTResult lt_capture_record_stop(lt_device_t d, int32_t discard);

/* [command] Complete the two-step save handshake (DEVIATION 12). */
LTResult lt_capture_record_save(lt_device_t d, const char* path);

/* [command] Discard the pending recording (DEVIATION 12). */
LTResult lt_capture_record_discard(lt_device_t d);

/* [query] CapturePreferences as JSON (save location, copy-on-capture,
 * record-bezel, open-after, sound). */
LTResult lt_capture_prefs_get(const char** out);

/* [command] Apply capture prefs JSON. */
LTResult lt_capture_prefs_set(const char* json);

/* ---- 7.9 Log & console ---- */

/* [query] JSON ["app", "helper:<uuid>", …] — available tails. */
LTResult lt_log_sources(const char** out);

/* [query] LogTail.read(url, from:) → { "text":…, "rotated":…, "offset":… }
 * (64 KB cap, whole lines). */
LTResult lt_log_read(const char* source, uint64_t from, const char** out);

/* [command] Subscribe/unsubscribe; appended lines → kLTEventLogLine;
 * rotation → kLTEventLogRotated. */
LTResult lt_log_follow(const char* source, int32_t follow);

/* [command] Truncate the log; emits kLTEventLogRotated. */
LTResult lt_log_clear(const char* source);

/* [pure] LogTail.filtered(_:by:). Filtering a visible buffer is cheaper in
 * QML; exported for parity/testing. Returns 0 on success. */
int32_t lt_log_filter(const char* text, const char* filter, const char** out);

/* ---- 7.10 Firmware catalog & jobs ---- */

/* [query] filterJson = { "text":…, "board":…|null, "status":…|null }.
 * Returns entries JSON [{ "id":…, "board":…, "productType":…,
 * "marketingName":…, "version":…, "build":…, "status":…,
 * "statusNote":…|null, "prereleaseBadge":…|null, "media":[…],
 * "released":…|null, "managesApps":…, "estimates": {…} }]. */
LTResult lt_catalog_list(const char* filterJson, const char** out);

/* [query] CatalogDetailsModel snapshot JSON (requirements, keys state,
 * recipe steps, emulator notes). */
LTResult lt_catalog_details(const char* entryId, const char** out);

/* [query] FirmwareJobs.jobs (shown) JSON
 * [{ "entryId":…, "kind": "download"|"prepare"|"thirdParty",
 * "progress":…|null,
 * "preparation": { "step":…, "steps":…, "name":…, "fraction":…,
 *   "detail":…|null, "remaining":…|null }|null, "failed":…|null }] */
LTResult lt_jobs_list(const char** out);

/* [command] downloadAndPrepare with optionsJson =
 * { "jailbreak":…, "skipSetup":…, "keepStock":… }.
 * Emits kLTEventJobProgress/kLTEventJobDone. */
LTResult lt_job_start(const char* entryId, const char* optionsJson);

/* [command] importIPSWs (dropped IPSWs / folder walk results). */
LTResult lt_job_import_ipsw(const char* pathsJson);

/* [command] cancel(_:). */
LTResult lt_job_cancel(const char* entryId);

/* [command] cancelAll(). */
LTResult lt_job_cancel_all(void);

/* [command] dismissFailure(_:). */
LTResult lt_job_dismiss_failure(const char* entryId);

/* [query] canDownload + unavailableReason. NOTE (DEVIATION 14): ABI.md
 * describes the result as JSON { "can":…, "reason":…|null }, but the C
 * signature carries only int32_t*; *out is 0/1 and the reason is expected
 * via lt_last_error() when can == 0. Swift side to confirm. */
LTResult lt_jobs_can_download(int32_t* out);

/* [pure] DroppedFiles(URL) classification → kLTDropIPSW/IPA/Media/Unsupported.
 * Qt calls it off the UI thread for large drops only for lt_drop_ipsws;
 * classification itself is synchronous and cheap. */
LTResult lt_drop_classify(const char* path, int32_t* out);

/* [query-ish] DroppedFiles.ipsws(in:depth:) — folder walk for IPSWs;
 * JSON array of paths. Runs synchronously; Qt calls it off the UI thread
 * for large drops. (Labeled [command]-safe: never blocks on @MainActor.) */
LTResult lt_drop_ipsws(const char* pathsJson, const char** out);

/* [command] Route a drop (target: "display:<uuid>"|"sidebar"|"apps:<uuid>"|
 * "window"). Result: kLTEventDropAction → { "action": "import-ipsw"|
 * "install-ipa"|"import-media"|"unsupported", "detail": {…} }. */
LTResult lt_drop_route(const char* pathsJson, const char* target, uint64_t* requestIdOut);

/* ---- 7.11 Settings & misc ---- */

/* [query] Read a Swift-owned string pref (keys: kLTSettingsKey*). */
LTResult lt_settings_get_string(const char* key, const char** out);

/* [command] Write a Swift-owned string pref. */
LTResult lt_settings_set_string(const char* key, const char* value);

/* [query] HelpTopic.topics → [{ "title":…, "body":… }].
 * [Device] substitution is Qt-side per selected device. */
LTResult lt_help_topics(const char** out);

/* [command] DiagnosticsExport + BugReportInfo → writes a zip, returns path.
 * Completion via kLTEventToast. */
LTResult lt_diagnostics_export(const char** out);

/* [pure] §2.4 diagnostics counter: events dropped while no callback was
 * registered. */
uint64_t lt_debug_dropped_events(void);

/* Human-readable detail for the most recent failing call on the calling
 * thread (thread-local). Caller frees with lt_free_string. Only meaningful
 * immediately after a call returns < 0 on the same thread. */
const char* lt_last_error(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LT_BRIDGE_H */
