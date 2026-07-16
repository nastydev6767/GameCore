#pragma once

#include <string>
#include <unordered_map>
#include <optional>

namespace GameCore::Core {

class Config {
public:
    static Config& Instance();

    bool Load(const std::string& filePath);
    bool Save(const std::string& filePath) const;

    void Set(const std::string& section,
             const std::string& key,
             const std::string& value);

    std::optional<std::string> Get(const std::string& section,
                                   const std::string& key) const;

    std::string GetOrDefault(const std::string& section,
                             const std::string& key,
                             const std::string& defaultValue) const;

    bool GetBool(const std::string& section,
                 const std::string& key,
                 bool defaultValue = false) const;

    int  GetInt (const std::string& section,
                 const std::string& key,
                 int  defaultValue = 0) const;

private:
    Config()  = default;
    ~Config() = default;

    Config(const Config&)            = delete;
    Config& operator=(const Config&) = delete;

    using SectionMap = std::unordered_map<std::string, std::string>;
    using DataMap    = std::unordered_map<std::string, SectionMap>;

    DataMap data_;

    static std::string Trim(const std::string& s);
};

} // namespace GameCore::Core