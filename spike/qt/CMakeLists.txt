cmake_minimum_required(VERSION 3.16)
project(lt_spike_qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Quick Qml)

qt_add_executable(spikeapp main.cpp)

target_link_libraries(spikeapp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    ${CMAKE_DL_LIBS}
)
