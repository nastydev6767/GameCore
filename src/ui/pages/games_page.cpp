#include "games_page.h"
#include "../themes/theme.h"

#include <imgui.h>
#include <algorithm>
#include <string>

namespace GameCore::UI {

bool GamesPage::PassesFilter(const Detector::DetectedGame& game) const
{
    switch (filter_) {
        case GameSourceFilter::All:    return true;
        case GameSourceFilter::Steam:
            return game.source == Detector::GameSource::Steam;
        case GameSourceFilter::Epic:
            return game.source == Detector::GameSource::Epic;
        case GameSourceFilter::Gog:
            return game.source == Detector::GameSource::Gog;
        case GameSourceFilter::Manual:
            return game.source == Detector::GameSource::Manual;
        default: return true;
    }
}

void GamesPage::SetGames(const std::vector<Detector::DetectedGame>& games)
{
    games_ = games;
}

void GamesPage::RefreshGames(Detector::GameDetector& detector)
{
    games_ = detector.ScanAll();
}

void GamesPage::RenderFilterTabs()
{
    auto tabButton = [this](const char* label, GameSourceFilter f) {
        const bool active = (filter_ == f);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, Theme::Accent);
        if (ImGui::Button(label, ImVec2(80, 32))) filter_ = f;
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    };

    tabButton("All",    GameSourceFilter::All);
    tabButton("Steam",  GameSourceFilter::Steam);
    tabButton("Epic",   GameSourceFilter::Epic);
    tabButton("GOG",    GameSourceFilter::Gog);
    tabButton("Manual", GameSourceFilter::Manual);

    if (ImGui::Button("+ Add Game", ImVec2(110, 32))) {
        showAddDialog_ = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh", ImVec2(80, 32))) {
        needsRefresh_ = true;
    }

    // Badge: show count of visible games
    int visibleCount = 0;
    for (const auto& g : games_)
        if (PassesFilter(g)) ++visibleCount;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    ImGui::TextColored(Theme::TextSecondary,
        "%d game%s", visibleCount, visibleCount == 1 ? "" : "s");

    ImGui::NewLine();
}

void GamesPage::RenderGameRow(const Detector::DetectedGame& game,
                              LaunchCallback onLaunch)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::Surface);
    ImGui::BeginChild(game.name.c_str(), ImVec2(-1, 60), true,
                      ImGuiWindowFlags_NoScrollbar);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    ImGui::SetCursorPosX(16);

    ImGui::BeginGroup();
    ImGui::TextColored(Theme::TextPrimary, "%s", game.name.c_str());
    if (game.isRunning) {
        ImGui::TextColored(Theme::Success, "Running");
    } else {
        ImGui::TextColored(Theme::TextSecondary, "Installed");
    }
    ImGui::EndGroup();

    ImGui::SameLine(ImGui::GetWindowWidth() - 130);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

    ImGui::PushStyleColor(ImGuiCol_Button, Theme::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::AccentHover);

    const std::string btnLabel = game.isRunning ? "Optimizing" : "Launch";
    if (ImGui::Button(btnLabel.c_str(), ImVec2(100, 36))) {
        if (onLaunch) onLaunch(game);
    }

    ImGui::PopStyleColor(2);

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void GamesPage::RenderAddGameDialog()
{
    if (!showAddDialog_) return;

    ImGui::OpenPopup("Add Game##dlg");
    if (ImGui::BeginPopupModal("Add Game##dlg", &showAddDialog_,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(Theme::TextSecondary,
            "Enter the full path to the game executable (.exe):");
        ImGui::Spacing();
        ImGui::SetNextItemWidth(480);
        ImGui::InputText("##gamepath", addGamePathBuffer_,
                         sizeof(addGamePathBuffer_));

        ImGui::Spacing();

        if (addGameError_[0] != '\0') {
            ImGui::TextColored(Theme::Danger, "%s", addGameError_);
            ImGui::Spacing();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, Theme::Accent);
        if (ImGui::Button("Add", ImVec2(100, 32))) {
            std::string path = addGamePathBuffer_;
            if (path.empty() || path.size() < 5 ||
                path.substr(path.size()-4) != ".exe")
            {
                snprintf(addGameError_, sizeof(addGameError_),
                    "Please enter a valid .exe path.");
            } else {
                // Build a DetectedGame from the path
                Detector::DetectedGame game{};
                game.executablePath = path;
                game.source         = Detector::GameSource::Manual;
                game.isRunning      = false;
                game.processId      = 0;

                // Extract exe filename as name
                auto slash = path.find_last_of("\\/");
                std::string exe = (slash != std::string::npos)
                    ? path.substr(slash + 1) : path;
                // Strip .exe
                if (exe.size() > 4)
                    exe = exe.substr(0, exe.size() - 4);
                // Replace _ and - with spaces
                std::replace(exe.begin(), exe.end(), '_', ' ');
                std::replace(exe.begin(), exe.end(), '-', ' ');
                game.name = exe;
                game.executableName = exe + ".exe";

                games_.push_back(game);
                showAddDialog_         = false;
                addGamePathBuffer_[0]  = '\0';
                addGameError_[0]       = '\0';
            }
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 32))) {
            showAddDialog_        = false;
            addGamePathBuffer_[0] = '\0';
            addGameError_[0]      = '\0';
        }

        ImGui::EndPopup();
    }
}

void GamesPage::Render(LaunchCallback onLaunch)
{
    ImGui::Spacing();
    RenderFilterTabs();
    ImGui::Spacing();

    // Apply search filter
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("##search", searchBuffer_, sizeof(searchBuffer_));
    ImGui::SameLine();
    ImGui::TextColored(Theme::TextSecondary, "Search");
    ImGui::Spacing();

    // Build visible list
    bool anyVisible = false;
    for (const auto& game : games_) {
        if (!PassesFilter(game)) continue;
        if (searchBuffer_[0] != '\0') {
            std::string lower = game.name;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            std::string needle = searchBuffer_;
            std::transform(needle.begin(), needle.end(), needle.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            if (lower.find(needle) == std::string::npos) continue;
        }
        RenderGameRow(game, onLaunch);
        anyVisible = true;
    }

    if (!anyVisible) {
        ImGui::Spacing();
        ImGui::TextColored(Theme::TextSecondary,
            games_.empty()
                ? "No games detected. Install games via Steam, Epic, or GOG,"
                : "No games match this filter.");
        if (games_.empty())
            ImGui::TextColored(Theme::TextSecondary,
                "or use '+ Add Game' to add manually.");
    }

    RenderAddGameDialog();
}

} // namespace GameCore::UI