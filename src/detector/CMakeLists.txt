add_library(GameCore_Detector STATIC
    game_detector/game_detector.cpp
    game_detector/game_db.cpp
)

target_include_directories(GameCore_Detector
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Detector PUBLIC cxx_std_20)

target_compile_options(GameCore_Detector PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_Detector
    PUBLIC
        GameCore_Core
)