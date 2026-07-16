#include "game_detector.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace GameCore::Detector {

static const std::vector<std::string> KnownGameExes = {
    "witcher3", "cyberpunk2077", "gta5", "gtav", "rdr2",
    "eldenring", "darksouls", "sekiro", "minecraft",
    "fortnite", "valorant", "csgo", "cs2", "dota2",
    "leagueoflegends", "overwatch", "battlefield",
    "callofduty", "warzone", "apex_legends", "apexlegends",
    "rainbow6", "siege", "pubg", "tslgame",
    "destiny2", "fallout4", "skyrim", "oblivion",
    "doom", "quake", "halflife", "portal",
    "terraria", "stardewvalley", "hollowknight",
    "godofwar", "spiderman", "returnal",
    "rocketleague", "nba2k", "fifa", "f12",
    "diablo", "pathofexile", "torchlight",
    "totalwar", "civilization", "ageofempires",
    "starcraft", "warcraft", "hearthstone",
    "among", "phasmophobia", "lethalcompany",
    "valheim", "rust", "dayz", "hogwartslegacy"
};

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

bool GameDetector::IsGameProcess(const std::string& exeName) {
    std::string name = ToLower(exeName);
    if (name.size() > 4 && name.substr(name.size()-4) == ".exe")
        name = name.substr(0, name.size()-4);
    for (const auto& known : KnownGameExes)
        if (name.find(known) != std::string::npos) return true;
    return false;
}

std::string GameDetector::GetProcessPath(DWORD pid) {
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return {};
    char path[MAX_PATH]{};
    DWORD size = MAX_PATH;
    QueryFullProcessImageNameA(hProcess, 0, path, &size);
    CloseHandle(hProcess);
    return std::string(path);
}

