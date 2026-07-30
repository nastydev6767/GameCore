// ─────────────────────────────────────────────────────────────────────────────
// FILE : src/thermal/ThermalBridge/ThermalBridge.h
// ─────────────────────────────────────────────────────────────────────────────
// C++/CLI project — compiled with /clr.
// Exports plain C functions so the native GameCore.exe can call them via
// LoadLibrary / GetProcAddress with no .NET knowledge required.
//
// This is the ONLY file that touches LibreHardwareMonitorLib.dll.
// Everything below is invisible to the rest of GameCore.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// All exports are plain C so they have predictable symbol names
extern "C" {

// Initialise LHM — detects all hardware, loads PawnIO if available.
// Returns true if at least one hardware source was found.
__declspec(dllexport) bool  __cdecl GC_Thermal_Init();

// Per-poll readings (return -1.f if unavailable)
__declspec(dllexport) float __cdecl GC_Thermal_CpuTemp();
__declspec(dllexport) float __cdecl GC_Thermal_GpuTemp();
__declspec(dllexport) float __cdecl GC_Thermal_CpuFanRpm();
__declspec(dllexport) float __cdecl GC_Thermal_GpuFanRpm();
__declspec(dllexport) float __cdecl GC_Thermal_CpuLoad();
__declspec(dllexport) float __cdecl GC_Thermal_GpuLoad();
__declspec(dllexport) float __cdecl GC_Thermal_GpuVramUsedMb();

// Fan maximization — sets every detected fan controller to max PWM.
// *controllersFound receives the number of controllers written to.
// Returns true if at least one controller was set.
__declspec(dllexport) bool  __cdecl GC_Thermal_MaximizeAll(int* controllersFound);

// Restore all fan controllers to automatic / firmware control.
__declspec(dllexport) void  __cdecl GC_Thermal_RestoreAll();

// Strings — buf must be caller-allocated, len = buffer size in bytes
__declspec(dllexport) void  __cdecl GC_Thermal_GetManufacturer(char* buf, int len);
__declspec(dllexport) void  __cdecl GC_Thermal_GetMethod(char* buf, int len);

// Clean shutdown — call before FreeLibrary
__declspec(dllexport) void  __cdecl GC_Thermal_Shutdown();

} // extern "C"