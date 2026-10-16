# LightTouch Qt shell — CMake build.
#
# One top-level build from this directory:
#   cmake -B build && cmake --build build && ctest --test-dir build
# produces a runnable app (build/lighttouch). Run it with:
#   LTM_BRIDGE_MOCK=1 ./build/lighttouch            # no library needed
#   LTM_APPCORE_LIB=/path/to/libLTAppCore.so ./build/lighttouch
#
# 2 vCPU machine: builds stay serial-ish via CMAKE_BUILD_PARALLEL_LEVEL=2.

cmake_minimum_required(VERSION 3.21)
project(LightTouchQt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# MSVC: treat all sources as UTF-8 (tr() strings contain non-ASCII text).
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# Portable Python for gen_bridge.py — Windows runners expose `python`,
# not `python3`.
find_program(LT_PYTHON_EXECUTABLE NAMES python3 python REQUIRED)
message(STATUS "Python for codegen: ${LT_PYTHON_EXECUTABLE}")

if(NOT DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL} AND NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    set(CMAKE_BUILD_PARALLEL_LEVEL 2)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Quick Qml Test)

# --- Swift toolchain (SwiftPM targets for LTAppCore / LTMockCore) ------------
set(LT_SWIFT_TOOLCHAIN_BIN "/home/hatch/.local/share/swiftly/toolchains/6.4.1/usr/bin"
    CACHE PATH "Swift toolchain bin dir — put first on PATH for swift invocations")
set(LT_SWIFT_TMPDIR "/home/hatch/.cache/swift-tmp"
    CACHE PATH "TMPDIR for every swift invocation (see SPIKE_RESULT.md note 2)")
find_program(LT_SWIFT_EXECUTABLE swift PATHS "${LT_SWIFT_TOOLCHAIN_BIN}" NO_DEFAULT_PATH)
if(LT_SWIFT_EXECUTABLE)
    get_filename_component(LT_SWIFT_RUNTIME_DIR
        "${LT_SWIFT_TOOLCHAIN_BIN}/../lib/swift/linux" ABSOLUTE)
    message(STATUS "Swift toolchain: ${LT_SWIFT_EXECUTABLE}")
    message(STATUS "Swift runtime dir: ${LT_SWIFT_RUNTIME_DIR}")
else()
    message(STATUS "No Swift toolchain at ${LT_SWIFT_TOOLCHAIN_BIN} — SwiftPM targets skipped")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(SwiftAppCore)

