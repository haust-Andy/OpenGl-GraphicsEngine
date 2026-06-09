#pragma once

#include <cstring>
#include <string>
#include <cstdio>
#include <ctime>
#include <sstream>

// ===== 日志级别 =====
enum class LogLevel
{
    Trace    = 0,
    Debug    = 1,
    Info     = 2,
    Warn     = 3,
    Error    = 4,
    Critical = 5,
    Off      = 6
};

// ===== 日志系统 =====
class Log
{
public:
    static void SetLevel(LogLevel level) { s_Level = level; }
    static LogLevel GetLevel()           { return s_Level; }

    static void Trace   (const char* file, int line, const std::string& msg);
    static void Debug   (const char* file, int line, const std::string& msg);
    static void Info    (const char* file, int line, const std::string& msg);
    static void Warn    (const char* file, int line, const std::string& msg);
    static void Error   (const char* file, int line, const std::string& msg);
    static void Critical(const char* file, int line, const std::string& msg);

private:
    static void WriteLog(LogLevel level, const char* file, int line, const std::string& msg);
    static const char* LevelToString(LogLevel level);
    static const char* LevelToColor(LogLevel level);

    static LogLevel s_Level;
};

// ===== 便捷宏 =====
// 使用 __FILE__ 和 __LINE__ 自动捕获位置信息

// __FILENAME__: 仅取文件名，去掉完整路径
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : \
                      strrchr(__FILE__, '/')  ? strrchr(__FILE__, '/')  + 1 : __FILE__)

#define CORE_TRACE(...)    Log::Trace   (__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))
#define CORE_DEBUG(...)    Log::Debug   (__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))
#define CORE_INFO(...)     Log::Info    (__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))
#define CORE_WARN(...)     Log::Warn    (__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))
#define CORE_ERROR(...)    Log::Error   (__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))
#define CORE_CRITICAL(...) Log::Critical(__FILENAME__, __LINE__, FormatLog(__VA_ARGS__))

// 辅助函数: 将多个参数拼接为字符串
template<typename... Args>
inline std::string FormatLog(Args&&... args)
{
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    return oss.str();
}
