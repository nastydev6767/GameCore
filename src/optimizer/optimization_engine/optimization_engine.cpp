#include "optimization_engine.h"
#include "core/logging/logger.h"

namespace GameCore::Optimizer {

void OptimizationEngine::Report(const ProgressCallback& cb,
                                 float progress,
                                 const std::string& msg)
{
    GC_LOG_INFO("[Optimizer] " + msg);
    if (cb) cb(progress, msg);
}

OptimizationResult OptimizationEngine::Optimize(
    const std::string&                              gameName,
    const GameCore::Detector::GameRequirements*     requirements,
    const GameCore::Detector::HardwareCapability*   capability,
    ProgressCallback                                progressCb,
    bool                                             extremeMode)
{
    OptimizationResult result{};
    OptimizationSnapshot snapshot{};

    std::string profile = "balanced";
    if (capability)
        profile = capability->optimizationProfile;

    result.optimizationProfile = profile;
    result.extremeModeApplied  = extremeMode;

    Report(progressCb, 0.05f,
        "Starting optimization for " + gameName + "...");

    // ── Step 1: Kill background processes ───────────────────────────
    Report(progressCb, 0.15f, "Scanning background processes...");

    auto killed = processOptimizer_.KillBackgroundProcesses();
    snapshot.killedProcesses = killed;
    result.processesKilled   = static_cast<int>(killed.size());

    Report(progressCb, 0.35f,
        "Terminated " + std::to_string(killed.size())
        + " background processes");

    // ── Step 2: Free memory ──────────────────────────────────────────
    Report(progressCb, 0.45f, "Freeing system memory...");

    auto memResult = memoryOptimizer_.FreeMemory();
    snapshot.freedMemoryMb  = memResult.freedMb;
    result.memoryFreedMb    = memResult.freedMb;

    Report(progressCb, 0.55f,
        "Freed " + std::to_string(static_cast<int>(memResult.freedMb))
        + " MB of RAM");

    // ── Step 3: CPU power plan (+ Extreme Mode cooling policy) ──────
    Report(progressCb, 0.65f,
        extremeMode
            ? "Maximizing CPU performance (Extreme Mode)..."
            : "Optimizing CPU performance...");

    auto cpuResult = cpuOptimizer_.Optimize(extremeMode);
    snapshot.cpuChanges     = cpuResult;
    result.cpuBoosted       = cpuResult.timerResolutionSet;
    result.powerPlanChanged = cpuResult.powerPlanChanged;

    if (extremeMode && cpuResult.activeCoolingSet) {
        Report(progressCb, 0.72f,
            "Fans set to ramp freely — expect more noise");
    }

    Report(progressCb, 0.75f,
        cpuResult.powerPlanChanged
            ? "CPU set to high performance mode"
            : "CPU already at optimal settings");

    // ── Step 4: Registry tweaks ──────────────────────────────────────
    Report(progressCb, 0.60f, "Applying Windows registry tweaks...");

    auto regResult = registryOptimizer_.Optimize(extremeMode);
    snapshot.registryTweakResult = regResult;
    result.registryTweaked = regResult.gameDvrDisabled
                          || regResult.mmcssGameProfileSet;

    Report(progressCb, 0.68f,
        result.registryTweaked
            ? "Registry: GameDVR off, MMCSS boosted, core parking disabled"
            : "Registry tweaks skipped (insufficient permissions)");

    // ── Step 5: Network optimization ────────────────────────────────
    Report(progressCb, 0.72f, "Optimizing network stack...");

    auto netResult = networkOptimizer_.Optimize(gameName + ".exe");
    snapshot.networkTweakResult = netResult;
    result.networkOptimized = netResult.naggleDisabled
                           || netResult.networkThrottleOff;

    Report(progressCb, 0.78f,
        result.networkOptimized
            ? "Network: Nagle off, DNS flushed, QoS policy applied"
            : "Network tweaks skipped (insufficient permissions)");

    // ── Step 6: Stop services (aggressive/balanced/extreme) ──────────
    if (profile == "aggressive" || profile == "balanced" || extremeMode) {
        Report(progressCb, 0.85f,
            "Pausing non-essential Windows services...");

        auto stopped = serviceOptimizer_.StopNonEssentialServices();
        snapshot.stoppedServices = stopped;
        result.servicesStopped   = static_cast<int>(stopped.size());

        Report(progressCb, 0.93f,
            "Paused " + std::to_string(stopped.size())
            + " background services");
    }

    snapshot.isActive = true;
    restoreEngine_.SaveSnapshot(snapshot);

    Report(progressCb, 1.0f,
        "Optimization complete. Launching " + gameName + "...");

    return result;
}

void OptimizationEngine::Restore()
{
    if (!restoreEngine_.IsActive()) return;

    GC_LOG_INFO("[Optimizer] Restoring system to pre-game state...");
    networkOptimizer_.Restore();
    registryOptimizer_.Restore();
    restoreEngine_.RestoreAll();
    GC_LOG_INFO("[Optimizer] System restored.");
}

} // namespace GameCore::Optimizer