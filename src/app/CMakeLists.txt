add_executable(GameCore WIN32
    main.cpp
)

target_compile_features(GameCore PRIVATE cxx_std_20)

target_compile_options(GameCore PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore
    PRIVATE
        GameCore_Core
        GameCore_Scanner
        GameCore_Monitor
        GameCore_Detector
        GameCore_Optimizer
        GameCore_UI
)

# Embed the manifest requesting admin rights + DPI awareness.
# MSVC linker supports this directly via /MANIFESTINPUT.
if (MSVC)
    target_link_options(GameCore PRIVATE
        "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
    )
endif()