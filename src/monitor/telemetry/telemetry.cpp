#include "telemetry.h"

namespace GameCore::Monitor {

Telemetry::Telemetry() = default;

void Telemetry::Update()
{
    fps_.Tick();

    const auto ram     = ram_.GetStatus();
    const auto thermal = temp_.GetStatus();

    snapshot_ = TelemetrySnapshot{
        .cpuUsagePercent = cpu_.GetUsagePercent(),
        .ramUsedGb       = ram.usedGb,
        .ramTotalGb      = ram.totalGb,
        .ramUsagePercent = ram.usagePercent,
        .cpuTempCelsius  = thermal.cpuTempCelsius,
        .gpuTempCelsius  = thermal.gpuTempCelsius,
        .cpuThrottling   = thermal.isThrottling,
        .fps             = fps_.GetFps(),
        .frameTimeMs     = fps_.GetFrameTimeMs(),
        .tempSource      = thermal.source,
    };
}

} // namespace GameCore::Monitor