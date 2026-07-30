// ─────────────────────────────────────────────────────────────────────────────
// FILE : src/optimizer/optimization_engine/optimization_engine.cpp  [MODIFIED]
// ─────────────────────────────────────────────────────────────────────────────
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
    bool                                            extremeMode)
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

    // ── Step 1: Kill background processes ───────────────────────────────────
    Report(progressCb, 0.12f, "Scanning background processes...");

    auto killed = processOptimizer_.KillBackgroundProcesses();
    snapshot.killedProcesses = killed;
    result.processesKilled   = static_cast<int>(killed.size());

    Report(progressCb, 0.22f,
        "Terminated " + std::to_string(killed.size())
        + " background processes");

    // ── Step 2: Free memory ─────────────────────────────────────────────────
    Report(progressCb, 0.28f, "Freeing system memory...");

    auto memResult       = memoryOptimizer_.FreeMemory();
    snapshot.freedMemoryMb = memResult.freedMb;
    result.memoryFreedMb   = memResult.freedMb;

    Report(progressCb, 0.35f,
        "Freed " + std::to_string(static_cast<int>(memResult.freedMb))
        + " MB of RAM");

    // ── Step 3: CPU power plan ──────────────────────────────────────────────
    Report(progressCb, 0.40f,
        extremeMode
            ? "Maximizing CPU performance (Extreme Mode)..."
            : "Optimizing CPU performance...");

    auto cpuResult        = cpuOptimizer_.Optimize(extremeMode);
    snapshot.cpuChanges   = cpuResult;
    result.cpuBoosted     = cpuResult.timerResolutionSet;
    result.powerPlanChanged = cpuResult.powerPlanChanged;

    Report(progressCb, 0.48f,
        cpuResult.powerPlanChanged
            ? "CPU set to high performance mode"
            : "CPU already at optimal settings");

    // ── Step 4: Registry tweaks ─────────────────────────────────────────────
    Report(progressCb, 0.52f, "Applying Windows registry tweaks...");

    auto regResult = registryOptimizer_.Optimize(extremeMode);
    snapshot.registryTweakResult = regResult;
    result.registryTweaked = regResult.gameDvrDisabled
                          || regResult.mmcssGameProfileSet;

    Report(progressCb, 0.58f,
        result.registryTweaked
            ? "Registry: GameDVR off, MMCSS boosted, core parking disabled"
            : "Registry tweaks skipped (insufficient permissions)");

    // ── Step 5: Network optimization ────────────────────────────────────────
    Report(progressCb, 0.62f, "Optimizing network stack...");

    auto netResult = networkOptimizer_.Optimize(gameName + ".exe");
    snapshot.networkTweakResult = netResult;
    result.networkOptimized = netResult.naggleDisabled
                           || netResult.networkThrottleOff;

    Report(progressCb, 0.68f,
        result.networkOptimized
            ? "Network: Nagle off, DNS flushed, QoS applied"
            : "Network tweaks skipped (insufficient permissions)");

    // ── Step 6: Thermal — maximize all fans silently ─────────────────────────
    // thermal_.Init() is a no-op if already initialised (idempotent).
    // On first call it triggers setup_thermal.ps1 if needed.
    Report(progressCb, 0.72f, "Configuring hardware performance...");

    if (!thermal_.IsReady())
        thermal_.Init();     // silent, blocks only on very first run

    if (thermal_.IsReady()) {
        auto thermalRes = thermal_.Maximize();
        snapshot.thermalResult    = thermalRes;
        result.thermalMaximized   = thermalRes.fansMaximized;
        result.fanControllersFound = thermalRes.fanControllersFound;

        Report(progressCb, 0.80f,
            thermalRes.fansMaximized
                ? "Hardware performance maximized"
                : "Hardware performance mode applied");
    } else {
        Report(progressCb, 0.80f, "Hardware performance: standard mode");
    }

    // ── Step 7: Stop non-essential services ─────────────────────────────────
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
    thermal_.Restore();           // ← restore fans first
    networkOptimizer_.Restore();
    registryOptimizer_.Restore();
    restoreEngine_.RestoreAll();
    GC_LOG_INFO("[Optimizer] System restored.");
}

} // namespace GameCore::Optimizer