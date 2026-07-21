#pragma once

#include <string>
#include <vector>

namespace GameCore::Optimizer {

struct NetworkTweakResult {
    bool naggleDisabled;       // Nagle algorithm off on all adapters
    bool autoTuningSet;        // TCP auto-tuning → restricted (gaming sweet spot)
    bool dnsCacheFlushed;      // DNS resolver cache cleared pre-launch
    bool qosPolicyApplied;     // DSCP QoS mark set for game process
    bool timestampsDisabled;   // TCP timestamps off (reduces CPU overhead)
    bool ecnDisabled;          // ECN off (compatibility win for gaming servers)
    bool initialRtoSet;        // Initial retransmit timeout tightened
    bool networkThrottleOff;   // MMCSS NetworkThrottlingIndex → disabled
    int  adaptersPatched;      // Number of NICs Nagle was disabled on
};

class NetworkOptimizer {
public:
    NetworkTweakResult Optimize(const std::string& gameExeName);
    void               Restore();

private:
    // TCP/IP stack tweaks via netsh & registry
    bool DisableNagle();
    bool SetTcpAutoTuning();
    bool DisableTcpTimestamps();
    bool DisableEcn();
    bool SetInitialRto();
    bool DisableNetworkThrottling();

    // DNS
    bool FlushDnsCache();

    // QoS — DSCP mark for game process (Expedited Forwarding = 46)
    bool ApplyQosPolicy(const std::string& gameExeName);
    void RemoveQosPolicy(const std::string& gameExeName);

    // Registry helpers
    static bool SetRegDword(const char* keyPath,
                            const char* valueName,
                            DWORD       value,
                            HKEY        root = HKEY_LOCAL_MACHINE);
    static bool GetRegDword(const char* keyPath,
                            const char* valueName,
                            DWORD&      outValue,
                            HKEY        root = HKEY_LOCAL_MACHINE);

    // State for restore
    bool    wasNetworkThrottlingEnabled_ { true };
    DWORD   previousThrottlingIndex_     { 10 };
    bool    qosPolicyApplied_            { false };
    std::string appliedGameExe_;
};

} // namespace GameCore::Optimizer
