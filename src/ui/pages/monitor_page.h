#pragma once

#include "monitor/telemetry/telemetry.h"
#include "detector/game_detector/game_detector.h"

#include <functional>

namespace GameCore::UI {

class MonitorPage {
public:
    using StopCallback = std::function<void()>;

    void Render(
        const Monitor::TelemetrySnapshot& snapshot,
        const Detector::DetectedGame*     runningGame,
        StopCallback                      onStop);

private:
    void OpenTaskManager();
};

} // namespace GameCore::UI