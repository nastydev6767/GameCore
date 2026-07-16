#pragma once

#include <string>
#include <vector>

namespace GameCore::Core::Utils {

std::string  Trim      (const std::string& s);
std::string  ToLower   (const std::string& s);
std::string  ToUpper   (const std::string& s);
bool         StartsWith(const std::string& s, const std::string& prefix);
bool         EndsWith  (const std::string& s, const std::string& suffix);
std::vector<std::string> Split(const std::string& s, char delimiter);
std::string  Join      (const std::vector<std::string>& parts, const std::string& sep);
std::wstring ToWide    (const std::string& s);
std::string  ToNarrow  (const std::wstring& s);

} // namespace GameCore::Core::Utils