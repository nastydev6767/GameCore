#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>

namespace GameCore::Optimizer {

struct RegistryTweakResult {
    bool gameDvrDisabled;          // Xbox Game Bar / GameDVR recording off
    bool hagsEnabled;              // Hardware Accelerated GPU Scheduling on
    bool coreParkinEnabled;        // CPU core parking disabled during gaming
    bool mmcssGameProfileSet;      // MMCSS "Games" scheduling profile boosted
    bool fsoEnabled;               // Fullscreen Optimizations forced on
    bool powerThrottlingDisabled;  // EcoQoS / power throttling off for games
    bool largeSystemCacheOff;      // LargeSystemCache=0 (frees RAM for apps)
    bool prioritySeparationSet;    // Win32PrioritySeparation tuned for gaming
};

struct RegistrySnapshot {
    DWORD prevGameDvr         { 1 };
    DWORD prevHags            { 0 };
    DWORD prevMmcssGpuPrio    { 8 };
    DWORD prevMmcssPrio       { 2 };
    DWORD prevPrioritySep     { 2 };
    DWORD prevLargeCache      { 0 };
    bool  hagsKeyExisted      { false };
    bool  taken               { false };
};

class RegistryOptimizer {
public:
    RegistryTweakResult Optimize(bool extremeMode = false);
    void                Restore();

private:
    bool DisableGameDvr();
    bool EnableHags();
    bool DisableCoreParking();
    bool BoostMmcssGameProfile();
    bool DisablePowerThrottling();
    bool SetLargeSystemCache();
    bool SetPrioritySeparation(bool extreme);

    static bool SetRegDword(HKEY        root,
                            const char* keyPath,
                            const char* valueName,
                            DWORD       value);
    static bool GetRegDword(HKEY        root,
                            const char* keyPath,
                            const char* valueName,
                            DWORD&      outValue);

    RegistrySnapshot snapshot_;
};

} // namespace GameCore::Optimizer
