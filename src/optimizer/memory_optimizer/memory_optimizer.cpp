#include "memory_optimizer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")

namespace GameCore::Optimizer {

// SYSTEM_MEMORY_LIST_COMMAND and SystemMemoryListInformation are
// undocumented NT internals — not present in the public Windows SDK
// headers, so we declare them ourselves.
enum SYSTEM_MEMORY_LIST_COMMAND_LOCAL {
    MemoryCaptureAccessedBits,
    MemoryCaptureAndResetAccessedBits,
    MemoryEmptyWorkingSets,
    MemoryFlushModifiedList,
    MemoryPurgeStandbyList,
    MemoryPurgeLowPriorityStandbyList,
    MemoryCommandMax
};

using NtSetSystemInformationFn = LONG(WINAPI*)(ULONG, PVOID, ULONG);

static NtSetSystemInformationFn NtSetSystemInfo = nullptr;

static void LoadNtSetSystemInformation()
{
    if (NtSetSystemInfo) return;
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return;
    NtSetSystemInfo = reinterpret_cast<NtSetSystemInformationFn>(
        GetProcAddress(ntdll, "NtSetSystemInformation"));
}

static double GetUsedMemoryMb()
{
    MEMORYSTATUSEX ms{};
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    const double total = static_cast<double>(ms.ullTotalPhys);
    const double avail = static_cast<double>(ms.ullAvailPhys);
    return (total - avail) / (1024.0 * 1024.0);
}

void MemoryOptimizer::EmptyWorkingSets()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    if (Process32First(snapshot, &entry)) {
        do {
            HANDLE hProcess = OpenProcess(
                PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION,
                FALSE, entry.th32ProcessID);
            if (hProcess) {
                SetProcessWorkingSetSize(hProcess,
                    static_cast<SIZE_T>(-1),
                    static_cast<SIZE_T>(-1));
                CloseHandle(hProcess);
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
}

void MemoryOptimizer::FlushFileSystemCache()
{
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hToken   = nullptr;

    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
        return;

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    LookupPrivilegeValue(nullptr, SE_INCREASE_QUOTA_NAME,
                         &tp.Privileges[0].Luid);

    AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr);
    CloseHandle(hToken);

    LoadNtSetSystemInformation();
    if (!NtSetSystemInfo) return;

    int cmd = MemoryEmptyWorkingSets;
    NtSetSystemInfo(
        80, // SystemMemoryListInformation (undocumented class number)
        &cmd, sizeof(cmd));
}

void MemoryOptimizer::CompactHeap()
{
    HeapCompact(GetProcessHeap(), 0);
}

MemoryResult MemoryOptimizer::FreeMemory()
{
    const double before = GetUsedMemoryMb();

    EmptyWorkingSets();
    FlushFileSystemCache();
    CompactHeap();

    Sleep(200);

    const double after = GetUsedMemoryMb();
    const double freed = before - after;

    return MemoryResult{
        .freedMb  = freed > 0.0 ? freed : 0.0,
        .beforeMb = before,
        .afterMb  = after,
    };
}

} // namespace GameCore::Optimizer