std::vector<DetectedGame> GameDetector::ScanProcesses() const {
    std::vector<DetectedGame> games;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return games;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot); return games;
    }

    do {
        std::string exeName = entry.szExeFile;
        if (IsGameProcess(exeName)) {
            DetectedGame game{};
            game.executableName = exeName;
            game.processId      = entry.th32ProcessID;
            game.isRunning      = true;
            game.executablePath = GetProcessPath(entry.th32ProcessID);

            std::string name = exeName;
            if (name.size() > 4 && name.substr(name.size()-4) == ".exe")
                name = name.substr(0, name.size()-4);
            std::replace(name.begin(), name.end(), '_', ' ');
            std::replace(name.begin(), name.end(), '-', ' ');
            game.name = name;
            games.push_back(game);
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return games;
}

std::vector<DetectedGame> GameDetector::ScanSteamLibrary() const {
    std::vector<DetectedGame> games;

    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\WOW6432Node\\Valve\\Steam",
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
        RegOpenKeyExA(HKEY_CURRENT_USER,
            "SOFTWARE\\Valve\\Steam", 0, KEY_READ, &hKey);

    if (!hKey) return games;

    char installPath[512]{};
    DWORD size = sizeof(installPath);
    RegQueryValueExA(hKey, "InstallPath", nullptr, nullptr,
                     reinterpret_cast<LPBYTE>(installPath), &size);
    RegCloseKey(hKey);
    if (strlen(installPath) == 0) return games;

    std::vector<std::string> libraryPaths;
    libraryPaths.push_back(std::string(installPath) + "\\steamapps");

    std::string vdfPath = std::string(installPath)
                        + "\\steamapps\\libraryfolders.vdf";
    std::ifstream vdf(vdfPath);
    if (vdf.is_open()) {
        std::string line;
        while (std::getline(vdf, line)) {
            auto pos = line.find("\"path\"");
            if (pos == std::string::npos) continue;
            auto q1 = line.find('"', pos + 6);
            if (q1 == std::string::npos) continue;
            auto q2 = line.find('"', q1 + 1);
            if (q2 == std::string::npos) continue;
            std::string path = line.substr(q1 + 1, q2 - q1 - 1);
            std::string fixed;
            for (size_t i = 0; i < path.size(); ++i) {
                if (path[i] == '\\' && i+1 < path.size()
                    && path[i+1] == '\\') {
                    fixed += '\\'; ++i;
                } else { fixed += path[i]; }
            }
            if (!fixed.empty())
                libraryPaths.push_back(fixed + "\\steamapps");
        }
    }

    for (const auto& libPath : libraryPaths) {
        std::error_code ec;
        for (const auto& entry :
             std::filesystem::directory_iterator(libPath, ec))
        {
            const auto fname = entry.path().filename().string();
            if (fname.find("appmanifest_") == std::string::npos) continue;

            std::ifstream acf(entry.path());
            if (!acf.is_open()) continue;

            std::string gameName, installDir, acfLine;
            while (std::getline(acf, acfLine)) {
                auto extract = [&](const std::string& key) -> std::string {
                    auto p = acfLine.find(key);
                    if (p == std::string::npos) return {};
                    auto q1 = acfLine.find('"', p + key.size());
                    if (q1 == std::string::npos) return {};
                    auto q2 = acfLine.find('"', q1 + 1);
                    if (q2 == std::string::npos) return {};
                    return acfLine.substr(q1 + 1, q2 - q1 - 1);
                };
                if (gameName.empty()) {
                    auto n = extract("\"name\"");
                    if (!n.empty()) gameName = n;
                }
                if (installDir.empty()) {
                    auto d = extract("\"installdir\"");
                    if (!d.empty()) installDir = d;
                }
            }

            if (!gameName.empty()) {
                DetectedGame game{};
                game.name           = gameName;
                game.executablePath = libPath + "\\common\\" + installDir;
                game.isRunning      = false;
                game.processId      = 0;
                games.push_back(game);
            }
        }
    }
    return games;
}

std::vector<DetectedGame> GameDetector::ScanEpicLibrary() const {
    std::vector<DetectedGame> games;

    char programData[MAX_PATH]{};
    ExpandEnvironmentStringsA("%PROGRAMDATA%", programData, MAX_PATH);
    std::string manifestPath = std::string(programData)
        + "\\Epic\\EpicGamesLauncher\\Data\\Manifests";

    std::error_code ec;
    for (const auto& entry :
         std::filesystem::directory_iterator(manifestPath, ec))
    {
        if (entry.path().extension() != ".item") continue;
        std::ifstream f(entry.path());
        if (!f.is_open()) continue;

        std::string content((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());

        auto extractJson = [&](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\": \"";
            auto pos = content.find(search);
            if (pos == std::string::npos) return {};
            pos += search.size();
            auto end = content.find('"', pos);
            if (end == std::string::npos) return {};
            return content.substr(pos, end - pos);
        };

        std::string name    = extractJson("DisplayName");
        std::string instDir = extractJson("InstallLocation");

        if (!name.empty()) {
            DetectedGame game{};
            game.name           = name;
            game.executablePath = instDir;
            game.isRunning      = false;
            game.processId      = 0;
            games.push_back(game);
        }
    }
    return games;
}

std::vector<DetectedGame> GameDetector::ScanGogLibrary() const {
    std::vector<DetectedGame> games;

    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\WOW6432Node\\GOG.com\\Games",
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return games;

    char subKeyName[256]{};
    DWORD index = 0;
    DWORD nameSize = sizeof(subKeyName);

    while (RegEnumKeyExA(hKey, index++, subKeyName,
           &nameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
    {
        nameSize = sizeof(subKeyName);
        HKEY hGame = nullptr;
        if (RegOpenKeyExA(hKey, subKeyName, 0, KEY_READ, &hGame)
            != ERROR_SUCCESS) continue;

        char gameName[512]{};
        DWORD sz = sizeof(gameName);
        RegQueryValueExA(hGame, "gameName", nullptr, nullptr,
                         reinterpret_cast<LPBYTE>(gameName), &sz);

        char installPath[512]{};
        sz = sizeof(installPath);
        RegQueryValueExA(hGame, "path", nullptr, nullptr,
                         reinterpret_cast<LPBYTE>(installPath), &sz);
        RegCloseKey(hGame);

        if (strlen(gameName) > 0) {
            DetectedGame game{};
            game.name           = gameName;
            game.executablePath = installPath;
            game.isRunning      = false;
            game.processId      = 0;
            games.push_back(game);
        }
    }
    RegCloseKey(hKey);
    return games;
}

std::vector<DetectedGame> GameDetector::ScanRunningGames() const {
    return ScanProcesses();
}

std::vector<DetectedGame> GameDetector::ScanInstalledGames() const {
    std::vector<DetectedGame> games;
    auto steam = ScanSteamLibrary();
    auto epic  = ScanEpicLibrary();
    auto gog   = ScanGogLibrary();
    games.insert(games.end(), steam.begin(), steam.end());
    games.insert(games.end(), epic.begin(),  epic.end());
    games.insert(games.end(), gog.begin(),   gog.end());
    return games;
}

std::vector<DetectedGame> GameDetector::ScanAll() const {
    auto running   = ScanRunningGames();
    auto installed = ScanInstalledGames();

    for (auto& inst : installed) {
        for (const auto& run : running) {
            if (ToLower(inst.name).find(ToLower(run.name)) != std::string::npos ||
                ToLower(run.name).find(ToLower(inst.name)) != std::string::npos)
            {
                inst.isRunning      = true;
                inst.processId      = run.processId;
                inst.executableName = run.executableName;
            }
        }
    }

    for (const auto& run : running) {
        bool found = false;
        for (const auto& inst : installed)
            if (ToLower(inst.name).find(ToLower(run.name)) != std::string::npos)
            { found = true; break; }
        if (!found) installed.push_back(run);
    }
    return installed;
}

} // namespace GameCore::Detector