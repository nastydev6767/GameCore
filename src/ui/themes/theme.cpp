#include "theme.h"

namespace GameCore::UI {

void Theme::Apply()
{
    ImGuiStyle& s = ImGui::GetStyle();

    // Rounding — subtle, not aggressive
    s.WindowRounding    = 6.0f;
    s.FrameRounding     = 4.0f;
    s.ScrollbarRounding = 4.0f;
    s.GrabRounding      = 4.0f;
    s.TabRounding       = 4.0f;
    s.ChildRounding     = 4.0f;
    s.PopupRounding     = 4.0f;

    // Padding — generous, readable
    s.WindowPadding     = { 14.0f, 14.0f };
    s.FramePadding      = { 10.0f,  6.0f };
    s.ItemSpacing       = { 10.0f,  8.0f };
    s.ItemInnerSpacing  = {  6.0f,  6.0f };
    s.IndentSpacing     = 20.0f;
    s.ScrollbarSize     = 12.0f;
    s.GrabMinSize       = 10.0f;

    // Borders
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 0.0f;
    s.PopupBorderSize   = 1.0f;

    // Colors
    ImVec4* c = s.Colors;

    c[ImGuiCol_WindowBg]             = Background;
    c[ImGuiCol_ChildBg]              = BackgroundAlt;
    c[ImGuiCol_PopupBg]              = Surface;
    c[ImGuiCol_Border]               = Border;
    c[ImGuiCol_BorderShadow]         = { 0,0,0,0 };

    c[ImGuiCol_Text]                 = TextPrimary;
    c[ImGuiCol_TextDisabled]         = TextSecondary;

    c[ImGuiCol_FrameBg]              = Surface;
    c[ImGuiCol_FrameBgHovered]       = SurfaceHover;
    c[ImGuiCol_FrameBgActive]        = SurfaceHover;

    c[ImGuiCol_TitleBg]              = NavBar;
    c[ImGuiCol_TitleBgActive]        = NavBar;
    c[ImGuiCol_TitleBgCollapsed]     = NavBar;
    c[ImGuiCol_MenuBarBg]            = NavBar;

    c[ImGuiCol_ScrollbarBg]          = Background;
    c[ImGuiCol_ScrollbarGrab]        = Border;
    c[ImGuiCol_ScrollbarGrabHovered] = TextSecondary;
    c[ImGuiCol_ScrollbarGrabActive]  = Accent;

    c[ImGuiCol_CheckMark]            = Accent;
    c[ImGuiCol_SliderGrab]           = Accent;
    c[ImGuiCol_SliderGrabActive]     = AccentHover;

    c[ImGuiCol_Button]               = Surface;
    c[ImGuiCol_ButtonHovered]        = SurfaceHover;
    c[ImGuiCol_ButtonActive]         = Accent;

    c[ImGuiCol_Header]               = Surface;
    c[ImGuiCol_HeaderHovered]        = SurfaceHover;
    c[ImGuiCol_HeaderActive]         = Accent;

    c[ImGuiCol_Separator]            = Border;
    c[ImGuiCol_SeparatorHovered]     = Accent;
    c[ImGuiCol_SeparatorActive]      = AccentHover;

    c[ImGuiCol_ResizeGrip]           = Border;
    c[ImGuiCol_ResizeGripHovered]    = Accent;
    c[ImGuiCol_ResizeGripActive]     = AccentHover;

    c[ImGuiCol_Tab]                  = NavBar;
    c[ImGuiCol_TabHovered]           = SurfaceHover;
    c[ImGuiCol_TabActive]            = NavActive;
    c[ImGuiCol_TabUnfocused]         = NavBar;
    c[ImGuiCol_TabUnfocusedActive]   = Surface;

    c[ImGuiCol_PlotLines]            = Accent;
    c[ImGuiCol_PlotLinesHovered]     = AccentHover;
    c[ImGuiCol_PlotHistogram]        = Accent;
    c[ImGuiCol_PlotHistogramHovered] = AccentHover;

    c[ImGuiCol_TableHeaderBg]        = NavBar;
    c[ImGuiCol_TableBorderStrong]    = Border;
    c[ImGuiCol_TableBorderLight]     = { 0.22f,0.22f,0.22f,1.0f };
    c[ImGuiCol_TableRowBg]           = { 0,0,0,0 };
    c[ImGuiCol_TableRowBgAlt]        = { 1,1,1,0.03f };

    c[ImGuiCol_NavHighlight]         = Accent;
    c[ImGuiCol_NavWindowingHighlight]= Accent;
    c[ImGuiCol_NavWindowingDimBg]    = { 0,0,0,0.4f };
    c[ImGuiCol_ModalWindowDimBg]     = { 0,0,0,0.4f };
}

} // namespace GameCore::UI