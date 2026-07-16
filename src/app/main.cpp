#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ui/app_window.h"
#include "core/logging/logger.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // Prevent multiple instances — important since GameCore lives in
    // the tray and auto-detects games; two copies would double-optimize.
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"GameCore_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(nullptr,
            "GameCore is already running in the system tray.",
            "GameCore", MB_ICONINFORMATION);
        return 0;
    }

    GameCore::Core::Logger::Instance().Init("gamecore.log");
    GameCore::Core::Logger::Instance().SetConsoleOutput(false);
    GC_LOG_INFO("GameCore starting...");

    GameCore::UI::AppWindow app;

    if (!app.Init(hInstance)) {
        MessageBoxA(nullptr,
            "Failed to initialize GameCore. "
            "Your GPU may not support DirectX 11.",
            "GameCore - Startup Error", MB_ICONERROR);
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }

    app.Run();
    app.Shutdown();

    GC_LOG_INFO("GameCore shutdown.");

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return 0;
}