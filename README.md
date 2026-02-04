# PyPixl

A complete rewrite of the PyPixl camera application using C++, Qt 6, and OpenCV.

## Features
- Fast & Lightweight: Written in C++17.
- Modern UI: Custom Dark Theme using Qt Style Sheets.
- Photo Capture: High-quality snapshots.
- Video Recording: Record .avi videos (MJPG codec).
- Camera Switching: Support for multiple video inputs (Camera 0-4).

## Dependencies
- Qt 6 (Core, Gui, Widgets)
- OpenCV 4 (videoio, core, imgproc, imgcodecs)
- CMake (3.16+)
- C++ Compiler (GCC/Clang/MSVC supporting C++17)

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make
./pypixl
```
