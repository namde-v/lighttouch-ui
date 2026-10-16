# UI Coverage: Mac reference → Qt 6 rewrite

Date: 2026-10-16 · Phase 2 research · read-only inventory of the Mac UI.

**Source of truth:** `~/workspace/lighttouch-cross/upstream/LightTouchMac/` — 95 Swift files in
`UI/` (68), `App/` (9), `Device/` (7), `Features/` (8), `Library/` (3).
**Surface numbers** are the 14-surface catalog in `docs/UI_APPROACH.md` §2:

1. Main window + sidebar · 2. Emulator display · 3. Device shell / skins · 4. Device inspector panels
5. Apps inspector · 6. Files browser · 7. Log window + console · 8. Capture
9. Firmware catalog & jobs · 10. Dialogs & sheets · 11. Menus · 12. Drag-and-drop
13. Notifications · 14. Taskbar/dock integration

**Status convention:** every row is `pending` until the Qt counterpart is built and passes the
offscreen CI suite. Do not mark anything done from this document alone.

Planned Qt counterpart paths assume the bridging design in `UI_APPROACH.md` §4:
`ui/qml/<Surface>/<Name>.qml` (views), `ui/src/adapters/*Adapter.cpp` (thin C++ mirrors of the
portable Swift view-models), `ui/src/items/*Item.cpp` (custom QQuickItems), `ui/src/native/*`
(QObject bridges: menus, notifications, taskbar).

---

## 1. Per-file table

`Path` is relative to `upstream/LightTouchMac/`. "Key behaviors" is the 1:1 contract: what the
user sees and does. When a file primarily serves a surface but is consumed elsewhere, the primary
surface is listed and the cross-use is noted.

