#pragma once

#include "optimizer/process_optimizer/process_optimizer.h"
#include "optimizer/memory_optimizer/memory_optimizer.h"
#include "optimizer/cpu_optimizer/cpu_optimizer.h"
#include "optimizer/service_optimizer/service_optimizer.h"
#include "optimizer/network_optimizer/network_optimizer.h"
#include "optimizer/registry_optimizer/registry_optimizer.h"
#include "optimizer/restore_engine/restore_engine.h"
#include "detector/game_detector/game_db.h"

#include <string>
#include <functional>

namespace GameCore::Optimizer {

using ProgressCallback = std::function<void(float progress,
                                            const std::string& status)>;

struct OptimizationResult {
    int    processesKilled;
    double memoryFreedMb;
    int    servicesStopped;
    bool   cpuBoosted;
    bool   powerPlanChanged;
    bool   extremeModeApplied;
    bool   networkOptimized;
    bool   registryTweaked;
    std::string optimizationProfile;
};

class OptimizationEngine {
public:
    OptimizationResult Optimize(
        const std::string&                            gameName,
        const GameCore::Detector::GameRequirements*   requirements,
        const GameCore::Detector::HardwareCapability*  capability,
        ProgressCallback                               progressCb = nullptr,
        bool                                            extremeMode = false);

    void Restore();

    bool IsOptimized() const { return restoreEngine_.IsActive(); }
    const OptimizationSnapshot& GetSnapshot() const {
        return restoreEngine_.GetSnapshot();
    }

private:
    ProcessOptimizer  processOptimizer_;
    MemoryOptimizer   memoryOptimizer_;
    CpuOptimizer      cpuOptimizer_;
    ServiceOptimizer  serviceOptimizer_;
    NetworkOptimizer  networkOptimizer_;
    RegistryOptimizer registryOptimizer_;
    RestoreEngine     restoreEngine_;

    static void Report(const ProgressCallback& cb,
                       float progress,
                       const std::string& msg);
};

} // namespace GameCore::Optimizer