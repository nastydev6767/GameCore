#include "ram_scanner.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <sysinfoapi.h>

namespace GameCore::Scanner {

double RamScanner::GetTotalGb() const
{
    MEMORYSTATUSEX ms{};
    ms.dwLength = sizeof(ms);
    if (!GlobalMemoryStatusEx(&ms)) return 0.0;

    const double gb = static_cast<double>(ms.ullTotalPhys)
                      / (1024.0 * 1024.0 * 1024.0);
    return static_cast<double>(static_cast<int>(gb * 100 + 0.5)) / 100.0;
}

} // namespace GameCore::Scanner