| Mac file | S# | What it does | Key behaviors to preserve 1:1 | Planned Qt counterpart | Status |
|---|---|---|---|---|---|
| `UI/MainWindowController.swift` | 1 | The device window: split view (sidebar / device+console / apps inspector), toolbar, menu-command routing via responder chain. | Window resizes to fit device until user resizes (`sizedToDevice` cleared on manual resize); pane container swap keeps selection state; per-window `CaptureController`; ⌥ monitor re-syncs rotation controls on app activation. | `ui/qml/main.qml` (SplitView: DeviceSidebar / DevicePane+console strip / inspector placeholder) | partial — three-pane split, selection routing, title tracks device+boot stage: done. Pending: toolbar (surface 11), sizedToDevice resize-to-fit, ⌥ rotation re-sync, per-window capture wiring (surface 8) |
| `UI/MainWindowController+Library.swift` | 1 | Selection wiring: sidebar selection → detail area (running session workspace, placeholder, "nothing selected", multi-select label); firmware-job notifications refresh. | Multi-select shows count line, not devices; detail swap does not reset split collapse state; unclean-shutdown notice offers "shut down first". | `ui/qml/views/DevicePane.qml` (StackLayout swap) | partial — all four detail states swap on selection; swap never touches the console strip (sibling). Pending: unclean-shutdown notice (needs core notice kind; surfaces 10/13) |
| `UI/MainWindowController+Status.swift` | 1 | Window chrome: title/subtitle, startup toast, device notice, dead overlay, catalog search (⌘F find panel). | Dead overlay offers Restart; subtitle tracks boot stage line; startup toast per device; ⌘F focuses catalog, not text. | `ui/qml/views/BannerStack.qml` + `LTDeviceAdapter` notice/boot-stage props | partial — notice banner (Resolve/Dismiss) + boot-stage toast via BannerStack; title tracks device + stage line: done. Pending: dead overlay with Restart (surface 2/11), ⌘F catalog find (surfaces 9/11) |
| `UI/DeviceLibraryViewController.swift` | 1 | The sidebar: catalog entries in catalog order, per-row state accessory, add/rename/delete. | Double-click renames in place; Delete removes; one-row delete asks via delegate (prepared device) vs instant (nothing on disk); multi-delete asks once for the lot and skips running/job rows; selection moves to next row Finder-style; failed jobs stay listed for their error rows. | `ui/qml/views/DeviceSidebar.qml` + `LTDeviceListAdapter` (uuid-scoped boot/halt/reset, `classifyDropUrl`) + `LTDeviceIconAdapter` | done — icons, accessories (running dot / spinner / error / busy text / per-row job bar / note), in-place rename (Esc cancels), guarded delete (one question, skips running/busy/job rows, next-row selection), multi-select (Ctrl-toggle, Shift-extend), type-to-select, context menu (Start/Shut Down…, Force Stop…, Import IPSW…, Cancel, Rename, Delete…/Remove…), IPSW drop import, Ctrl+Backspace = ⌘⌫ (bare Delete does nothing, per Mac). Mock-only: row `"prepared"` field + job `"deviceUuid"` linkage until the real core exposes them. Context-menu items needing other surfaces are not listed yet: Download and Prepare (s9), Show in Finder (no ABI), Jailbreak…/Erase… (s10/s4) |
| `UI/DevicePlaceholderViewController.swift` | 1 | Detail area for a device not running in this window: dimmed art, state line, one primary action. | Each job keeps its own progress bar (no animating between values when switching rows); bar caption = percent + time-left + speed; "Skip Setup Assistant" and "Jailbreak" checkboxes only before prepare; IPSW drop target on the whole pane; info popover for catalog notes; "Show in Finder"/"Don't Save" when a Finder edit is open. | `ui/qml/views/DevicePlaceholder.qml` | partial — dimmed art, state line, one primary action (Start), per-job bar with % + time-left + detail caption, failed-job error + Dismiss, IPSW drop target, catalog-note popover: done. "Skip Setup Assistant"/"Jailbreak" shown disabled for unprepared rows (the Add Device sheet owns those sets — surface 10). "Show in Finder"/"Don't Save": blocked — no ABI for Finder-edit state |
| `UI/DeviceViewController.swift` | 1 | Hosts DisplayView in the main column; wires drop callbacks and free-form panel Apply/Revert banner. | DisplayView is first responder for keyboard passthrough; free-form pending size shows a glass banner with Apply (Return) / Revert (Esc); dropping media checks `MediaSupport` before queueing. | `ui/qml/views/DevicePane.qml` | partial — hosts DeviceScreen/placeholder, pane-level IPSW drop, keyboard focus routing (surface 2), free-form Apply/Revert via Return/Esc + WxH readout (surface 2): done. Pending: MediaSupport-gated media drops (surface 12; extension check via `lt_drop_classify` exists) |
| `UI/DeviceContentView.swift` | 1 | "One continuous scene": gradient backdrop, device screen, transient status stack pinned to bottom. | Transient feedback (CaptureStatus banners) floats over the scene, never under it and never inside window captures. | `ui/qml/views/DevicePane.qml` + `ui/qml/views/BannerStack.qml` | partial — BannerStack floats over the scene bottom with primary/secondary actions (reused by surfaces 6/8 later): done. Gradient backdrop pending (arrives with the surface-2 scene); "never inside window captures" is a surface-8 capture-path requirement, noted |
| `UI/Board+Icon.swift` | 1 | Device icons for sidebar + Add Device sheet: Finder's own artwork via product type, SF Symbol fallback. | Text always uses the app's marketing names (e.g. both iPods read "iPod touch", not macOS's label); fallback icon on macOS that doesn't declare the model. | `ui/src/adapters/LTDeviceIconAdapter.{h,cpp}` (Qt resource artwork) | partial — per-form-factor bundled PNG art (`qml/art/`; SVG masters kept as source, PNG shipped because the QtSvg image plugin is not available on all targets), board→art mapping, generic fallback: done. Blocked: per-model Apple product artwork cannot ship off-macOS; generic silhouettes are the honest approximation. Marketing names are core-owned (row JSON), untouched |
| `UI/AppleDeviceType.swift` | 1 | UTTagClass extension used to look up Apple's product-type artwork. | Internal helper — behavior is in Board+Icon. | (folded into LTDeviceIconAdapter) | done |
| `UI/DisplayView.swift` | 2 | The device screen view: shell+LCD transform, frame presentation, power badges, home button, touch overlays. | Power states dim shell (sleeping 0.45 / off 0.25 opacity, screen hidden when off) with a "Start"/"Wake" badge; background click touches the device immediately (`acceptsFirstMouse`); right-click menu: Home Screen, Lock, Rotate L/R, Shake, Copy/Save Screenshot; VoiceOver custom actions mirror those. | `ui/qml/views/DeviceScreen.qml` + `ui/src/items/DisplayItem.{h,cpp}` + `ui/src/items/FrameRing.h` + `ui/src/adapters/LTDisplayAdapter.{h,cpp}` | done (2026-10-16): flat bezel shell (surface 3 phase 1) with the DisplayItem presenting the POSIX shm frame ring (UI-thread acquire via FrameRingReader, render-thread texture upload; single-segment `[status][3 frames]` layout, mock-only synthetic ring until libLTAppCore lands); power dim (sleeping 0.55) + Wake badge; home-button overlay; finger-dot overlay; right-click menu (Home Screen, Lock, Rotate L/R, Shake, Save Screenshot). Gap: VoiceOver custom actions not ported (no Qt screen-reader action equivalent wired). |
| `UI/DisplayView+Frames.swift` | 2 | 60 fps frame loop: CADisplayLink `step()` reads the helper's IOSurface ring. | Frame layer shows the front surface itself (no copy); only the 3D model gets a CGImage; finger-dot touch overlay (opt-in, `showsTouches`) fades; canvas capture reads the newest ring surface under a use-count even when hidden/minimized; vibration motor trembles the device (disabled with Reduce Motion). | same as above | done (2026-10-16): DisplayLinkReady → mmap ring; FrameAvailable → UI-thread acquire (reader publishes its hold in the status segment — mapping is PROT_READ\|PROT_WRITE); render thread uploads the held segment via QSGSimpleTextureNode (BGRA→ARGB32, no copy); finger dots opt-in via device.fingerDots. Canvas capture → surface 8; vibration tremble n/a (host-side). |
| `UI/DisplayView+Input.swift` | 2 | Mouse → guest touch; trackpad gestures routed by cursor position. | Pinch over panel = **guest pinch** via `LinkCommand.touch2` (never host zoom; continuous magnification tracking around the gesture anchor, starts at 0.12 spread); two-finger scroll over panel = finger drag (`ScrollDrag`); two-finger double-tap = guest double-tap; same gestures off-panel drive host zoom / tilt; 14 pt screen-edge margin makes edge swipes (Notification Center) land even when the pointer misses; Option-drag shows simulator-style two-finger pair rings. | `ui/qml/views/DeviceScreen.qml` (MouseArea + PinchHandler + WheelHandler-equivalent) | done (2026-10-16): press/drag/release → lt_input_touch with the 14pt edge margin (scaled to item px); wheel → one-finger flick; pinch → GUEST pinch (two touchIds around the anchor, 0.12 start spread, continuous tracking — never host zoom); Alt+drag → tilt. Gaps: two-finger double-tap → guest double-tap not mapped; Option-drag pair rings not drawn; keyboard-pointer mode (Space/Shift+arrows) not ported. |
| `UI/DisplayView+Keyboard.swift` | 2 | Physical keyboard → guest (keycodes), IME composition → text; keyboard-pointer mode when typing disabled. | Space held = touch; Shift+arrows = drag; arrow-key pointer moves a cursor dot; Tab moves focus out; dead keys/IMEs arrive as text (`typeText`), composition window shown by the IM's own window; keys released when window resigns key. | `ui/qml/views/DeviceScreen.qml` Keys handlers | done (2026-10-16): text → lt_input_text; special keys → lt_input_key with a best-effort Qt→HID map (ABI.md §6 gap 8); Send Keyboard Input menu toggle gates it. Gaps: IME composition window, Tab focus-out, keys-released-on-focus-loss. |
| `UI/DisplayView+Tilt.swift` | 2 | Drag the chassis to tilt; accelerometer follows; side buttons hit-test. | Off-panel wheel/drag = tilt gesture, springs back to rest on release; twist = roll; Option toggles rotation controls; attitude indicator (horizon widget) shows live pitch/roll and clicks to level; volume buttons on the chassis/3D model send `pressVolumeUp/Down`; sleep/wake control sends `pressLock`. | `ui/qml/views/DeviceScreen.qml` (attitude indicator inline) | done (2026-10-16): Alt+drag tilts via lt_input_tilt, springs back on release; horizon widget shows live pitch/roll and clicks to level; volume/lock buttons in the right-click menu + Device menu. Gaps: twist=roll, side-button hit-testing on the chassis (surface 3). |
| `UI/DisplayView+Layout.swift` | 2 | Single-transform layout: shell layer at native pixels, LCD as its child; rotation animation 0.4 s. | Rotation swings shell+LCD together (one matrix, no drift); respects Reduce Motion for the swing; home button positioned in view space. | `ui/qml/views/DeviceScreen.qml` | partial — rotation via rotationDegrees swings screen+home button together; home button positioned in view space. Gaps: no 0.4s rotation animation (instant), no Reduce Motion equivalent. |
| `UI/DisplayView+Zoom.swift` | 2 | Zoom ladder: points-per-guest-pixel, Fit/Physical/Pixel-Accurate. | Fit leaves a flat 16 pt inset (not a fraction); Physical uses the display's real points-per-mm via `DisplayMeasurements`; filtering = nearest when zoom shows guest pixels ≥ backing pixels, else linear — applies to flat shell and 3D model alike; zoom remembered per board. | `ui/src/adapters/LTDisplayAdapter.{h,cpp}` + View menu | done (2026-10-16): Fit (16pt inset), Physical (no Qt physical-size API — falls back to Fit, ABI.md §6 gap 9), Pixel-Accurate, ⌘+/− ladder via lt_zoom_stops_json/lt_zoom_step; nearest-vs-linear filtering per drawsNearest; zoom remembered per board in QSettings; mock has no ladder (lt_zoom_step → -1) so zoomIn/Out are no-ops against the mock. |
| `UI/DisplayView+FreeForm.swift` | 2 | Free-form resizable screen (View ▸ Free-Form Screen): edges/corners are handles, Apply/Restart semantics. | 10 pt handle band with resize cursors; ⌥ resizes symmetrically; sizes snap to what the board's `panel=` accepts, with the limit named in the readout; Return applies (records size, restarts device if running — UIKit takes panel size at boot only), Esc reverts; last frame stays unscaled during the wait; "Restarting at W×H…" notice; window subtitle reads the size. | `ui/qml/views/DeviceScreen.qml` | done (2026-10-16): 8 handles, ⌥ resizes symmetrically, Return applies via lt_display_set_panel_size (+ device reset when running — UIKit takes panel size at boot only), Esc reverts, live WxH readout. Gaps: lt_resize_drag snap is called but the mock returns '{}' → raw-math fallback; no "Restarting at W×H…" notice text; window subtitle doesn't read the size. |
| `UI/DisplayView+DragAndDrop.swift` | 12 | Drops onto the screen: .ipa, media, IPSW, Legacy Store rows. | Drop refused *at the drag layer* (green copy badge suppressed) while the device can't queue installs — never one error sheet per file; IPSW drops route to the library even mid-boot; only outside drags install (local rows carry fileURL but are rejected); store rows decode the private `ltmCatalogApp` pasteboard type. | `ui/qml/views/DeviceScreen.qml` + `LTDeviceListAdapter.routeDrop` (lt_drop_route) | partial |
| `UI/DropHighlight.swift` | 12 | Drop-target cue: accent ring just inside the view. | Ring shows only for accepted operations; never intercepts clicks/drags. | `ui/qml/views/DropHighlight.qml` | done |
| `UI/KeyModifiers+AppKit.swift` | 2 | NSEvent modifier flags → portable `KeyModifiers`. | Internal glue — no user-visible behavior beyond correct ⌥/⇧/⌘ interpretation in input and capture keys. | `LTDisplayAdapter.sendKey(qtKey, down, modifiers)` | done (2026-10-16): Qt modifier flags forwarded to lt_input_key; ⌥/⇧/⌘ interpretation is the best-effort HID map (ABI.md §6 gap 8). |
| `UI/SleepyView.swift` | 2 | "Sleeping" animation in the power badge (bundled Core Animation archive). | Animation only while the sleeping badge is up; missing resource degrades to static badge. | `ui/qml/views/DeviceScreen.qml` (Wake badge) | partial — static Wake badge while sleeping. Gap: no breathing/pulse animation (Core Animation archive has no Qt equivalent; a QML pulse was not added). |
| `UI/AttitudeIndicatorButton.swift` | 2 | Horizon widget outside captured pixels: live pitch/roll, click to level. | Accessibility label/value ("Pitch N°, roll M°"); hidden except while tilting controls active. | `ui/qml/views/DeviceScreen.qml` (attitude indicator inline) | done (2026-10-16): horizon widget visible while tilting/tilted, live pitch/roll readout, click to level. Gap: no accessibleName/Value (screen-reader label) set. |
| `Device/EmulatorController+Input.swift` | 2 | Guest input entry points: pressHome/Lock/VolumeUp/VolumeDown, shake, paste, typeText, compass, rotation, keyboard toggles. | "Connect Hardware Keyboard" (⇧⌘K, per device): unplugged → iOS shows its on-screen keyboard. | `ui/src/adapters/LTDeviceAdapter.{h,cpp}` (pressButton/setLocked/pressVolumeUp/Down/shake/typeText/rotateGuest) | done (2026-10-16): all input entry points bound; "Send Keyboard Input" toggle in the Device menu (in-memory). Gap: "Connect Hardware Keyboard" toggle exists in-menu but is in-memory state only. |
| `Features/DisplayRefresh.swift` | 2 | Settings ▸ General ▸ 120 Hz refresh: boots the panel with its frame interrupt. | Guest-side only; setting takes effect on next boot. | Settings ▸ General ▸ 120 Hz toggle (`LTSettingsAdapter.refresh120Hz`, QSettings-backed) | done (2026-10-16): toggle persists; guest-side frame-interrupt application is the core's job at boot. |
| `Features/VibrationSound.swift` | 2 | Plays the device's vibration motor through host audio while the guest buzzes. | Engine runs only around a buzz; off when Settings ▸ General ▸ "Play vibration sound" is off; paused/stopped device is still. | — | blocked: no QtMultimedia QML module installed in this environment and no ABI vibration event to trigger on; host-audio vibration cannot be built or tested. |
| `UI/DeviceModelView.swift` | 3 | 3D device skins: USDZ + Metal/RealityKit, live LCD as a material on the asset. | One-second model load budget — falls back to flat shell art if slow; side controls (sleep/wake, volume) hit-tested on the model; screen-off state dims the model's display; A4-panel devices keep fixed surface rotation when turning; RealityKit kept off-refresh when still. | `ui/qml/views/DeviceScreen.qml` (flat bezel; 3D skins phase 2) | partial |
| `UI/DisplayView+Bezel.swift` | 3 | Bezel modes: 3D model / 2D flat shell art / off (screen alone); View ▸ Device Bezels. | Preference is app-wide, remembered, migrates the old on/off pref; iPhone 4 / original iPhone have no 3D model — "3D" shows them in 2D, checkmark follows what's shown; free-form forces bezel off and restores on exit. | `ui/qml/views/DeviceScreen.qml` (2d/off, app-wide pref, free-form forces off) | partial |
| `UI/CarrierPanel.swift` | 4 | Fake cellular network for running iPhones (M68/N88/N90): carrier identity, calls, SMS, modem GPS. | Network settings are the device's (applied at every boot); calls: Ring/Answer/Hang Up with live state poll (1 Hz while visible); SMS send with validation; GPS map: lat/long entry + "Apple Park" + "This Mac's Location" (CoreLocation) + walking simulation; one window per device. | `ui/qml/panes/CarrierPanel.qml` (GPS map → coords+presets) | done |
| `UI/TweaksPanel.swift` | 4 | One device's hidden switches (`TweaksPanelModel`), by section; shows finger dots toggle. | Rows stay visible but disabled on firmware lacking the tweak; captions say what each changes; one window per device, running or stopped. | `ui/qml/panes/TweaksPanel.qml` | done |
| `UI/StorageSettingsView.swift` | 4 | Storage pane: per-device and store sizes, Remove IPSW, Clear Caches, Delete Device. | Referenced IPSWs take no space (note says so); "Remove IPSW" leaves the file; per-device rows offer Delete Device (guarded by canDelete). | `ui/qml/panes/StorageSettings.qml` | done |
| `UI/ProxySettingsView.swift` | 4 | Device ▸ Network ▸ Proxy…: per-device web proxy routing + CA trust, as a sheet. | Choices are the device's; sheet has Cancel / OK (default); archive dates shown as calendar days, local. | `ui/qml/panes/ProxySettings.qml` (Cancel/OK draft semantics) | done |
| `UI/DebugPortView.swift` | 4 | Device ▸ Debugging ▸ Debug Port…: GDB remote stub switch, address, attach commands with per-command Copy buttons. | Port shown is the one this boot listens on (nil when none); each lldb command has its own Copy button. | `ui/qml/panes/DebugPort.qml` (per-command Copy) | done |
| `UI/CaptureStatusView.swift` | 4 | Glass banner stack over the device scene: capture notice, file-transfer status, startup toast. | Banner carries a text button (primary/secondary actions, e.g. Apply/Revert, Show Logs, Cancel); thumbnail preview variant; transient — hidden from window captures. Reused by surfaces 1/6/8. | no shared BannerStack; per-view banners + notifications cover uses | partial |
| `UI/CaptureOptionsView.swift` | 4 | Settings ▸ Capture pane: save folder, screenshot app, sounds, open-after-capture. | Pop-ups' last item is "Other…" asking for a folder/app; changes retitle the toolbar's "Open Screenshot" item. | `ui/qml/panes/CaptureSettings.qml` (extracted; Other... pickers) | done |
| `UI/AppsInspectorViewController.swift` | 5 | Right-hand inspector: installed apps in home-screen order, Installed/Store segmented switch, add/remove. | Default mode is **Store** (suggested list), not Installed; table is source-list style, 56 pt rows; typing in the toolbar search flips to Store; pending install rows sit above installed apps with the job's own progress, cancellable from context menu; a finished job renders as an ordinary app row. | `ui/qml/views/AppsInspector.qml` (Store default, pending rows) | partial |
| `UI/AppsInspectorViewController+Loading.swift` | 5 | Loading machinery: forever-polling list, stale banner, pending-row pruning. | Polls for the life of the view (post-boot empty answers from installation_proxy must not stop it); failed read keeps the old list + stale banner ("kept, no longer current") — never blanks; rows rebuilt only where changed, selection preserved. | stale banner kept; no forever-poll (event-driven refresh) | partial |
| `UI/AppsInspectorViewController+Store.swift` | 5 | Legacy Store mode: suggested list, search, icon warming, install. | Empty search = suggested (most-archived compatible) list; icons warm in and repaint row-by-row (content-addressed, never stale); double-click installs; version picker via context menu (`CatalogDetailsViewController`). | search flips to Store; icon warming not ported | partial |
| `UI/AppsInspectorViewController+Table.swift` | 5 | NSTableView data source/delegate for both modes. | Home-screen order drives installed list; rows dragged to reorder the home screen itself (AppKit drag feedback); .ipa drops from Finder install; an installed row drags out to Finder as its .ipa. | flat list; no drag-reorder or drag-out-to-Finder | partial |
| `UI/AppsInspectorViewController+Actions.swift` | 5 | Add (install .ipa via open panel) / Remove (uninstall) with confirmation. | Open panel takes multiple .ipas at once ("Choose decrypted .ipa files"); uninstall asks once per selection ("Uninstall N apps? … removes the app and its data"), destructive-styled; install accepted while confirming still queues. | Add/Remove + confirm; single-file open panel (Mac: multi) | partial |
| `UI/AppsInspectorViewController+ContextMenu.swift` | 5 | Row context menu + Apps menu, built from LightTouchCore's `AppsMenu` model. | Submenu "Install on" targets other running devices ("<name> iOS <ver>"), marking this device; cancel/dismiss per pending job; "View on Legacy Store" opens browser; main-menu vs context variants share the model. | row context menu; no "Install on" cross-device submenu | partial |
| `UI/AppRowCells.swift` | 5 | Row views: Legacy Store result with install button, in-flight job row. | Pure drawing over `AppsInspectorRows` — all text/progress comes from the core model. | `ui/qml/views/AppRow.qml`, `StoreRow.qml` | done |
| `UI/PaneMessage.swift` | 5 | Centered pane messages (empty/loading/error) and status caption. | Message wraps without widening the pane; caption truncates in the middle. | `ui/qml/views/PaneMessage.qml` | done |
| `UI/InlineActionButton.swift` | 5 | Small inline button helper (open/inspect actions). | Rounded, small control size. | inline in QML rows | done |
| `Features/AppInstaller+Device.swift` | 5 | Install queue on a running device: `EmulatorController` as `InstallDevice`, AppKit alerts. | Errors surface as alerts from the presenting window; queue is per device. | `LTAppsAdapter.install` (per-device queue) | done |
| `Library/AppMetadataCache+Icons.swift` | 5 | Installed apps' icons as images, decoded once from the metadata cache. | Content-addressed memo: icons repaint rows as they land, never go stale. | placeholder icon; no image-provider caching | partial |
| `UI/DeviceFilesViewController.swift` | 6 | Files browser: media folder or app container or whole filesystem (jailbroken), in its own window. | Column browser; files drag in (onto folder/column) and out via file promises, several at once, serially; Space = Quick Look; ⌘⌫ Delete… with confirmation (bare Delete does nothing); context menu makes/renames/deletes; source menu picks root vs installed app; new-file target = selected folder else column's folder; replace-name conflicts ask once; transfer progress with cancel in the file status banner; "Show Hidden Files" view toggle. | `ui/qml/views/FilesBrowser.qml` (flat table; no column browser/drag-out/source menu) | partial |
| `UI/DeviceFilesWindowController.swift` | 6 | Retains the Files window; binds its title. | Title follows the device + source (e.g. "<device> Files"). | title follows device (in FilesBrowser) | done |
| `Library/DeviceFilesystemEdits+App.swift` | 6 | App's filesystem-edit plumbing: volumes open in Finder, errors in alerts. | "Show in Finder" opens the device volume; unclean-shutdown offers shut-down-first. | QDesktopServices for downloads; no volume open | partial |
| `UI/LogWindowController.swift` | 7 | Device Logs window: live tail of `app.log`, filter, pause, clear; titlebar notice accessory. | Polling tail (paused via toggle); filter re-renders the visible text; source picker (which log); titlebar accessory carries the device notice with Show Logs / action / dismiss. | `ui/qml/views/LogView.qml` + `LogWindow.qml` | done |
| `UI/ConsoleSplit.swift` | 7 | Bottom console in the main pane, Xcode-style: device on top, divider bar, toggle. | Divider drags, double-click toggles; hidden → bar floats transparent over the device's bottom edge (device keeps the whole pane); bar's toggle is a plain button, the rest of the strip is grab area except where the device takes presses; console source picker (log URLs); clear + filter in the bar. | `ui/qml/views/ConsoleSplit.qml` | done |
| `UI/InlineLiveTextView.swift` | 7 | Live Text overlay on the screen ("Select Text on Screen"): Vision analysis overlay with done button. | Overlay swallows mouse/keyboard while up (no touch leaks to the guest); Done closes; screenshots end Live Text first. | no Qt Vision equivalent; needs Tesseract/OCR or drop | blocked |
| `Features/CaptureController.swift` | 8 | Screenshots (copy/save/save-as/open) + screen recordings with status banner. | Screenshot busy-guard (no double-take); copy shows a brief confirmation; recording: Save panel names "<device> Recording.mov" in the capture folder, offers open-in-Finder on completion, failed saves reveal the kept take; quitting/minimizing during recording finishes or stops it (quit waits for save); recording blocked while Live Text is up. | `LTCaptureAdapter` + RecordingButton (no canvas grab, no busy-guard) | partial |
| `Features/CanvasCapture.swift` | 8 | Whole-window "canvas" screenshot (bezel + scene) vs screen-only capture. | `captureMode` chooses canvas (bezel included) vs LCD pixels only. | screen-only capture; whole-window grab not ported | partial |
| `Features/CaptureSound.swift` | 8 | Shutter/record start/stop sounds. | Plays through host audio; honors capture preferences. | `ui/qml/views/CaptureSounds.qml` (QtMultimedia SoundEffect) | done |
| `UI/RecordingToolbarButton.swift` | 8 | Toolbar record button: record / elapsed / stop / saving. | Idle: "Start Recording (⌘R)"; recording: live elapsed mm:ss as the label, stop symbol; saving: spinner + "Saving Recording…"; no-op updates touch nothing (runs on every validation). | `ui/qml/views/RecordingButton.qml` | done |
| `Features/CaptureNotifications.swift` | 13 | "Ready to use" notification when a device finishes preparing while another is selected. | Asks permission only the first time there is news; clicking it shows that device; presents banner+list while app is frontmost. Listed here (surface 13); capture flows reuse it. | `LTNotificationsAdapter` (Qt abstraction; per-OS backends noted) | done |
| `UI/CatalogDetailsViewController.swift` | 9 | Version picker for a Legacy Store app: versions, compatibility, install. | Sizes to content; install eligibility rechecked by the emulator endpoint at choose time; shows installed version. | `ui/qml/views/CatalogDetailsDialog.qml` | done |
| `UI/CatalogFilterButton.swift` | 9 | Store filter pop-up (e.g. iPad/iPhone compatibility). | Filter applies to fetched results; button state is the filter. | kind filter in `FirmwareJobsWindow.qml` | done |
| `Features/CatalogIcons.swift` | 9 | Store rows' icon fetching (AppKit side of `CatalogClient`). | Icons warm per-row as they land (see surface 5). | placeholder; no icon warming | partial |
| `Library/FirmwareJobs+App.swift` | 9 | App's firmware jobs: errors with no row go to an alert. | Job progress aggregates for the dock/taskbar bar (surface 14). | `LTJobsAdapter` + `setJobsAggregate` -> taskbar | done |
| `UI/AddDeviceView.swift` | 10 | Add Device sheet: firmware catalog split by device; versions list with support status. | **Sheet, not window** (modal to the main window); versions already in sidebar are checked and unpickable; "Coming Soon" dimmed; only stable builds until "Show experimental" (remembered); picks survive device switching; versions list has keyboard focus on open (arrows move, Tab/⇧Tab between lists). | `ui/qml/AddDeviceSheet.qml` | done (2026-10-16): `qml/dialogs/AddDeviceSheet.qml` -- board groups, stable/experimental filter (remembered), already-in-sidebar checked+unpickable, Coming Soon dimmed, picks survive switches, focus choreography, double-click/Return adds, jailbreak/skipSetup options to `jobs.start` per pick. Gaps: "Supported" feature grid needs per-entry DeviceFeatures (no ABI export); downloaded set empty (no IPSW-store query); added-match is a board+firmware heuristic (rows carry no entry id) |
| `UI/AddDeviceSummary.swift` | 10 | Summary under the versions: per-version facts (released, download, source, prepared size) + capabilities; totals for multi-pick. | Shows download+prepare cost before Add commits. | (part of AddDeviceSheet) | done (2026-10-16): facts grid (Released/IPSW/Source/Prepared) + multi-pick totals in the sheet; feature grid omitted -- needs DeviceFeatures from the core |
| `UI/SettingsWindowController.swift` | 10 | Settings…: General / Capture / Storage panes, toolbar-tab style. | Window resizes to each pane (top edge anchored); Storage re-measures every time its pane is shown; titled after the pane. | `ui/qml/SettingsWindow.qml` + panes | done (2026-10-16): `qml/dialogs/SettingsDialog.qml` -- General/Capture/Storage tabs, titled per pane, sized to pane. Partial: Storage lists devices with delete but no measured sizes -- no `lt_storage_usage` export in the ABI |
| `UI/HelpWindowController.swift` | 10 | "Light Touch Help": Help.txt topics sidebar + text, 780×560. | `[Device]` placeholders re-rendered with the current device name, keeping the topic; text view has find bar. | `ui/qml/HelpWindow.qml` | done (2026-10-16): `qml/dialogs/HelpWindow.qml` -- topics sidebar, `[Device]` re-rendered with the current device name, find field; non-modal 780x560 window |
| `UI/JailbreakAlert.swift` | 10 | Device ▸ Jailbreak…: explains what jailbreak adds (Cydia+Substrate on iOS 3–6, afc2) and that Erase removes it. | Copy varies by iOS major version. | Qt dialog from Device menu action | partial (2026-10-16): `qml/dialogs/JailbreakDialog.qml` built+tested with the exact version-dependent copy (Substrate on iOS 3-6). Confirm path blocked: `DeviceFilesystemEdits.perform(.jailbreak)` has no `lt_` export in the ABI; menu item stays dimmed until the core lands one |
| `App/AboutPanel.swift` | 10 | About Light Touch + Licenses window (bundled license texts, sidebar list). | Standard about panel with build-input credits; "Show Licenses" opens a 860×560 sidebar window reading license files from the bundle (works under App Translocation); license text selectable, monospace. | `ui/qml/AboutDialog.qml` + `ui/qml/LicensesWindow.qml` | done (2026-10-16): `qml/dialogs/AboutDialog.qml` (version + AboutCredits runs + Show Licenses) and `qml/dialogs/LicensesDialog.qml` (sidebar + selectable monospace) |
| `App/NetworkAccessPrompt.swift` | 10 | First-start question: "Connect your <device> to the internet?" | Asked once, answer saved; Connect / Use Offline; changeable later in Device ▸ Network. | Qt dialog at first boot | done (2026-10-16): `qml/dialogs/NetworkAccessDialog.qml`; `LTDeviceAdapter.boot()` and `LTDeviceListAdapter.bootDevice()` gate on the saved answer and emit `networkAccessPromptRequested` -- answer saved via `LTSettingsAdapter.networkAccessAnswer`, then the boot goes ahead |
| `App/QuitRequest.swift` | 10 | Deferred quit via run-loop timer (not nested modal loop). | Quit during a recording waits for the save; `applicationShouldTerminate` native reply for system logout. | `ui/src/native/QuitRequest.cpp` | done (2026-10-16): deferred quit via `lt_quit_request` -> `kLTEventQuitReady` -> `QCoreApplication::quit` (no nested modal loop); system logout hooks `commitDataRequest`; the "unless awaiting" half is core-side |
| `App/WindowRestorationPolicy.swift` | 10 | Frame autosave/restore policy; custom NSApplication subclass hook. | Reopen where the user left it (HIG); saved size is the user's, not the device's. | Qt window geometry save/restore (QSettings) | done (2026-10-16): frame-only save/restore via `LTSettingsAdapter.saveWindowGeometry`/`windowGeometry` (QSettings) in main.qml; never contents, no AppKit restoration funnel |
| `App/BugReportCopy.swift` | 10 | "Copy Bug Report Info" clipboard text (Help menu). | One click collects version/pins for a report. | Help menu action → clipboard | done (2026-10-16, surface 11): `LTSettingsAdapter.copyBugReportInfo()` -- version + credits on the clipboard |
| `App/AppDelegate.swift` | 10 | App lifecycle: launch, first-run device selection, open-URL (.ipa/.ipsw), dock menu, terminate. | `applicationShouldTerminateAfterLastWindowClosed` does not quit (device windows); reopen focuses; dock menu offers window actions; URL opens route to installer/importer; legacy state erased once. | `ui/src/native/AppDelegate.cpp` (QApplication hooks) | done (2026-10-16): `LightTouchApplication` (QApplication subclass) -- `QEvent::FileOpen` routes .ipsw to importer/.ipa to installer; `setQuitOnLastWindowClosed(false)`; `commitDataRequest` -> graceful quit |
| `App/main.swift` | 10 | Entry point; retains the app delegate. | No behavior beyond startup. | `ui/src/main.cpp` | done: `ui/src/main.cpp` (pre-existing entry point) |
| `App/MainMenu.swift` | 11 | Full menu bar: app/File/Edit/View/Device/Apps/Capture/Window/Help, built programmatically. | Shortcuts: ⌘N Add Device, ⌘S screenshot, ⌘R record, ⌘⇧I install app, ⌘⌫ delete device, ⌘+/− zoom, ⌘0 fit, ⌘8 physical, ⌘9 pixel-accurate, ⇧⌘H home, ⌘L lock, ⌘. shut down, ⇧⌘K hardware keyboard, ⌃⌘F full screen, ⌘? help; Escape discards a recording only when safe (CaptureMenu key-equivalent guard); ⌘⌫ over a text view deletes to line-start instead (responder chain wins). | `ui/qml/views/MainMenuBar.qml` (Qt Quick MenuBar; Mac→Qt mapping ⌘→Ctrl, ⌥→Alt, ⌃→Meta) | done (2026-10-16): all 8 menus; items whose surface hasn't landed are listed but dimmed; Mac-only dropped with note: Services menu, Hide/Show All, "Check for Updates…" (Sparkle), toolbar customization, "Open Screenshot in Preview" → "Open Screenshot" (OS default app). Gaps: Delete over a text view deletes the selection (no responder-chain line-kill); Discard Recording Escape guard is enablement-only (no sheet/text-edit state tracked). Surface 2: zoom ladder (Physical/Pixel/Fit/⌘+/−) and Free-Form are now live (enabled while a device runs); Native Size stays dimmed (not in Mac zoom behavior). |
| `UI/MainWindowController+MenuValidation.swift` | 11 | Enablement + checkmarks for every menu item from `DeviceMenuState`. | Titles morph with state (e.g. Start/Stop); bezel items: devices with no 3D model show 2D checked for 3D; zoom items gray per direction; Device ▸ Motion/Compass/Input submenus validate per board (compass only on compass boards). | (part of MenuBar bridge; state from adapters) | done (2026-10-16): `MainMenuBar.refreshValidations()` on every `Menu.onAboutToShow` (= Mac per-open validation); titles morph (Start/Shut Down…, Pause/Resume, Show/Hide Sidebar/Inspector/Console, Start/Stop Recording, Show/Hide Finger Dots); checkmarks (bezels, finger dots, motion pose, compass heading, rotate-auto, keyboard input, hardware keyboard, local-network attach); Device-menu hardware items validate via `lt_menu_validate` (`LTDeviceAdapter.validateMenuItem`, kLTMenuItem 1–7 per lt_bridge.h §6; hand-written mock in Bridge.cpp + MockCore.swift + LT_MOCK_EXCLUDE_CSV). Gaps: bezel "2D checked for 3D" fallback is trivially dimmed (3D is surface 3); zoom items validate on running state (per-direction graying n/a — the mock has no ladder); compass-per-board falls back to input-capable (mock); motion/compass state is per-window in-memory until surface 10 persists it. |
| `UI/MainWindowController+DeviceMenu.swift` | 11 | Device menu actions: home/lock/volume/rotate/motion/compass/carrier/tweaks/proxy/network/debugging/pause/restart/shutdown/force-stop/jailbreak/erase/install/import. | Shut Down / Force Stop / Restart… / Erase all confirm first; "Restart…" is a reset; carrier/tweaks open one window per device; Option toggles rotate-left/right labels; motion poses + compass headings are tagged menu items. | `ui/qml/views/MainMenuBar.qml` device menu (home/lock/rotate/motion/compass/volume/keyboard/network/proxy/carrier/tweaks/debug/pause/restart/shutdown/force-stop/jailbreak/erase) | done |
| `UI/MainWindowController+Commands.swift` | 11 | Library commands behind menus/toolbar: add, download+prepare, import IPSW, delete, show-in-Finder, jailbreak, IPSW open-file routing. | Jailbreak confirms via JailbreakAlert; delete asks with device name; IPSW files opened from Finder route by SHA1 match; "Prepare Again…" re-runs preparation. | `ui/qml/views/MainMenuBar.qml` file/apps menus (add, download+prepare, import IPSW, delete, jailbreak) | done |
| `UI/MainWindowController+EditMenu.swift` | 11 | Edit menu actions beyond text: screenshot variants, paste-to-guest, capture toggles. | ⌘C with the device screen focused copies the screenshot (after text/native responders decline); Edit ▸ Paste with screen focused pastes text *into the guest* (`pasteToGuest`). | `ui/qml/views/MainMenuBar.qml` edit menu (pasteToGuest, find); Live Text blocked (surface 7) | partial |
| `UI/MainWindowController+Toolbar.swift` | 11 | Unified toolbar mirroring the menu bar: device controls, capture, zoom, search, inspector toggle. | Customizable, autosaved; tracking separators keep the inspector/search aligned over the right pane; migrations insert new items (Add Device, floating-bar actions) once without disturbing user layout; zoom is a segmented control; search field lives in the toolbar but is owned by the inspector. | header/footer ToolBar in `ui/qml/main.qml`; no customizable autosaved toolbar | partial |
| `UI/MainWindowController+Zoom.swift` | 11 | Zoom control logic: toolbar segmented control + View menu + ⌘+/−, one ZoomMode per board. | ⌘+ steps a ladder from the current size (pinch never zooms the host); gray out the direction with no room; ⌘0/⌘8/⌘9 jump to Fit/Physical/Pixel-Accurate; zoom remembered per board, re-read on window entry. | View menu zoom actions; no per-board ZoomMode ladder | partial |
| `App/DockProgress.swift` | 14 | Dock icon progress bar during firmware downloads/preparations; holds idle sleep. | One bar under the icon while any job runs (aggregated `FirmwareJob.dockProgress`); >= 0.5% change redraws; cleared when jobs end; system kept awake meanwhile. | `LTTaskbarAdapter` (0.5pct rule, clear-at-zero; backends not wired) | done |

**Cross-cutting rows (device backend the UI reads — not views, but the UI contract depends on them):**

| Mac file | S# | What it does | Behavior the UI depends on |
|---|---|---|---|
| `Device/DeviceSession.swift` | 1 | Per-device session (record + EmulatorController); session host for all windows. | One session per entry; a restart replaces the session wholesale; vanished device folders stop their session; deletions/erases show "Deleting"/"Erasing" rows and block Start. |
| `Device/EmulatorController.swift` | 1/2 | One running device: lifecycle state machine (`VMState`), status poll, guest agent watch, services, web proxy, settings. | Everything the UI enables/disables keys off `state`; status line is one line per poll; boot toast counts per device; unresponsive management service raises a standing issue with recovery. |
| `Device/EmulatorController+Boot.swift` | 1 | Boot assembly: storage, argv, Setup-vs-springboard routing, carrier settings applied at every boot. | Boot-stage line for the toast; "Setup" detection (iOS 5+, no Skip Setup, unfinished overlay). |
| `Device/EmulatorController+Lifecycle.swift` | 1 | Start/stop/pause; hidden-window throttling (few frames/s, slower poll, idle-sleep allowed). | Pause freezes the device; hidden windows throttle — the Qt side must reproduce the throttle or burn CPU. |
| `Device/EmulatorController+Machine.swift` | 1/2 | Machine control: panel changes, guest audio for recordings, quit backstop (halt→kill→services). | Free-form panel change = hard stop + fresh helper; recording audio taps guest audio. |
| `Device/EmulatorController+Liveness.swift` | 1 | Liveness: frames-in-last-2s, helper status block, agent aliveness, one-line status. | "The guest takes input whenever its screen is live" — readiness never gates input. |
| `Device/EmulatorController+Apps.swift` | 5 | Per-device app services for the inspector/installer. | Inspector's reachability/install state comes from here, not the status line. |
| `UI/MainWindowController+Diagnostics.swift` | 10 | Export Diagnostics… (Help menu): zips logs + provenance, scrubs identity secrets. | One-click evidence bundle for bug reports; secrets scrubbed. | | done (pre-existing): `LTSettingsAdapter.diagnosticsExport()` → Help menu shows the zip path as a footer toast |

---

## 2. Per-surface summaries

### Surface 1 — Main window + sidebar (9 UI files + Device backend ≈ 2,260 lines)
Files: `MainWindowController.swift` (+`Library`, `+Status`), `DeviceLibraryViewController.swift`,
`DevicePlaceholderViewController.swift`, `DeviceViewController.swift`, `DeviceContentView.swift`,
`Board+Icon.swift`, `AppleDeviceType.swift`.
**Hardest 1:1:** the selection-driven detail swap with per-job progress bars that never cross-animate,
plus Finder-grade delete semantics (one question for many, next-row selection, running rows skipped).
**Mac-only:** Finder's per-model product artwork (`Board+Icon` via `AppleDeviceType`); Qt needs bundled
artwork per board. Nothing else is Mac-only — `NSSplitViewController` maps to QML `SplitView`.
**Port status (2026-10-16):** sidebar, placeholder, pane swap, banner stack, icon adapter built and tested
(`tst_surface1`: 10 tests; full suite 4/4 green). Mock-only scaffolding: device rows carry `"prepared"`, firmware jobs carry
`"deviceUuid"` — the real `libLTAppCore` must expose the row↔job linkage and prepared state when it lands
(see `Bridge.cpp` mock notes).

### Surface 2 — Emulator display (10 files ≈ 2,300 lines incl. backend input)
Files: `DisplayView.swift` (+`Frames`, `Input`, `Keyboard`, `Layout`, `Tilt`, `Zoom`, `FreeForm`),
`AttitudeIndicatorButton.swift`, `SleepyView.swift`, `KeyModifiers+AppKit.swift`,
`Device/EmulatorController+Input.swift`, `Features/DisplayRefresh.swift`, `Features/VibrationSound.swift`.
**Hardest 1:1:** the gesture-routing matrix — pinch always goes to the guest (never host zoom),
scroll-over-panel is a finger drag, same gestures off-panel tilt/zoom, the 14 pt edge margin, and
continuous delta forwarding. Qt gives `QTouchEvent`/gestures, but the *decision table* (cursor position
× gesture × phase × edge margin × pair rings) must be reimplemented exactly; this is the surface users
will judge the port by.
**Mac-only:** `CADisplayLink` (→ `QTimer`/vsync via `QQuickWindow::frameSwapped`), `NSScreen` physical-size
API (→ `QScreen::physicalSize()`), `acceptsFirstMouse` (→ Qt focus/click-through flags). All replaceable.

### Surface 3 — Device shell / skins (2 files ≈ 760 lines)
Files: `DeviceModelView.swift`, `DisplayView+Bezel.swift`.
**Hardest 1:1:** phase 1 is the flat shell (already in the codebase — ship it); phase 2 is converting
USDZ→glTF and hit-testing side buttons on a 3D model with the live LCD as a texture.
**Mac-only:** Metal + RealityKit + USDZ. No Qt equivalent — honest flag. Plan per UI_APPROACH §1.5:
flat-shell mode in phase 1, Pixar USD→glTF conversion + Qt Quick 3D or custom GL in phase 2.

### Surface 4 — Device inspector panels (7 files ≈ 1,320 lines)
Files: `CarrierPanel.swift`, `TweaksPanel.swift`, `StorageSettingsView.swift`, `ProxySettingsView.swift`,
`DebugPortView.swift`, `CaptureStatusView.swift`, `CaptureOptionsView.swift`.
**Hardest 1:1:** `CarrierPanel` — the walking GPS simulation with live 1 Hz modem polling, the map with
"Apple Park"/"This Mac's Location" presets, and per-boot application of carrier settings. Qt has
`QtLocation`/`QtPositioning` for the map; the modem polling binds to the existing Swift model.
**Mac-only:** CoreLocation (→ `QtPositioning`), MapKit SwiftUI map (→ QtLocation or a static map);
`NSHostingController` glass banner (→ plain QML).

### Surface 5 — Apps inspector (9 files + 2 backend ≈ 1,830 lines)
Files: `AppsInspectorViewController.swift` (+`Actions`, `+ContextMenu`, `+Loading`, `+Store`, `+Table`),
`AppRowCells.swift`, `PaneMessage.swift`, `InlineActionButton.swift`,
`Features/AppInstaller+Device.swift`, `Library/AppMetadataCache+Icons.swift`.
**Hardest 1:1:** the loading contract — poll forever, never blank a correct list, stale banner instead of
empty table, selection-preserving incremental row rebuild — plus the Installed/Store mode split where the
default view is the Store's suggested list. All logic already lives in `LightTouchCore` (`AppsMenu`,
`AppsInspectorRows`); the Qt side is a virtualized list + adapters.
**Mac-only:** `NSOpenPanel` (→ `QFileDialog`), `NSSourceList` drag feedback (→ Qt list drag visuals),
Quick Look is surface 6. Nothing structural.

