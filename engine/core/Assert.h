#pragma once

#include "Log.h"
#include <cstdlib>

// ===== 断言系统 =====
// CORE_ASSERT(expr, msg)    - Debug/Release 都有效（致命）
// CORE_ASSERT_DEBUG(expr, msg) - 仅 Debug 有效
// CORE_STATIC_ASSERT(expr, msg) - 编译期断言

#ifdef CORE_DISABLE_ASSERTS
    #define CORE_ASSERT(expr, msg)        ((void)0)
    #define CORE_ASSERT_DEBUG(expr, msg)  ((void)0)
#else
    #define CORE_ASSERT(expr, msg) \
        do { \
            if (!(expr)) { \
                CORE_CRITICAL("ASSERTION FAILED: ", msg, "\n  Expression: ", #expr); \
                std::abort(); \
            } \
        } while(0)

    #ifdef _DEBUG
        #define CORE_ASSERT_DEBUG(expr, msg) CORE_ASSERT(expr, msg)
    #else
        #define CORE_ASSERT_DEBUG(expr, msg) ((void)0)
    #endif
#endif

// 编译期断言 (C++11 static_assert)
#define CORE_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
