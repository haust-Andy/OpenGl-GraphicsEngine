#pragma once

#include <memory>
#include <cstdint>

// ===== 基础类型别名 =====
// 避免平台相关的类型歧义
using int8   = int8_t;
using int16  = int16_t;
using int32  = int32_t;
using int64  = int64_t;
using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// ===== 智能指针别名 =====
// Ref<T>  = 共享所有权 (std::shared_ptr)
// Scope<T> = 独占所有权 (std::unique_ptr)
// 统一别名便于未来切换内存分配策略

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