### Surface 6 — Files browser (3 files ≈ 875 lines)
Files: `DeviceFilesViewController.swift`, `DeviceFilesWindowController.swift`,
`Library/DeviceFilesystemEdits+App.swift`.
**Hardest 1:1:** the drag-out file-promise flow (serial AFC transfers, several at once) and the
key bindings: Space = Quick Look, ⌘⌫ = Delete… with confirmation, bare Delete = nothing. Qt needs a
promise-style drop implementation (`QMimeData` with delayed data) and a Quick Look substitute.
**Mac-only:** Quick Look (no Qt/Linux equivalent — use a built-in preview pane or `xdg-open`; honest flag),
`NSBrowser` (→ QML tree/table), file promises (→ custom `QMimeData`).

### Surface 7 — Log window + console (3 files ≈ 730 lines)
Files: `LogWindowController.swift`, `ConsoleSplit.swift`, `InlineLiveTextView.swift`.
**Hardest 1:1:** the console split's collapsed state — the bar floats transparent over the device's bottom
edge and the device keeps the whole pane, with press-routing that still reaches the device under it.
**Mac-only:** `InlineLiveTextView` is Apple Vision Live Text on the guest screen — **no Qt equivalent**;
flag as replace-with-Tesseract/OCR or drop. The log tail itself (`LogTail` model) is portable.

