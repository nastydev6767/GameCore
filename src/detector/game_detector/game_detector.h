#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace GameCore::Detector {

enum class GameSource {
    Unknown,
    Steam,
    Epic,
    Gog,
    Running,  // detected from running process
    Manual,   // added by user
};

struct DetectedGame {
    std::string name;
    std::string executableName;
    std::string executablePath;
    DWORD       processId;
    bool        isRunning;
    GameSource  source { GameSource::Unknown };
};

class GameDetector {
public:
    std::vector<DetectedGame> ScanRunningGames()   const;
    std::vector<DetectedGame> ScanInstalledGames() const;
    std::vector<DetectedGame> ScanAll()            const;

private:
    std::vector<DetectedGame> ScanProcesses()    const;
    std::vector<DetectedGame> ScanSteamLibrary() const;
    std::vector<DetectedGame> ScanEpicLibrary()  const;
    std::vector<DetectedGame> ScanGogLibrary()   const;

    static bool        IsGameProcess(const std::string& exeName);
    static std::string GetProcessPath(DWORD pid);
};

} // namespace GameCore::Detector