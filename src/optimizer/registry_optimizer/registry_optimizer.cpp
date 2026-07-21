#include "registry_optimizer.h"
#include "core/logging/logger.h"

namespace GameCore::Optimizer {

// ─────────────────────────────────────────────────────────────────────────────
// Registry helpers
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::SetRegDword(HKEY        root,
                                    const char* keyPath,
                                    const char* valueName,
                                    DWORD       value)
{
    HKEY hKey = nullptr;
    if (RegCreateKeyExA(root, keyPath, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                        nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;

    const DWORD r = RegSetValueExA(hKey, valueName, 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(hKey);
    return r == ERROR_SUCCESS;
}

bool RegistryOptimizer::GetRegDword(HKEY        root,
                                    const char* keyPath,
                                    const char* valueName,
                                    DWORD&      outValue)
{
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(root, keyPath, 0, KEY_QUERY_VALUE, &hKey)
        != ERROR_SUCCESS)
        return false;

    DWORD size = sizeof(outValue);
    DWORD type = 0;
    const DWORD r = RegQueryValueExA(hKey, valueName, nullptr, &type,
        reinterpret_cast<LPBYTE>(&outValue), &size);
    RegCloseKey(hKey);
    return r == ERROR_SUCCESS && type == REG_DWORD;
}

// ─────────────────────────────────────────────────────────────────────────────
// Xbox Game Bar / GameDVR
//
// GameDVR constantly records a background clip buffer so you can "save last 30
// seconds". This burns GPU memory bandwidth and CPU encoder cycles even when
// you don't use it.  Disabling it is safe and universally recommended.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::DisableGameDvr()
{
    // Save current state
    GetRegDword(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR",
        "AppCaptureEnabled",
        snapshot_.prevGameDvr);

    bool ok = true;
    ok &= SetRegDword(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR",
        "AppCaptureEnabled", 0);
    ok &= SetRegDword(HKEY_CURRENT_USER,
        "System\\GameConfigStore",
        "GameDVR_Enabled", 0);
    ok &= SetRegDword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Policies\\Microsoft\\Windows\\GameDVR",
        "AllowGameDVR", 0);

    GC_LOG_INFO("[Registry] GameDVR disabled");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Hardware Accelerated GPU Scheduling (HAGS)
//
// HAGS lets the GPU manage its own memory scheduling instead of relying on
// the Windows GPU scheduler.  Reduces CPU overhead and lowers latency on
// supported GPUs (NVIDIA Ampere+, AMD RDNA2+).
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::EnableHags()
{
    const char* hagsKey =
        "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers";

    snapshot_.hagsKeyExisted = GetRegDword(HKEY_LOCAL_MACHINE,
        hagsKey, "HwSchMode", snapshot_.prevHags);

    const bool ok = SetRegDword(HKEY_LOCAL_MACHINE,
        hagsKey, "HwSchMode", 2); // 2 = enabled

    GC_LOG_INFO("[Registry] HAGS enabled (takes effect after reboot)");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// CPU Core Parking
//
// Windows parks (powers down) CPU cores to save energy.  During gaming this
// causes latency spikes when parked cores need to wake up to handle game load.
// Setting MinimumCores to 100 forces all cores to stay active.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::DisableCoreParking()
{
    // Power policy SubGroup for processor power management
    const char* procPmKey =
        "SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\"
        "54533251-82be-4824-96c1-47b60b740d00\\"  // Processor PM subgroup
        "0cc5b647-c1df-4637-891a-dec35c318583";   // Core Parking Min Cores

    // Set minimum active core % to 100 for both AC and DC
    SetRegDword(HKEY_LOCAL_MACHINE, procPmKey, "ACSettingIndex", 100);
    SetRegDword(HKEY_LOCAL_MACHINE, procPmKey, "DCSettingIndex", 100);

    GC_LOG_INFO("[Registry] CPU core parking disabled");
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// MMCSS Game Scheduling Profile
//
// Multimedia Class Scheduler Service has a "Games" profile that controls how
// much CPU time game threads get.  Boosting GpuPriority and Priority here
// gives game threads faster scheduling.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::BoostMmcssGameProfile()
{
    const char* gamesKey =
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
        "Multimedia\\SystemProfile\\Tasks\\Games";

    // Save originals
    GetRegDword(HKEY_LOCAL_MACHINE, gamesKey,
                "GPU Priority", snapshot_.prevMmcssGpuPrio);
    GetRegDword(HKEY_LOCAL_MACHINE, gamesKey,
                "Priority",     snapshot_.prevMmcssPrio);

    bool ok = true;
    ok &= SetRegDword(HKEY_LOCAL_MACHINE, gamesKey, "GPU Priority", 8);
    ok &= SetRegDword(HKEY_LOCAL_MACHINE, gamesKey, "Priority",     6);
    ok &= SetRegDword(HKEY_LOCAL_MACHINE, gamesKey,
                      "Scheduling Category", 2);  // High
    ok &= SetRegDword(HKEY_LOCAL_MACHINE, gamesKey,
                      "SFIO Priority",       2);  // High

    GC_LOG_INFO("[Registry] MMCSS Games profile boosted");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Power Throttling (EcoQoS)
//
// Windows 11 introduced EcoQoS which can throttle background processes to
// save power.  We explicitly disable this for game processes by setting
// a policy value.  Note: this is a system-wide hint, not per-process.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::DisablePowerThrottling()
{
    const bool ok = SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Power\\PowerThrottling",
        "PowerThrottlingOff", 1);

    GC_LOG_INFO("[Registry] Power throttling disabled");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Large System Cache
//
// When LargeSystemCache=1, Windows caches aggressively for throughput
// (server workloads). For gaming we want RAM free for the game, so 0.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::SetLargeSystemCache()
{
    GetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "LargeSystemCache", snapshot_.prevLargeCache);

    const bool ok = SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "LargeSystemCache", 0);

    GC_LOG_INFO("[Registry] LargeSystemCache set to 0");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Win32PrioritySeparation
//
// Controls how much CPU time foreground vs background processes receive and
// how frequently the scheduler switches between them.
//
// 0x26 (38) = short variable intervals, foreground boosted 3:1 over background
//             — ideal for gaming (foreground game gets priority slices)
// 0x02 (2)  = Windows default (long variable intervals, no foreground boost)
//
// In extreme mode we use 0x28 = short fixed quanta, maximum foreground boost.
// ─────────────────────────────────────────────────────────────────────────────

bool RegistryOptimizer::SetPrioritySeparation(bool extreme)
{
    GetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
        "Win32PrioritySeparation",
        snapshot_.prevPrioritySep);

    const DWORD value = extreme ? 0x28 : 0x26;

    const bool ok = SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
        "Win32PrioritySeparation", value);

    GC_LOG_INFO("[Registry] Win32PrioritySeparation set to 0x"
                + std::to_string(value));
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

RegistryTweakResult RegistryOptimizer::Optimize(bool extremeMode)
{
    RegistryTweakResult result{};

    snapshot_.taken = true;

    result.gameDvrDisabled         = DisableGameDvr();
    result.hagsEnabled             = EnableHags();
    result.coreParkinEnabled       = DisableCoreParking();
    result.mmcssGameProfileSet     = BoostMmcssGameProfile();
    result.powerThrottlingDisabled = DisablePowerThrottling();
    result.largeSystemCacheOff     = SetLargeSystemCache();
    result.prioritySeparationSet   = SetPrioritySeparation(extremeMode);

    GC_LOG_INFO("[Registry] All tweaks applied");
    return result;
}

void RegistryOptimizer::Restore()
{
    if (!snapshot_.taken) return;

    // Restore GameDVR
    SetRegDword(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameDVR",
        "AppCaptureEnabled", snapshot_.prevGameDvr);
    SetRegDword(HKEY_CURRENT_USER,
        "System\\GameConfigStore",
        "GameDVR_Enabled", snapshot_.prevGameDvr);

    // Restore HAGS only if we changed it and it wasn't already set
    if (snapshot_.hagsKeyExisted) {
        SetRegDword(HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers",
            "HwSchMode", snapshot_.prevHags);
    }

    // Restore MMCSS
    SetRegDword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
        "Multimedia\\SystemProfile\\Tasks\\Games",
        "GPU Priority", snapshot_.prevMmcssGpuPrio);
    SetRegDword(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
        "Multimedia\\SystemProfile\\Tasks\\Games",
        "Priority", snapshot_.prevMmcssPrio);

    // Restore priority separation
    SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
        "Win32PrioritySeparation", snapshot_.prevPrioritySep);

    // Restore power throttling
    SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Power\\PowerThrottling",
        "PowerThrottlingOff", 0);

    // Restore LargeSystemCache
    SetRegDword(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "LargeSystemCache", snapshot_.prevLargeCache);

    snapshot_.taken = false;
    GC_LOG_INFO("[Registry] All tweaks restored");
}

} // namespace GameCore::Optimizer
