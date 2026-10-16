# libLTAppCore C ABI surface design

Date: 2026-10-16 · Phase 2 design · Status: design only — nothing here is implemented.

**Normative basis.** This document designs the C boundary between the portable Swift
core (`libLTAppCore`, the 7 `port/Packages` libraries) and the Qt 6 (C++/QML) UI,
exactly as specified in `docs/UI_APPROACH.md` §4:

- the boundary is C, not C++/Swift (`@_cdecl` exports, C types only);
- calls are synchronous and non-blocking (commands enqueue onto `@MainActor` and
  return; queries read lock-protected snapshots);
- events flow one way, on one thread (Swift → C callback → queued re-post onto the
  Qt UI thread);
- C++ adapters mirror the Swift view-models — they hold no business logic;
- the display path bypasses the bridge entirely (Qt maps the POSIX shm / named file
  mapping directly; Swift's rendezvous only hands it the fd/handle);
- Swift never touches Qt objects; Qt never touches Swift objects.

Where `UI_APPROACH.md` is silent and a choice was required, this document chooses
the simplest C-ABI-idiomatic option and marks it **(decision)**.

**Scope.** This document covers the *shape* of the boundary: naming, types, the event
union, every exported function grouped by domain (with the Swift API it fronts and
the Qt adapter that consumes it), the threading contract, versioning, and the gaps —
Swift APIs that do not exist yet and must be added before the UI can be built. It
contains no Swift, C++, or QML code. It does not design those missing Swift APIs,
only names them.

Terms used below:

- **Command** — an `@_cdecl` call that enqueues work on Swift's `@MainActor` and
  returns immediately (result `lt_accepted`). The outcome, if any, arrives later as
  an `LTEvent`. Never blocks the caller.
- **Query** — an `@_cdecl` call that returns a snapshot synchronously. Pure,
  non-`@MainActor` Swift (value types, `nonisolated` functions) computes it inline;
  `@MainActor` state is snapshotted by Swift under a lock on its own thread and the
  copy is handed back. A query never waits for `@MainActor` and never touches the
  network, disk (beyond a stat), or the helper.
- **Qt adapter** — the thin C++ class in `lt_bridge` exposing `Q_PROPERTY`s to QML,
  mirroring one Swift view-model (naming convention: `LT` + Swift model name, e.g.
  `LTTweaksPanelAdapter` for `TweaksPanelModel`).

---

## 1. Naming and typing conventions

### 1.1 Names

- Every exported symbol starts with `lt_`. Types start with `LT` (`LTEvent`,
  `LTResult`, `LTEventKind`). Enum constants start with `kLT` (`kLTEventDeviceListChanged`).
- Export names are verbs grouped by domain: `lt_device_boot`, `lt_job_cancel`,
  `lt_files_list`, `lt_capture_screenshot`, `lt_settings_get_string`,
  `lt_input_touch`. On Windows the names go through a `.def`-style export list so
  MSVC and Swift agree (per `UI_APPROACH.md` §4.2 rule 1); no name mangling, no
  decoration — plain C linkage on both platforms.
- One header, `lt_bridge.h`, is the contract. It is versioned as a whole (see §5).

### 1.2 Types

Only C types cross the boundary:

| Swift side | C side | Notes |
|---|---|---|
| `Bool` | `int32_t` (`0`/`1`) | Never C99 `_Bool` in structs (ABI alignment differs by compiler); plain `int32_t` fields. |
| `Int`/`Int32` | `int32_t` | |
| `Int64`/`UInt64`/`TimeInterval`-as-epoch | `int64_t`/`uint64_t`/`double` | Timestamps cross as `double` seconds since the Unix epoch (UTC); Qt converts to `QDateTime`. |
| `Double`/`Float` | `double` | No `float` in the ABI. |
| `String` | `const char*` (UTF-8, NUL-terminated) | Ownership per §1.3. |
| `UUID` | `const char*` canonical lowercase string (`8-4-4-4-12`) | Human-debuggable, matches `DeviceInstance.id`'s `uuidString`. |
| `CGFloat`/`CGSize`/`CGPoint`/`CGVector`/`CGRect` | `double` / pairs of `double` | Never `CGFloat` in the ABI (its width differs per platform). Geometry crosses as plain `double` fields, points, y-down where the Swift model says so. |
| Device/session reference | `lt_device_t` = `uint64_t`, `0` = invalid | **(decision)** Opaque cookie minted by Swift's app controller at startup for each library device; stable for the process lifetime. Hot paths (input, display) take the handle instead of re-parsing UUID strings. `lt_device_find(const char* uuid, lt_device_t* out)` and `lt_device_uuid(lt_device_t, const char** out)` translate both ways. |
| Enums | `int32_t` with `kLT…` constants | Each enum documents its full value list in `lt_bridge.h`; unknown values from a newer Swift core must be tolerated by Qt (see §5). |
| Complex state | `const char*` JSON (UTF-8) | **(decision)** Read snapshots of structured state (device list rows, tweaks, job tables, carrier state, capture availability) cross as a single JSON document rather than dozens of bespoke structs. Rationale: the models are already `Codable`/`Equatable` value types; JSON keeps `lt_bridge.h` small, keeps field-addition backward compatible (§5), and matches the codebase's existing length-framed-JSON habit (`DeviceLink`). The schema of each JSON document is specified per function in §3. Payloads are small (KBs); no binary plist, no CBOR — debuggability wins. |

No Swift classes, no `QString`, no `std::string`, no exceptions, no callbacks with
Swift calling conventions, no function pointers except the single event callback
(§2.4). Structs are plain-old-data, packed by the platform C ABI; `lt_bridge.h`
uses `<stdint.h>` fixed-width types exclusively so MSVC and Swift agree.

### 1.3 String ownership

**Rule: the callee allocates, the caller frees — always with `lt_free_string`.**

- Any `const char*` returned by an `lt_` function (query results, JSON snapshots,
  error strings, UUID strings) is heap-allocated by Swift with `malloc` semantics
  and owned by the caller. The caller frees it with `void lt_free_string(const char* s);`
  (`NULL`-safe). It must never be freed with `free()` directly from C++ — the
  allocator belongs to the Swift runtime's C interop layer; `lt_free_string` is the
  one legal deallocator. (Same-process, but the rule keeps the contract portable to
  any future out-of-process shape.)
- Any `const char*` *passed into* an `lt_` function is borrowed: Swift copies what it
  needs before returning. The caller retains ownership and may free or reuse the
  buffer immediately after the call returns.
- Event payload strings (§2) are allocated by Swift and owned by the C callback
  invocation: the bridge's trampoline must copy or consume them synchronously and
  then release them with `void lt_free_event_strings(const LTEvent* e);`, which
  deep-frees exactly the string fields of the event's active union member. After
  the callback returns, Swift retains no references to them.
- Strings are always UTF-8, NUL-terminated. Embedded NULs never occur (Swift
  `String` → UTF-8 CString conversion replaces or rejects them at the boundary;
  a string that cannot be represented fails the call with `kLTBadArgument`).

### 1.4 Error reporting

**Convention (decision):** every command and fallible query returns `LTResult`:

```c
typedef int32_t LTResult;
#define kLTOk            0   /* accepted / succeeded */
#define kLTAccepted      0   /* alias: command enqueued; outcome arrives as LTEvent */
#define kLTBadArgument  -1  /* null pointer, malformed UTF-8/JSON/UUID, out-of-range */
#define kLTNoSuchDevice -2  /* unknown lt_device_t or UUID */
#define kLTNotReady     -3  /* core not initialized, or session not in a usable state */
#define kLTBusy         -4  /* an exclusive operation is already running */
#define kLTFailed       -5  /* anything else; detail in lt_last_error() */
```

- `kLTOk`/`kLTAccepted` mean *the call itself* succeeded: for a command, the work
  was enqueued on `@MainActor` (it may still fail later — that failure arrives as
  an event, e.g. `kLTEventJobFailed`, `kLTEventToast`); for a query, the snapshot
  was produced.
- Human-readable detail: `const char* lt_last_error(void);` returns a
  **thread-local**, Swift-allocated message for the most recent failing call on
  *the calling thread*; the caller frees it with `lt_free_string`. It is only
  meaningful immediately after a call returns `< 0` on the same thread. Qt
  adapters surface it in QML (status bars, dialogs) or log it.
- No exceptions cross, no `errno` contract, no out-params for errors: the return
  code *is* the error channel; `lt_last_error` is the explanation channel.
- Queries that cannot fail (pure functions like `lt_zoom_step`) return their value
  directly instead of `LTResult`.

### 1.5 Call classes

Every export is labeled in §3 as **command** (enqueue + return), **query**
(synchronous snapshot), or **pure** (synchronous, `nonisolated` Swift, no locks,
no `@MainActor` — safe to call from any thread, including the Qt render thread).
The label is normative: a command must never block, a query must never wait on
`@MainActor`.

---

## 2. The `LTEvent` tagged union (Swift → C++)

### 2.1 Delivery

```c
typedef void (*lt_event_fn)(const LTEvent* event, void* ctx);
LTResult lt_set_event_callback(lt_event_fn fn, void* ctx);   /* command */
```

One callback per process, registered once at startup (re-registering replaces the
previous). Swift invokes it from *whatever thread produced the event* — the helper
link thread, a job's task, `@MainActor`. The pointed-to `LTEvent` is valid only for
the duration of the call. The bridge trampoline copies the event into a heap
`LTEvent` copy, frees the strings via `lt_free_event_strings`, and re-posts with
`QMetaObject::invokeMethod(..., Qt::QueuedConnection)` onto the Qt UI thread (§4).
All string fields in the copy are deep-copied by the trampoline before the callback
returns.

### 2.2 The union

```c
typedef int32_t LTEventKind;   /* kLTEvent… constants below */

typedef struct LTEvent {
    LTEventKind kind;
    lt_device_t device;        /* 0 when the event is not device-scoped */
    uint64_t    seq;           /* per-kind monotonic sequence; 0 when unused */
    union {
        /* every payload below */
    } u;
} LTEvent;
```

`device` scopes the event; `seq` orders events within a kind (frame sequence
numbers, log offsets, job generations) so the Qt side can drop stale deliveries.
The union is only as large as its largest member; payload structs are flat C.

### 2.3 Event kinds and payloads

Device library (surface 1):

- `kLTEventDeviceListChanged` — the library's device set changed (add, delete,
  rename, duplicate). Payload: none; Qt re-queries `lt_device_list()`. (Coalesced:
  Swift debounces bursts to at most one per runloop turn.)
