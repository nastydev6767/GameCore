#include "logger.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <chrono>
#include <ctime>
#include <format>

namespace GameCore::Core {

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

Logger::~Logger()
{
    if (file_.is_open())
        file_.close();
}

void Logger::Init(const std::string& logFilePath)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open())
        file_.close();
    file_.open(logFilePath, std::ios::out | std::ios::app);
}

void Logger::Log(LogLevel level, const std::string& message)
{
    if (level < minLevel_)
        return;

    std::lock_guard<std::mutex> lock(mutex_);
    const std::string line = std::format("[{}] [{}] {}",
        Timestamp(), LevelToString(level), message);

    if (file_.is_open())
        file_ << line << '\n';

    if (consoleOutput_)
        std::cout << line << '\n';
}

std::string Logger::LevelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        default:                return "?????";
    }
}

std::string Logger::Timestamp()
{
    const auto now    = std::chrono::system_clock::now();
    const auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time_t);

    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

} // namespace GameCore::Core