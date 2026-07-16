#include "process_optimizer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

#include <algorithm>
#include <string>

namespace GameCore::Optimizer {

// ── Protected: NEVER kill these ─────────────────────────────────────

// Streaming / recording apps — user may be live
static const std::vector<std::string> StreamingApps = {
    "obs64.exe", "obs32.exe", "obs.exe",
    "streamlabs obs.exe", "streamlabsobs.exe",
    "xsplit.core.exe", "xsplitbroadcaster.exe",
    "nvcontainer.exe",      // ShadowPlay (NVIDIA Share)
    "nvidia share.exe",
    "shadowplay.exe",
    "action.exe",           // Mirillis Action
    "bandicam.exe",
    "fraps.exe",
    "dxtory.exe",
    "playclaw.exe",
    "medal.exe",            // Medal.tv
    "outplayed.exe",        // Outplayed
    "overwolf.exe",
    "amd encoder.exe",
    "relive.exe",           // AMD ReLive
};

// Communication — user may be in call
static const std::vector<std::string> CommunicationApps = {
    "discord.exe",
    "teams.exe",
    "zoom.exe",
    "skype.exe",
    "slack.exe",
    "teamspeak3.exe",
    "ts3client_win64.exe",
    "mumble.exe",
    "ventrilo.exe",
};

// Windows system — never touch
static const std::vector<std::string> SystemProcesses = {
    "system", "system idle process",
    "smss.exe", "csrss.exe", "wininit.exe",
    "winlogon.exe", "lsass.exe", "lsm.exe",
    "services.exe", "svchost.exe", "dwm.exe",
    "explorer.exe", "taskmgr.exe", "taskhostw.exe",
    "sihost.exe", "ctfmon.exe", "fontdrvhost.exe",
    "spoolsv.exe", "searchindexer.exe",
    "registry", "memory compression",
    "runtimebroker.exe", "shellexperiencehost.exe",
    "startmenuexperiencehost.exe", "searchhost.exe",
    "textinputhost.exe", "conhost.exe", "cmd.exe",
    "powershell.exe", "windowsterminal.exe",
    "gamecore.exe",     // ourselves
    "gamecore_ui.exe",
};

// Safe to kill — known bloat
static const std::vector<std::string> SafeToKill = {
    // Browser background processes
    "chrome.exe", "msedge.exe", "firefox.exe",
    "opera.exe", "brave.exe", "vivaldi.exe",
    "iexplore.exe",

    // Launchers (not the game itself)
    "epicwebhelper.exe", "eosoverlayrenderer.exe",
    "galaxyclient helper.exe",

    // Background updaters
    "adobeupdater.exe", "adobecrashhandler.exe",
    "adobeipcbroker.exe",
    "officeclicktorun.exe",
    "onedrive.exe",
    "dropbox.exe",
    "googledrivefs.exe",
    "box.exe",

    // Antivirus UI (not service — the UI only)
    "avgui.exe", "avastui.exe", "mbam.exe",

    // Misc bloat
    "spotify.exe",
    "itunes.exe",
    "applicationframehost.exe",
    "yourphone.exe",
    "phoneexperiencehost.exe",
    "gamingservices.exe",   // Xbox gaming services UI
    "xboxpcapp.exe",
};

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool InList(const std::string& name,
                   const std::vector<std::string>& list)
{
    const std::string lower = ToLower(name);
    for (const auto& entry : list)
        if (lower == ToLower(entry)) return true;
    return false;
}

bool ProcessOptimizer::IsStreamingApp(const std::string& name) {
    return InList(name, StreamingApps) || InList(name, CommunicationApps);
}

bool ProcessOptimizer::IsSystemProcess(const std::string& name) {
    return InList(name, SystemProcesses);
}

bool ProcessOptimizer::IsProtected(const std::string& name) {
    return IsSystemProcess(name) || IsStreamingApp(name);
}

bool ProcessOptimizer::IsSafeToKill(const std::string& name) {
    if (IsProtected(name)) return false;
    return InList(name, SafeToKill);
}

double ProcessOptimizer::GetProcessMemoryMb(DWORD pid) {
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return 0.0;

    PROCESS_MEMORY_COUNTERS pmc{};
    pmc.cb = sizeof(pmc);
    double mb = 0.0;
    if (GetProcessMemoryInfo(hProcess,
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
        mb = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);

    CloseHandle(hProcess);
    return mb;
}

std::vector<ProcessInfo> ProcessOptimizer::KillBackgroundProcesses()
{
    killed_.clear();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return killed_;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return killed_;
    }

    do {
        std::string name = entry.szExeFile;
        if (!IsSafeToKill(name)) continue;

        const DWORD pid = entry.th32ProcessID;
        const double mem = GetProcessMemoryMb(pid);

        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) continue;

        const bool killed = TerminateProcess(hProcess, 0) != 0;
        CloseHandle(hProcess);

        if (killed)
            killed_.push_back({ pid, name, mem, true });

    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return killed_;
}

bool ProcessOptimizer::BoostProcessPriority(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    const bool ok = SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS) != 0;
    CloseHandle(hProcess);
    return ok;
}

} // namespace GameCore::Optimizer