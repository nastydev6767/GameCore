// ─────────────────────────────────────────────────────────────────────────────
// FILE : src/ui/app_window.cpp   [MODIFIED]
// Changes from Sprint 1:
//   • thermal_.Init() called async on a background thread after UI is ready
//   • Optimization summary shows thermal result line
//   • Telemetry::Update now pulls LHM fan/GPU readings from ThermalManager
// ─────────────────────────────────────────────────────────────────────────────
#include "app_window.h"
#include "themes/theme.h"
#include "core/logging/logger.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <shellapi.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <thread>
#include <chrono>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GameCore::UI {

AppWindow* AppWindow::instance_ = nullptr;

AppWindow::AppWindow()
    : dashboardPage_(std::make_unique<DashboardPage>())
    , gamesPage_(std::make_unique<GamesPage>())
    , monitorPage_(std::make_unique<MonitorPage>())
    , settingsPage_(std::make_unique<SettingsPage>())
{
    instance_ = this;
}

AppWindow::~AppWindow()
{
    Shutdown();
    instance_ = nullptr;
}

LRESULT WINAPI AppWindow::WndProc(HWND hWnd, UINT msg,
                                  WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            return 0;

        case WM_CLOSE:
            if (instance_) instance_->HideToTray();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case SystemTray::WM_TRAYICON:
            if (instance_) instance_->tray_.HandleMessage(wParam, lParam);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool AppWindow::CreateAppWindowHandle(HINSTANCE hInstance)
{
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"GameCoreWindowClass";
    wc.hCursor       = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCEW(1));

    RegisterClassExW(&wc);

    hwnd_ = CreateWindowW(
        wc.lpszClassName, L"GameCore",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1100, 720,
        nullptr, nullptr, hInstance, nullptr);

    return hwnd_ != nullptr;
}

bool AppWindow::CreateDeviceD3D()
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount       = 2;
    sd.BufferDesc.Width  = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow      = hwnd_;
    sd.SampleDesc.Count  = 1;
    sd.Windowed          = TRUE;
    sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
        levels, 1, D3D11_SDK_VERSION, &sd,
        &swapChain_, &device_, &featureLevel, &context_);

    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

void AppWindow::CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer) {
        device_->CreateRenderTargetView(backBuffer, nullptr, &renderTarget_);
        backBuffer->Release();
    }
}

void AppWindow::CleanupRenderTarget()
{
    if (renderTarget_) { renderTarget_->Release(); renderTarget_ = nullptr; }
}

void AppWindow::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (swapChain_) { swapChain_->Release(); swapChain_ = nullptr; }
    if (context_)   { context_->Release();   context_   = nullptr; }
    if (device_)    { device_->Release();    device_    = nullptr; }
}

bool AppWindow::Init(HINSTANCE hInstance)
{
    if (!CreateAppWindowHandle(hInstance)) return false;
    if (!CreateDeviceD3D())               return false;

    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    Theme::Apply();

    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(device_, context_);

    tray_.Init(hwnd_, hInstance);
    tray_.SetTooltip("GameCore - Idle");
    tray_.SetShowWindowCallback([this]() { ShowFromTray(); });
    tray_.SetExitCallback([this]() { ExitApplication(); });

    systemInfo_ = scanner_.Scan();
    GC_LOG_INFO("UI initialized. CPU: " + systemInfo_.cpuName);

    LoadSettings();
    gamesPage_->RefreshGames(gameDetector_);

    // ── Thermal init on a background thread ──────────────────────────────────
    // Runs setup_thermal.ps1 silently on first launch (downloads LHM + PawnIO).
    // All subsequent launches complete in milliseconds.
    std::thread([this]() {
        optimizer_.GetThermal().Init();
    }).detach();

    return true;
}

void AppWindow::LoadSettings()
{
    auto& cfg = GameCore::Core::Config::Instance();
    cfg.Load(kConfigPath);

    settings_.aggressiveOptimization =
        cfg.GetBool("Settings", "AggressiveOptimization", false);
    settings_.autoOptimizeBackground =
        cfg.GetBool("Settings", "AutoOptimizeBackground", true);
    settings_.minimizeToTray =
        cfg.GetBool("Settings", "MinimizeToTray", true);
    settings_.extremeMode =
        cfg.GetBool("Settings", "ExtremeMode", false);

    GC_LOG_INFO("Settings loaded from " + std::string(kConfigPath));
}

