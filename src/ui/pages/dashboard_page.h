#pragma once

#include "scanner/system_scanner.h"
#include "monitor/telemetry/telemetry.h"

namespace GameCore::UI {

class DashboardPage {
public:
    void Render(const Scanner::SystemInfo& info,
               const Monitor::TelemetrySnapshot& snapshot);

private:
    void RenderHardwareCard(const char* title,
                            const char* value,
                            const char* subValue);
    void OpenTaskManager();
};

} // namespace GameCore::UI