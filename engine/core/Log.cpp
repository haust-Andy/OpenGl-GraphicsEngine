#include "Log.h"
#include <iostream>
#include <chrono>
#include <iomanip>

LogLevel Log::s_Level = LogLevel::Trace;

void Log::Trace(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Trace, file, line, msg);
}

void Log::Debug(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Debug, file, line, msg);
}

void Log::Info(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Info, file, line, msg);
}

void Log::Warn(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Warn, file, line, msg);
}

void Log::Error(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Error, file, line, msg);
}

void Log::Critical(const char* file, int line, const std::string& msg)
{
    WriteLog(LogLevel::Critical, file, line, msg);
}

void Log::WriteLog(LogLevel level, const char* file, int line, const std::string& msg)
{
    if (level < s_Level) return;

    // 时间戳
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif

    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm_buf);

    // 输出格式: [HH:MM:SS.mmm] [LEVEL] file:line: message
    FILE* out = (level >= LogLevel::Error) ? stderr : stdout;

    fprintf(out, "%s[%s.%03ld]%s %s%-7s%s %s%s:%d%s: %s\n",
            "\033[90m",                              // 灰色时间戳开始
            timeBuf, ms.count(),
            "\033[0m",                               // 重置
            LevelToColor(level),
            LevelToString(level),
            "\033[0m",
            "\033[37m",                              // 白色文件路径
            file, line,
            "\033[0m",
            msg.c_str());

    fflush(out);
}

const char* Log::LevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warn:     return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "FATAL";
        default:                 return "UNKNOWN";
    }
}

const char* Log::LevelToColor(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Trace:    return "\033[37m";   // 白色
        case LogLevel::Debug:    return "\033[36m";   // 青色
        case LogLevel::Info:     return "\033[32m";   // 绿色
        case LogLevel::Warn:     return "\033[33m";   // 黄色
        case LogLevel::Error:    return "\033[31m";   // 红色
        case LogLevel::Critical: return "\033[35m";   // 紫色
        default:                 return "\033[0m";
    }
}
