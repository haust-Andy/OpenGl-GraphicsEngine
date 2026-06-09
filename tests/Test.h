#pragma once

// =====================================================
//  Minimal Header-Only Test Framework
//  Usage:
//    #include "Test.h"
//    TEST(SuiteName, TestName) { CHECK_EQ(1 + 1, 2); }
//  Then call TestRunner::RunAll() in main().
// =====================================================

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <exception>
#include <cmath>

namespace Test {

// ----- 测试结果 -----
struct Result {
    std::string suite;
    std::string name;
    bool        passed   = true;
    std::string message;
    double      duration = 0.0;
};

// ----- 测试注册器 -----
struct Case {
    std::string suite;
    std::string name;
    std::function<void()> func;
};

inline std::vector<Case>& AllCases() {
    static std::vector<Case> cases;
    return cases;
}

inline void Register(const std::string& suite, const std::string& name, std::function<void()> func) {
    AllCases().push_back({suite, name, std::move(func)});
}

// ----- 运行器 -----
class Runner {
public:
    static int RunAll() {
        auto& cases = AllCases();
        if (cases.empty()) {
            std::cout << "\n  No tests registered.\n\n";
            return 0;
        }

        std::vector<Result> results;
        int passed = 0, failed = 0;

        std::string currentSuite;
        for (auto& tc : cases) {
            if (tc.suite != currentSuite) {
                std::cout << "\n  [" << tc.suite << "]\n";
                currentSuite = tc.suite;
            }

            Result r;
            r.suite = tc.suite;
            r.name  = tc.name;

            auto start = std::chrono::high_resolution_clock::now();
            try {
                tc.func();
                r.passed = true;
            } catch (const std::exception& e) {
                r.passed  = false;
                r.message = e.what();
            } catch (...) {
                r.passed  = false;
                r.message = "Unknown exception thrown";
            }
            auto end = std::chrono::high_resolution_clock::now();
            r.duration = std::chrono::duration<double, std::milli>(end - start).count();

            if (r.passed) {
                passed++;
                std::cout << "    \033[32mPASS\033[0m " << r.name
                          << " (" << r.duration << "ms)\n";
            } else {
                failed++;
                std::cout << "    \033[31mFAIL\033[0m " << r.name
                          << "  " << r.message << "\n";
            }
            results.push_back(r);
        }

        std::cout << "\n  " << passed << " passed, " << failed << " failed, "
                  << results.size() << " total\n\n";

        return (failed > 0) ? 1 : 0;
    }
};

} // namespace Test

// ----- 宏 -----

#define TEST_CONCAT_IMPL(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT_IMPL(a, b)

#define TEST(suite, name)                                                                       \
    static void TEST_CONCAT(test_func_, __LINE__)();                                            \
    namespace {                                                                                 \
        struct TEST_CONCAT(test_reg_, __LINE__) {                                               \
            TEST_CONCAT(test_reg_, __LINE__)() {                                                \
                Test::Register(#suite, #name, TEST_CONCAT(test_func_, __LINE__));               \
            }                                                                                   \
        } TEST_CONCAT(test_reg_inst_, __LINE__);                                                \
    }                                                                                           \
    static void TEST_CONCAT(test_func_, __LINE__)()

// ----- 断言 -----

#define CHECK(expr)                                                                   \
    do { if (!(expr)) throw std::runtime_error(                                      \
        "CHECK failed: " #expr " [" + std::string(__FILE__) + ":" +                  \
        std::to_string(__LINE__) + "]"); } while(0)

#define CHECK_TRUE(expr)  CHECK(expr)
#define CHECK_FALSE(expr) CHECK(!(expr))

#define CHECK_EQ(a, b)                                                                 \
    do { auto va = (a); auto vb = (b);                                                \
        if (!(va == vb)) {                                                            \
            throw std::runtime_error(std::string("CHECK_EQ failed: ") + #a + " == " + #b \
                + "  [" + std::to_string(va) + " != " + std::to_string(vb) + "]");    \
        } } while(0)

#define CHECK_NE(a, b)                                                                 \
    do { auto va = (a); auto vb = (b);                                                \
        if (!(va != vb)) {                                                            \
            throw std::runtime_error(std::string("CHECK_NE failed: ") + #a + " != " + #b \
                + "  [both are " + std::to_string(va) + "]");                         \
        } } while(0)

#define CHECK_FLOAT_EQ(a, b, eps)                                                     \
    do { auto va = (a); auto vb = (b);                                                \
        if (std::abs(va - vb) > (eps)) {                                              \
            throw std::runtime_error(std::string("CHECK_FLOAT_EQ failed: ") + #a + " == " + #b \
                + "  [" + std::to_string(va) + " != " + std::to_string(vb)            \
                + ", eps=" + std::to_string(eps) + "]");                              \
        } } while(0)

#define CHECK_STR_EQ(a, b)                                                             \
    do { std::string va = (a); std::string vb = (b);                                  \
        if (va != vb) {                                                               \
            throw std::runtime_error(std::string("CHECK_STR_EQ failed: ") + #a + " == " + #b \
                + "  [\"" + va + "\" != \"" + vb + "\"]");                            \
        } } while(0)

#define CHECK_THROWS(expr)                                                             \
    do { bool threw = false; try { (void)(expr); } catch (...) { threw = true; }       \
        if (!threw) throw std::runtime_error(                                         \
            "CHECK_THROWS: no exception thrown: " #expr); } while(0)

#define CHECK_NOTHROW(expr)                                                            \
    do { try { (void)(expr); } catch (const std::exception& e) {                       \
        throw std::runtime_error(std::string("CHECK_NOTHROW: unexpected exception: ")  \
            + e.what() + " [" #expr "]"); }                                           \
        catch (...) { throw std::runtime_error(                                       \
            "CHECK_NOTHROW: unknown exception [" #expr "]"); }                        \
    } while(0)
