#include "restore_engine.h"
#include "core/logging/logger.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

#include <algorithm>

namespace GameCore::Optimizer {

static const std::vector<std::string> SafeToRelaunch = {
    "onedrive.exe",
    "dropbox.exe",
    "spotify.exe",
};

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

bool RestoreEngine::IsSafeToRelaunch(const std::string& processName)
{
    const std::string lower = ToLower(processName);
    for (const auto& entry : SafeToRelaunch)
        if (lower == ToLower(entry)) return true;
    return false;
}

void RestoreEngine::RelaunchSafeApps(
    const std::vector<ProcessInfo>& killed)
{
    for (const auto& proc : killed) {
        if (!IsSafeToRelaunch(proc.name)) continue;

        HINSTANCE result = ShellExecuteA(nullptr, "open",
            proc.name.c_str(), nullptr, nullptr, SW_SHOWMINNOACTIVE);

        if (reinterpret_cast<INT_PTR>(result) <= 32) {
            GC_LOG_WARNING("[Restore] Could not relaunch " + proc.name);
        } else {
            GC_LOG_INFO("[Restore] Relaunched " + proc.name);
        }
    }
}

void RestoreEngine::SaveSnapshot(const OptimizationSnapshot& snapshot)
{
    snapshot_ = snapshot;
    snapshot_.isActive = true;
}

void RestoreEngine::RestoreAll()
{
    if (!snapshot_.isActive) return;

    cpuOptimizer_.Restore(snapshot_.cpuChanges);
    serviceOptimizer_.RestoreServices(snapshot_.stoppedServices);
    RelaunchSafeApps(snapshot_.killedProcesses);

    snapshot_.isActive = false;
}

} // namespace GameCore::Optimizer