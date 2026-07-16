#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>

namespace GameCore::Optimizer {

struct ProcessInfo {
    DWORD       pid;
    std::string name;
    double      memoryMb;
    bool        wasKilled;
};

class ProcessOptimizer {
public:
    std::vector<ProcessInfo> KillBackgroundProcesses();
    bool BoostProcessPriority(DWORD pid);

    const std::vector<ProcessInfo>& GetKilledProcesses() const {
        return killed_;
    }

private:
    std::vector<ProcessInfo> killed_;

    static bool IsSafeToKill(const std::string& processName);
    static bool IsProtected  (const std::string& processName);
    static bool IsSystemProcess(const std::string& processName);
    static bool IsStreamingApp (const std::string& processName);
    static double GetProcessMemoryMb(DWORD pid);
};

} // namespace GameCore::Optimizer