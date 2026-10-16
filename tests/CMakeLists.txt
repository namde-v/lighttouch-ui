# ui/tests — Qt Test suite, run offscreen via ctest.
#
#   tst_bridge    Bridge in mock mode: init, version, device list, event
#                 round-trip through the real trampoline path, clean failure
#                 on a missing library, and the true dlopen/dlsym binding
#                 against the Swift-built libLTMockCore.so stand-in.
#   tst_adapters  Adapter unit tests against the mock: list population,
#                 event → Q_PROPERTY round-trips, status/prefs/settings.
#   tst_applaunch App-launch smoke test: QML loads offscreen, window creates,
#                 sidebar populates with mock devices, no crash.

function(lt_add_qtest name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE ltbridge Qt6::Quick Qt6::Qml Qt6::Test)
    target_include_directories(${name} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../src")
    target_compile_definitions(${name} PRIVATE
        LT_QML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../qml")
    if(UNIX)
        # So dlopen'd Swift stand-in finds its runtime in tests too.
        set_target_properties(${name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "$ORIGIN;${LT_SWIFT_RUNTIME_DIR}")
    endif()
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LTM_BRIDGE_MOCK=1;LD_LIBRARY_PATH=${LT_SWIFT_RUNTIME_DIR}")
endfunction()

lt_add_qtest(tst_bridge tst_bridge.cpp)
lt_add_qtest(tst_adapters tst_adapters.cpp)
lt_add_qtest(tst_applaunch tst_applaunch.cpp)
lt_add_qtest(tst_surface1 tst_surface1.cpp)
lt_add_qtest(tst_surface10 tst_surface10.cpp)
lt_add_qtest(tst_surface11 tst_surface11.cpp)
lt_add_qtest(tst_surface2 tst_surface2.cpp)
lt_add_qtest(tst_surface3 tst_surface3.cpp)
lt_add_qtest(tst_surface4 tst_surface4.cpp)
lt_add_qtest(tst_surface5 tst_surface5.cpp)
lt_add_qtest(tst_surface6 tst_surface6.cpp)
lt_add_qtest(tst_surface7 tst_surface7.cpp)
lt_add_qtest(tst_surface8 tst_surface8.cpp)
lt_add_qtest(tst_surface9 tst_surface9.cpp)
lt_add_qtest(tst_surface12 tst_surface12.cpp)
lt_add_qtest(tst_surface13 tst_surface13.cpp)
lt_add_qtest(tst_surface14 tst_surface14.cpp)

# The binding subtest inside tst_bridge needs the staged Swift stand-in.
if(TARGET lt_mockcore)
    set_tests_properties(tst_bridge PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LTM_BRIDGE_MOCK=1;LD_LIBRARY_PATH=${LT_SWIFT_RUNTIME_DIR};LT_MOCKCORE_SO=${LT_MOCKCORE_STAGED}")
endif()

