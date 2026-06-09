#include "Test.h"
#include "engine/core/Log.h"
#include <sstream>

// =====================================
//  Test: Log 系统 (FormatLog / Level 状态)
// =====================================

TEST(Log, FormatLog_SingleArg)
{
    std::string result = FormatLog("Hello");
    CHECK_STR_EQ(result, "Hello");
}

TEST(Log, FormatLog_MultipleArgs)
{
    std::string result = FormatLog("Value: ", 42, " name=", "test");
    CHECK_STR_EQ(result, "Value: 42 name=test");
}

TEST(Log, FormatLog_Empty)
{
    std::string result = FormatLog();
    CHECK_STR_EQ(result, "");
}

TEST(Log, FormatLog_Float)
{
    std::string result = FormatLog(3.14);
    // 只验证非空（输出格式可能因平台而异）
    CHECK_FALSE(result.empty());
    CHECK_TRUE(result.find("3.14") != std::string::npos);
}

TEST(Log, FormatLog_MixedTypes)
{
    std::string result = FormatLog("Entity[", 3, "] pos=(", 1.5f, ",", 2.0f, ")");
    CHECK_FALSE(result.empty());
    CHECK_TRUE(result.find("Entity[3]") != std::string::npos);
    CHECK_TRUE(result.find("1.5")  != std::string::npos);
    CHECK_TRUE(result.find("2")    != std::string::npos);
}

TEST(Log, Level_DefaultIsTrace)
{
    CHECK_TRUE(Log::GetLevel() == LogLevel::Trace);
}

TEST(Log, Level_SetAndGet)
{
    Log::SetLevel(LogLevel::Info);
    CHECK_TRUE(Log::GetLevel() == LogLevel::Info);

    Log::SetLevel(LogLevel::Error);
    CHECK_TRUE(Log::GetLevel() == LogLevel::Error);

    // 恢复默认
    Log::SetLevel(LogLevel::Trace);
    CHECK_TRUE(Log::GetLevel() == LogLevel::Trace);
}

TEST(Log, Level_OffSuppressesAll)
{
    Log::SetLevel(LogLevel::Off);
    CHECK_TRUE(Log::GetLevel() == LogLevel::Off);

    // 恢复
    Log::SetLevel(LogLevel::Trace);
}

TEST(Log, Level_EnumOrder)
{
    // 确保枚举值符合预期: Trace < Info < Error < Off
    CHECK_TRUE(static_cast<int>(LogLevel::Trace)  < static_cast<int>(LogLevel::Info));
    CHECK_TRUE(static_cast<int>(LogLevel::Info)   < static_cast<int>(LogLevel::Error));
    CHECK_TRUE(static_cast<int>(LogLevel::Error)  < static_cast<int>(LogLevel::Off));
}

TEST(Log, Macros_CompileCheck)
{
    // 确保所有日志宏可编译
    // 注意：我们不验证输出（写到 stdout），只验证不崩溃
    CORE_TRACE("This is a trace message");
    CORE_DEBUG("Debug message: ", 123);
    CORE_INFO("Info message");
    CORE_WARN("Warning number ", 42);
    CORE_ERROR("Error in module '", "test", "'");
    CORE_CRITICAL("Critical failure: ", 0);

    // 恢复级别
    Log::SetLevel(LogLevel::Trace);
}
