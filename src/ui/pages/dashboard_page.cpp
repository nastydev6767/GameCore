#include "dashboard_page.h"
#include "../themes/theme.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <imgui.h>
#include <Shellapi.h>

#include <imgui.h>
namespace GameCore::UI {

void DashboardPage::OpenTaskManager()
{
    ShellExecuteA(nullptr, "open", "taskmgr.exe", nullptr, nullptr, SW_SHOW);
}

void DashboardPage::RenderHardwareCard(const char* title,
                                       const char* value,
                                       const char* subValue)
{
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Surface);
    ImGui::BeginChild(title, ImVec2(220, 90), true,
                      ImGuiWindowFlags_NoScrollbar);

    ImGui::TextColored(Theme::TextSecondary, "%s", title);
    ImGui::Spacing();
    ImGui::TextColored(Theme::TextPrimary, "%s", value);
    if (subValue && subValue[0] != '\0') {
        ImGui::TextColored(Theme::TextSecondary, "%s", subValue);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
}

void DashboardPage::Render(const Scanner::SystemInfo& info,
                          const Monitor::TelemetrySnapshot& snapshot)
{
    ImGui::Spacing();
    ImGui::TextColored(Theme::TextPrimary, "Your System");
    ImGui::Spacing();
    ImGui::Spacing();

    // Hardware cards row
    RenderHardwareCard("CPU", info.cpuName.c_str(), "");
    ImGui::SameLine();

    char ramText[64];
    snprintf(ramText, sizeof(ramText), "%.1f GB", info.ramGb);
    RenderHardwareCard("RAM", ramText, "");
    ImGui::SameLine();

    RenderHardwareCard("GPU", info.gpuName.c_str(), "");

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(Theme::TextSecondary, "Operating System");
    ImGui::TextColored(Theme::TextPrimary, "%s", info.windowsVersion.c_str());

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Live mini stats
    ImGui::TextColored(Theme::TextSecondary, "Current Usage");
    ImGui::Spacing();

    ImGui::Text("CPU");
    ImGui::SameLine(100);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Theme::Accent);
    ImGui::ProgressBar(
        static_cast<float>(snapshot.cpuUsagePercent) / 100.0f,
        ImVec2(200, 18));
    ImGui::PopStyleColor();

    ImGui::Text("RAM");
    ImGui::SameLine(100);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Theme::Success);
    ImGui::ProgressBar(
        static_cast<float>(snapshot.ramUsagePercent) / 100.0f,
        ImVec2(200, 18));
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Open Task Manager", ImVec2(200, 36))) {
        OpenTaskManager();
    }

    ImGui::SameLine();
    ImGui::TextColored(Theme::TextSecondary,
        "  Compare these numbers to verify GameCore's readings");
}

} // namespace GameCore::UI