### Surface 8 — Capture (4 files ≈ 650 lines)
Files: `Features/CaptureController.swift`, `Features/CanvasCapture.swift`, `Features/CaptureSound.swift`,
`UI/RecordingToolbarButton.swift`.
**Hardest 1:1:** the recording lifecycle intertwined with app lifecycle — quit/minimize/hide during a
recording finishes or stops it, failed saves reveal the kept take, and the toolbar button's
elapsed-time/saving states update through validation without relayout.
**Mac-only:** `NSSavePanel` (→ `QFileDialog`), AVFoundation movie writer is in the portable core's
`ScreenMovieWriter`? (verify in Phase 2 — if AppKit-bound, needs a QtMultimedia/ffmpeg replacement).

### Surface 9 — Firmware catalog & jobs (4 files ≈ 220 lines UI + core models)
Files: `CatalogDetailsViewController.swift`, `CatalogFilterButton.swift`, `Features/CatalogIcons.swift`,
`Library/FirmwareJobs+App.swift`.
**Hardest 1:1:** the Add Device sheet's pick semantics actually live here too — catalog truth
(`firmware-catalog.json`) is already portable (`FirmwareCatalog` in core); the UI is thin.
**Mac-only:** none.

### Surface 10 — Dialogs & sheets (12 files ≈ 1,420 lines)
Files: `AddDeviceView.swift`, `AddDeviceSummary.swift`, `SettingsWindowController.swift`,
`HelpWindowController.swift`, `JailbreakAlert.swift`, `App/AboutPanel.swift`,
`App/NetworkAccessPrompt.swift`, `App/QuitRequest.swift`, `App/WindowRestorationPolicy.swift`,
`App/BugReportCopy.swift`, `App/AppDelegate.swift`, `App/main.swift`, plus
`UI/MainWindowController+Diagnostics.swift` (Export Diagnostics).
**Hardest 1:1:** the Add Device sheet — picks surviving device switches, keyboard-focus choreography
(arrow keys in versions, Tab/⇧Tab between lists), "already in sidebar" checks, Coming Soon dimming,
experimental gate. Qt `QDialog`/QML sheets cover modality; focus behavior needs explicit work.
**Mac-only:** standard About panel (→ custom QML), `NSAlert` (→ `QMessageBox`/QML), Services menu
(drop it), `applicationShouldTerminate` semantics (→ `QApplication::commitData`).
**Status (2026-10-16): done.** `qml/dialogs/` now holds AddDeviceSheet, SettingsDialog,
AboutDialog, LicensesDialog, HelpWindow, JailbreakDialog, NetworkAccessDialog;
`tests/tst_surface10.cpp` (12 tests green). Hand-written mocks added for
`lt_settings_get/set_string` (in-memory store), `lt_catalog_list`, `lt_about_credits`,
`lt_help_topics` (Bridge.cpp + MockCore.swift + LT_MOCK_EXCLUDE_CSV). Remaining gaps on the
per-file rows: Storage pane has no measured sizes and the Add sheet no feature grid or
downloaded set (no `lt_storage_usage` / DeviceFeatures / IPSW-store exports in the ABI);
Jailbreak confirm blocked (no `lt_device_jailbreak`).

