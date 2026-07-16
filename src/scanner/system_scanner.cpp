#include "system_scanner.h"
#include "cpu/cpu_scanner.h"
#include "gpu/gpu_scanner.h"
#include "ram/ram_scanner.h"
#include "windows/windows_scanner.h"

namespace GameCore::Scanner {

SystemInfo SystemScanner::Scan() const
{
    return SystemInfo{
        .cpuName        = CpuScanner{}.GetName(),
        .gpuName        = GpuScanner{}.GetName(),
        .ramGb          = RamScanner{}.GetTotalGb(),
        .windowsVersion = WindowsScanner{}.GetVersion(),
    };
}

} // namespace GameCore::Scanner