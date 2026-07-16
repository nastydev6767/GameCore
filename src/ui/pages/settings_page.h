#pragma once

namespace GameCore::UI {

struct AppSettings {
    bool aggressiveOptimization { false };
    bool autoOptimizeBackground { true };
    bool minimizeToTray         { true };
    bool extremeMode            { false }; // max cooling + performance, louder fans
};

class SettingsPage {
public:
    void Render(AppSettings& settings);
};

} // namespace GameCore::UI