### Surface 11 — Menus (7 files ≈ 1,710 lines)
Files: `App/MainMenu.swift`, `MainWindowController+MenuValidation.swift`, `+DeviceMenu.swift`,
`+Commands.swift`, `+EditMenu.swift`, `+Toolbar.swift`, `+Zoom.swift`.
**Hardest 1:1:** validation-driven menus — every item's title, checkmark and enablement recomputed from
`DeviceMenuState` on each menu open, plus the responder-chain subtleties: ⌘C copies the screenshot only
when the screen has focus *and* no text does; ⌘⌫ over a text view deletes to line-start instead of the
device; Escape discards a recording only when no sheet/text is active.
**Mac-only:** the global menu bar itself — on Linux, Qt shows a window menu bar (fine); on Windows, same.
`NSToolbar` customization palette (→ QML toolbar without customization, or `QToolBar`); Services/Hide
standard items (drop).
**Status (2026-10-16): done.** `qml/views/MainMenuBar.qml` + `tests/tst_surface11.cpp` (12 tests green).
All 8 menus, Mac→Qt shortcut mapping, per-open `refreshValidations()` (= Mac validation), morphing
titles, checkmarks, `lt_menu_validate` through the adapter (hand-written mock). Remaining gaps are
tracked on the per-file rows above: zoom ladder + Free-Form live since surface 2 (Native Size stays dimmed); motion/
compass/keyboard-input state in-memory until surface 10 persists it; Delete-over-text is plain
selection delete; Live Text blocked (no Qt Vision equivalent).

