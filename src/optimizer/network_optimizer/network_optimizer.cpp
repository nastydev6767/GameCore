#include "network_optimizer.h"
#include "core/logging/logger.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <qos2.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "qwave.lib")

#include <string>
#include <vector>
#include <sstream>

namespace GameCore::Optimizer {

// ─────────────────────────────────────────────────────────────────────────────
// Registry helpers
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::SetRegDword(const char* keyPath,
                                   const char* valueName,
                                   DWORD       value,
                                   HKEY        root)
{
    HKEY hKey = nullptr;
    if (RegCreateKeyExA(root, keyPath, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                        nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;

    const DWORD r = RegSetValueExA(hKey, valueName, 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(hKey);
    return r == ERROR_SUCCESS;
}

bool NetworkOptimizer::GetRegDword(const char* keyPath,
                                   const char* valueName,
                                   DWORD&      outValue,
                                   HKEY        root)
{
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(root, keyPath, 0, KEY_QUERY_VALUE, &hKey)
        != ERROR_SUCCESS)
        return false;

    DWORD size = sizeof(outValue);
    DWORD type = 0;
    const DWORD r = RegQueryValueExA(hKey, valueName, nullptr, &type,
        reinterpret_cast<LPBYTE>(&outValue), &size);
    RegCloseKey(hKey);
    return r == ERROR_SUCCESS && type == REG_DWORD;
}

// ─────────────────────────────────────────────────────────────────────────────
// Nagle algorithm — disable on every physical NIC
//
// Nagle buffers small TCP packets to reduce overhead.  Great for throughput,
// terrible for latency-sensitive game traffic.  Windows lets you disable it
// per-interface via the Tcpip\Parameters\Interfaces registry subtree.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::DisableNagle()
{
    // Enumerate NIC GUIDs from the Tcpip interfaces key
    const char* ifacesKey =
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";

    HKEY hIfaces = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, ifacesKey, 0,
                      KEY_ENUMERATE_SUB_KEYS, &hIfaces) != ERROR_SUCCESS)
        return false;

    int patched = 0;
    char subKeyName[256]{};
    DWORD nameLen = sizeof(subKeyName);
    DWORD index   = 0;

    while (RegEnumKeyExA(hIfaces, index++, subKeyName, &nameLen,
                         nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
    {
        nameLen = sizeof(subKeyName);

        // Only patch adapters that have a DhcpIPAddress or IPAddress set
        // (i.e. are actually configured) — skip loopback / tunnel adapters.
        std::string subPath = std::string(ifacesKey) + "\\" + subKeyName;
        HKEY hIface = nullptr;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subPath.c_str(), 0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE, &hIface)
            != ERROR_SUCCESS)
            continue;

        // Check if the adapter has an IP address assigned
        char ipAddr[64]{};
        DWORD ipSz   = sizeof(ipAddr);
        DWORD ipType = 0;
        bool hasIp =
            RegQueryValueExA(hIface, "DhcpIPAddress", nullptr, &ipType,
                reinterpret_cast<LPBYTE>(ipAddr), &ipSz) == ERROR_SUCCESS ||
            RegQueryValueExA(hIface, "IPAddress", nullptr, &ipType,
                reinterpret_cast<LPBYTE>(ipAddr), &ipSz) == ERROR_SUCCESS;

        if (hasIp && strlen(ipAddr) > 0 && strcmp(ipAddr, "0.0.0.0") != 0) {
            // TcpAckFrequency=1 → send ACK immediately (no delay)
            // TCPNoDelay=1      → disable Nagle on this adapter
            DWORD one = 1;
            RegSetValueExA(hIface, "TcpAckFrequency", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&one), sizeof(one));
            RegSetValueExA(hIface, "TCPNoDelay", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&one), sizeof(one));
            ++patched;
        }

