add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_win32.cpp
    imgui_impl_dx11.cpp
)

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(imgui PUBLIC cxx_std_17)

target_link_libraries(imgui
    PUBLIC
        d3d11
        dxgi
        d3dcompiler
)