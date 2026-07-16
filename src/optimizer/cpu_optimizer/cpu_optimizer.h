#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>

namespace GameCore::Optimizer {

enum class PowerPlan {
    Balanced,
    HighPerformance,
    UltimatePower,
};

struct CpuOptimizeResult {
    bool        powerPlanChanged;
    PowerPlan   previousPlan;
    PowerPlan   newPlan;
    bool        timerResolutionSet;
    bool        activeCoolingSet;
    bool        previousCoolingActive;
};

class CpuOptimizer {
public:
    CpuOptimizeResult Optimize(bool extremeMode = false);
    void              Restore(const CpuOptimizeResult& previous);

private:
    static bool       SetHighPerformancePlan();
    static bool       SetUltimatePowerPlan();
    static PowerPlan  GetCurrentPlan();
    static bool       SetTimerResolution(UINT resolutionMs);
    static void       RestoreTimerResolution();
    static bool       SetActiveCoolingPolicy(bool active, bool& previousValue);

    UINT previousTimerResolution_ { 0 };
};

} // namespace GameCore::Optimizer