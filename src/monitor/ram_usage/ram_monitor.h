#pragma once

namespace GameCore::Monitor {

struct RamStatus {
    double totalGb;
    double usedGb;
    double availableGb;
    double usagePercent;
};

class RamMonitor {
public:
    RamStatus GetStatus() const;
};

} // namespace GameCore::Monitor