### Surface 12 — Drag-and-drop (2 files ≈ 110 lines + core `DroppedFiles`)
Files: `UI/DisplayView+DragAndDrop.swift`, `UI/DropHighlight.swift` (sidebar DnD lives in the
sidebar adapter).
**Hardest 1:1:** the refusal-at-the-drag-layer rule — never accept a drop the device will reject
(no green badge, no per-file error sheets), and the local-vs-outside drag distinction (installed rows
drag *out* as .ipa, never back in).
**Mac-only:** pasteboard types (→ MIME types), file promises are surface 6. Straightforward in Qt.

### Surface 13 — Notifications (1 file ≈ 70 lines)
Files: `Features/CaptureNotifications.swift`.
**Hardest 1:1:** permission asked only on first real news; clicking "ready to use" selects that device;
banner+list presentation while the app is frontmost.
**Mac-only:** `UNUserNotificationCenter` — on Linux use the freedesktop notification portal
(`org.freedesktop.Notifications`) with click actions; on Windows use toast notifications. Both need
platform backends behind one Qt interface; the "click → show device" action must be wired on each.

### Surface 14 — Taskbar/dock integration (1 file ≈ 60 lines)
Files: `App/DockProgress.swift`.
**Hardest 1:1:** trivial — aggregate job fraction to one bar, clear at zero, hold idle sleep.
**Mac-only:** `NSApp.dockTile` — Windows: `QWinTaskbarProgress`/`ITaskbarList3`; Linux: Unity launcher
entry `com.canonical.Unity.LauncherEntry` progress signal (works on GNOME via the same DBus interface
in most docks, absent elsewhere — degrade silently). Also the idle-sleep hold (→ `QSystemTray`? no —
platform power-inhibit: Windows `SetThreadExecutionState`, Linux `org.freedesktop.ScreenSaver` inhibit).