void AppWindow::SaveSettings()
{
    auto& cfg = GameCore::Core::Config::Instance();

    cfg.Set("Settings", "AggressiveOptimization",
            settings_.aggressiveOptimization ? "true" : "false");
    cfg.Set("Settings", "AutoOptimizeBackground",
            settings_.autoOptimizeBackground ? "true" : "false");
    cfg.Set("Settings", "MinimizeToTray",
            settings_.minimizeToTray ? "true" : "false");
    cfg.Set("Settings", "ExtremeMode",
            settings_.extremeMode ? "true" : "false");

    cfg.Save(kConfigPath);
    GC_LOG_INFO("Settings saved to " + std::string(kConfigPath));
}

void AppWindow::HideToTray()
{
    ShowWindow(hwnd_, SW_HIDE);
    windowVisible_ = false;
    tray_.ShowNotification("GameCore",
        "Still running in the background. "
        "GameCore will auto-optimize when you launch a game.");
}

void AppWindow::ShowFromTray()
{
    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
    windowVisible_ = true;
}

void AppWindow::ExitApplication()
{
    running_ = false;
}

void AppWindow::NewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void AppWindow::RenderTopNav()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 56));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::NavBar);
    ImGui::Begin("##TopNav", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    ImGui::SetCursorPos(ImVec2(20, 16));
    ImGui::TextColored(Theme::TextPrimary, "GameCore");

    if (settings_.extremeMode) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::Danger, "  EXTREME MODE");
    }

    ImGui::SameLine(220);
    ImGui::SetCursorPosY(12);

    auto navButton = [this](const char* label, ActiveTab tab) {
        const bool active = (activeTab_ == tab);
        ImGui::PushStyleColor(ImGuiCol_Button,
            active ? Theme::NavActive : Theme::NavBar);
        if (ImGui::Button(label, ImVec2(100, 32)))
            activeTab_ = tab;
        ImGui::PopStyleColor();
        ImGui::SameLine();
    };

    navButton("Dashboard", ActiveTab::Dashboard);
    navButton("Games",     ActiveTab::Games);
    navButton("Monitor",   ActiveTab::Monitor);
    navButton("Settings",  ActiveTab::Settings);

    ImGui::End();
    ImGui::PopStyleColor();
}

void AppWindow::LaunchGame(const Detector::DetectedGame& game)
{
    state_              = AppState::Optimizing;
    optimizingGameName_ = game.name;
    optimizeProgress_   = 0.0f;
    optimizeStatus_     = "Starting...";

    auto req = Detector::GameDb::Instance().Find(game.name);
    const Detector::GameRequirements* reqPtr = req ? &(*req) : nullptr;

    Detector::HardwareCapability cap{};
    const Detector::HardwareCapability* capPtr = nullptr;
    if (req) {
        cap = Detector::GameDb::Instance().Analyze(
            *req, 10.0, systemInfo_.ramGb, 2048);
        capPtr = &cap;
    }

    optimizer_.Optimize(game.name, reqPtr, capPtr,
        [this](float progress, const std::string& status) {
            optimizeProgress_ = progress;
            optimizeStatus_   = status;
        },
        settings_.extremeMode);

    if (!game.executablePath.empty()) {
        ShellExecuteA(nullptr, "open", game.executablePath.c_str(),
                     nullptr, nullptr, SW_SHOW);
    }

    currentGame_           = game;
    currentGame_.isRunning = true;
    hasRunningGame_        = true;

    tray_.SetTooltip("GameCore - " + game.name + " running"
        + (settings_.extremeMode ? " (Extreme Mode)" : ""));
    tray_.ShowNotification("GameCore",
        "Optimized and launched " + game.name
        + (settings_.extremeMode
            ? ". Extreme Mode active."
            : ""));

    state_     = AppState::GameRunning;
    activeTab_ = ActiveTab::Monitor;
}

void AppWindow::CheckBackgroundGames()
{
    static int frameCounter = 0;
    if (++frameCounter < 180) return;
    frameCounter = 0;

    if (hasRunningGame_) return;
    if (!settings_.autoOptimizeBackground) return;

    auto running = gameDetector_.ScanRunningGames();
    if (!running.empty() && !optimizer_.IsOptimized())
        LaunchGame(running.front());
}

