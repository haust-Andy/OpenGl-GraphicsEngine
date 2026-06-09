#include "Test.h"
#include "engine/core/Base.h"
#include <string>

// =====================================
//  Test: Base types & smart pointer aliases
// =====================================

TEST(Base, FixedWidthInts_Sizes)
{
    CHECK_EQ(sizeof(int8),  1u);
    CHECK_EQ(sizeof(uint8), 1u);
    CHECK_EQ(sizeof(int16),  2u);
    CHECK_EQ(sizeof(uint16), 2u);
    CHECK_EQ(sizeof(int32),  4u);
    CHECK_EQ(sizeof(uint32), 4u);
    CHECK_EQ(sizeof(int64),  8u);
    CHECK_EQ(sizeof(uint64), 8u);
}

TEST(Base, CreateRef_BuildsSharedPtr)
{
    // CreateRef 产生 shared_ptr<int>
    auto r = CreateRef<int>(42);
    CHECK_TRUE(r != nullptr);
    CHECK_EQ(*r, 42);
    CHECK_EQ(r.use_count(), 1);

    // 拷贝增加引用计数
    {
        Ref<int> r2 = r;
        CHECK_EQ(r.use_count(), 2);
    }
    CHECK_EQ(r.use_count(), 1);
}

struct TestObj {
    int x;
    float y;
    TestObj(int a, float b) : x(a), y(b) {}
};

TEST(Base, CreateRef_ForwardsToConstructor)
{
    auto r = CreateRef<TestObj>(10, 3.14f);
    CHECK_EQ(r->x, 10);
    CHECK_FLOAT_EQ(r->y, 3.14f, 0.001f);
}

TEST(Base, CreateScope_BuildsUniquePtr)
{
    auto s = CreateScope<TestObj>(5, 2.5f);
    CHECK_TRUE(s != nullptr);
    CHECK_EQ(s->x, 5);
    CHECK_FLOAT_EQ(s->y, 2.5f, 0.001f);

    // unique_ptr 不可拷贝 (编译期约束)
    // 但可移动
    Scope<TestObj> s2 = std::move(s);
    CHECK_TRUE(s2 != nullptr);
    CHECK_TRUE(s == nullptr);
}

TEST(Base, TypeAliases_CompileCheck)
{
    // int32 / uint32 是可用的类型别名
    int32  a = -100;
    uint32 b = 200;
    CHECK_EQ(a, -100);
    CHECK_EQ(b, 200u);

    // 确保 int32 是有符号的
    CHECK_TRUE(a < 0);
}
