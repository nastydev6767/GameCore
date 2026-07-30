// FILE : src/thermal/thermal_manager.cpp
#include "thermal_manager.h"
#include "core/logging/logger.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace GameCore::Thermal {

template<typename T>
T ThermalManager::Resolve(const char* name)
{
    if (!bridgeDll_) return nullptr;
    auto ptr = reinterpret_cast<T>(GetProcAddress(bridgeDll_, name));
    if (!ptr)
        GC_LOG_WARNING("[Thermal] Missing export: " + std::string(name));
    return ptr;
}

bool ThermalManager::EnsureSetup()
{
    char exePath[MAX_PATH]{};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path exeDir    = fs::path(exePath).parent_path();
    fs::path flagPath  = exeDir / kReadyFlag;
    fs::path scriptPath = exeDir / kSetupScript;
    fs::path bridgePath = exeDir / kBridgeDll;

    if (fs::exists(bridgePath) && fs::exists(flagPath))
        return true;

    if (!fs::exists(scriptPath)) {
        GC_LOG_WARNING("[Thermal] setup_thermal.ps1 not found — thermal features disabled");
        return false;
    }

    GC_LOG_INFO("[Thermal] First-run setup — configuring performance hardware access...");

    std::string cmd =
        "powershell.exe -NoProfile -NonInteractive "
        "-ExecutionPolicy Bypass -WindowStyle Hidden -File \""
        + scriptPath.string() + "\"";

    STARTUPINFOA si{};
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(nullptr, const_cast<char*>(cmd.c_str()),
                        nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        GC_LOG_WARNING("[Thermal] Failed to launch setup script");
        return false;
    }

    WaitForSingleObject(pi.hProcess, 60'000);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0) {
        GC_LOG_WARNING("[Thermal] Setup script exited with code "
                    + std::to_string(exitCode));
        return false;
    }

    GC_LOG_INFO("[Thermal] Setup complete");
    return fs::exists(bridgePath);
}

bool ThermalManager::LoadBridge()
{
    char exePath[MAX_PATH]{};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path bridgePath = fs::path(exePath).parent_path() / kBridgeDll;

    bridgeDll_ = LoadLibraryA(bridgePath.string().c_str());
    if (!bridgeDll_) {
        GC_LOG_WARNING("[Thermal] Cannot load ThermalBridge.dll ("
                    + std::to_string(GetLastError()) + ")");
        return false;
    }

    fn_Init_      = Resolve<FnInit>       ("GC_Thermal_Init");
    fn_CpuTemp_   = Resolve<FnPollCpuTemp>("GC_Thermal_CpuTemp");
    fn_GpuTemp_   = Resolve<FnPollGpuTemp>("GC_Thermal_GpuTemp");
    fn_CpuFan_    = Resolve<FnPollCpuFan> ("GC_Thermal_CpuFanRpm");
    fn_GpuFan_    = Resolve<FnPollGpuFan> ("GC_Thermal_GpuFanRpm");
    fn_CpuLoad_   = Resolve<FnPollCpuLoad>("GC_Thermal_CpuLoad");
    fn_GpuLoad_   = Resolve<FnPollGpuLoad>("GC_Thermal_GpuLoad");
    fn_GpuVram_   = Resolve<FnPollVram>   ("GC_Thermal_GpuVramUsedMb");
    fn_Maximize_  = Resolve<FnMaximize>   ("GC_Thermal_MaximizeAll");
    fn_Restore_   = Resolve<FnRestore>    ("GC_Thermal_RestoreAll");
    fn_GetMfr_    = Resolve<FnGetMfr>     ("GC_Thermal_GetManufacturer");
    fn_GetMethod_ = Resolve<FnGetMethod>  ("GC_Thermal_GetMethod");
    fn_Shutdown_  = Resolve<FnShutdown>   ("GC_Thermal_Shutdown");

    if (!fn_Init_ || !fn_Maximize_ || !fn_Restore_) {
        GC_LOG_WARNING("[Thermal] ThermalBridge.dll missing required exports");
        FreeLibrary(bridgeDll_);
        bridgeDll_ = nullptr;
        return false;
    }

    GC_LOG_INFO("[Thermal] ThermalBridge.dll loaded");
    return true;
}

ThermalManager::ThermalManager()  = default;

ThermalManager::~ThermalManager()
{
    if (bridgeLoaded_ && fn_Shutdown_)
        fn_Shutdown_();
    if (bridgeDll_)
        FreeLibrary(bridgeDll_);
}

bool ThermalManager::Init()
{
    if (bridgeLoaded_) return true;   // idempotent

    if (!EnsureSetup()) {
        GC_LOG_WARNING("[Thermal] Setup failed — running without thermal control");
        return false;
    }

    if (!LoadBridge())
        return false;

    if (!fn_Init_()) {
        GC_LOG_WARNING("[Thermal] LHM initialisation failed (no supported hardware)");
        return false;
    }

    bridgeLoaded_ = true;

    char mfr[128]{};
    char method[64]{};
    if (fn_GetMfr_)    fn_GetMfr_(mfr,    sizeof(mfr));
    if (fn_GetMethod_) fn_GetMethod_(method, sizeof(method));
    GC_LOG_INFO("[Thermal] Hardware ready — manufacturer: "
                + std::string(mfr) + "  method: " + std::string(method));
    return true;
}

ThermalReading ThermalManager::Poll()
{
    ThermalReading r{};
    if (!bridgeLoaded_) return r;

    r.lhmReady = true;
    if (fn_CpuTemp_)  r.cpuTemp     = fn_CpuTemp_();
    if (fn_GpuTemp_)  r.gpuTemp     = fn_GpuTemp_();
    if (fn_CpuFan_)   r.cpuFanRpm   = fn_CpuFan_();
    if (fn_GpuFan_)   r.gpuFanRpm   = fn_GpuFan_();
    if (fn_CpuLoad_)  r.cpuLoad     = fn_CpuLoad_();
    if (fn_GpuLoad_)  r.gpuLoad     = fn_GpuLoad_();
    if (fn_GpuVram_)  r.gpuVramUsed = fn_GpuVram_();
    return r;
}

ThermalOptimizeResult ThermalManager::Maximize()
{
    ThermalOptimizeResult res{};
    if (!bridgeLoaded_) return res;

    if (fn_CpuTemp_) res.cpuTempAtLaunch = fn_CpuTemp_();
    if (fn_GpuTemp_) res.gpuTempAtLaunch = fn_GpuTemp_();

    char mfr[128]{};
    char method[64]{};
    if (fn_GetMfr_)    fn_GetMfr_(mfr,    sizeof(mfr));
    if (fn_GetMethod_) fn_GetMethod_(method, sizeof(method));
    res.manufacturer = mfr;
    res.method       = method;

    int found = 0;
    res.fansMaximized       = fn_Maximize_(&found);
    res.fanControllersFound = found;
    res.thermalReady        = true;
    maximized_              = res.fansMaximized;

    GC_LOG_INFO("[Thermal] Maximized " + std::to_string(found)
                + " fan controller(s) via " + res.method);
    return res;
}

void ThermalManager::Restore()
{
    if (!bridgeLoaded_ || !maximized_) return;
    if (fn_Restore_) fn_Restore_();
    maximized_ = false;
    GC_LOG_INFO("[Thermal] Fan control restored to automatic");
}

} // namespace GameCore::Thermal