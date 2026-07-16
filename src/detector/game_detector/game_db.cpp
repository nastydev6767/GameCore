#include "game_db.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace GameCore::Detector {

GameDb& GameDb::Instance() {
    static GameDb instance;
    return instance;
}

std::string GameDb::ToLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return r;
}

int GameDb::FuzzyScore(const std::string& a, const std::string& b) {
    const std::string la = ToLower(a);
    const std::string lb = ToLower(b);
    if (la == lb) return 100;
    if (la.find(lb) != std::string::npos) return 80;
    if (lb.find(la) != std::string::npos) return 80;

    int score = 0;
    std::string word;
    std::istringstream ss(lb);
    while (ss >> word)
        if (word.size() > 2 && la.find(word) != std::string::npos)
            score += 20;
    return std::min(score, 75);
}

GameDb::GameDb() { BuildDatabase(); }

void GameDb::BuildDatabase() {
    db_ = {
        { "Cyberpunk 2077",
          6, 8.0, 6144, 70000, 8, 16.0, 8192, 60,
          true, true, true, true,
          "Extremely CPU+GPU heavy. Prioritize GPU memory." },

        { "Red Dead Redemption 2",
          8, 12.0, 4096, 150000, 8, 16.0, 8192, 60,
          true, true, true, true,
          "One of the most demanding open world games." },

        { "The Witcher 3",
          4, 6.0, 2048, 35000, 6, 8.0, 4096, 60,
          true, true, false, false,
          "CPU heavy in cities. GPU heavy in landscapes." },

        { "Elden Ring",
          4, 8.0, 2048, 60000, 8, 12.0, 4096, 60,
          false, true, false, false,
          "GPU heavy. Poor PC optimization — needs process priority." },

        { "GTA V",
          4, 4.0, 2048, 72000, 8, 8.0, 4096, 60,
          true, true, false, true,
          "Old but CPU hungry in online mode." },

        { "Hogwarts Legacy",
          6, 12.0, 6144, 85000, 8, 16.0, 8192, 60,
          true, true, true, true,
          "Very RAM and VRAM hungry." },

        { "Valorant",
          2, 4.0, 1024, 8000, 4, 8.0, 2048, 144,
          true, false, false, false,
          "Low requirements. Maximize FPS — disable all overlays." },

        { "CS2",
          4, 8.0, 1024, 15000, 8, 16.0, 4096, 144,
          true, false, false, false,
          "CPU bound. Kill all background tasks for max FPS." },

        { "Fortnite",
          4, 8.0, 2048, 29000, 8, 16.0, 4096, 144,
          true, true, false, false,
          "CPU heavy in battles. Optimize process priority." },

        { "Apex Legends",
          6, 8.0, 2048, 22000, 8, 16.0, 4096, 144,
          true, true, false, false,
          "CPU heavy. Benefits greatly from process priority boost." },

        { "Overwatch 2",
          4, 6.0, 2048, 30000, 6, 8.0, 4096, 144,
          true, false, false, false,
          "Well optimized. Focus on CPU performance." },

        { "Rainbow Six Siege",
          4, 6.0, 2048, 30000, 6, 8.0, 4096, 144,
          true, false, false, false,
          "CPU bound competitive shooter." },

        { "PUBG",
          4, 8.0, 2048, 40000, 8, 16.0, 4096, 60,
          true, true, true, true,
          "Poorly optimized. Needs aggressive background cleanup." },

        { "Warzone",
          6, 8.0, 4096, 101000, 8, 16.0, 8192, 60,
          true, true, true, true,
          "Extremely large install. VRAM hungry." },

        { "Dark Souls 3",
          4, 4.0, 2048, 15000, 4, 8.0, 4096, 60,
          false, true, false, false,
          "Locked 60fps. GPU focused." },

        { "Sekiro",
          4, 8.0, 2048, 13000, 6, 8.0, 4096, 60,
          false, true, false, false,
          "Locked 60fps. Well optimized." },

        { "Diablo IV",
          4, 8.0, 4096, 45000, 6, 16.0, 8192, 60,
          true, true, true, false,
          "Always online. CPU+GPU balanced." },

        { "Path of Exile",
          4, 8.0, 2048, 40000, 6, 8.0, 4096, 60,
          true, false, true, false,
          "CPU heavy in dense maps." },

        { "Skyrim",
          2, 4.0, 1024, 12000, 4, 8.0, 2048, 60,
          false, true, false, false,
          "Old engine. Mod-heavy installs need more RAM." },

        { "Fallout 4",
          2, 8.0, 2048, 30000, 4, 8.0, 4096, 60,
          true, false, true, false,
          "RAM hungry especially with mods." },

        { "Total War Warhammer 3",
          6, 8.0, 4096, 120000, 8, 16.0, 8192, 60,
          true, true, true, false,
          "Very CPU heavy in battles." },

        { "Civilization VI",
          4, 8.0, 2048, 15000, 8, 16.0, 4096, 60,
          true, false, true, false,
          "Late game becomes very CPU heavy." },

        { "Age of Empires IV",
          4, 8.0, 2048, 50000, 8, 16.0, 4096, 60,
          true, true, false, false,
          "CPU heavy in large battles." },

        { "Minecraft",
          2, 4.0, 512, 4000, 4, 8.0, 2048, 60,
          true, false, false, false,
          "Java edition is CPU heavy. Allocate RAM carefully." },

        { "Terraria",
          1, 1.0, 256, 200, 2, 4.0, 512, 60,
          false, false, false, false,
          "Very light. Almost any PC handles it." },

        { "Stardew Valley",
          1, 2.0, 256, 500, 2, 4.0, 512, 60,
          false, false, false, false,
          "Extremely light. No optimization needed." },

        { "Hollow Knight",
          2, 4.0, 512, 2000, 4, 8.0, 1024, 60,
          false, false, false, false,
          "Light. Runs on almost anything." },

        { "Lethal Company",
          2, 4.0, 512, 1000, 4, 8.0, 2048, 60,
          true, false, false, false,
          "Unity game. Light but CPU spikes in storms." },

        { "Phasmophobia",
          4, 8.0, 2048, 13000, 8, 16.0, 4096, 60,
          false, true, false, false,
          "GPU heavy for lighting and shadows." },

        { "Rocket League",
          2, 4.0, 512, 7000, 4, 8.0, 2048, 144,
          true, false, false, false,
          "CPU bound. Great for high FPS targets." },

        { "F1 23",
          4, 8.0, 3072, 80000, 6, 16.0, 8192, 60,
          true, true, false, false,
          "GPU heavy on track." },

        { "Valheim",
          2, 8.0, 2048, 1000, 4, 8.0, 4096, 60,
          true, false, false, false,
          "CPU heavy. Single threaded bottleneck." },

        { "Rust",
          6, 8.0, 4096, 25000, 8, 16.0, 4096, 60,
          true, true, false, false,
          "Very CPU heavy on large servers." },

        { "DayZ",
          4, 8.0, 4096, 16000, 8, 16.0, 8192, 60,
          true, true, true, false,
          "CPU+RAM hungry open world survival." },
    };
}

