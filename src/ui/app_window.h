#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>

#include <string>
#include <memory>

#include "pages/dashboard_page.h"
#include "pages/games_page.h"
#include "pages/monitor_page.h"
#include "pages/settings_page.h"
#include "system_tray.h"

#include "scanner/system_scanner.h"
#include "monitor/telemetry/telemetry.h"
#include "detector/game_detector/game_detector.h"
#include "detector/game_detector/game_db.h"
#include "optimizer/optimization_engine/optimization_engine.h"

namespace GameCore::UI {

enum class ActiveTab {
    Dashboard,
    Games,
    Monitor,
    Settings
};

enum class AppState {
    Normal,
    Optimizing,
    GameRunning
};

class AppWindow {
public:
    AppWindow();
    ~AppWindow();

    bool Init(HINSTANCE hInstance);
    void Run();
    void Shutdown();

private:
    bool CreateAppWindowHandle(HINSTANCE hInstance);
    bool CreateDeviceD3D();
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg,
                                  WPARAM wParam, LPARAM lParam);

    void NewFrame();
    void RenderTopNav();
    void RenderCurrentPage();
    void EndFrame();

    void LaunchGame(const Detector::DetectedGame& game);
    void CheckBackgroundGames();

    void HideToTray();
    void ShowFromTray();
    void ExitApplication();

    HWND                     hwnd_         { nullptr };
    ID3D11Device*            device_        { nullptr };
    ID3D11DeviceContext*     context_       { nullptr };
    IDXGISwapChain*          swapChain_     { nullptr };
    ID3D11RenderTargetView*  renderTarget_  { nullptr };

    bool      running_       { true };
    bool      windowVisible_ { true };
    ActiveTab activeTab_      { ActiveTab::Dashboard };
    AppState  state_          { AppState::Normal };

    Scanner::SystemScanner            scanner_;
    Scanner::SystemInfo               systemInfo_;
    Monitor::Telemetry                telemetry_;
    Detector::GameDetector            gameDetector_;
    Optimizer::OptimizationEngine     optimizer_;
    SystemTray                        tray_;

    std::unique_ptr<DashboardPage> dashboardPage_;
    std::unique_ptr<GamesPage>     gamesPage_;
    std::unique_ptr<MonitorPage>   monitorPage_;
    std::unique_ptr<SettingsPage>  settingsPage_;

    // Persisted across tabs so LaunchGame() can read
    // settings_.extremeMode when starting optimization.
    AppSettings settings_;

    float       optimizeProgress_ { 0.0f };
    std::string optimizeStatus_;
    std::string optimizingGameName_;

    Detector::DetectedGame currentGame_;
    bool                    hasRunningGame_ { false };

    static AppWindow* instance_;
};

} // namespace GameCore::UI