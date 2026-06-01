cmake_minimum_required(VERSION 3.20)

project(RealCarMonitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RealCarMonitor
    src/main.cpp
    src/obd_parser.cpp
    src/onnx_classifier.cpp
    src/dashboard.cpp
    src/dms_monitor.cpp
    src/dms_hud.cpp
)

target_include_directories(RealCarMonitor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)