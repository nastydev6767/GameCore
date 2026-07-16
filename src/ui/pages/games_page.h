#pragma once

#include "detector/game_detector/game_detector.h"
#include "detector/game_detector/game_db.h"

#include <vector>
#include <functional>
#include <string>

namespace GameCore::UI {

enum class GameSourceFilter {
    All,
    Steam,
    Epic,
    Gog,
    Manual
};

class GamesPage {
public:
    using LaunchCallback =
        std::function<void(const Detector::DetectedGame&)>;

    void SetGames(const std::vector<Detector::DetectedGame>& games);
    void Render(LaunchCallback onLaunch);
    void RefreshGames(Detector::GameDetector& detector);

private:
    std::vector<Detector::DetectedGame> games_;
    GameSourceFilter filter_ { GameSourceFilter::All };
    char addGamePathBuffer_[512] { 0 };
    bool showAddDialog_ { false };

    void RenderFilterTabs();
    void RenderGameRow(const Detector::DetectedGame& game,
                       LaunchCallback onLaunch);
    void RenderAddGameDialog();
};

} // namespace GameCore::UI