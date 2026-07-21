add_library(GameCore_Optimizer STATIC
    process_optimizer/process_optimizer.cpp
    memory_optimizer/memory_optimizer.cpp
    cpu_optimizer/cpu_optimizer.cpp
    service_optimizer/service_optimizer.cpp
    network_optimizer/network_optimizer.cpp
    registry_optimizer/registry_optimizer.cpp
    restore_engine/restore_engine.cpp
    optimization_engine/optimization_engine.cpp
)

target_include_directories(GameCore_Optimizer
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Optimizer PUBLIC cxx_std_20)

target_compile_options(GameCore_Optimizer PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_Optimizer
    PUBLIC
        GameCore_Core
        GameCore_Scanner
        GameCore_Detector
        powrprof
        psapi
        shell32
        iphlpapi
        ws2_32
        qwave
        dnsapi
)