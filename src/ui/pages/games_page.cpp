#include "games_page.h"
#include "../themes/theme.h"

#include <imgui.h>
#include <algorithm>

namespace GameCore::UI {

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

    if (ImGui::Button("+ Add Game", ImVec2(110, 32))) {
        showAddDialog_ = true;
    }

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

    ImGui::OpenPopup("Add Game");
    if (ImGui::BeginPopupModal("Add Game", &showAddDialog_,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter path to game executable:");
        ImGui::InputText("##gamepath", addGamePathBuffer_,
                         sizeof(addGamePathBuffer_));

        ImGui::Spacing();

        if (ImGui::Button("Add", ImVec2(100, 32))) {
            // Caller will be notified via games_ vector update
            // handled at app_window level in a future iteration
            showAddDialog_ = false;
            addGamePathBuffer_[0] = '\0';
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 32))) {
            showAddDialog_ = false;
        }

        ImGui::EndPopup();
    }
}

void GamesPage::Render(LaunchCallback onLaunch)
{
    ImGui::Spacing();
    RenderFilterTabs();
    ImGui::Spacing();

    if (games_.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(Theme::TextSecondary,
            "No games detected. Install games via Steam, Epic, or GOG,");
        ImGui::TextColored(Theme::TextSecondary,
            "or use '+ Add Game' to add manually.");
    } else {
        for (const auto& game : games_) {
            RenderGameRow(game, onLaunch);
        }
    }

    RenderAddGameDialog();
}

} // namespace GameCore::UI