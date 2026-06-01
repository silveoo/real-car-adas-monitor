# Real Car ADAS Monitor

C++ application for vehicle telemetry monitoring and driver state analysis.

## Features

- OBD-II CSV telemetry parser
- Driving style classification
- OpenCV dashboard visualization
- Driver Monitoring System using webcam
- Face and eye detection
- Multithreaded architecture
- Video recording and alert logging
- Google Test unit tests
- Doxygen documentation

## Technology stack

| Technology | Purpose |
|---|---|
| C++17 | Core application |
| CMake | Build system |
| MinGW / MSYS2 UCRT64 | Compiler toolchain |
| OpenCV | Dashboard, camera processing, DMS |
| Google Test | Unit tests |
| ONNX | Model format |
| Python | Dataset generation and model training |

## Build

```bash
mkdir build
cd build
cmake .. -G Ninja
cmake --build .