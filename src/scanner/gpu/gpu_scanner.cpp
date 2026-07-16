#include "gpu_scanner.h"
#include "scanner/wmi_helper.h"

namespace GameCore::Scanner {

std::string GpuScanner::GetName() const
{
    auto name = WmiQueryFirst(
        L"SELECT Name FROM Win32_VideoController",
        L"Name");
    Trim(name);
    return name.empty() ? "Unknown GPU" : name;
}

} // namespace GameCore::Scanner