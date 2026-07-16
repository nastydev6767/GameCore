#include "cpu_monitor.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace GameCore::Monitor {

long long CpuMonitor::FileTimeToInt64(const void* ft)
{
    const auto* f = static_cast<const FILETIME*>(ft);
    return (static_cast<long long>(f->dwHighDateTime) << 32)
           | static_cast<long long>(f->dwLowDateTime);
}

CpuMonitor::CpuMonitor()
{
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        lastIdle_   = FileTimeToInt64(&idle);
        lastKernel_ = FileTimeToInt64(&kernel);
        lastUser_   = FileTimeToInt64(&user);
    }
}

double CpuMonitor::GetUsagePercent()
{
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user)) return 0.0;

    const long long currentIdle   = FileTimeToInt64(&idle);
    const long long currentKernel = FileTimeToInt64(&kernel);
    const long long currentUser   = FileTimeToInt64(&user);

    const long long deltaIdle   = currentIdle   - lastIdle_;
    const long long deltaKernel = currentKernel - lastKernel_;
    const long long deltaUser   = currentUser   - lastUser_;

    lastIdle_   = currentIdle;
    lastKernel_ = currentKernel;
    lastUser_   = currentUser;

    const long long deltaTotal = deltaKernel + deltaUser;
    if (deltaTotal == 0) return 0.0;

    const long long deltaBusy = deltaTotal - deltaIdle;
    return (static_cast<double>(deltaBusy)
            / static_cast<double>(deltaTotal)) * 100.0;
}

} // namespace GameCore::Monitor