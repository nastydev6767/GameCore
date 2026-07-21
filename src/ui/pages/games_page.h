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
    GameSourceFilter filter_       { GameSourceFilter::All };
    char addGamePathBuffer_[512]   { 0 };
    char addGameError_[256]        { 0 };
    char searchBuffer_[256]        { 0 };
    bool showAddDialog_            { false };
    bool needsRefresh_             { false };

    bool PassesFilter(const Detector::DetectedGame& game) const;

    void RenderFilterTabs();
    void RenderGameRow(const Detector::DetectedGame& game,
                       LaunchCallback onLaunch);
    void RenderAddGameDialog();

public:
    bool NeedsRefresh() const  { return needsRefresh_; }
    void ClearRefresh()        { needsRefresh_ = false; }
};

} // namespace GameCore::UI