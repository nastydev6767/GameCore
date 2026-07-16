#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>

namespace GameCore::Optimizer {

struct ServiceInfo {
    std::string name;
    std::string displayName;
    bool        wasStopped;
};

class ServiceOptimizer {
public:
    std::vector<ServiceInfo> StopNonEssentialServices();
    void RestoreServices(const std::vector<ServiceInfo>& stopped);

private:
    // Named to avoid colliding with the Win32 StartService/StopService
    // macros, which expand to StartServiceA/StopServiceA and break
    // methods with those exact names.
    static bool StopWindowsService (const std::string& serviceName);
    static bool StartWindowsService(const std::string& serviceName);
    static bool IsServiceRunning   (const std::string& serviceName);
};

} // namespace GameCore::Optimizer