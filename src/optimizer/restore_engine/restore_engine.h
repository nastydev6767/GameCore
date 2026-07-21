#pragma once

#include "optimizer/process_optimizer/process_optimizer.h"
#include "optimizer/cpu_optimizer/cpu_optimizer.h"
#include "optimizer/service_optimizer/service_optimizer.h"
#include "optimizer/network_optimizer/network_optimizer.h"
#include "optimizer/registry_optimizer/registry_optimizer.h"

#include <string>
#include <vector>

namespace GameCore::Optimizer {

struct OptimizationSnapshot {
    std::vector<ProcessInfo> killedProcesses;
    std::vector<ServiceInfo> stoppedServices;
    CpuOptimizeResult        cpuChanges;
    NetworkTweakResult       networkTweakResult;
    RegistryTweakResult      registryTweakResult;
    double                   freedMemoryMb;
    bool                     isActive { false };
};

class RestoreEngine {
public:
    void SaveSnapshot(const OptimizationSnapshot& snapshot);
    void RestoreAll();

    bool IsActive() const { return snapshot_.isActive; }
    const OptimizationSnapshot& GetSnapshot() const { return snapshot_; }

private:
    OptimizationSnapshot snapshot_;
    CpuOptimizer         cpuOptimizer_;
    ServiceOptimizer     serviceOptimizer_;

    // Relaunches apps that are safe to silently restart
    // (browsers, sync clients) — NOT all killed processes, since
    // some (installers, one-off tools) shouldn't be auto-reopened.
    void RelaunchSafeApps(const std::vector<ProcessInfo>& killed);
    static bool IsSafeToRelaunch(const std::string& processName);
};

} // namespace GameCore::Optimizer