#pragma once

namespace GameCore::Monitor {

class CpuMonitor {
public:
    CpuMonitor();
    double GetUsagePercent();

private:
    long long lastIdle_   { 0 };
    long long lastKernel_ { 0 };
    long long lastUser_   { 0 };

    static long long FileTimeToInt64(const void* ft);
};

} // namespace GameCore::Monitor