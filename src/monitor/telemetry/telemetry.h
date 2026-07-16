#pragma once

#include "monitor/cpu_usage/cpu_monitor.h"
#include "monitor/ram_usage/ram_monitor.h"
#include "monitor/temperatures/temp_monitor.h"
#include "monitor/fps/fps_counter.h"

namespace GameCore::Monitor {

struct TelemetrySnapshot {
    double      cpuUsagePercent;
    double      ramUsedGb;
    double      ramTotalGb;
    double      ramUsagePercent;
    double      cpuTempCelsius;   // -1.0 if unavailable
    double      gpuTempCelsius;   // -1.0 if unavailable
    bool        cpuThrottling;
    double      fps;
    double      frameTimeMs;
    std::string tempSource;       // "HWiNFO64", "PDH", "WMI-ACPI", "unavailable"
};

class Telemetry {
public:
    Telemetry();
    void Update();
    const TelemetrySnapshot& GetSnapshot() const { return snapshot_; }

private:
    CpuMonitor  cpu_;
    RamMonitor  ram_;
    TempMonitor temp_;
    FpsCounter  fps_;
    TelemetrySnapshot snapshot_{};
};

} // namespace GameCore::Monitor