---

## 3. Reference reading order for implementers

Read in this order per surface. "Why" = what the file teaches that nothing else does.
Files marked ★ are the ones to read first overall — they define the app's architecture.

| Order | Surface | File | Why |
|---|---|---|---|
| ★1 | 1 | `UI/MainWindowController.swift` | The window: panes, selection, session wiring. The app's skeleton. |
| ★2 | 1 | `UI/DeviceLibraryViewController.swift` | Sidebar: the list model, delete semantics, rename. |
| ★3 | 2 | `UI/DisplayView.swift` | The screen view: power badges, shell/LCD layers, menus. |
| ★4 | 2 | `UI/DisplayView+Input.swift` | The gesture-routing decision table. Read before any input code. |
| ★5 | 11 | `App/MainMenu.swift` | The full command surface: every menu, every shortcut. |
| ★6 | 11 | `UI/MainWindowController+MenuValidation.swift` | What enables what, when — the state→UI contract. |
| 7 | 1 | `UI/DevicePlaceholderViewController.swift` | The not-running detail area; job progress presentation. |
| 8 | 2 | `UI/DisplayView+Frames.swift` | Frame loop, touch overlay, capture reads. |
| 9 | 2 | `UI/DisplayView+Zoom.swift` + `MainWindowController+Zoom.swift` | Zoom ladder + per-board memory. |
| 10 | 2 | `UI/DisplayView+Keyboard.swift` | Keyboard→guest, IME, keyboard-pointer mode. |
| 11 | 2 | `UI/DisplayView+Tilt.swift` | Chassis tilt, accelerometer, side buttons. |
| 12 | 2 | `UI/DisplayView+FreeForm.swift` | Free-form resize + restart semantics. |
| 13 | 3 | `UI/DisplayView+Bezel.swift` | Bezel modes; the phase-1 skin plan. (Read `DeviceModelView.swift` only for phase 2.) |
| 14 | 4 | `UI/CarrierPanel.swift` | The most complex panel: calls/SMS/GPS/walking. |
| 15 | 4 | `UI/TweaksPanel.swift`, `UI/StorageSettingsView.swift`, `UI/ProxySettingsView.swift`, `UI/DebugPortView.swift` | Simpler panels; all bind portable core models. |
| 16 | 4/8 | `UI/CaptureStatusView.swift`, `UI/CaptureOptionsView.swift` | Banner stack + capture settings. |
| 17 | 5 | `UI/AppsInspectorViewController.swift` (+`Loading`, `+Store`) | Inspector modes and the never-blank loading contract. |
| 18 | 5 | `UI/AppsInspectorViewController+ContextMenu.swift` (+`Actions`, `+Table`) | Menus and install/remove flows. |
| 19 | 6 | `UI/DeviceFilesViewController.swift` | Files browser: drag in/out, Quick Look, keys. |
| 20 | 7 | `UI/ConsoleSplit.swift`, `UI/LogWindowController.swift` | Console split behavior; log tail. |
| 21 | 8 | `Features/CaptureController.swift` | Screenshot/recording lifecycle. |
| 22 | 9/10 | `UI/AddDeviceView.swift`, `UI/AddDeviceSummary.swift` | The Add Device sheet's pick semantics. |
| 23 | 10 | `UI/SettingsWindowController.swift`, `App/AppDelegate.swift` | Settings panes; app lifecycle and URL opens. |
| 24 | 10 | `App/AboutPanel.swift`, `UI/HelpWindowController.swift`, `UI/JailbreakAlert.swift` | Remaining dialogs; copy details. |
| 25 | 11 | `UI/MainWindowController+DeviceMenu.swift`, `+Commands.swift`, `+Toolbar.swift`, `+EditMenu.swift` | Device actions, library commands, toolbar, edit quirks. |
| 26 | 12/13/14 | `UI/DisplayView+DragAndDrop.swift`, `Features/CaptureNotifications.swift`, `App/DockProgress.swift` | Small; read as needed when building each. |
| 27 | — | `Device/EmulatorController.swift` (+`Boot`, `+Lifecycle`, `+Liveness`, `+Machine`, `+Input`, `+Apps`), `Device/DeviceSession.swift` | The backend contract every surface reads. Read alongside, not first. |
| 28 | — | `UI/DeviceViewController.swift`, `UI/DeviceContentView.swift`, `UI/ConsoleSplit.swift` details | Wiring glue; skim when assembling the main window. |