std::optional<GameRequirements> GameDb::Find(const std::string& gameName) const
{
    int bestScore = 30;
    const GameRequirements* best = nullptr;

    for (const auto& entry : db_) {
        int score = FuzzyScore(gameName, entry.name);
        if (score > bestScore) {
            bestScore = score;
            best      = &entry;
        }
    }

    if (best) return *best;
    return std::nullopt;
}

HardwareCapability GameDb::Analyze(
    const GameRequirements& game,
    double cpuCores,
    double ramGb,
    int    vramMb) const
{
    HardwareCapability cap{};

    cap.canRunMinimum =
        cpuCores >= game.minCpuCores &&
        ramGb    >= game.minRamGb    &&
        vramMb   >= game.minVramMb;

    cap.canRunRecommended =
        cpuCores >= game.recCpuCores &&
        ramGb    >= game.recRamGb    &&
        vramMb   >= game.recVramMb;

    double cpuH  = std::min((cpuCores / game.recCpuCores) * 100.0, 150.0);
    double ramH  = std::min((ramGb    / game.recRamGb)    * 100.0, 150.0);
    double vramH = (game.recVramMb > 0)
        ? std::min((static_cast<double>(vramMb) / game.recVramMb) * 100.0, 150.0)
        : 100.0;

    cap.headroomPercent = (cpuH + ramH + vramH) / 3.0 - 100.0;

    if (!cap.canRunMinimum) {
        cap.tier                = "below_minimum";
        cap.recommendedFps      = 0;
        cap.optimizationProfile = "aggressive";
    } else if (!cap.canRunRecommended) {
        cap.tier                = "low";
        cap.recommendedFps      = std::min(game.targetFps, 30);
        cap.optimizationProfile = "aggressive";
    } else if (cap.headroomPercent < 20.0) {
        cap.tier                = "mid";
        cap.recommendedFps      = std::min(game.targetFps, 60);
        cap.optimizationProfile = "balanced";
    } else if (cap.headroomPercent < 60.0) {
        cap.tier                = "high";
        cap.recommendedFps      = game.targetFps;
        cap.optimizationProfile = "light";
    } else {
        cap.tier                = "ultra";
        cap.recommendedFps      = game.targetFps;
        cap.optimizationProfile = "minimal";
    }

    return cap;
}

} // namespace GameCore::Detector