- `kLTEventDeviceRowChanged` — one row's display state changed (status text, busy
  badge, firmware note). Payload: `{ lt_device_t device; }`; Qt re-queries
  `lt_device_row(device)`.
- `kLTEventSidebarChanged` — the sidebar firmware list changed. Payload: none;
  Qt re-queries `lt_sidebar_list()`.

Device lifecycle / status (surfaces 1, 4, 11):

- `kLTEventDeviceStatusChanged` — machine/session state moved. Payload:
  `{ int32_t machine; /* kLTVMState… */ int32_t running; int32_t sleeping; }`.
  Qt re-queries `lt_device_status(device)` for the full `DeviceMenuState`/`CaptureAvailability`
  snapshot. Fires on boot progress, pause/resume, lock/wake, shutdown, erase, death.
- `kLTEventBootStage` — boot-phase detail for the progress UI. Payload:
  `{ int32_t stage; /* kLTBootStage… */ const char* detail; double fraction; }`
  (`fraction < 0` = indeterminate).
- `kLTEventNoticeChanged` — `DeviceNotices.message` changed. Payload:
  `{ const char* message; /* NULL = cleared */ const char* operation; int32_t offersErase; }`.
  Qt shows/dismisses the notice banner; actions via `lt_notice_resolve`/`lt_notice_dismiss`.
- `kLTEventStorageBusyChanged` — `DeviceStorageWork.busy` changed. Payload: none;
  Qt re-queries `lt_storage_busy()`.

Display (surface 2; note the frames themselves bypass this union — §4.2 rule 5):

- `kLTEventDisplayLinkReady` — rendezvous completed; Qt may map the frame ring.
  Payload: `{ int32_t fd; /* POSIX fd, same process */ int32_t width; int32_t height; int32_t stride; int32_t format; /* kLTPixelFormat… */ uint64_t ringGeneration; }`.
  On Windows `fd` is instead the opaque handle value of the named file mapping,
  documented in `lt_bridge.h` per platform. Qt's `DisplayItem` maps it and starts
  presenting. `ringGeneration` invalidates any previous mapping.
- `kLTEventDisplayLinkClosed` — the ring is gone (halt, helper death). Payload:
  none; Qt unmaps and shows the placeholder.
- `kLTEventFrameAvailable` — a new frame was published. Payload:
  `{ uint64_t frameSeq; int32_t width; int32_t height; }`. The `seq` field of
  `LTEvent` carries `frameSeq` redundantly for ordering. Qt reads the newest frame
  from the mapped ring on its render thread — **no Swift call in the hot loop**.
- `kLTEventRotationChanged` — guest rotation changed. Payload:
  `{ int32_t degrees; int32_t width; int32_t height; }` (upright guest pixels).
- `kLTEventZoomChanged` — the effective points-per-pixel changed (mode switch,
  pane resize recompute, free-form resize applied). Payload: `{ double points; const char* mode; }`.
  Qt resizes the display item; the frame path is unaffected.

Input-adjacent (surface 2):

- `kLTEventFingerDotsChanged` — the window's finger-dots toggle state changed
  elsewhere. Payload: `{ int32_t shown; }`. (The `TweaksPanelModel.fingerDots`
  get/set closures are wired by the app controller to this event + `lt_display_set_finger_dots`.)

Panels — tweaks & carrier (surface 4):

- `kLTEventTweaksChanged` — `TweaksPanelModel.tweaks`/`status`/`message` changed.
  Payload: none; Qt re-queries `lt_tweaks_get(device)`.
- `kLTEventHiddenAppsListed` — async `listHiddenApps()` finished. Payload:
  `{ const char* appsJson; /* NULL on failure; message in lt_tweaks_get */ }`.
- `kLTEventCarrierChanged` — `CarrierPanelModel` polled state changed (modem
  status, sent SMS list, message). Payload: none; Qt re-queries
  `lt_carrier_get(device)`. Throttled by Swift to at most 2/sec.

Apps (surface 5):

- `kLTEventAppsChanged` — the app list or its banners changed
  (`AppsInspectorRows` inputs: paused, stale, installing, transfers). Payload:
  none; Qt re-queries `lt_apps_list(device)`.
- `kLTEventInstallProgress` — one install's progress. Payload:
  `{ const char* bundleId; double fraction; const char* phase; }`.
- `kLTEventInstallDone` — an install finished or failed. Payload:
  `{ const char* bundleId; int32_t ok; const char* message; }` (message NULL on success).

Files (surface 6):

- `kLTEventFilesListed` — async directory read finished. Payload:
  `{ uint64_t requestId; const char* entriesJson; /* NULL on failure */ }`.
- `kLTEventFilesProgress` — a transfer's progress. Payload:
  `{ uint64_t transferId; double fraction; const char* currentFile; int64_t bytesDone; int64_t bytesTotal; }`.
- `kLTEventFilesDone` — a transfer finished, failed, or was cancelled. Payload:
  `{ uint64_t transferId; int32_t ok; int32_t cancelled; const char* message; }`.
- `kLTEventFilesConnectionChanged` — `FilesConnection.state` moved
  (disconnected/connected/transferring). Payload: `{ int32_t state; /* kLTFilesState… */ }`.

Log & console (surface 7):

