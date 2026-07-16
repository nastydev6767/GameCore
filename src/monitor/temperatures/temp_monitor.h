#pragma once

#include <string>

namespace GameCore::Monitor {

struct ThermalStatus {
    double cpuTempCelsius;   // -1.0 if unavailable
    double gpuTempCelsius;   // -1.0 if unavailable
    bool   isThrottling;
    std::string source;      // where data came from
};

class TempMonitor {
public:
    ThermalStatus GetStatus() const;
    static constexpr double SafeMaxCelsius = 90.0;

private:
    // Tier 1: PDH thermal zone counters (works on ~40% of PCs)
    double TryPdh()        const;

    // Tier 2: HWiNFO64 shared memory (works if user has HWiNFO running)
    bool   TryHwinfo(double& cpuTemp, double& gpuTemp) const;

    // Tier 3: WMI ACPI thermal zone (fallback)
    double TryWmiAcpi()    const;

    // Tier 4: WMI MSAcpi (last resort)
    double TryWmiMsAcpi()  const;
};

} // namespace GameCore::Monitor