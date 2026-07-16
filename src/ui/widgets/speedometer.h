#pragma once

#include <imgui.h>
#include <string>

namespace GameCore::UI {

class Speedometer {
public:
    // Draw circular speedometer dial
    // center: center position in window
    // radius: dial radius in pixels
    // value: current value (0.0 - maxValue)
    // maxValue: maximum value
    // label: text shown below dial
    // unit: text shown inside dial (e.g. "%", "MB")
    static void Draw(
        ImVec2             center,
        float              radius,
        float              value,
        float              maxValue,
        const std::string& label,
        const std::string& unit,
        ImVec4             color);

    // Draw a simple horizontal bar
    static void DrawBar(
        float              value,
        float              maxValue,
        float              width,
        float              height,
        ImVec4             color,
        const std::string& label);
};

} // namespace GameCore::UI