#pragma once
#include <string>

namespace GameCore::Scanner {

class WindowsScanner {
public:
    std::string GetVersion() const;
};

} // namespace GameCore::Scanner