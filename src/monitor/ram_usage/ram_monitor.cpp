#include "ram_monitor.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <sysinfoapi.h>

namespace GameCore::Monitor {

RamStatus RamMonitor::GetStatus() const
{
    MEMORYSTATUSEX ms{};
    ms.dwLength = sizeof(ms);
    if (!GlobalMemoryStatusEx(&ms)) return {};

    constexpr double GB = 1024.0 * 1024.0 * 1024.0;

    RamStatus status{};
    status.totalGb      = static_cast<double>(ms.ullTotalPhys) / GB;
    status.availableGb  = static_cast<double>(ms.ullAvailPhys) / GB;
    status.usedGb       = status.totalGb - status.availableGb;
    status.usagePercent = (status.usedGb / status.totalGb) * 100.0;
    return status;
}

} // namespace GameCore::Monitor