**Skim-only** (thin helpers, behavior documented in the table): `PaneMessage.swift`,
`InlineActionButton.swift`, `KeyModifiers+AppKit.swift`, `AppleDeviceType.swift`,
`DeviceFilesWindowController.swift`, `UI/CatalogFilterButton.swift`,
`UI/CatalogDetailsViewController.swift`, `UI/RecordingToolbarButton.swift`,
`Features/CaptureSound.swift`, `Features/CatalogIcons.swift`, `Features/DisplayRefresh.swift`,
`Features/VibrationSound.swift`, `Library/AppMetadataCache+Icons.swift`,
`Library/DeviceFilesystemEdits+App.swift`, `Library/FirmwareJobs+App.swift`,
`App/main.swift`, `App/QuitRequest.swift`, `App/WindowRestorationPolicy.swift`,
`App/NetworkAccessPrompt.swift`, `App/BugReportCopy.swift`, `UI/SleepyView.swift`,
`UI/DropHighlight.swift`, `UI/AttitudeIndicatorButton.swift`, `UI/Board+Icon.swift`,
`UI/InlineLiveTextView.swift` (flagged: no Qt equivalent).

---

## 4. Honest Mac-only flags (no Qt equivalent)

1. **RealityKit/Metal/USDZ 3D skins** (`DeviceModelView.swift`) — no cross-platform USDZ/RealityKit.
   Phase 1 ships the flat bezel; phase 2 converts USDZ→glTF and renders via Qt Quick 3D or custom GL.
2. **Vision Live Text overlay** (`InlineLiveTextView.swift`) — no Qt equivalent. Needs Tesseract/OCR
   replacement or the "Select Text on Screen" feature is dropped in v1.
3. **Quick Look** (Space in Files browser) — no Qt/Linux equivalent. Replace with a built-in preview
   pane or `xdg-open`/default-app open.
4. **Finder product artwork** (`Board+Icon.swift` via `AppleDeviceType`) — bundle per-board artwork
   as Qt resources instead.
5. **UNUserNotificationCenter** — needs per-OS backends (freedesktop portal on Linux, toasts on
   Windows) behind one Qt interface; click-to-show-device must be wired on each.
6. **Dock tile progress** (`NSApp.dockTile`) — Windows taskbar progress and Unity launcher DBus signal;
   degrade silently where unsupported.
7. **Global menu bar / Services / standard About panel** — window-local menu bars on Linux/Windows;
   custom About dialog; drop the Services menu.
8. **CoreLocation / MapKit** (Carrier panel's "This Mac's Location" + map) — `QtPositioning`/`QtLocation`
   are the replacements; verify map-tile availability offline.

Everything else — split views, source lists, sheets, toolbar customization, drag-drop, validation-driven
menus, the 60 fps display, multi-touch gestures — has a direct Qt 6 counterpart.
