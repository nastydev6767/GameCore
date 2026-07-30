# ─────────────────────────────────────────────────────────────────────────────
# FILE : src/thermal/CMakeLists.txt
# ─────────────────────────────────────────────────────────────────────────────

# ThermalManager — native C++ static lib.
# It LoadLibrary's GameCore.ThermalBridge.dll at runtime; no compile-time
# dependency on the C++/CLI project.

add_library(GameCore_Thermal STATIC
    thermal_manager.cpp
)

target_include_directories(GameCore_Thermal
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(GameCore_Thermal PUBLIC cxx_std_20)

target_compile_options(GameCore_Thermal PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(GameCore_Thermal
    PUBLIC
        GameCore_Core
)

# ── ThermalBridge (C++/CLI DLL) ───────────────────────────────────────────────
# CMake cannot generate /clr projects natively, so we include the pre-authored
# .vcxproj as an external project.  This causes VS to show it in the solution
# explorer and build it as part of the solution.

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    include_external_msproject(
        GameCore.ThermalBridge
        ${CMAKE_CURRENT_SOURCE_DIR}/ThermalBridge/GameCore.ThermalBridge.vcxproj
        TYPE "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"   # vcxproj GUID
    )
    # Make the main app depend on the bridge DLL being built first
    add_dependencies(GameCore_Thermal GameCore.ThermalBridge)
endif()

# ── Copy data and scripts to output directory ─────────────────────────────────
add_custom_command(TARGET GameCore_Thermal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:GameCore_Thermal>/scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/scripts/setup_thermal.ps1"
        "$<TARGET_FILE_DIR:GameCore_Thermal>/scripts/setup_thermal.ps1"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/data/thermal_db.json"
        "$<TARGET_FILE_DIR:GameCore_Thermal>/thermal_db.json"
    COMMENT "Copying thermal scripts and data"
)