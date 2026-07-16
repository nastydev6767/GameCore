cmake_minimum_required(VERSION 3.21)
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
add_subdirectory(src/ui)
add_subdirectory(src/app)