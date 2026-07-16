#pragma once

#include <imgui.h>

namespace GameCore::UI {

// Clean Windows-native professional theme
// No neon, no gaming aesthetic — looks like Task Manager / Sysinternals
struct Theme {
    // Apply theme to current ImGui context
    static void Apply();

    // Colors
    static constexpr ImVec4 Background     = { 0.13f, 0.13f, 0.13f, 1.0f };
    static constexpr ImVec4 BackgroundAlt  = { 0.17f, 0.17f, 0.17f, 1.0f };
    static constexpr ImVec4 Surface        = { 0.20f, 0.20f, 0.20f, 1.0f };
    static constexpr ImVec4 SurfaceHover   = { 0.26f, 0.26f, 0.26f, 1.0f };
    static constexpr ImVec4 Border         = { 0.30f, 0.30f, 0.30f, 1.0f };
    static constexpr ImVec4 TextPrimary    = { 0.92f, 0.92f, 0.92f, 1.0f };
    static constexpr ImVec4 TextSecondary  = { 0.60f, 0.60f, 0.60f, 1.0f };
    static constexpr ImVec4 Accent         = { 0.20f, 0.52f, 0.85f, 1.0f }; // Windows blue
    static constexpr ImVec4 AccentHover    = { 0.26f, 0.60f, 0.95f, 1.0f };
    static constexpr ImVec4 Success        = { 0.25f, 0.72f, 0.40f, 1.0f };
    static constexpr ImVec4 Warning        = { 0.90f, 0.65f, 0.10f, 1.0f };
    static constexpr ImVec4 Danger         = { 0.85f, 0.25f, 0.25f, 1.0f };
    static constexpr ImVec4 NavBar         = { 0.11f, 0.11f, 0.11f, 1.0f };
    static constexpr ImVec4 NavActive      = { 0.20f, 0.20f, 0.20f, 1.0f };
    static constexpr ImVec4 NavIndicator   = { 0.20f, 0.52f, 0.85f, 1.0f };
};

} // namespace GameCore::UI