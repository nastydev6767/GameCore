#include "settings_page.h"
#include "../themes/theme.h"

#include <imgui.h>

namespace GameCore::UI {

void SettingsPage::Render(AppSettings& settings)
{
    ImGui::Spacing();
    ImGui::TextColored(Theme::TextPrimary, "Optimization");
    ImGui::Spacing();

    ImGui::Checkbox("Aggressive optimization (stops more services)",
                    &settings.aggressiveOptimization);
    ImGui::TextColored(Theme::TextSecondary,
        "  Recommended only for low-spec PCs");

    ImGui::Spacing();
    ImGui::Checkbox("Auto-optimize when a game launches outside GameCore",
                    &settings.autoOptimizeBackground);
    ImGui::TextColored(Theme::TextSecondary,
        "  Detects games launched directly (not through GameCore)");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Extreme Mode ──────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Danger);
    ImGui::Text("Extreme Mode");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::Surface);
    ImGui::Checkbox("Enable Extreme Mode", &settings.extremeMode);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 420);
    ImGui::TextColored(Theme::Warning,
        "Warning: This pushes your CPU to maximum performance and tells "
        "Windows to let fans run at full speed instead of throttling the "
        "CPU. Your laptop or PC fans will likely run loud and constant "
        "while a game is active.");
    ImGui::Spacing();
    ImGui::TextColored(Theme::TextSecondary,
        "Note: GameCore cannot directly set exact fan RPM — that requires "
        "manufacturer-specific software (e.g. Armoury Crate, MSI Center). "
        "Extreme Mode only removes Windows' throttling preference so your "
        "existing fans are allowed to ramp up as high as your hardware "
        "permits.");
    ImGui::PopTextWrapPos();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(Theme::TextPrimary, "General");
    ImGui::Spacing();

    ImGui::Checkbox("Minimize to system tray instead of closing",
                    &settings.minimizeToTray);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(Theme::TextSecondary, "GameCore v1.0.0");
    ImGui::TextColored(Theme::TextSecondary, "Free and open source");
}

} // namespace GameCore::UI