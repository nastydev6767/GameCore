cmake_minimum_required(VERSION 3.21)
# ─────────────────────────────────────────────────────────────────────────────
# FILE : CMakeLists.txt   [MODIFIED]
# ─────────────────────────────────────────────────────────────────────────────
project(GameCore
    VERSION 1.0.0
    DESCRIPTION "Gaming Optimization Tool"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(third_party/imgui)

add_subdirectory(src/core)
add_subdirectory(src/scanner)
add_subdirectory(src/monitor)
add_subdirectory(src/detector)
add_subdirectory(src/optimizer)
add_subdirectory(src/thermal)       # ← NEW
add_subdirectory(src/ui)
add_subdirectory(src/app)

# ── Copy data/ and scripts/ to the runtime output directory ──────────────────
# These are needed by thermal_manager at runtime.
add_custom_target(GameCore_CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/scripts/setup_thermal.ps1"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts/setup_thermal.ps1"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/data/thermal_db.json"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thermal_db.json"
    COMMENT "Copying GameCore runtime assets"
)