# --- Configure-time codegen from lt_bridge.h ---------------------------------
# lt_api_members.inc / lt_api_bind.inc / lt_mock_stubs.inc / lt_mock_bind.inc
# land in <build>/generated and are #included by Bridge.{h,cpp}. They can never
# drift from the header: regenerating is part of every configure.
set(LT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
execute_process(
    COMMAND "${LT_PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gen_bridge.py"
            --header "${CMAKE_CURRENT_SOURCE_DIR}/src/lt_bridge.h"
            --mode cpp --outdir "${LT_GENERATED_DIR}"
            --mock-exclude "${LT_MOCK_EXCLUDE_CSV}"
    RESULT_VARIABLE _gen_rc)
if(NOT _gen_rc EQUAL 0)
    message(FATAL_ERROR "gen_bridge.py (cpp) failed — lt_bridge.h may have a malformed declaration")
endif()

# --- Bridge + adapters static library (shared by the app and the tests) ------
set(LT_ADAPTER_SOURCES
    src/adapters/LTAdapterBase.cpp
    src/adapters/LTDeviceListAdapter.cpp
    src/adapters/LTSidebarAdapter.cpp
    src/adapters/LTDeviceAdapter.cpp
    src/adapters/LTTweaksAdapter.cpp
    src/adapters/LTCarrierAdapter.cpp
    src/adapters/LTAppsAdapter.cpp
    src/adapters/LTFilesAdapter.cpp
    src/adapters/LTCaptureAdapter.cpp
    src/adapters/LTLogAdapter.cpp
    src/adapters/LTCatalogAdapter.cpp
    src/adapters/LTJobsAdapter.cpp
    src/adapters/LTSettingsAdapter.cpp
    src/adapters/LTDeviceIconAdapter.cpp
    src/adapters/LTDisplayAdapter.cpp
    src/adapters/LTStorageAdapter.cpp
    src/adapters/LTProxyAdapter.cpp
    src/adapters/LTDebugPortAdapter.cpp
    src/adapters/LTNotificationsAdapter.cpp
    src/adapters/LTTaskbarAdapter.cpp
    src/items/DisplayItem.cpp)

add_library(ltbridge STATIC src/Bridge.cpp ${LT_ADAPTER_SOURCES})
target_include_directories(ltbridge PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${LT_GENERATED_DIR}")
# No Linux-only APIs in UI code paths: only Qt abstractions (QLibrary etc.).
# _GNU_SOURCE etc. must never be needed here.
target_link_libraries(ltbridge PUBLIC Qt6::Core Qt6::Gui Qt6::Quick PRIVATE Qt6::Widgets)

# --- The Qt app ---------------------------------------------------------------
qt_add_executable(lighttouch src/main.cpp)
target_link_libraries(lighttouch PRIVATE ltbridge Qt6::Quick Qt6::Qml Qt6::Widgets)
# QML resources land under :/qml/ with these aliases (main.qml's relative
# `import "views"` resolves against :/qml/ the same way as on disk).
set_source_files_properties(qml/main.qml PROPERTIES QT_RESOURCE_ALIAS "main.qml")
set_source_files_properties(qml/views/DeviceSidebar.qml PROPERTIES QT_RESOURCE_ALIAS "views/DeviceSidebar.qml")
set_source_files_properties(qml/views/DevicePlaceholder.qml PROPERTIES QT_RESOURCE_ALIAS "views/DevicePlaceholder.qml")
set_source_files_properties(qml/views/DevicePane.qml PROPERTIES QT_RESOURCE_ALIAS "views/DevicePane.qml")
set_source_files_properties(qml/views/BannerStack.qml PROPERTIES QT_RESOURCE_ALIAS "views/BannerStack.qml")
set_source_files_properties(qml/views/DisplayPlaceholder.qml PROPERTIES QT_RESOURCE_ALIAS "views/DisplayPlaceholder.qml")
set_source_files_properties(qml/views/DeviceScreen.qml PROPERTIES QT_RESOURCE_ALIAS "views/DeviceScreen.qml")
set_source_files_properties(qml/views/MainMenuBar.qml PROPERTIES QT_RESOURCE_ALIAS "views/MainMenuBar.qml")
set_source_files_properties(qml/dialogs/DeleteDevicesDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/DeleteDevicesDialog.qml")
set_source_files_properties(qml/dialogs/ConfirmDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/ConfirmDialog.qml")
set_source_files_properties(qml/dialogs/AddDeviceSheet.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/AddDeviceSheet.qml")
set_source_files_properties(qml/dialogs/SettingsDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/SettingsDialog.qml")
set_source_files_properties(qml/dialogs/AboutDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/AboutDialog.qml")
set_source_files_properties(qml/dialogs/LicensesDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/LicensesDialog.qml")
set_source_files_properties(qml/dialogs/HelpWindow.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/HelpWindow.qml")
set_source_files_properties(qml/dialogs/JailbreakDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/JailbreakDialog.qml")
set_source_files_properties(qml/dialogs/NetworkAccessDialog.qml PROPERTIES QT_RESOURCE_ALIAS "dialogs/NetworkAccessDialog.qml")
set_source_files_properties(qml/panes/CarrierPanel.qml PROPERTIES QT_RESOURCE_ALIAS "panes/CarrierPanel.qml")
set_source_files_properties(qml/panes/TweaksPanel.qml PROPERTIES QT_RESOURCE_ALIAS "panes/TweaksPanel.qml")
set_source_files_properties(qml/panes/StorageSettings.qml PROPERTIES QT_RESOURCE_ALIAS "panes/StorageSettings.qml")
set_source_files_properties(qml/panes/ProxySettings.qml PROPERTIES QT_RESOURCE_ALIAS "panes/ProxySettings.qml")
set_source_files_properties(qml/panes/DebugPort.qml PROPERTIES QT_RESOURCE_ALIAS "panes/DebugPort.qml")
set_source_files_properties(qml/panes/CaptureSettings.qml PROPERTIES QT_RESOURCE_ALIAS "panes/CaptureSettings.qml")
set_source_files_properties(qml/views/FilesBrowser.qml PROPERTIES QT_RESOURCE_ALIAS "views/FilesBrowser.qml")
set_source_files_properties(qml/views/LogWindow.qml PROPERTIES QT_RESOURCE_ALIAS "views/LogWindow.qml")
set_source_files_properties(qml/views/LogView.qml PROPERTIES QT_RESOURCE_ALIAS "views/LogView.qml")
set_source_files_properties(qml/views/ConsoleSplit.qml PROPERTIES QT_RESOURCE_ALIAS "views/ConsoleSplit.qml")
set_source_files_properties(qml/views/AppsInspector.qml PROPERTIES QT_RESOURCE_ALIAS "views/AppsInspector.qml")
set_source_files_properties(qml/views/AppRow.qml PROPERTIES QT_RESOURCE_ALIAS "views/AppRow.qml")
set_source_files_properties(qml/views/StoreRow.qml PROPERTIES QT_RESOURCE_ALIAS "views/StoreRow.qml")
set_source_files_properties(qml/views/FirmwareJobsWindow.qml PROPERTIES QT_RESOURCE_ALIAS "views/FirmwareJobsWindow.qml")
set_source_files_properties(qml/views/CatalogDetailsDialog.qml PROPERTIES QT_RESOURCE_ALIAS "views/CatalogDetailsDialog.qml")
set_source_files_properties(qml/views/RecordingButton.qml PROPERTIES QT_RESOURCE_ALIAS "views/RecordingButton.qml")
set_source_files_properties(qml/views/DropHighlight.qml PROPERTIES QT_RESOURCE_ALIAS "views/DropHighlight.qml")
set_source_files_properties(qml/views/CaptureSounds.qml PROPERTIES QT_RESOURCE_ALIAS "views/CaptureSounds.qml")
set_source_files_properties(qml/sounds/shutter.wav PROPERTIES QT_RESOURCE_ALIAS "sounds/shutter.wav")
set_source_files_properties(qml/sounds/record-start.wav PROPERTIES QT_RESOURCE_ALIAS "sounds/record-start.wav")
set_source_files_properties(qml/sounds/record-stop.wav PROPERTIES QT_RESOURCE_ALIAS "sounds/record-stop.wav")
set_source_files_properties(qml/art/iphone.svg PROPERTIES QT_RESOURCE_ALIAS "art/iphone.svg")
set_source_files_properties(qml/art/ipod.svg PROPERTIES QT_RESOURCE_ALIAS "art/ipod.svg")
set_source_files_properties(qml/art/ipad.svg PROPERTIES QT_RESOURCE_ALIAS "art/ipad.svg")
set_source_files_properties(qml/art/device-generic.svg PROPERTIES QT_RESOURCE_ALIAS "art/device-generic.svg")
qt_add_resources(lighttouch "qml" PREFIX "/qml" FILES
    qml/main.qml
    qml/views/DeviceSidebar.qml
    qml/views/DevicePlaceholder.qml
    qml/views/DevicePane.qml
    qml/views/BannerStack.qml
    qml/views/DisplayPlaceholder.qml
    qml/views/DeviceScreen.qml
    qml/views/MainMenuBar.qml
    qml/dialogs/DeleteDevicesDialog.qml
    qml/dialogs/ConfirmDialog.qml
    qml/dialogs/AddDeviceSheet.qml
    qml/dialogs/SettingsDialog.qml
    qml/dialogs/AboutDialog.qml
    qml/dialogs/LicensesDialog.qml
    qml/dialogs/HelpWindow.qml
    qml/dialogs/JailbreakDialog.qml
    qml/dialogs/NetworkAccessDialog.qml
    qml/panes/CarrierPanel.qml
    qml/panes/TweaksPanel.qml
    qml/panes/StorageSettings.qml
    qml/panes/ProxySettings.qml
    qml/panes/DebugPort.qml
    qml/panes/CaptureSettings.qml
    qml/views/FilesBrowser.qml
    qml/views/LogWindow.qml
    qml/views/LogView.qml
    qml/views/ConsoleSplit.qml
    qml/views/AppsInspector.qml
    qml/views/AppRow.qml
    qml/views/StoreRow.qml
    qml/views/FirmwareJobsWindow.qml
    qml/views/CatalogDetailsDialog.qml
    qml/views/RecordingButton.qml
    qml/views/DropHighlight.qml
    qml/views/CaptureSounds.qml
    qml/sounds/shutter.wav
    qml/sounds/record-start.wav
    qml/sounds/record-stop.wav)
# Device artwork (qrc:/art/*.svg) compiles into the ltbridge static lib so the
# app and every test resolve it; LTDeviceIconAdapter's constructor runs
# Q_INIT_RESOURCE(art) (static libs don't auto-register resources).
qt6_add_resources(LT_ART_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/qml/art.qrc")
target_sources(ltbridge PRIVATE ${LT_ART_SOURCES})

# rpath: $ORIGIN (libLTAppCore.so / libLTMockCore.so staged beside the binary)
# plus the Swift runtime dir, so no LD_LIBRARY_PATH is needed in dev.
if(UNIX)
    set_target_properties(lighttouch PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN;${LT_SWIFT_RUNTIME_DIR}")
endif()

# --- SwiftPM targets (real library when it exists; mock stand-in now) ---------
lt_add_swift_appcore(lighttouch)
lt_add_swift_mockcore(lighttouch)

# --- Tests --------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
