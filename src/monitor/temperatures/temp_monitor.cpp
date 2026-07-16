#include "temp_monitor.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

#include <cstring>
#include <string>

namespace GameCore::Monitor {

// ============================================================
// HWiNFO64 Shared Memory Layout
// Documented public API — used by RTSS, Discord, Wallpaper Engine
// ============================================================

#define HWINFO_SENSORS_MAP_FILE_NAME   L"Global\\HWiNFO_SENS_SM2"
#define HWINFO_SENSORS_MAP_FILE_NAME2  L"Global\\HWiNFO_SENS_SM2a"

#pragma pack(push, 1)

struct HWiNFO_SENSORS_READING {
    DWORD   tReading;           // type: 0=None,1=Temp,2=Volt,3=Fan,4=Current,5=Power,6=Clock,7=Usage,8=Other
    DWORD   dwSensorIndex;
    DWORD   dwReadingID;
    char    szLabelOrig[128];
    char    szLabelUser[128];
    char    szUnit[16];
    double  Value;
    double  ValueMin;
    double  ValueMax;
    double  ValueAvg;
};

struct HWiNFO_SENSORS_SENSOR_ELEMENT {
    DWORD   dwSensorID;
    DWORD   dwSensorInst;
    char    szSensorNameOrig[128];
    char    szSensorNameUser[128];
};

struct HWiNFO_SENSORS_SHARED_MEM2 {
    DWORD   dwSignature;        // 'HWiS' if valid
    DWORD   dwVersion;
    DWORD   dwRevision;
    LONGLONG  poll_time;
    DWORD   dwOffsetOfSensorSection;
    DWORD   dwSizeOfSensorElement;
    DWORD   dwNumSensorElements;
    DWORD   dwOffsetOfReadingSection;
    DWORD   dwSizeOfReadingElement;
    DWORD   dwNumReadingElements;
};

#pragma pack(pop)

// Reading type for temperature
static constexpr DWORD SENSOR_TYPE_TEMP = 1;

// ============================================================
// Tier 2: HWiNFO Shared Memory
// ============================================================
bool TempMonitor::TryHwinfo(double& cpuTemp, double& gpuTemp) const
{
    cpuTemp = -1.0;
    gpuTemp = -1.0;

    // Try primary mapping, then secondary
    HANDLE hMap = OpenFileMappingW(FILE_MAP_READ, FALSE,
                                   HWINFO_SENSORS_MAP_FILE_NAME);
    if (!hMap)
        hMap = OpenFileMappingW(FILE_MAP_READ, FALSE,
                                HWINFO_SENSORS_MAP_FILE_NAME2);
    if (!hMap)
        return false;

    const auto* pMem = static_cast<const BYTE*>(
        MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));

    if (!pMem) {
        CloseHandle(hMap);
        return false;
    }

    const auto* pSM = reinterpret_cast<const HWiNFO_SENSORS_SHARED_MEM2*>(pMem);

    // Validate signature ('HWiS' = 0x53695748)
    if (pSM->dwSignature != 0x53695748) {
        UnmapViewOfFile(pMem);
        CloseHandle(hMap);
        return false;
    }

    bool foundCpu = false;
    bool foundGpu = false;

    for (DWORD i = 0; i < pSM->dwNumReadingElements; ++i) {
        const auto* pReading = reinterpret_cast<const HWiNFO_SENSORS_READING*>(
            pMem
            + pSM->dwOffsetOfReadingSection
            + i * pSM->dwSizeOfReadingElement);

        if (pReading->tReading != SENSOR_TYPE_TEMP)
            continue;

        // Validate temp range
        if (pReading->Value < 0.0 || pReading->Value > 150.0)
            continue;

        std::string label = pReading->szLabelUser;
        if (label.empty())
            label = pReading->szLabelOrig;

        // Convert to lowercase for matching
        std::string lower = label;
        for (auto& c : lower) c = static_cast<char>(tolower(c));

        // CPU temp — look for CPU package/die temp (not per-core)
        if (!foundCpu) {
            if ((lower.find("cpu") != std::string::npos ||
                 lower.find("package") != std::string::npos ||
                 lower.find("tdie") != std::string::npos) &&
                lower.find("core") == std::string::npos)
            {
                cpuTemp  = pReading->Value;
                foundCpu = true;
            }
            // Fallback: first CPU core temp
            else if (lower.find("cpu core") != std::string::npos ||
                     lower.find("core #0") != std::string::npos)
            {
                cpuTemp  = pReading->Value;
                foundCpu = true;
            }
        }

        // GPU temp
        if (!foundGpu) {
            if (lower.find("gpu") != std::string::npos &&
                (lower.find("temp") != std::string::npos ||
                 lower.find("diode") != std::string::npos ||
                 lower.find("hot spot") == std::string::npos))
            {
                gpuTemp  = pReading->Value;
                foundGpu = true;
            }
        }

        if (foundCpu && foundGpu) break;
    }

    UnmapViewOfFile(pMem);
    CloseHandle(hMap);

    return foundCpu || foundGpu;
}

