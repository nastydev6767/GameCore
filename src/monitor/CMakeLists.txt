add_library(GameCore_Monitor STATIC
    cpu_usage/cpu_monitor.cpp
    ram_usage/ram_monitor.cpp
    temperatures/temp_monitor.cpp
    fps/fps_counter.cpp
    telemetry/telemetry.cpp
)

target_include_directories(GameCore_Monitor
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Monitor PUBLIC cxx_std_20)

target_compile_options(GameCore_Monitor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_Monitor
    PUBLIC
        GameCore_Core
        GameCore_Scanner
        wbemuuid
        ole32
        oleaut32
        pdh
)