#include "monitor_page.h"
#include "../themes/theme.h"
#include "../widgets/speedometer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <imgui.h>

namespace GameCore::UI {

void MonitorPage::OpenTaskManager()
{
    ShellExecuteA(nullptr, "open", "taskmgr.exe", nullptr, nullptr, SW_SHOW);
}

void MonitorPage::Render(
    const Monitor::TelemetrySnapshot& snapshot,
    const Detector::DetectedGame*     runningGame,
    StopCallback                      onStop)
{
    ImGui::Spacing();

    if (runningGame) {
        ImGui::TextColored(Theme::TextSecondary, "Currently Playing");
        ImGui::TextColored(Theme::Success, "%s  [RUNNING]",
                           runningGame->name.c_str());
    } else {
        ImGui::TextColored(Theme::TextSecondary,
            "No game running. Launch a game from the Games tab.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    // Speedometer row
    const float dialRadius = 60.0f;
    const float spacing    = 180.0f;
    ImVec2 cursorStart = ImGui::GetCursorScreenPos();

    ImVec2 cpuCenter(cursorStart.x + 80, cursorStart.y + dialRadius + 10);
    Speedometer::Draw(cpuCenter, dialRadius,
        static_cast<float>(snapshot.cpuUsagePercent), 100.0f,
        "CPU", "%", Theme::Accent);

    ImVec2 ramCenter(cursorStart.x + 80 + spacing, cursorStart.y + dialRadius + 10);
    Speedometer::Draw(ramCenter, dialRadius,
        static_cast<float>(snapshot.ramUsagePercent), 100.0f,
        "RAM", "%", Theme::Success);

    ImVec2 fpsCenter(cursorStart.x + 80 + spacing * 2, cursorStart.y + dialRadius + 10);
    Speedometer::Draw(fpsCenter, dialRadius,
        static_cast<float>(snapshot.fps), 240.0f,
        "FPS", "fps", Theme::Warning);

    ImGui::Dummy(ImVec2(0, dialRadius * 2 + 50));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Detail rows
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(Theme::TextSecondary, "CPU Temperature");
    if (snapshot.cpuTempCelsius < 0.0)
        ImGui::TextColored(Theme::TextSecondary, "N/A (%s)",
                           snapshot.tempSource.c_str());
    else {
        ImVec4 tempColor = snapshot.cpuThrottling ? Theme::Danger : Theme::TextPrimary;
        ImGui::TextColored(tempColor, "%.0f C", snapshot.cpuTempCelsius);
    }

    ImGui::NextColumn();

    ImGui::TextColored(Theme::TextSecondary, "RAM Usage");
    ImGui::TextColored(Theme::TextPrimary, "%.1f / %.1f GB",
        snapshot.ramUsedGb, snapshot.ramTotalGb);

    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Open Task Manager", ImVec2(180, 36))) {
        OpenTaskManager();
    }

    ImGui::SameLine();

    if (runningGame) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::Danger);
        if (ImGui::Button("Stop & Restore", ImVec2(160, 36))) {
            if (onStop) onStop();
        }
        ImGui::PopStyleColor();
    }
}

} // namespace GameCore::UI