#include "service_optimizer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>

namespace GameCore::Optimizer {

static const std::vector<std::string> SafeToStop = {
    "WSearch",
    "SysMain",
    "DiagTrack",
    "WdiServiceHost",
    "TabletInputService",
    "Fax",
    "PrintSpooler",
    "RemoteRegistry",
    "SharedAccess",
    "TrkWks",
    "WbioSrvc",
    "XblAuthManager",
    "XblGameSave",
    "XboxNetApiSvc",
    "MapsBroker",
    "RetailDemo",
};

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ServiceOptimizer::IsServiceRunning(const std::string& serviceName)
{
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hSvc = OpenServiceA(hSCM, serviceName.c_str(),
                                  SERVICE_QUERY_STATUS);
    if (!hSvc) {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status{};
    bool running = false;
    if (QueryServiceStatus(hSvc, &status))
        running = (status.dwCurrentState == SERVICE_RUNNING);

    CloseServiceHandle(hSvc);
    CloseServiceHandle(hSCM);
    return running;
}

bool ServiceOptimizer::StopWindowsService(const std::string& serviceName)
{
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hSvc = OpenServiceA(hSCM, serviceName.c_str(),
                                  SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!hSvc) {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status{};
    const bool ok = ControlService(hSvc, SERVICE_CONTROL_STOP, &status) != 0;

    CloseServiceHandle(hSvc);
    CloseServiceHandle(hSCM);
    return ok;
}

bool ServiceOptimizer::StartWindowsService(const std::string& serviceName)
{
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hSvc = OpenServiceA(hSCM, serviceName.c_str(), SERVICE_START);
    if (!hSvc) {
        CloseServiceHandle(hSCM);
        return false;
    }

    const bool ok = StartServiceA(hSvc, 0, nullptr) != 0;

    CloseServiceHandle(hSvc);
    CloseServiceHandle(hSCM);
    return ok;
}

std::vector<ServiceInfo> ServiceOptimizer::StopNonEssentialServices()
{
    std::vector<ServiceInfo> stopped;

    for (const auto& name : SafeToStop) {
        if (!IsServiceRunning(name)) continue;

        if (StopWindowsService(name))
            stopped.push_back({ name, name, true });
    }

    return stopped;
}

void ServiceOptimizer::RestoreServices(
    const std::vector<ServiceInfo>& stopped)
{
    for (const auto& svc : stopped) {
        if (svc.wasStopped)
            StartWindowsService(svc.name);
    }
}

} // namespace GameCore::Optimizer