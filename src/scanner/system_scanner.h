#pragma once

#include <string>

namespace GameCore::Scanner {

struct SystemInfo {
    std::string cpuName;
    std::string gpuName;
    double      ramGb;
    std::string windowsVersion;
};

class SystemScanner {
public:
    SystemInfo Scan() const;
};

} // namespace GameCore::Scanner