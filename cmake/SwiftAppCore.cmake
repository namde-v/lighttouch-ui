# SwiftAppCore.cmake — SwiftPM build targets consumed by the Qt shell.
#
# Two libraries can back the Bridge:
#   lt_appcore  — the real libLTAppCore (port/Packages/LTAppCore), built by a
#                 parallel worker. This target is OPT-IN (not part of ALL):
#                 the library is another worker's in-progress artifact and may
#                 not compile yet; the Qt shell must stay green regardless.
#                 Build it explicitly once it does:
#                   cmake --build build --target lt_appcore
#                 then run with LTM_APPCORE_LIB=build/libLTAppCore.so (it is
#                 staged next to the binary) or just ./build/lighttouch.
#   lt_mockcore — LTMockCore, a tiny Swift stand-in exporting the real lt_*
#                 ABI (hand-written interesting functions + generated stubs).
#                 Lets the Qt side exercise the true dlopen/dlsym path before
#                 the real library exists.
#
# Swift runtime shipping plan (see SPIKE_RESULT.md note 1):
#   The Swift runtime lives in <toolchain>/usr/lib/swift/linux. For dev, the
#   app/test targets set INSTALL_RPATH to $ORIGIN plus that dir, and the .so
#   is copied next to the binary by the POST_BUILD step below. Tests additionally
#   get LD_LIBRARY_PATH. For shipping: bundle libswiftCore et al. beside the
#   binary ($ORIGIN covers it) or point rpath at a bundled runtime dir.
#   Windows: same shape with LTAppCore.dll; the Swift toolchain's runtime DLLs
#   ship beside the binary.

# Space-separated list of exports with hand-written mocks (C++ in Bridge.cpp,
# Swift in mockcore/Sources/LTMockCore/MockCore.swift). Everything else gets a
# generated stub. Kept in one place so both emitters agree.
set(LT_MOCK_EXCLUDE_CSV
    "lt_init,lt_shutdown,lt_set_event_callback,lt_free_string,lt_free_event_strings,lt_last_error,lt_abi_version,lt_version,lt_device_list,lt_device_row,lt_device_status,lt_device_find,lt_device_uuid,lt_device_create,lt_device_delete,lt_device_rename,lt_jobs_list,lt_drop_classify,lt_menu_validate,lt_settings_get_string,lt_settings_set_string,lt_catalog_list,lt_about_credits,lt_help_topics,lt_device_boot,lt_device_halt,lt_tweaks_get,lt_carrier_get,lt_storage_get,lt_proxy_get,lt_proxy_set,lt_debug_port_get,lt_debug_port_set,lt_apps_list,lt_apps_install,lt_files_state,lt_files_list,lt_files_transfer,lt_files_cancel,lt_log_sources,lt_log_read,lt_capture_availability,lt_capture_screenshot,lt_capture_record_start,lt_capture_record_stop,lt_capture_record_save,lt_capture_record_discard,lt_capture_prefs_get,lt_capture_prefs_set,lt_catalog_details,lt_job_start,lt_job_cancel,lt_jobs_can_download,lt_drop_route")

function(lt_swift_env_setup)
    if(NOT LT_SWIFT_EXECUTABLE)
        message(FATAL_ERROR "lt_swift_* called without a Swift toolchain")
    endif()
    file(MAKE_DIRECTORY "${LT_SWIFT_TMPDIR}")
    # Exported to the custom commands below via generator-free variables.
    set(LT_SWIFT_ENV "PATH=${LT_SWIFT_TOOLCHAIN_BIN}:$ENV{PATH}" "TMPDIR=${LT_SWIFT_TMPDIR}" PARENT_SCOPE)
endfunction()

function(lt_add_swift_appcore target)
    set(_dir "${CMAKE_SOURCE_DIR}/../port/Packages/LTAppCore")
    if(NOT EXISTS "${_dir}/Package.swift")
        message(STATUS "lt_appcore: ${_dir}/Package.swift not found — skipping real-library build (LTM_BRIDGE_MOCK=1 or LTM_APPCORE_LIB to run)")
        return()
    endif()
    if(NOT LT_SWIFT_EXECUTABLE)
        message(WARNING "lt_appcore: LTAppCore package present but no Swift toolchain found — skipping")
        return()
    endif()
    lt_swift_env_setup()
    set(_lib "${CMAKE_SHARED_LIBRARY_PREFIX}LTAppCore${CMAKE_SHARED_LIBRARY_SUFFIX}")
    # NOTE: deliberately NOT part of ALL. libLTAppCore is another worker's
    # in-progress artifact; a broken Swift build must not fail the Qt shell.
    # Invoke explicitly: cmake --build build --target lt_appcore
    add_custom_target(lt_appcore
        COMMAND ${CMAKE_COMMAND} -E env ${LT_SWIFT_ENV}
                "${LT_SWIFT_EXECUTABLE}" build --package-path "${_dir}" -c release
        COMMENT "SwiftPM: building libLTAppCore (release)"
        VERBATIM)
    message(STATUS "lt_appcore: opt-in target (cmake --build build --target lt_appcore); package at ${_dir}")
    # Wire the produced .so next to the Qt binary so the default
    # app-dir-relative lookup finds it with no env vars.
    add_custom_command(TARGET lt_appcore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_dir}/.build/release/${_lib}"
                "$<TARGET_FILE_DIR:${target}>/${_lib}"
        COMMENT "Staging ${_lib} next to ${target}")
endfunction()

function(lt_add_swift_mockcore target)
    if(NOT LT_SWIFT_EXECUTABLE)
        message(STATUS "lt_mockcore: no Swift toolchain — skipping stand-in library")
        return()
    endif()
    lt_swift_env_setup()
    set(_dir "${CMAKE_CURRENT_SOURCE_DIR}/mockcore")
    set(_gen "${_dir}/Sources/LTMockCore/GeneratedStubs.swift")
    # Regenerate at configure time so the stubs can never drift from lt_bridge.h.
    execute_process(
        COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gen_bridge.py"
                --header "${CMAKE_CURRENT_SOURCE_DIR}/src/lt_bridge.h"
                --mode swift --out "${_gen}"
                --mock-exclude "${LT_MOCK_EXCLUDE_CSV}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "gen_bridge.py (swift) failed")
    endif()
    set(_lib "${CMAKE_SHARED_LIBRARY_PREFIX}LTMockCore${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_target(lt_mockcore ALL
        COMMAND ${CMAKE_COMMAND} -E env ${LT_SWIFT_ENV}
                "${LT_SWIFT_EXECUTABLE}" build --package-path "${_dir}"
        COMMENT "SwiftPM: building libLTMockCore stand-in (debug)"
        VERBATIM)
    add_custom_command(TARGET lt_mockcore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_dir}/.build/debug/${_lib}"
                "$<TARGET_FILE_DIR:${target}>/${_lib}"
        COMMENT "Staging ${_lib} next to ${target}")
    add_dependencies(${target} lt_mockcore)
    # Path for the binding test (ctest ENVIRONMENT below).
    set(LT_MOCKCORE_STAGED "$<TARGET_FILE_DIR:${target}>/${_lib}" PARENT_SCOPE)
endfunction()
