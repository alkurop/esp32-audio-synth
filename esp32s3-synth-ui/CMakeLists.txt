# Root CMakeLists.txt (project/esp32s3-synth-ui/CMakeLists.txt)

cmake_minimum_required(VERSION 3.16)

# ────────────────────────────────────────────────────────────────
# Force C++17 for *all* components in this project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ────────────────────────────────────────────────────────────────

# Now include the ESP-IDF build logic
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# This macro kicks off the rest of the build
project(esp32s3-synth-ui)
