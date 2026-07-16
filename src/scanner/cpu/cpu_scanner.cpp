#include "cpu_scanner.h"
#include "scanner/wmi_helper.h"

namespace GameCore::Scanner {

std::string CpuScanner::GetName() const
{
    auto name = WmiQueryFirst(
        L"SELECT Name FROM Win32_Processor",
        L"Name");
    Trim(name);
    return name.empty() ? "Unknown CPU" : name;
}

} // namespace GameCore::Scanner