// ============================================================
// Tier 1: PDH Thermal Zone
// ============================================================
double TempMonitor::TryPdh() const
{
    PDH_HQUERY   query   = nullptr;
    PDH_HCOUNTER counter = nullptr;

    if (PdhOpenQuery(nullptr, 0, &query) != ERROR_SUCCESS)
        return -1.0;

    // Try specific thermal zone first
    static const wchar_t* paths[] = {
        L"\\Thermal Zone Information(_TZ.THRM)\\Temperature",
        L"\\Thermal Zone Information(_TZ.TZ00)\\Temperature",
        L"\\Thermal Zone Information(_TZ.CPU0)\\Temperature",
        L"\\Thermal Zone Information(*)\\Temperature",
    };

    bool added = false;
    for (const auto* path : paths) {
        if (PdhAddCounterW(query, path, 0, &counter) == ERROR_SUCCESS) {
            added = true;
            break;
        }
    }

    if (!added) {
        PdhCloseQuery(query);
        return -1.0;
    }

    PdhCollectQueryData(query);
    Sleep(50);
    PdhCollectQueryData(query);

    PDH_FMT_COUNTERVALUE value{};
    const bool ok = PdhGetFormattedCounterValue(
        counter, PDH_FMT_DOUBLE, nullptr, &value) == ERROR_SUCCESS;

    PdhCloseQuery(query);

    if (!ok) return -1.0;

    const double celsius = value.doubleValue - 273.15;
    if (celsius < 1.0 || celsius > 150.0) return -1.0;
    return celsius;
}

// ============================================================
// Tier 3: WMI ACPI ROOT\WMI
// ============================================================
double TempMonitor::TryWmiAcpi() const
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comInit = SUCCEEDED(hr);

    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr);

    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemLocator* locator = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          reinterpret_cast<void**>(&locator));
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemServices* services = nullptr;
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\WMI"),
        nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
    locator->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
        nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;
    hr = services->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT CurrentTemperature "
                L"FROM MSAcpi_ThermalZoneTemperature"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &enumerator);
    services->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    enumerator->Next(WBEM_INFINITE, 1, &obj, &returned);
    enumerator->Release();

    double result = -1.0;
    if (obj && returned > 0) {
        VARIANT var{};
        VariantInit(&var);
        if (SUCCEEDED(obj->Get(L"CurrentTemperature",
                               0, &var, nullptr, nullptr))
            && var.vt == VT_I4)
        {
            const double c = (static_cast<double>(var.lVal) / 10.0) - 273.15;
            if (c > 1.0 && c < 150.0) result = c;
        }
        VariantClear(&var);
        obj->Release();
    }

    if (comInit) CoUninitialize();
    return result;
}

// ============================================================
// Tier 4: WMI MSAcpi via CIMV2
// ============================================================
double TempMonitor::TryWmiMsAcpi() const
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comInit = SUCCEEDED(hr);

    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr);

    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemLocator* locator = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          reinterpret_cast<void**>(&locator));
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemServices* services = nullptr;
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
        nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
    locator->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
        nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;
    hr = services->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM Win32_PerfRawData_Counters_ThermalZoneInformation"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &enumerator);
    services->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        return -1.0;
    }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    enumerator->Next(WBEM_INFINITE, 1, &obj, &returned);
    enumerator->Release();

    double result = -1.0;
    if (obj && returned > 0) {
        VARIANT var{};
        VariantInit(&var);
        if (SUCCEEDED(obj->Get(L"Temperature", 0, &var, nullptr, nullptr))) {
            double kelvin = 0.0;
            if (var.vt == VT_I4)
                kelvin = static_cast<double>(var.lVal);
            else if (var.vt == VT_UI4)
                kelvin = static_cast<double>(var.uintVal);
            else if (var.vt == VT_I8)
                kelvin = static_cast<double>(var.llVal);

            if (kelvin > 273.0) {
                const double c = kelvin / 10.0 - 273.15;
                if (c > 1.0 && c < 150.0) result = c;
            }
        }
        VariantClear(&var);
        obj->Release();
    }

    if (comInit) CoUninitialize();
    return result;
}

// ============================================================
// Main entry — tries all tiers in order
// ============================================================
ThermalStatus TempMonitor::GetStatus() const
{
    ThermalStatus status{};
    status.cpuTempCelsius = -1.0;
    status.gpuTempCelsius = -1.0;
    status.isThrottling   = false;
    status.source         = "unavailable";

    // ── Tier 2 first: HWiNFO shared memory ──────────────────────────
    // Most accurate — try this before anything else
    double hwinfoCpu = -1.0, hwinfoGpu = -1.0;
    if (TryHwinfo(hwinfoCpu, hwinfoGpu)) {
        status.cpuTempCelsius = hwinfoCpu;
        status.gpuTempCelsius = hwinfoGpu;
        status.source         = "HWiNFO64";
        status.isThrottling   = (hwinfoCpu > SafeMaxCelsius && hwinfoCpu > 0.0);
        return status;
    }

    // ── Tier 1: PDH thermal zone ─────────────────────────────────────
    const double pdh = TryPdh();
    if (pdh > 0.0) {
        status.cpuTempCelsius = pdh;
        status.source         = "PDH";
        status.isThrottling   = (pdh > SafeMaxCelsius);
        return status;
    }

    // ── Tier 3: WMI ACPI ROOT\WMI ───────────────────────────────────
    const double acpi = TryWmiAcpi();
    if (acpi > 0.0) {
        status.cpuTempCelsius = acpi;
        status.source         = "WMI-ACPI";
        status.isThrottling   = (acpi > SafeMaxCelsius);
        return status;
    }

    // ── Tier 4: WMI CIMV2 perf counters ─────────────────────────────
    const double cimv2 = TryWmiMsAcpi();
    if (cimv2 > 0.0) {
        status.cpuTempCelsius = cimv2;
        status.source         = "WMI-CIMV2";
        status.isThrottling   = (cimv2 > SafeMaxCelsius);
        return status;
    }

    // ── All failed ───────────────────────────────────────────────────
    // Temp sensors locked by manufacturer (common on Intel laptops)
    status.source = "unavailable";
    return status;
}

} // namespace GameCore::Monitor