- `kLTEventLogLine` — one appended line batch from a followed log. Payload:
  `{ const char* source; /* "app" | "helper:<uuid>" */ const char* text; uint64_t offset; }`.
  `offset` is the byte offset after this batch; Qt passes it back to resume after
  restart. Batches are whole lines, capped per post.
- `kLTEventLogRotated` — the tailed file was replaced. Payload:
  `{ const char* source; }`; Qt resets its offset to 0.

Capture (surface 8):

- `kLTEventCaptureAvailabilityChanged` — `CaptureAvailability` changed. Payload:
  none; Qt re-queries `lt_capture_availability(device)`.
- `kLTEventCaptureDone` — screenshot or recording finished. Payload:
  `{ int32_t kind; /* kLTCaptureScreenshot | kLTCaptureRecording */ const char* path; int32_t ok; const char* message; }`.
- `kLTEventRecordingTick` — once per second while recording. Payload:
  `{ uint64_t elapsedSeconds; }`. (Qt could run its own timer, but the event keeps
  one source of truth with `ScreenRecordingSession.elapsed`.)
- `kLTEventRecordingPhaseChanged` — starting/recording/saving/idle transitions.
  Payload: `{ int32_t phase; /* kLTRecordingPhase… */ }`.

Firmware catalog & jobs (surface 9):

- `kLTEventCatalogChanged` — the catalog or sidebar-relevant state changed.
  Payload: none; Qt re-queries `lt_catalog_list()`.
- `kLTEventJobProgress` — a firmware job's `Preparation`/`FirmwareJob.progress`
  moved. Payload: `{ const char* entryId; double fraction; /* -1 = indeterminate */ const char* detail; double etaSeconds; /* -1 = unknown */ int32_t step; int32_t steps; }`.
- `kLTEventJobDone` — a job finished, failed, or was cancelled. Payload:
  `{ const char* entryId; int32_t ok; int32_t cancelled; const char* message; }`.
- `kLTEventTaskbarProgress` — aggregate progress for the taskbar/dock badge
  (`DeviceRow.dockProgress`). Payload: `{ double fraction; /* <0 = clear badge */ }`.

Cross-cutting (surfaces 10, 12, 13):

- `kLTEventToast` — a user-facing notification (`LTMNotifier`,
  `CaptureNotifications`, install completions). Payload:
  `{ const char* title; const char* body; const char* category; }`. Qt maps this
  to `QSystemTrayIcon::showMessage` (and the Windows/macOS native equivalents via Qt).
- `kLTEventDropAction` — result of routing dropped files (surface 12). Payload:
  `{ uint64_t requestId; const char* action; /* kLTDropAction… as string */ const char* detailJson; }`.
- `kLTEventNetworkAccessChanged` — local-network permission state changed.
  Payload: `{ int32_t state; /* kLTNetworkAccess… */ }`.
- `kLTEventQuitReady` — `QuitCoordinator` finished graceful shutdown; the app may
  now exit. Payload: none.
- `kLTEventShutdown` — the core is going down (after `lt_shutdown`); the callback
  will not be invoked again. Payload: none.

Reserved: `kLTEventReservedStart = 0x1000` — first value available to future
extension without renumbering (§5).

### 2.4 Callback mechanics

- Registration is idempotent; passing `fn == NULL` unregisters.
- Swift **never blocks** waiting for the callback to return; it posts and continues.
  The callback must therefore be fast: copy, queue, return. It must not call back
  into `lt_` functions (re-entrancy is forbidden — see §4).
- Event order per producer is preserved; across producers there is no global order
  (Qt adapters must be written to tolerate `deviceStatusChanged` arriving before
  the `jobDone` that caused it — snapshots are re-queried, never assumed).
- If the callback is not registered, events are dropped (with a counter readable
  via `lt_debug_dropped_events()` for diagnostics) — Swift never buffers
  unboundedly.

---

## 3. Exported functions

Conventions for this section: each entry gives the C signature, the **class**
(command/query/pure), what it does, the Swift view-model or API it fronts, and
the Qt adapter that consumes it. JSON schemas are described inline; exact field
names are fixed in `lt_bridge.h`. `LTResult` returns are `kLTOk`/`kLTAccepted` on
success per §1.4.

### 3.1 Lifecycle

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_init(const char* configJson);` | command (runs once) | Initializes the app controller: storage layout (`StorageLocations.prepare`), library load, catalog load, firmware-jobs wiring, event pump start. `configJson`: `{ "appSupportDir": …, "logDir": …, "resourceDir": … }` — paths the Qt side owns on Linux/Windows (no `Bundle.main`). Returns `kLTOk` when the core is usable; `kLTFailed` (with `lt_last_error`) when storage can't be prepared. Must be called once, on the Qt UI thread, before any other `lt_` call. |
| `LTResult lt_shutdown(void);` | command | Graceful teardown: stops jobs, halts sessions (via `QuitCoordinator` semantics), closes helper links, then posts `kLTEventShutdown`. After it returns, only `lt_init` (fresh process state) is legal. |
| `LTResult lt_set_event_callback(lt_event_fn fn, void* ctx);` | command | Registers the §2 callback. |
| `int32_t lt_abi_version(void);` | pure | The ABI version this library was built against (§5). |
| `LTResult lt_version(const char** out);` | query | App version string (`AboutCredits`). Caller frees with `lt_free_string`. |
| `void lt_free_string(const char* s);` | pure | Deallocator for every returned string (§1.3). |
| `void lt_free_event_strings(const LTEvent* e);` | pure | Deep-frees an event's string payloads (§1.3). |
| `LTResult lt_quit_request(void);` | command | `QuitCoordinator`: begins graceful shutdown (halts devices, finishes/ cancels jobs per policy). Result arrives as `kLTEventQuitReady` or a `kLTEventToast` explaining the blocker. |
| `LTResult lt_about_credits(const char** out);` | query | JSON `{ "credits": […], "licenses": […] }` from `AboutCredits`. About panel. |

Swift front: a new app-controller layer (§6 gap 1) using `StorageLocations`,
`DeviceLibrary`, `FirmwareCatalog`, `FirmwareJobs`, `QuitCoordinator`,
`AboutCredits`. Qt consumers: application bootstrap, About dialog, quit handling.

### 3.2 Device library & rows (surface 1)

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_device_list(const char** out);` | query | JSON array of `DeviceRow` snapshots: `{ "id": uuid, "name": …, "board": …, "firmware": …, "status": kLTDeviceRowState…, "statusText": …, "busy": "deleting"\|"erasing"\|null, "firmwareNote": … }`. Fronts `DeviceLibrary.instances` + `DeviceRow` synthesis + `DeviceStorageWork.busy`. Adapter: `LTDeviceListAdapter` (sidebar model). |
| `LTResult lt_device_find(const char* uuid, lt_device_t* out);` | query | UUID → handle (§1.2). |
| `LTResult lt_device_uuid(lt_device_t device, const char** out);` | query | Handle → UUID string. |
| `LTResult lt_device_row(lt_device_t device, const char** out);` | query | One row's JSON (same schema as list entries). |
| `LTResult lt_device_create(const char* entryId, const char* name, lt_device_t* out);` | command | Creates a library device from a catalog entry (Add-device flow). Emits `kLTEventDeviceListChanged`; the new handle is returned synchronously in `out` (the row exists immediately, creation of storage is enqueued). Fronts `DeviceLibrary.save` + `PreparationJob`-adjacent setup. Adapter: Add-device sheet. |
| `LTResult lt_device_delete(lt_device_t device);` | command | Guarded delete (the ⌘⌫ guard lives in Qt; Swift re-checks `DeviceInUse`/running state). Emits `kLTEventDeviceListChanged` / failure toast. |
| `LTResult lt_device_rename(lt_device_t device, const char* name);` | command | Rename; emits `kLTEventDeviceListChanged`. |
| `LTResult lt_device_duplicate(lt_device_t device, lt_device_t* out);` | command | Duplicate device. |
| `LTResult lt_device_erase(lt_device_t device);` | command | Erase storage (guest reset). Guarded by `DeviceStorageWork`; emits `kLTEventStorageBusyChanged` + `kLTEventDeviceStatusChanged`. |
| `LTResult lt_storage_busy(const char** out);` | query | JSON `{ "<uuid>": "deleting"\|"erasing" }` from `DeviceStorageWork.busy`. Adapter: `LTDeviceListAdapter` busy badges. |
| `LTResult lt_sidebar_list(const char** out);` | query | JSON `{ "ids": […], "names": {…}, "labels": [{ "id":…, "title":…, "subtitle":… }] }` from `SidebarList` + `FirmwareCatalog`. Adapter: `LTSidebarAdapter`. |
| `LTResult lt_sidebar_add(const char* idsJson);` | command | `SidebarList.add` + save; emits `kLTEventSidebarChanged`. |
| `LTResult lt_sidebar_remove(const char* entryId);` | command | `SidebarList.remove` + save. |
| `LTResult lt_sidebar_rename(const char* entryId, const char* name);` | command | `SidebarList.rename` + save. |