        RegCloseKey(hIface);
    }

    RegCloseKey(hIfaces);

    // Global Nagle disable fallback
    SetRegDword("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                "TcpAckFrequency", 1);
    SetRegDword("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                "TCPNoDelay", 1);

    GC_LOG_INFO("[Network] Nagle disabled on " +
                std::to_string(patched) + " adapter(s)");
    return patched > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// TCP Auto-Tuning
//
// "normal" is good for bulk downloads; "restricted" or "highlyrestricted"
// gives lower, more predictable latency for game traffic.
// We set it to "restricted" — a safe middle ground.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::SetTcpAutoTuning()
{
    // AutoTuningLevelLocal: 0=disabled, 1=highlyrestricted, 2=restricted,
    // 3=normal, 4=experimental
    // We target 2 (restricted) — reduces buffer bloat without killing throughput
    const bool ok = SetRegDword(
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        "AutoTuningLevelLocal", 2);

    GC_LOG_INFO("[Network] TCP auto-tuning set to restricted");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// TCP Timestamps — disable for gaming
//
// RFC 1323 timestamps add 10 bytes to every TCP header and consume CPU
// cycles on every packet.  Competitive gamers typically disable these.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::DisableTcpTimestamps()
{
    const bool ok = SetRegDword(
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        "Tcp1323Opts", 0);

    GC_LOG_INFO("[Network] TCP timestamps disabled");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// ECN — disable Explicit Congestion Notification
//
// ECN is great for general internet fairness but adds negotiation overhead
// and can behave poorly with routers/game servers that mishandle it.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::DisableEcn()
{
    // ECNCapability: 0=disabled, 1=enabled
    const bool ok = SetRegDword(
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        "ECNCapability", 0);

    GC_LOG_INFO("[Network] ECN disabled");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initial Retransmit Timeout
//
// Default Windows InitialRtt is 3000 ms — way too long for gaming.
// Set to 300 ms so dropped packets are detected and retransmitted faster.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::SetInitialRto()
{
    const bool ok = SetRegDword(
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        "InitialRtt", 300);

    GC_LOG_INFO("[Network] Initial retransmit timeout set to 300ms");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// MMCSS Network Throttling
//
// Windows throttles network throughput to 10 packets/ms by default when
// running multimedia applications (MMCSS).  This was designed to protect
// audio/video but hurts gaming.  Setting NetworkThrottlingIndex to 0xFFFFFFFF
// disables the cap.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::DisableNetworkThrottling()
{
    const char* key =
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile";

    // Save previous value for restore
    if (!GetRegDword(key, "NetworkThrottlingIndex",
                     previousThrottlingIndex_,
                     HKEY_LOCAL_MACHINE))
    {
        previousThrottlingIndex_ = 10; // Windows default
    }

    wasNetworkThrottlingEnabled_ = (previousThrottlingIndex_ != 0xFFFFFFFF);

    const bool ok = SetRegDword(key, "NetworkThrottlingIndex", 0xFFFFFFFF);

    GC_LOG_INFO("[Network] MMCSS network throttling disabled");
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// DNS Cache Flush
//
// Stale DNS entries can slow connection to game servers.
// ipconfig /flushdns via DnsFlushResolverCache() API.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::FlushDnsCache()
{
    // DnsFlushResolverCache is in dnsapi.dll
    HMODULE hDns = LoadLibraryA("dnsapi.dll");
    if (!hDns) return false;

    using DnsFlushFn = BOOL(WINAPI*)();
    auto fn = reinterpret_cast<DnsFlushFn>(
        GetProcAddress(hDns, "DnsFlushResolverCache"));

    bool ok = false;
    if (fn) {
        ok = fn() != 0;
        GC_LOG_INFO("[Network] DNS cache flushed");
    }

    FreeLibrary(hDns);
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// QoS Policy — mark game process packets as Expedited Forwarding (DSCP 46)
//
// This tells routers that care about DSCP to prioritize the game's packets.
// Helps on home networks with QoS-aware routers / ISP equipment.
// ─────────────────────────────────────────────────────────────────────────────

bool NetworkOptimizer::ApplyQosPolicy(const std::string& gameExeName)
{
    if (gameExeName.empty()) return false;

    // QoS policies live under HKLM\Software\Policies\Microsoft\Windows\QoS
    const std::string policyKey =
        "SOFTWARE\\Policies\\Microsoft\\Windows\\QoS\\"
        "GameCore_" + gameExeName;

    HKEY hKey = nullptr;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, policyKey.c_str(), 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                        nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;

    auto setStr = [&](const char* name, const char* val) {
        RegSetValueExA(hKey, name, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(val),
            static_cast<DWORD>(strlen(val) + 1));
    };

    setStr("Version",             "1.0");
    setStr("Application Name",    gameExeName.c_str());
    setStr("Protocol",            "*");           // TCP + UDP
    setStr("Local Port",          "*");
    setStr("Local IP",            "*");
    setStr("Remote Port",         "*");
    setStr("Remote IP",           "*");
    setStr("DSCP Value",          "46");          // Expedited Forwarding
    setStr("Throttle Rate",       "-1");          // No throttle

    RegCloseKey(hKey);

    appliedGameExe_   = gameExeName;
    qosPolicyApplied_ = true;

    GC_LOG_INFO("[Network] QoS DSCP=46 policy applied for " + gameExeName);
    return true;
}

void NetworkOptimizer::RemoveQosPolicy(const std::string& gameExeName)
{
    if (gameExeName.empty()) return;

    const std::string policyKey =
        "SOFTWARE\\Policies\\Microsoft\\Windows\\QoS\\"
        "GameCore_" + gameExeName;

    RegDeleteKeyA(HKEY_LOCAL_MACHINE, policyKey.c_str());
    GC_LOG_INFO("[Network] QoS policy removed for " + gameExeName);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

NetworkTweakResult NetworkOptimizer::Optimize(const std::string& gameExeName)
{
    NetworkTweakResult result{};

    result.naggleDisabled      = DisableNagle();
    result.autoTuningSet       = SetTcpAutoTuning();
    result.timestampsDisabled  = DisableTcpTimestamps();
    result.ecnDisabled         = DisableEcn();
    result.initialRtoSet       = SetInitialRto();
    result.networkThrottleOff  = DisableNetworkThrottling();
    result.dnsCacheFlushed     = FlushDnsCache();
    result.qosPolicyApplied    = ApplyQosPolicy(gameExeName);

    GC_LOG_INFO("[Network] Optimization complete");
    return result;
}

void NetworkOptimizer::Restore()
{
    // Restore MMCSS throttling index
    if (wasNetworkThrottlingEnabled_) {
        SetRegDword(
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
            "NetworkThrottlingIndex",
            previousThrottlingIndex_);
        GC_LOG_INFO("[Network] MMCSS throttling restored");
    }

    // Remove QoS policy
    if (qosPolicyApplied_)
        RemoveQosPolicy(appliedGameExe_);

    // Note: Nagle, TCP timestamps, ECN, and RTO tweaks are left as-is
    // on restore.  They are universally beneficial and not game-session-specific.
    // The user can revert via Windows network reset if desired.

    GC_LOG_INFO("[Network] Restore complete");
}

} // namespace GameCore::Optimizer
