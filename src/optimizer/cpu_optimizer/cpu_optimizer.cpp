#include "cpu_optimizer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <powrprof.h>
#include <powersetting.h>

#pragma comment(lib, "powrprof.lib")

#include <string>

namespace GameCore::Optimizer {

static const GUID GUID_BALANCED = {
    0x381b4222, 0xf694, 0x41f0,
    { 0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e }
};
static const GUID GUID_HIGH_PERFORMANCE = {
    0x8c5e7fda, 0xe8bf, 0x4a96,
    { 0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c }
};
static const GUID GUID_ULTIMATE_POWER = {
    0xe9a42b02, 0xd5df, 0x448d,
    { 0xaa, 0x00, 0x03, 0xf1, 0x4d, 0xea, 0x5d, 0x29 }
};

static const GUID GUID_SUB_PROCESSOR = {
    0x54533251, 0x82be, 0x4824,
    { 0x96, 0xc1, 0x47, 0xb6, 0x0b, 0x74, 0x0d, 0x00 }
};

static const GUID GUID_SYSTEM_COOLING_POLICY = {
    0x94d3a615, 0xa899, 0x4ac5,
    { 0xae, 0x2b, 0xe4, 0xd8, 0xf6, 0x34, 0x36, 0x7f }
};

using NtSetTimerResolutionFn =
    LONG(WINAPI*)(ULONG, BOOLEAN, PULONG);

static NtSetTimerResolutionFn NtSetTimerRes = nullptr;

static void LoadNtFunctions()
{
    if (NtSetTimerRes) return;
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;
    NtSetTimerRes = reinterpret_cast<NtSetTimerResolutionFn>(
        GetProcAddress(ntdll, "NtSetTimerResolution"));
}

bool CpuOptimizer::SetTimerResolution(UINT resolutionMs)
{
    LoadNtFunctions();
    if (!NtSetTimerRes) return false;
    ULONG actual = 0;
    return NtSetTimerRes(resolutionMs * 10000, TRUE, &actual) == 0;
}

void CpuOptimizer::RestoreTimerResolution()
{
    LoadNtFunctions();
    if (!NtSetTimerRes) return;
    ULONG actual = 0;
    NtSetTimerRes(156250, FALSE, &actual);
}

PowerPlan CpuOptimizer::GetCurrentPlan()
{
    GUID* pGuid = nullptr;
    if (PowerGetActiveScheme(nullptr, &pGuid) != ERROR_SUCCESS)
        return PowerPlan::Balanced;

    PowerPlan plan = PowerPlan::Balanced;
    if (IsEqualGUID(*pGuid, GUID_HIGH_PERFORMANCE))
        plan = PowerPlan::HighPerformance;
    else if (IsEqualGUID(*pGuid, GUID_ULTIMATE_POWER))
        plan = PowerPlan::UltimatePower;

    LocalFree(pGuid);
    return plan;
}

bool CpuOptimizer::SetUltimatePowerPlan()
{
    return PowerSetActiveScheme(nullptr, &GUID_ULTIMATE_POWER) == ERROR_SUCCESS;
}

bool CpuOptimizer::SetHighPerformancePlan()
{
    return PowerSetActiveScheme(nullptr, &GUID_HIGH_PERFORMANCE) == ERROR_SUCCESS;
}

bool CpuOptimizer::SetActiveCoolingPolicy(bool active, bool& previousValue)
{
    GUID* activeScheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &activeScheme) != ERROR_SUCCESS)
        return false;

    DWORD currentAC = 0;
    DWORD type = 0;
    DWORD size = sizeof(currentAC);

    // Correct signature:
    // PowerReadACValue(RootPowerKey, SchemeGuid, SubGroupGuid,
    //                  SettingGuid, Type*, Buffer, BufferSize*)
    PowerReadACValue(nullptr, activeScheme, &GUID_SUB_PROCESSOR,
        &GUID_SYSTEM_COOLING_POLICY, &type,
        reinterpret_cast<PUCHAR>(&currentAC), &size);
    previousValue = (currentAC != 0);

    const DWORD newValue = active ? 1 : 0;

    const DWORD r1 = PowerWriteACValueIndex(nullptr, activeScheme,
        &GUID_SUB_PROCESSOR, &GUID_SYSTEM_COOLING_POLICY, newValue);
    const DWORD r2 = PowerWriteDCValueIndex(nullptr, activeScheme,
        &GUID_SUB_PROCESSOR, &GUID_SYSTEM_COOLING_POLICY, newValue);

    const bool ok = (r1 == ERROR_SUCCESS) && (r2 == ERROR_SUCCESS);

    if (ok)
        PowerSetActiveScheme(nullptr, activeScheme);

    LocalFree(activeScheme);
    return ok;
}

CpuOptimizeResult CpuOptimizer::Optimize(bool extremeMode)
{
    CpuOptimizeResult result{};
    result.previousPlan = GetCurrentPlan();

    if (!SetUltimatePowerPlan()) {
        result.powerPlanChanged = SetHighPerformancePlan();
        result.newPlan = PowerPlan::HighPerformance;
    } else {
        result.powerPlanChanged = true;
        result.newPlan = PowerPlan::UltimatePower;
    }

    result.timerResolutionSet = SetTimerResolution(1);

    if (extremeMode) {
        bool prevCooling = false;
        result.activeCoolingSet      = SetActiveCoolingPolicy(true, prevCooling);
        result.previousCoolingActive = prevCooling;
    }

    return result;
}

void CpuOptimizer::Restore(const CpuOptimizeResult& previous)
{
    const GUID* guid = &GUID_BALANCED;
    if (previous.previousPlan == PowerPlan::HighPerformance)
        guid = &GUID_HIGH_PERFORMANCE;
    else if (previous.previousPlan == PowerPlan::UltimatePower)
        guid = &GUID_ULTIMATE_POWER;

    PowerSetActiveScheme(nullptr, guid);

    if (previous.timerResolutionSet)
        RestoreTimerResolution();

    if (previous.activeCoolingSet) {
        bool unused = false;
        SetActiveCoolingPolicy(previous.previousCoolingActive, unused);
    }
}

} // namespace GameCore::Optimizer