Fronts: `DeviceLibrary`, `DeviceInstance`, `DeviceRow`, `SidebarList`,
`DeviceStorageWork`, `FirmwareCatalog`. Qt consumers: `LTDeviceListAdapter`,
`LTSidebarAdapter`, main window, context menus.

### 3.3 Session lifecycle & status (surfaces 1, 4, 11)

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_device_boot(lt_device_t device, const char* optionsJson);` | command | `BootCycle.begin()` with `BootSettings` (`panel`, `network`, `debugPort`, `bootArgs` from `optionsJson`). Progress via `kLTEventBootStage`; readiness via `kLTEventDeviceStatusChanged`. |
| `LTResult lt_device_halt(lt_device_t device);` | command | `ShutdownLadder` graceful halt. |
| `LTResult lt_device_pause(lt_device_t device, int32_t paused);` | command | Pause/resume machine. |
| `LTResult lt_device_reset(lt_device_t device);` | command | `BootCycle.reset()` (in-place restart; resumes a paused guest first). |
| `LTResult lt_device_lock(lt_device_t device, int32_t locked);` | command | Lock/Wake/Start (the Device-menu Lock item's three titles). |
| `LTResult lt_device_status(lt_device_t device, const char** out);` | query | JSON snapshot merging `DeviceMenuState` fields and `CaptureAvailability`: `{ "machine": kLTVMState…, "running":…, "sleeping":…, "paused":…, "shuttingDown":…, "storageFailed":…, "installing":…, "acceptsInput":…, "hasCompass":…, "hasCellular":…, "compassHeading":…, "editingText":… }`. Adapter: `LTDeviceAdapter` (drives menu validation + inspector enablement). |
| `LTResult lt_menu_validate(lt_device_t device, int32_t item, int32_t arg, const char** out);` | query | `DeviceMenuState.validate(_:)` for one `Item` (`kLTMenuItemLock/Rotate/CompassHeading/Carrier/Input/Pause/Restart`; `arg` = heading for compass). Returns `{ "enabled":…, "title":…|null, "checked":…|null }`. Called by Qt during menu validation; pure-ish (reads the status snapshot). Adapter: main-menu validation. |
| `LTResult lt_capture_availability(lt_device_t device, const char** out);` | query | `CaptureAvailability` as JSON `{ "canTakeScreenshot":…, "canStartRecording":…, "canToggleRecording":…, "screenshotBusy":…, "recordingSaving":…, "recordingCanStop":… }`. Adapter: `LTCaptureAdapter` (Capture menu + toolbar). |
| `LTResult lt_notice_get(lt_device_t device, const char** out);` | query | `DeviceNotices.message` snapshot `{ "message":…|null, "operation":…|null, "offersErase":… }`. |
| `LTResult lt_notice_resolve(lt_device_t device, const char* operation);` | command | `DeviceNotices.resolve(_:)`. |
| `LTResult lt_notice_dismiss(lt_device_t device);` | command | `DeviceNotices.dismiss()`. |
| `LTResult lt_network_status(const char** out);` | query | `LocalNetworkAccess` state JSON `{ "state": kLTNetworkAccess…, "explanation": … }`. |
| `LTResult lt_network_set_allowed(int32_t allowed);` | command | Records the NetworkAccessPrompt answer; emits `kLTEventNetworkAccessChanged`. |

Swift front: new per-device session object (§6 gap 2) composing `BootCycle`,
`ShutdownLadder`, `MachineState`, `DeviceNotices`, `DeviceMenuState`,
`CaptureAvailability`, `LocalNetworkAccess`. Qt consumers: `LTDeviceAdapter`,
menu validation, notice banner, network prompt dialog.

### 3.4 Display & input (surface 2)

The frame path bypasses the bridge (§4.2 rule 5, §2.3 `kLTEventDisplayLinkReady` /
`kLTEventFrameAvailable`). Everything below is the *control* plane.

Input commands — all **command** class, all take `lt_device_t`:

| Function | Swift front |
|---|---|
| `LTResult lt_input_touch(lt_device_t d, int32_t phase, double x, double y, int32_t touchId);` | Normalized touch (0–1, y-down) → `MouseTouchPair` → `LinkCommand.touch` into the helper wire. `phase`: `kLTTouchBegan/Moved/Ended/Cancelled`. Multi-touch: one call per touch id. |
| `LTResult lt_input_key(lt_device_t d, uint16_t code, int32_t down, int32_t modifiers);` | `KeyboardInput.sendKey(macKeyCode:down:)` → helper key command. `code` is the USB HID usage the guest expects; Qt translates its native key codes (§6 gap 8). `modifiers`: `KeyModifiers` bitmask. `GuestKeyboard.passesThrough` logic stays in Swift where applicable; see `lt_input_text`. |
| `LTResult lt_input_text(lt_device_t d, const char* text, int32_t shiftHeld);` | `DeviceInput.typeText` / `.paste` — text that doesn't map to key codes (IME, dead keys). |
| `LTResult lt_input_button(lt_device_t d, int32_t button);` | `DeviceInput.tap(_:)` — Home/Menu, volume, etc. (`DeviceInput.Button` raw values as `kLTButton…`). |
| `LTResult lt_input_shake(lt_device_t d);` | `DeviceInput.shake()`. |
| `LTResult lt_input_tilt(lt_device_t d, double angle, double pitch);` | `DeviceInput.setTilt(angle:pitch:)` → accelerometer. Driven by Qt's tilt control (which mirrors `ChassisTilt` math in QML — pure, duplicated only as view math; the *command* goes through Swift). |
| `LTResult lt_input_motion_pose(lt_device_t d, int32_t pose);` | `DeviceInput.setMotionPose` (`kLTMotionPoseUpright/Flat`). |
| `LTResult lt_input_rotate(lt_device_t d, int32_t degrees);` | `DeviceRotation` — guest rotation request. Resulting geometry arrives as `kLTEventRotationChanged`. |
| `LTResult lt_input_compass(lt_device_t d, int32_t heading);` | Compass heading override (Device-menu compass items). |
| `LTResult lt_input_keyboard_pointer_key(lt_device_t d, uint16_t code, int32_t down, int32_t modifiers);` | `KeyboardPointer.key(…)` — the keyboard-as-mouse drive mode; returns nothing, but the resulting `Touch` is forwarded internally to the touch path (Swift-side, no event needed). |

Display control — queries are **pure** where the model is `nonisolated`:

| Function | Class | Swift front |
|---|---|---|
| `LTResult lt_display_set_zoom(lt_device_t d, const char* mode);` | command | Sets `ZoomMode` (`"fit"`/`"physical"`/`"pixel"`/`"p:1.5"` — the `defaultsValue` form) and persists per board (`ZoomMode.save(for:)`). Emits `kLTEventZoomChanged`. |
| `LTResult lt_display_get_zoom(lt_device_t d, const char** out);` | query | `{ "mode": "p:1.5", "points": 2.0, "shown": "fit" }` — mode, effective p, and `ZoomContext.shown(_:)` (Physical→Fit fallback naming). |
| `LTResult lt_zoom_stops_json(double fit, double physical /* -1 = none */, double backing, const char** out);` | pure | `ZoomContext.stops` as JSON `[{ "mode":…, "points":… }]` for the ⌘+/⌘− ladder and the View menu checkmarks. Qt computes `fit`/`backing`/physical from its pane (physical via Qt screen info; `DisplayMeasurements` has no Linux/Windows backend — §6 gap). |
| `double lt_zoom_step(double fit, double physical, double backing, double p, int32_t direction);` | pure | `ZoomContext.step(from:direction:)` → the mode string is resolved Qt-side from the stops list; returns `-1` past either end (`canStep`). |
| `int32_t lt_zoom_draws_nearest(double p, double backing);` | pure | `ZoomContext.drawsNearest` — Qt picks nearest vs. linear texture filtering. |
| `LTResult lt_resize_drag(const char* board, double startW, double startH, double edgeX, double edgeY, double deltaX, double deltaY, double pointsPerPixel, int32_t symmetric, int32_t quarterTurned, const char** out);` | pure | `FreeFormResize(board:).drag(…)` → `{ "width":…, "height":…, "shiftX":…, "shiftY":…, "limit":…|null }` in upright guest pixels. Qt applies the size via `lt_display_set_panel_size`. |
| `LTResult lt_display_set_panel_size(lt_device_t d, int32_t width, int32_t height);` | command | Applies the free-form size to the running session (rebinds `CarrierPanelModel`/session on geometry change per `rebind` semantics). Emits `kLTEventRotationChanged` (geometry) + `kLTEventZoomChanged`. |
| `LTResult lt_display_set_finger_dots(lt_device_t d, int32_t shown);` | command | The window's finger-dots toggle (the `TweaksPanelModel.fingerDots` setter target). Qt owns the actual overlay rendering. |
| `LTResult lt_display_get_finger_dots(lt_device_t d, int32_t* out);` | query | Current toggle state. |

Qt consumers: `LTDisplayAdapter` (input forwarding, zoom/rotate/resize commands),
the `DisplayItem` QQuickItem (frame presentation, gesture → input calls), View
menu. Note `KeyboardPointer`/`ScrollDrag`/`ChassisTilt` gesture *recognition* math
is view-layer work in QML/C++; only the resulting guest commands cross.

### 3.5 Inspector panels (surface 4)

**Tweaks** — adapter `LTTweaksAdapter` fronts `TweaksPanelModel`:

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_tweaks_get(lt_device_t d, const char** out);` | query | JSON `{ "running":…, "tweaks": [{ "id":…, "caption":…, "on":…, "available":… }], "color": …, "clock": epoch|null, "status": …|null, "message": …|null, "hiddenApps": […]|null }`. `caption(_:)` is computed Swift-side (needs `tweaks.version`). |
| `LTResult lt_tweaks_set(lt_device_t d, const char* tweakId, int32_t on);` | command | `set(_:on:)` (persists via `DeviceTweaks.change`). Emits `kLTEventTweaksChanged`. |
| `LTResult lt_tweaks_set_color(lt_device_t d, int32_t color);` | command | `set(coreAnimationColor:)` (`CoreAnimationColor` raw value). |
| `LTResult lt_tweaks_set_clock(lt_device_t d, double epoch);` | command | `set(clock:)`. |
| `LTResult lt_tweaks_list_hidden_apps(lt_device_t d);` | command | Async `listHiddenApps()`; result in `kLTEventHiddenAppsListed`. |
| `LTResult lt_tweaks_open_app(lt_device_t d, const char* bundleId);` | command | `open(_:)`; failures surface via `kLTEventTweaksChanged` (message). |

