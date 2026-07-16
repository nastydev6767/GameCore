add_library(GameCore_Core STATIC
    logging/logger.cpp
    config/config.cpp
    events/event_bus.cpp
    utilities/string_utils.cpp
)

target_include_directories(GameCore_Core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Core PUBLIC cxx_std_20)

target_compile_options(GameCore_Core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)