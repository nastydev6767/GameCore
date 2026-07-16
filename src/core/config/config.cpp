#include "config.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace GameCore::Core {

Config& Config::Instance()
{
    static Config instance;
    return instance;
}

std::string Config::Trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}

bool Config::Load(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string currentSection;
    std::string line;

    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            currentSection = Trim(line.substr(1, line.size() - 2));
            continue;
        }

        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        const auto key   = Trim(line.substr(0, eq));
        const auto value = Trim(line.substr(eq + 1));

        if (!key.empty() && !currentSection.empty())
            data_[currentSection][key] = value;
    }
    return true;
}

bool Config::Save(const std::string& filePath) const
{
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    for (const auto& [section, keys] : data_) {
        file << '[' << section << "]\n";
        for (const auto& [key, value] : keys)
            file << key << '=' << value << '\n';
        file << '\n';
    }
    return true;
}

void Config::Set(const std::string& section,
                 const std::string& key,
                 const std::string& value)
{
    data_[section][key] = value;
}

std::optional<std::string> Config::Get(const std::string& section,
                                        const std::string& key) const
{
    const auto secIt = data_.find(section);
    if (secIt == data_.end()) return std::nullopt;
    const auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) return std::nullopt;
    return keyIt->second;
}

std::string Config::GetOrDefault(const std::string& section,
                                  const std::string& key,
                                  const std::string& defaultValue) const
{
    return Get(section, key).value_or(defaultValue);
}

bool Config::GetBool(const std::string& section,
                     const std::string& key,
                     bool defaultValue) const
{
    const auto val = Get(section, key);
    if (!val) return defaultValue;
    return (*val == "true" || *val == "1" || *val == "yes");
}

int Config::GetInt(const std::string& section,
                   const std::string& key,
                   int defaultValue) const
{
    const auto val = Get(section, key);
    if (!val) return defaultValue;
    try { return std::stoi(*val); }
    catch (...) { return defaultValue; }
}

} // namespace GameCore::Core