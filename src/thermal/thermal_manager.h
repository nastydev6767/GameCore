#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// FILE : src/thermal/thermal_manager.h
// ─────────────────────────────────────────────────────────────────────────────
// Pure native C++ interface.  GameCore calls this; it delegates everything
// to ThermalBridge.dll (C++/CLI) which owns LibreHardwareMonitorLib.dll.
// No mention of fans, PawnIO, or LHM anywhere visible to the rest of the app.
// ─────────────────────────────────────────────────────────────────────────────

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>

namespace GameCore::Thermal {

// ── Sensor readings LHM can give us ─────────────────────────────────────────
struct ThermalReading {
    float cpuTemp     { -1.f };   // °C, -1 = unavailable
    float gpuTemp     { -1.f };   // °C
    float cpuFanRpm   { -1.f };   // RPM
    float gpuFanRpm   { -1.f };   // RPM
    float cpuLoad     { -1.f };   // %
    float gpuLoad     { -1.f };   // %
    float gpuVramUsed { -1.f };   // MB
    bool  lhmReady    { false };
};

// ── Result from a maximize call ──────────────────────────────────────────────
struct ThermalOptimizeResult {
    bool  thermalReady        { false };
    bool  fansMaximized       { false };
    int   fanControllersFound { 0 };
    float cpuTempAtLaunch     { -1.f };
    float gpuTempAtLaunch     { -1.f };
    std::string manufacturer;
    std::string method;           // e.g. "lhm_wmi", "lhm_ec", "lhm_smm"
};

// ─────────────────────────────────────────────────────────────────────────────
class ThermalManager {
public:
    ThermalManager();
    ~ThermalManager();

    // Called once at startup — downloads LHM + PawnIO silently if needed,
    // loads ThermalBridge.dll, initialises LHM Computer object.
    // Returns false only if the machine has no supported hardware at all.
    bool Init();

    // Reads current temps/fans/load from LHM.
    ThermalReading  Poll();

    // Called when a game is about to launch — maximises all fans.
    ThermalOptimizeResult Maximize();

    // Called when the game exits — restores fans to automatic control.
    void Restore();

    bool IsReady() const { return bridgeLoaded_; }

private:
    // ── Bridge DLL function pointer types ───────────────────────────────────
    using FnInit         = bool    (*)();
    using FnPollCpuTemp  = float   (*)();
    using FnPollGpuTemp  = float   (*)();
    using FnPollCpuFan   = float   (*)();
    using FnPollGpuFan   = float   (*)();
    using FnPollCpuLoad  = float   (*)();
    using FnPollGpuLoad  = float   (*)();
    using FnPollVram     = float   (*)();
    using FnMaximize     = bool    (*)(int* controllersFound);
    using FnRestore      = void    (*)();
    using FnGetMfr       = void    (*)(char* buf, int len);
    using FnGetMethod    = void    (*)(char* buf, int len);
    using FnShutdown     = void    (*)();

    // ── Helpers ─────────────────────────────────────────────────────────────
    bool  EnsureSetup();          // runs setup_thermal.ps1 if needed
    bool  LoadBridge();           // LoadLibrary + GetProcAddress
    template<typename T>
    T     Resolve(const char* name);

    // ── State ────────────────────────────────────────────────────────────────
    HMODULE bridgeDll_    { nullptr };
    bool    bridgeLoaded_ { false };
    bool    maximized_    { false };

    // ── Resolved function pointers ───────────────────────────────────────────
    FnInit        fn_Init_        { nullptr };
    FnPollCpuTemp fn_CpuTemp_     { nullptr };
    FnPollGpuTemp fn_GpuTemp_     { nullptr };
    FnPollCpuFan  fn_CpuFan_      { nullptr };
    FnPollGpuFan  fn_GpuFan_      { nullptr };
    FnPollCpuLoad fn_CpuLoad_     { nullptr };
    FnPollGpuLoad fn_GpuLoad_     { nullptr };
    FnPollVram    fn_GpuVram_     { nullptr };
    FnMaximize    fn_Maximize_    { nullptr };
    FnRestore     fn_Restore_     { nullptr };
    FnGetMfr      fn_GetMfr_      { nullptr };
    FnGetMethod   fn_GetMethod_   { nullptr };
    FnShutdown    fn_Shutdown_    { nullptr };

    static constexpr const char* kBridgeDll   = "GameCore.ThermalBridge.dll";
    static constexpr const char* kSetupScript  = "scripts\\setup_thermal.ps1";
    static constexpr const char* kReadyFlag    = "thermal_ready";
};

} // namespace GameCore::Thermal