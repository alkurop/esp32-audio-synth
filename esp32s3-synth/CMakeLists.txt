cmake_minimum_required(VERSION 3.16)

# ────────────────────────────────────────────────────────────────
# Force C++17 for *all* components in this project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ──────────────────────────────────────
# Add shared components directory (relative to this project dir)
set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/../common)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3-synth)
