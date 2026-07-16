add_library(GameCore_Scanner STATIC
    system_scanner.cpp
    cpu/cpu_scanner.cpp
    gpu/gpu_scanner.cpp
    ram/ram_scanner.cpp
    windows/windows_scanner.cpp
)

target_include_directories(GameCore_Scanner
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Scanner PUBLIC cxx_std_20)

target_compile_options(GameCore_Scanner PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_Scanner
    PUBLIC
        wbemuuid
        ole32
        oleaut32
)