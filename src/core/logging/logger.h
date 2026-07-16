#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace GameCore::Core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& Instance();

    void Init(const std::string& logFilePath);
    void Log(LogLevel level, const std::string& message);

    void Debug  (const std::string& msg) { Log(LogLevel::Debug,   msg); }
    void Info   (const std::string& msg) { Log(LogLevel::Info,    msg); }
    void Warning(const std::string& msg) { Log(LogLevel::Warning, msg); }
    void Error  (const std::string& msg) { Log(LogLevel::Error,   msg); }

    void SetConsoleOutput(bool enabled) { consoleOutput_ = enabled; }
    void SetMinLevel(LogLevel level)    { minLevel_ = level; }

private:
    Logger()  = default;
    ~Logger();

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream file_;
    std::mutex    mutex_;
    bool          consoleOutput_ { true };
    LogLevel      minLevel_      { LogLevel::Debug };

    static std::string LevelToString(LogLevel level);
    static std::string Timestamp();
};

#define GC_LOG_DEBUG(msg)   GameCore::Core::Logger::Instance().Debug(msg)
#define GC_LOG_INFO(msg)    GameCore::Core::Logger::Instance().Info(msg)
#define GC_LOG_WARNING(msg) GameCore::Core::Logger::Instance().Warning(msg)
#define GC_LOG_ERROR(msg)   GameCore::Core::Logger::Instance().Error(msg)

} // namespace GameCore::Core