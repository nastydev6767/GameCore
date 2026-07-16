#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include <string>
#include <functional>

namespace GameCore::UI {

class SystemTray {
public:
    using ShowWindowCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    bool Init(HWND ownerHwnd, HINSTANCE hInstance);
    void Shutdown();

    void HandleMessage(WPARAM wParam, LPARAM lParam);

    void SetShowWindowCallback(ShowWindowCallback cb) { onShow_ = cb; }
    void SetExitCallback(ExitCallback cb) { onExit_ = cb; }

    void SetTooltip(const std::string& text);
    void ShowNotification(const std::string& title,
                          const std::string& message);

    static constexpr UINT WM_TRAYICON = WM_USER + 1;

private:
    HWND ownerHwnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;

    NOTIFYICONDATAW nid_{};

    bool initialized_ = false;

    ShowWindowCallback onShow_;
    ExitCallback onExit_;

    void ShowContextMenu();
};

} // namespace GameCore::UI