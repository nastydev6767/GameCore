#pragma once

#include <string>
#include <vector>
#include <optional>

namespace GameCore::Detector {

struct GameRequirements {
    std::string name;
    int    minCpuCores;
    double minRamGb;
    int    minVramMb;
    int    minStorageMb;
    int    recCpuCores;
    double recRamGb;
    int    recVramMb;
    int    targetFps;
    bool   cpuHeavy;
    bool   gpuHeavy;
    bool   ramHeavy;
    bool   ioHeavy;
    std::string notes;
};

struct HardwareCapability {
    std::string tier;
    bool        canRunMinimum;
    bool        canRunRecommended;
    int         recommendedFps;
    double      headroomPercent;
    std::string optimizationProfile;
};

class GameDb {
public:
    static GameDb& Instance();

    std::optional<GameRequirements> Find(const std::string& gameName) const;

    HardwareCapability Analyze(
        const GameRequirements& game,
        double cpuCores,
        double ramGb,
        int    vramMb) const;

    size_t Count() const { return db_.size(); }

private:
    GameDb();
    ~GameDb() = default;

    GameDb(const GameDb&)            = delete;
    GameDb& operator=(const GameDb&) = delete;

    std::vector<GameRequirements> db_;

    void BuildDatabase();
    static std::string ToLower(const std::string& s);
    static int  FuzzyScore(const std::string& a, const std::string& b);
};

} // namespace GameCore::Detector