**Carrier** — adapter `LTCarrierAdapter` fronts `CarrierPanelModel`. The panel's
poll loop is Swift-side: `lt_carrier_set_visible(d, visible)` starts/stops the
`poll()` timer; changes emit `kLTEventCarrierChanged` (throttled):

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_carrier_set_visible(lt_device_t d, int32_t visible);` | command | Starts/stops polling + (re)binds the session backend (`rebind(to:)` on session change). |
| `LTResult lt_carrier_get(lt_device_t d, const char** out);` | query | JSON `{ "carrierName":…, "mcc":…, "mnc":…, "settings": {…}, "status": {…}|null, "sent": [{ "id":…, "number":…, "text":… }], "message":…|null, "networkValid":…, "networkEdited":…, "applyingNetwork":…, "callNumber":…, "smsNumber":…, "smsText":…, "callState":…, "canRing":…, "canAnswer":…, "canHangUp":…, "latitude":…, "longitude":…, "walking":…, "reportedLocation": {…} }`. All derived properties computed Swift-side — the adapter binds, it doesn't recompute. |
| `LTResult lt_carrier_set_field(lt_device_t d, const char* field, const char* value);` | command | Typed-but-unapplied edits: `carrierName`, `mcc`, `mnc`, `callNumber`, `smsNumber`, `smsText`, `latitude`, `longitude`. (One setter keeps the ABI small; validation stays in the model.) |
| `LTResult lt_carrier_apply_network(lt_device_t d);` | command | `applyNetwork()`; validation messages via `kLTEventCarrierChanged`. |
| `LTResult lt_carrier_set_registered/sim/bars` | command | `set(registered:)` / `set(simPresent:)` / `set(bars:)`. (Three tiny commands; alternatively fold into `lt_carrier_set_field` — kept separate to mirror the model's method names.) |
| `LTResult lt_carrier_ring/answer/hangup(lt_device_t d);` | command | `ring()` / `answer()` / `hangUp()`. |
| `LTResult lt_carrier_send_sms(lt_device_t d);` | command | `sendSMS()` (number/text already set via `lt_carrier_set_field`). |
| `LTResult lt_carrier_apply_location(lt_device_t d);` | command | `applyLocation()`. |
| `LTResult lt_carrier_set_location(lt_device_t d, double lat, double lon);` | command | `set(location:)` — presets, map clicks. |
| `LTResult lt_carrier_toggle_walk(lt_device_t d);` | command | `toggleWalk()`. |
| `LTResult lt_carrier_notice(lt_device_t d, const char* text);` | command | `notice(_:)` — e.g. host location unavailable. |

**Storage / proxy / debug port** — adapters `LTStorageAdapter`, `LTProxyAdapter`,
`LTDebugPortAdapter`:

| Function | Class | Swift front |
|---|---|---|
| `LTResult lt_storage_get(lt_device_t d, const char** out);` | query | Storage settings snapshot (capacity plan, generations) from `StorageLocations`/`DeviceInstance.storage` + the StorageSettingsView-backing state. Exact schema fixed when the Swift API lands (§6 gap 6 — the Mac view's backers live in the app target). |
| `LTResult lt_storage_set(lt_device_t d, const char* json);` | command | Applies storage settings. |
| `LTResult lt_proxy_get/set(lt_device_t d, …);` | query/command | `WebProxyConfiguration` get/set JSON. |
| `LTResult lt_debug_port_get/set(lt_device_t d, …);` | query/command | `DebugPort` (HostRuntime) get/set. |

### 3.6 Apps inspector (surface 5)

Adapter `LTAppsAdapter` fronts `DeviceApps` + `AppInstallPipeline`/`InstallationQueue`:

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_apps_list(lt_device_t d, const char** out);` | query | JSON `{ "banner": { "text":…|null }, "device": { "canQueueInstall":…, "canReachDevice":…, "installing":…, "preparingDevice":…, "hasFileTransfer":…, "reconnecting":…, "transfersPaused":… }, "rows": [{ "identity":…, "title":…, "subtitle":…, "kind": "app"\|"install"\|"open"\|"progress"\|"failed"\|"resume", "progress":…|null, "enabled":… }] }` — the `AppsDevice` + `AppRowAppearance` synthesis done Swift-side (`AppsInspectorRows`). Virtualized Qt table binds to `rows`. |
| `LTResult lt_apps_refresh(lt_device_t d);` | command | Re-reads the device (`AppChangeWatch` trigger); emits `kLTEventAppsChanged`. |
| `LTResult lt_apps_install(lt_device_t d, const char* ipaPath);` | command | `DeviceApps.install` (queues via `AppInstaller`/`InstallationQueue`); progress via `kLTEventInstallProgress`, outcome via `kLTEventInstallDone`. |
| `LTResult lt_apps_remove(lt_device_t d, const char* bundleId);` | command | Uninstall. |
| `LTResult lt_apps_cancel_install(lt_device_t d, const char* bundleId);` | command | Dequeues a queued install. |
| `LTResult lt_apps_restart_springboard(lt_device_t d);` | command | `restartSpringBoard()`. |
| `LTResult lt_apps_import_media(lt_device_t d, const char* pathsJson);` | command | `DeviceApps.importMedia` (`PreparedMedia` classification Qt-side via `lt_drop_classify`, or Swift-side — the model owns `PreparedMedia.extensions`). |
| `LTResult lt_apps_store_search(lt_device_t d, const char* query, const char** out);` | query | Store browser search (the `AppsInspectorViewController+Store` logic's portable core, if any — likely a gap; see §6). |

### 3.7 Files browser (surface 6)

Adapter `LTFilesAdapter` fronts `DeviceServicesClient` file APIs + `FilesConnection`:

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_files_follow(lt_device_t d, int32_t follow);` | command | Drives `FilesConnection.shared.follow(device:endpoint:reachable:)` for the app's one Files window: `follow=1` binds to this device's boot endpoint, `0` releases. Emits `kLTEventFilesConnectionChanged`. (Called by Qt on selection/session change — the Swift side owns the transfer-pinning rules.) |
| `LTResult lt_files_state(const char** out);` | query | `{ "state": "disconnected"\|"connected"\|"transferring", "device": uuid|null }`. |
| `LTResult lt_files_list(lt_device_t d, const char* path, uint64_t* requestIdOut);` | command | Async `DeviceServicesClient.files(in:)`; result in `kLTEventFilesListed` with the `requestId`. Stale responses (Qt navigated away) are dropped by `requestId`. |
| `LTResult lt_files_transfer(lt_device_t d, const char* opJson, uint64_t* transferIdOut);` | command | Starts upload/download/move/mkdir/delete: `opJson` = `{ "op": "upload"\|"download"\|"delete"\|"move"\|"mkdir", "src":…, "dst":… }`. Sets `FilesConnection.setTransferring(true)` for the duration (Swift-side). Progress → `kLTEventFilesProgress`; end → `kLTEventFilesDone`. |
| `LTResult lt_files_cancel(uint64_t transferId);` | command | Cancels a transfer. |

### 3.8 Capture (surface 8)

Adapter `LTCaptureAdapter` fronts `ScreenRecordingSession`, `PanelCapture`,
`CapturePreferences`, `FFmpeg`. **Frame sourcing (decision):** Swift maps the same
shm ring the Qt `DisplayItem` maps (the fd is process-local; see
`kLTEventDisplayLinkReady`). Screenshots and recordings are therefore produced
Swift-side with zero bridge traffic in the hot loop, reusing `ScreenMovieWriter`/
`FFmpeg` unchanged. Qt never ships pixels across the boundary.

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_capture_screenshot(lt_device_t d);` | command | Captures the latest frame (rotated per `PanelCapture.quarterTurns`), writes via `CapturePreferences.captureDestination`, honors copy-to-clipboard/open-after prefs Swift-side where portable. Result: `kLTEventCaptureDone`. Clipboard/open-app are Qt-side effects driven by the event + prefs snapshot. |
| `LTResult lt_capture_record_start(lt_device_t d);` | command | `ScreenRecordingSession.start` with the shm-ring frame supplier. Phases → `kLTEventRecordingPhaseChanged`; ticks → `kLTEventRecordingTick`. |
| `LTResult lt_capture_record_stop(lt_device_t d, int32_t discard);` | command | `stop(discard:)`; save destination chosen via `chooseSaveDestination` → bridged as a Qt save dialog: Swift posts a `kLTEventToast`-like request? **(decision)** — the save-destination prompt is Qt-owned: on stop, Swift emits `kLTEventCaptureDone` with `path = ""` meaning "ask the user", Qt shows the save dialog, then calls `lt_capture_record_save(path)`. Simpler: two-step `lt_capture_record_stop` → event `kLTEventRecordingSaveNeeded` → Qt dialog → `lt_capture_record_save(d, path)` / `lt_capture_record_discard(d)`. The event is listed in §2.3's capture group as `kLTEventRecordingSaveNeeded` (payload: `{ const char* suggestedName; }`). |
| `LTResult lt_capture_prefs_get(const char** out);` | query | `CapturePreferences` as JSON (save location, copy-on-capture, record-bezel, open-after, sound). |
| `LTResult lt_capture_prefs_set(const char* json);` | command | Applies prefs. |

### 3.9 Log & console (surface 7)

Adapter `LTLogAdapter` fronts `LogTail` (+ `AppEventLog`/`LTMLogging` sources):

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_log_sources(const char** out);` | query | JSON `["app", "helper:<uuid>", …]` — available tails. |
| `LTResult lt_log_read(const char* source, uint64_t from, const char** out);` | query | `LogTail.read(url, from:)` → `{ "text":…, "rotated":…, "offset":… }` (64 KB cap, whole lines — the model's contract, unchanged). |
| `LTResult lt_log_follow(const char* source, int32_t follow);` | command | Subscribes/unsubscribes; appended lines → `kLTEventLogLine`; rotation → `kLTEventLogRotated`. |
| `LTResult lt_log_clear(const char* source);` | command | Truncates the log (the Clear button); emits `kLTEventLogRotated`. |
| `int32_t lt_log_filter(const char* text, const char* filter, const char** out);` | pure | `LogTail.filtered(_:by:)` — Qt *may* call this, but filtering a visible buffer is cheaper in QML; exported for parity/testing. |

`ConsoleSplitLayout` math (detents, drag resolve) is pure and trivial — it is
mirrored in QML, not bridged (view geometry is Qt's job; the *saved* height goes
through `lt_settings_*`, §3.11).

### 3.10 Firmware catalog & jobs (surface 9)

Adapters `LTCatalogAdapter`, `LTFirmwareJobsAdapter` front `FirmwareCatalog`,
`FirmwareJobs`, `FirmwareJobTable`, `PreparationJob`, `CatalogDetailsModel`,
`CatalogFilter`:

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_catalog_list(const char* filterJson, const char** out);` | query | `filterJson` = `{ "text":…, "board":…|null, "status":…|null }` (`CatalogFilter`); returns entries JSON `[{ "id":…, "board":…, "productType":…, "marketingName":…, "version":…, "build":…, "status":…, "statusNote":…|null, "prereleaseBadge":…|null, "media":[…], "released":…|null, "managesApps":…, "estimates": {…} }]`. |
| `LTResult lt_catalog_details(const char* entryId, const char** out);` | query | `CatalogDetailsModel` snapshot JSON (requirements, keys state, recipe steps, emulator notes). |
| `LTResult lt_jobs_list(const char** out);` | query | `FirmwareJobs.jobs` (shown) JSON `[{ "entryId":…, "kind": "download"\|"prepare"\|"thirdParty", "progress":…|null, "preparation": { "step":…, "steps":…, "name":…, "fraction":…, "detail":…|null, "remaining":…|null }|null, "failed":…|null }]`. |
| `LTResult lt_job_start(const char* entryId, const char* optionsJson);` | command | `downloadAndPrepare` with `optionsJson` = `{ "jailbreak":…, "skipSetup":…, "keepStock":… }` (`offersJailbreak`/`offersSkipSetup` computed Swift-side and included in `lt_catalog_details`). Emits `kLTEventJobProgress`/`kLTEventJobDone`. The JailbreakAlert content is Qt UI driven by the details snapshot. |
| `LTResult lt_job_import_ipsw(const char* pathsJson);` | command | `importIPSWs` (dropped IPSWs / folder walk results). |
| `LTResult lt_job_cancel(const char* entryId);` | command | `cancel(_:)`. |
| `LTResult lt_job_cancel_all(void);` | command | `cancelAll()`. |
| `LTResult lt_job_dismiss_failure(const char* entryId);` | command | `dismissFailure(_:)`. |
| `LTResult lt_jobs_can_download(int32_t* out);` | query | `canDownload` + `unavailableReason` → `{ "can":…, "reason":…|null }` (preparer tool presence). |
| `LTResult lt_drop_classify(const char* path, int32_t* out);` | pure | `DroppedFiles(URL)` → `kLTDropIPSW/IPA/Media/Unsupported` (surface 12, extension check only). |
| `LTResult lt_drop_ipsws(const char* pathsJson, const char** out);` | query | `DroppedFiles.ipsws(in:depth:)` — folder walk for IPSWs; JSON array of paths. Runs synchronously; Qt calls it off the UI thread for large drops. |
| `LTResult lt_drop_route(const char* pathsJson, const char* target, uint64_t* requestIdOut);` | command | Routes a drop (`target`: `"display:<uuid>"` | `"sidebar"` | `"apps:<uuid>"` | `"window"`): classifies, walks folders, and enqueues the right action (catalog import / install queue / media import). Result: `kLTEventDropAction` → `{ "action": "import-ipsw"\|"install-ipa"\|"import-media"\|"unsupported", "detail": {…} }`. |

### 3.11 Settings & misc (surfaces 10, 11)

Persistence split **(decision)**: device and firmware state (property lists under
the state dir: `DeviceSettingsFile`, `TweakSettings`, `SidebarList`,
`ZoomMode.save(for:)`) is owned by Swift and crossed via typed functions; pure
window-chrome state (console split height, window frames, column widths) lives in
`QSettings` on the C++ side and never crosses. `lt_settings_*` covers the narrow
band of Swift-owned scalar prefs the Qt UI must read/write (network prompt
answer, capture prefs are typed in §3.8, zoom is typed in §3.4):

| Function | Class | Semantics |
|---|---|---|
| `LTResult lt_settings_get_string(const char* key, const char** out);` | query | Reads a Swift-owned string pref. Keys are documented in `lt_bridge.h` (`"networkAccess.answer"`, …). |
| `LTResult lt_settings_set_string(const char* key, const char* value);` | command | Writes it (via the port's `UserDefaults` equivalent — §6 gap 5). |
| `LTResult lt_help_topics(const char** out);` | query | `HelpTopic.topics(Help.txt)` → `[{ "title":…, "body":… }]`; `[Device]` substitution is Qt-side per selected device (`naming(_:)` is pure — optionally also exported as `lt_help_naming`, but string replace is trivial in QML; **not exported**). Help window (surface 10). |
| `LTResult lt_diagnostics_export(const char** out);` | command | `DiagnosticsExport` + `BugReportInfo` → writes a zip, returns path; completion via `kLTEventToast` (or a dedicated event if the UI needs progress — currently synchronous enough to be a command with immediate path return; **(decision)** keep it a command). |
| `uint64_t lt_debug_dropped_events(void);` | pure | §2.4 diagnostics counter. |

Menu validation for the Device/Capture menus is `lt_menu_validate` (§3.3); the
rest of the menu bar (App/File/Edit/View/Window/Help) is static Qt with
per-action enablement bound to adapter properties — no bridge needed beyond the
commands above.

---

## 4. Threading contract

This section is normative. It is the invariant the Linux thread-sanitizer CI build
enforces (`UI_APPROACH.md` §4.2 rule 6).

### 4.1 The three threads (and the ones that don't exist)

1. **Qt UI thread** — the only thread that touches Qt objects, QML, adapters, and
   the `DisplayItem`. All `lt_` calls are made on this thread unless a function's
   row in §3 says otherwise (`pure` functions may be called from any thread; the
   documented exception is `lt_drop_ipsws`, which Qt calls from a worker thread).
2. **Swift `@MainActor`** — the only execution context that touches Swift model
   state. Commands never run there synchronously; they enqueue (via
   `MainActor.assumeIsolated` hop / `Task { @MainActor … }`) and return.
3. **Swift producers** — helper-link threads, job tasks, the log follower, the
   carrier poll timer. They never touch Qt; they hand `LTEvent`s to the C callback.

There is no shared mutable state across the boundary. The C boundary is the only
contact surface.

### 4.2 Call direction Qt → Swift

- A command **must return within microseconds**: it validates arguments, copies
  strings, enqueues onto `@MainActor`, and returns `kLTAccepted`. If the target
  device/session does not exist, it returns `kLTNoSuchDevice` immediately rather
  than enqueueing.
- A query reads a **snapshot**: Swift maintains lock-protected (`NSLock`/
  `Mutex`) copies of the small number of `@MainActor` states Qt polls
  (`lt_device_status`, `lt_tweaks_get`, `lt_carrier_get`, `lt_device_list`,
  `lt_jobs_list`). `@MainActor` refreshes the copies when the source changes
  (coalesced, §2.3); the `@_cdecl` query only takes the lock, serializes the copy
  to JSON, and returns. It never hops to `@MainActor`, never blocks on it.
- A **pure** call executes `nonisolated` Swift directly on the caller's thread.
  Pure functions must not take locks, allocate unboundedly, or touch
  `@MainActor` state — they are value-type math (`ZoomMode`, `FreeFormResize`,
  `DroppedFiles` classification, `LogTail.filtered`).
- Timeouts: none of the three classes has a timeout because none can block by
  construction. If a query's lock is contended, it spins briefly (snapshots are
  small); contention beyond that is a Swift-side bug, surfaced by TSan.

### 4.3 Event direction Swift → Qt

1. Swift builds the `LTEvent` (stack or short-lived heap), malloc's its strings,
   and invokes the registered `lt_event_fn` **synchronously from the producing
   thread**. It does not wait, does not retain the event afterwards.
2. The bridge trampoline (C, in `lt_bridge`) **must not do Qt work**: it
   deep-copies the event (including strings), calls `lt_free_event_strings` on
   the original, and re-posts the copy with `QMetaObject::invokeMethod(target,
   Qt::QueuedConnection)`. Then it returns. Total work: a `memcpy` and a few
   `strdup`s.
3. On the Qt UI thread, the queued handler routes the copy to the owning adapter,
   which updates its `Q_PROPERTY`s (QML re-renders), then frees the copy's
   strings. Adapter updates are idempotent — a duplicate or reordered event
   (§2.4) only causes a redundant property set, never corruption, because every
   event either carries its full payload or triggers a snapshot re-query.
4. Backpressure: the queued connection is unbounded by nature; Swift coalesces
   per §2.3 (one `kLTEventDeviceListChanged` per runloop turn, carrier at 2/sec,
   log lines batched). If the UI thread stalls, events queue — they are small.
   `frameAvailable` is the exception: the render thread reads the newest frame
   directly from the mapped ring, so a stale `frameAvailable` is simply skipped
   by sequence check.

### 4.4 Forbidden

- Swift touching Qt objects (no `QObject*` in Swift, ever).
- Qt/C++ touching Swift objects (no Swift class references in C++; only the
  `lt_` C functions and the event callback).
- Calling an `lt_` function from inside the event callback (no re-entrancy;
  the callback is a leaf).
- Blocking calls across the boundary in either direction: no semaphores, no
  "send and wait for reply", no synchronous IPC. Request/response correlation
  uses ids (`requestId`, `transferId`) plus events, never blocking waits.
- Throwing exceptions across the boundary (C++ exceptions must not propagate
  through Swift frames and vice versa; the trampoline is `noexcept`).
- The Qt render thread calling anything but **pure** functions and reading the
  mapped frame ring.

### 4.5 Startup/shutdown ordering

`lt_init` (UI thread) → `lt_set_event_callback` → all other calls →
`lt_quit_request` → `kLTEventQuitReady` → `lt_shutdown` → `kLTEventShutdown`.
Calling any `lt_` before `lt_init` returns `kLTNotReady`. After `lt_shutdown`,
the event callback is never invoked again.

---

## 5. Versioning

- `lt_abi_version()` returns an integer, starting at `1`. The Qt side checks it
  at startup: if the library's version is **older** than the minimum the UI was
  built against, the UI refuses to start with a clear error (no silent
  misbehavior); if **newer**, the UI starts — newer libraries are backward
  compatible by the rules below.
- **Adding** is always allowed without bumping the major contract: new `lt_`
  functions, new `LTEventKind` values (from `kLTEventReservedStart` upward), new
  enum constants, new JSON fields in query/event payloads, new keys in
  `lt_settings_*`. Qt ignores what it doesn't know: unknown event kinds are
  dropped in the trampoline, unknown JSON fields are ignored by the adapters,
  unknown enum values map to a documented `kLTOther/Unknown` fallback.
- **Removing or renumbering** is never allowed within a major version. Renaming a
  function keeps the old name as a thin alias for one full ABI version.
- **Changing semantics** of an existing function (e.g. making a query block, or
  changing a JSON field's type) requires a new ABI version and is treated as a
  breaking change: both sides bump, CI's ABI check (which diffs `lt_bridge.h`
  against a checked-in copy) fails the build until both are updated together.
- JSON payload schemas are the main evolution surface by design (§1.2): adding a
  field to `lt_device_list` output does not touch `lt_bridge.h` at all. The
  schemas are documented in `lt_bridge.h` comments and validated by a contract
  test that round-trips each query/event through a JSON-schema checker.

---

## 6. Gaps — Swift APIs the UI needs that do not exist yet

Nothing in this section is designed here; each item names the missing Swift-side
capability, which UI surface needs it, and what existing code it would build on.
The `@_cdecl` functions in §3 that front these are specified against the intended
API, not an existing one.

1. **App-core orchestrator (the largest gap).** No `EmulatorController`
   equivalent exists in `port/` — it lives in the Mac-only
   `upstream/LightTouchMac/Device/EmulatorController.swift`. The bridge needs a
   portable, `@MainActor` Swift owner for: the `DeviceLibrary` instance, one
   session object per device (boot/halt/input/apps/files/tweaks/carrier/capture
   wiring), the `FirmwareJobs` instance, settings/preference stores, the
   `lt_device_t` handle registry, the lock-protected snapshot caches (§4.2), and
   the `LTEvent` fan-out pump feeding the C callback. Surfaces: all. Builds on:
   `DeviceLibrary`, `FirmwareJobs`, `BootCycle`, `DeviceApps`, `DeviceInput`,
   `DeviceNotices`, `FilesConnection`, `TweaksPanelModel`, `CarrierPanelModel`,
   `ScreenRecordingSession`.
2. **Event fan-out.** `FirmwareJobs` posts `NotificationCenter` notifications and
   models use `@Observable`/`ObservationLoop` (AppKit-oriented); there is no
   unified Swift→C event queue producing `LTEvent`s. Needed to feed §2 without
   each model knowing about the bridge. Surfaces: all.
3. **Snapshot DTOs.** The JSON documents in §3 (device rows, `DeviceMenuState`,
   tweaks, carrier, jobs, apps rows) need `Codable` snapshot types; most models
   expose live objects, not snapshots. Surfaces: 1, 4, 5, 9, 11.
4. **Swift-side frame-ring reader for capture.** `ScreenRecordingSession.start`
   takes a `frame: () throws -> LTMImage?` supplier; the supplier that reads the
   POSIX shm ring (which the Qt `DisplayItem` also maps) does not exist — in the
   Mac app the frames came from `DisplayView`. Without it, `lt_capture_screenshot`
   / `lt_capture_record_start` have no pixels. Surface: 8. Builds on:
   `DeviceRendezvous`/`SharedStatus` (fd source), `LTMImage`, `FFmpeg`.
5. **`UserDefaults` equivalent on Linux/Windows.** `ZoomMode.save(for:)`,
   `SidebarList.save`, `CapturePreferences`, `RestorationDefaults`,
   `ConsoleSplitLayout.load/save`, `NetworkAccessPreference` all target
   `UserDefaults.standard`, which has no Linux/Windows backend in the port that
   this design could verify — confirm what Phase 2 provides; if it is a stub,
   a file-backed implementation is a gap. Surfaces: 2, 4, 8, 10.
6. **Storage-settings model.** `StorageSettingsView` (Mac app target) has no
   portable backer in `port/`; `lt_storage_get/set` (§3.5) is specified against
   a model that must be written (capacity plans, per-device storage generation
   choice). Surfaces: 4. Builds on: `StorageLocations`, `StorageGeneration`,
   `DeviceInstance.Storage`.
7. **Notification backend.** `SystemNotifier.post` targets `UNUserNotificationCenter`
   (macOS); on Linux/Windows the toast path is Qt-side (`kLTEventToast` →
   `QSystemTrayIcon`), so Swift needs either a no-op or a redirect that emits the
   event instead of posting natively. Surfaces: 8, 13.
8. **Qt key-code → guest key-code translation.** `KeyboardInput.sendKey` and the
   `GuestKeyboard.us` table are keyed by macOS virtual key codes; Qt delivers
   `Qt::Key`/native scan codes (evdev on Linux, VK on Windows). A translation
   table to the USB HID usages the guest expects does not exist. Surface: 2.
9. **Physical-size display data.** `DisplayMeasurements.pointsPerMillimeter` is
   stubbed to `nil` on Linux/Windows (verified in source); "Physical Size" zoom
   therefore always falls back to Fit until Qt supplies real EDID data — which
   means either a Qt-side provider (preferred: Qt knows the screen's physical
   size) passed into `lt_zoom_stops_json`, or a Swift implementation per OS.
   Surface: 2.
10. **Store browser core.** `AppsInspectorViewController+Store` is Mac-app-target
    code; if any of its logic (search, catalog app metadata) is portable it has
    not been identified — `lt_apps_store_search` may need a new Swift API or may
    be implementable Qt-side against `AppMetadataCache`/`CatalogApp`. Surface: 5.
11. **Save-destination prompt plumbing.** `ScreenRecordingSession.chooseSaveDestination`
    is a Swift closure the Mac app satisfied with a save panel; the two-step
    `lt_capture_record_stop` → `kLTEventRecordingSaveNeeded` →
    `lt_capture_record_save` handshake (§3.8) needs the Swift side to suspend the
    session in "awaiting destination" state. Surface: 8.
12. **Files-window follow wiring.** `FilesConnection.follow(device:endpoint:reachable:)`
    is driven in the Mac app by selection/boot observation; the Qt side calls
    `lt_files_follow`, but the endpoint/reachability inputs (`DeviceApps.filesEndpoint`,
    `filesReachable`) must be wired to session events inside the new app controller
    (gap 1), not by Qt polling. Surface: 6.

Out of scope for the Swift core (Qt-owned, listed here only to prevent
mis-assignment): 3D device skins (phase 2, Qt Quick 3D + converted glTF), menu
bar construction and validation UI, drag-and-drop event sources, the
`DisplayItem` frame-upload and gesture recognizers, window-chrome persistence
(`QSettings`), and the Add-device sheet flow (which orchestrates `lt_catalog_*`,
`lt_device_create`, `lt_sidebar_add` — all bridged — but whose wizard UI is QML).