void AppWindow::RenderCurrentPage()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 56));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 56));

    ImGui::Begin("##MainContent", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove);

    if (state_ == AppState::Optimizing) {
        const float winW = ImGui::GetWindowWidth();
        const float winH = ImGui::GetWindowHeight();

        ImGui::SetCursorPos(ImVec2(winW * 0.5f - 200, winH * 0.5f - 140));
        ImGui::BeginGroup();

        ImGui::TextColored(Theme::TextPrimary,
            "Optimizing for %s", optimizingGameName_.c_str());

        if (settings_.extremeMode)
            ImGui::TextColored(Theme::Danger, "Extreme Mode");

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
            settings_.extremeMode ? Theme::Danger : Theme::Accent);
        ImGui::ProgressBar(optimizeProgress_, ImVec2(400, 24));
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::TextColored(Theme::TextSecondary, "%s",
                           optimizeStatus_.c_str());

        if (optimizeProgress_ >= 1.0f) {
            ImGui::Spacing();
            ImGui::Spacing();
            const auto& snap = optimizer_.GetSnapshot();

            ImGui::TextColored(Theme::Success,
                "✓ %d processes closed   ✓ %.0f MB freed   ✓ %d services paused",
                static_cast<int>(snap.killedProcesses.size()),
                snap.freedMemoryMb,
                static_cast<int>(snap.stoppedServices.size()));

            if (snap.networkTweakResult.naggleDisabled ||
                snap.networkTweakResult.networkThrottleOff)
                ImGui::TextColored(Theme::Success,
                    "✓ Network: Nagle off, DNS flushed, QoS applied");

            if (snap.registryTweakResult.gameDvrDisabled ||
                snap.registryTweakResult.mmcssGameProfileSet)
                ImGui::TextColored(Theme::Success,
                    "✓ Registry: GameDVR off, HAGS on, MMCSS boosted");

            // Thermal summary — no fan/PawnIO mentions
            if (snap.thermalResult.thermalReady) {
                ImGui::TextColored(Theme::Success,
                    snap.thermalResult.fansMaximized
                        ? "✓ Hardware performance: Maximum (%d controller%s)"
                        : "✓ Hardware performance: Standard",
                    snap.thermalResult.fanControllersFound,
                    snap.thermalResult.fanControllersFound == 1 ? "" : "s");
            }
        }

        ImGui::EndGroup();

    } else {
        switch (activeTab_) {
            case ActiveTab::Dashboard:
                dashboardPage_->Render(systemInfo_, telemetry_.GetSnapshot());
                break;

            case ActiveTab::Games:
                gamesPage_->Render([this](const Detector::DetectedGame& g) {
                    LaunchGame(g);
                });
                break;

            case ActiveTab::Monitor:
                monitorPage_->Render(
                    telemetry_.GetSnapshot(),
                    hasRunningGame_ ? &currentGame_ : nullptr,
                    [this]() {
                        optimizer_.Restore();
                        hasRunningGame_ = false;
                        state_ = AppState::Normal;
                        tray_.SetTooltip("GameCore - Idle");
                    });
                break;

            case ActiveTab::Settings: {
                AppSettings prev = settings_;
                settingsPage_->Render(settings_);
                if (settings_.aggressiveOptimization != prev.aggressiveOptimization ||
                    settings_.autoOptimizeBackground  != prev.autoOptimizeBackground ||
                    settings_.minimizeToTray          != prev.minimizeToTray         ||
                    settings_.extremeMode             != prev.extremeMode)
                {
                    SaveSettings();
                }
                break;
            }
        }
    }

    ImGui::End();
}

void AppWindow::EndFrame()
{
    ImGui::Render();

    const float clearColor[4] = { 0.13f, 0.13f, 0.13f, 1.0f };
    context_->OMSetRenderTargets(1, &renderTarget_, nullptr);
    context_->ClearRenderTargetView(renderTarget_, clearColor);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain_->Present(1, 0);
}

void AppWindow::Run()
{
    MSG msg{};

    while (running_) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running_ = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running_) break;

        telemetry_.Update();
        CheckBackgroundGames();

        if (gamesPage_->NeedsRefresh()) {
            gamesPage_->RefreshGames(gameDetector_);
            gamesPage_->ClearRefresh();
        }

        if (windowVisible_) {
            NewFrame();
            RenderTopNav();
            RenderCurrentPage();
            EndFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void AppWindow::Shutdown()
{
    SaveSettings();

    if (optimizer_.IsOptimized())
        optimizer_.Restore();

    tray_.Shutdown();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();

    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

} // namespace GameCore::UI