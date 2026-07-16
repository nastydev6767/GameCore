#include "system_tray.h"
#include <shellapi.h>
#include <vector>

namespace GameCore::UI {

static std::wstring ToWide(const std::string& s)
{
    if (s.empty()) return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(len, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), len);
    return out;
}

bool SystemTray::Init(HWND ownerHwnd, HINSTANCE hInstance)
{
    ownerHwnd_ = ownerHwnd;
    hInstance_ = hInstance;

    ZeroMemory(&nid_, sizeof(nid_));

    nid_.cbSize = sizeof(NOTIFYICONDATAW);
    nid_.hWnd = ownerHwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON;
    nid_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    wcscpy_s(nid_.szTip, L"GameCore");

    initialized_ = Shell_NotifyIconW(NIM_ADD, &nid_);
    return initialized_;
}

void SystemTray::Shutdown()
{
    if (!initialized_) return;
    Shell_NotifyIconW(NIM_DELETE, &nid_);
    initialized_ = false;
}

void SystemTray::SetTooltip(const std::string& text)
{
    if (!initialized_) return;

    auto wide = ToWide(text);
    wcsncpy_s(nid_.szTip, wide.c_str(), _TRUNCATE);

    nid_.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &nid_);
}

void SystemTray::ShowNotification(const std::string& title,
                                  const std::string& message)
{
    if (!initialized_) return;

    NOTIFYICONDATAW data = {};
    data.cbSize = sizeof(data);
    data.hWnd = ownerHwnd_;
    data.uID = 1;
    data.uFlags = NIF_INFO;

    auto wTitle = ToWide(title);
    auto wMsg = ToWide(message);

    wcscpy_s(data.szInfoTitle, wTitle.c_str());
    wcscpy_s(data.szInfo, wMsg.c_str());
    data.dwInfoFlags = NIIF_INFO;

    Shell_NotifyIconW(NIM_MODIFY, &data);
}

void SystemTray::ShowContextMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"Open");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 2, L"Exit");

    SetForegroundWindow(ownerHwnd_);

    int cmd = TrackPopupMenu(menu,
        TPM_RETURNCMD | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, ownerHwnd_, nullptr);

    DestroyMenu(menu);

    if (cmd == 1 && onShow_) onShow_();
    if (cmd == 2 && onExit_) onExit_();
}

void SystemTray::HandleMessage(WPARAM wParam, LPARAM lParam)
{
    if (wParam != nid_.uID) return;

    switch (lParam)
    {
        case WM_LBUTTONDBLCLK:
            if (onShow_) onShow_();
            break;

        case WM_RBUTTONUP:
            ShowContextMenu();
            break;
    }
}

} // namespace GameCore::UI