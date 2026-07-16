add_library(GameCore_UI STATIC
    app_window.cpp
    system_tray.cpp
    pages/dashboard_page.cpp
    pages/games_page.cpp
    pages/monitor_page.cpp
    pages/settings_page.cpp
    widgets/speedometer.cpp
    themes/theme.cpp
)

target_include_directories(GameCore_UI
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_UI PUBLIC cxx_std_20)

target_compile_options(GameCore_UI PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_UI
    PUBLIC
        GameCore_Core
        GameCore_Scanner
        GameCore_Monitor
        GameCore_Detector
        GameCore_Optimizer
        imgui
        dwmapi
        comctl32
        Shell32 
)