#include "windows_scanner.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <format>

namespace GameCore::Scanner {

std::string WindowsScanner::GetVersion() const
{
    using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOEXW*);

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return "Windows (unknown)";

    auto RtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
        GetProcAddress(ntdll, "RtlGetVersion"));
    if (!RtlGetVersion) return "Windows (unknown)";

    OSVERSIONINFOEXW osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (RtlGetVersion(&osvi) != 0) return "Windows (unknown)";

    std::string name;
    if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
        name = (osvi.dwBuildNumber >= 22000) ? "Windows 11" : "Windows 10";
    else
        name = "Windows";

    return std::format("{} (Build {})", name, osvi.dwBuildNumber);
}

} // namespace GameCore::Scanner