cmake_minimum_required(VERSION 3.20)

project(RealCarMonitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "C:/msys64/ucrt64/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)

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

target_link_libraries(RealCarMonitor PRIVATE
    ${OpenCV_LIBS}
)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(RunTests
    tests/test_obd_parser.cpp
    tests/test_dms_monitor.cpp
    src/obd_parser.cpp
    src/dms_monitor.cpp
)

target_include_directories(RunTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(RunTests PRIVATE
    GTest::gtest_main
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(RunTests)