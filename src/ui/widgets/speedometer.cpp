#include "speedometer.h"
#include "../themes/theme.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace GameCore::UI {

static constexpr float PI = 3.14159265358979323846f;

void Speedometer::Draw(
    ImVec2 center,
    float radius,
    float value,
    float maxValue,
    const std::string& label,
    const std::string& unit,
    ImVec4 color)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const float startAngle = PI * 0.75f;
    const float endAngle   = PI * 2.25f;

    const float fraction =
        std::clamp(value / std::max(maxValue, 1.0f), 0.0f, 1.0f);

    const float valueAngle =
        startAngle + (endAngle - startAngle) * fraction;

    const ImU32 trackColor =
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

    const ImU32 fillColor =
        ImGui::ColorConvertFloat4ToU32(color);

    const ImU32 textColor =
        ImGui::ColorConvertFloat4ToU32(Theme::TextPrimary);

    constexpr int segments = 64;

    draw->PathArcTo(center, radius, startAngle, endAngle, segments);
    draw->PathStroke(trackColor, 0, 10.0f);

    if (fraction > 0.001f)
    {
        draw->PathArcTo(center, radius, startAngle, valueAngle, segments);
        draw->PathStroke(fillColor, 0, 10.0f);
    }

    char valueText[32];
    std::snprintf(valueText, sizeof(valueText), "%.0f", value);

    ImVec2 valueSize = ImGui::CalcTextSize(valueText);

    draw->AddText(
        ImVec2(
            center.x - valueSize.x * 0.5f,
            center.y - valueSize.y * 0.5f - 8.0f),
        textColor,
        valueText);

    ImVec2 unitSize = ImGui::CalcTextSize(unit.c_str());

    draw->AddText(
        ImVec2(
            center.x - unitSize.x * 0.5f,
            center.y + valueSize.y * 0.5f - 2.0f),
        ImGui::ColorConvertFloat4ToU32(Theme::TextSecondary),
        unit.c_str());

    ImVec2 labelSize = ImGui::CalcTextSize(label.c_str());

    draw->AddText(
        ImVec2(
            center.x - labelSize.x * 0.5f,
            center.y + radius + 10.0f),
        textColor,
        label.c_str());
}

void Speedometer::DrawBar(
    float value,
    float maxValue,
    float width,
    float height,
    ImVec4 color,
    const std::string& label)
{
    ImGui::BeginGroup();

    ImGui::TextColored(Theme::TextSecondary, "%s", label.c_str());

    float fraction =
        std::clamp(value / std::max(maxValue, 1.0f), 0.0f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(fraction, ImVec2(width, height), "");
    ImGui::PopStyleColor();

    ImGui::EndGroup();
}

} // namespace GameCore::UI