#include "Test.h"
#include "engine/core/Timestep.h"

// =====================================
//  Test: Timestep
// =====================================

TEST(Timestep, DefaultConstructor)
{
    Timestep ts;
    CHECK_FLOAT_EQ(ts.GetSeconds(), 0.0f, 0.0001f);
    CHECK_FLOAT_EQ(ts.GetMilliseconds(), 0.0f, 0.0001f);
}

TEST(Timestep, ValueConstructor)
{
    Timestep ts(0.016f); // ~60 FPS
    CHECK_FLOAT_EQ(ts.GetSeconds(), 0.016f, 0.0001f);
    CHECK_FLOAT_EQ(ts.GetMilliseconds(), 16.0f, 0.0001f);
}

TEST(Timestep, ImplicitFloatConversion)
{
    Timestep ts(0.5f);
    float f = ts;
    CHECK_FLOAT_EQ(f, 0.5f, 0.0001f);
}

TEST(Timestep, DefaultTimeScale_IsOne)
{
    Timestep ts(0.1f);
    CHECK_FLOAT_EQ(ts.GetTimeScale(), 1.0f, 0.0001f);
}

TEST(Timestep, SetTimeScale_AffectsScaledSeconds)
{
    Timestep ts(0.1f);         // 0.1 秒
    ts.SetTimeScale(2.0f);     // 2x 快进

    CHECK_FLOAT_EQ(ts.GetSeconds(),        0.1f, 0.0001f);   // 原始不变
    CHECK_FLOAT_EQ(ts.GetScaledSeconds(),  0.2f, 0.0001f);   // 缩放后
    CHECK_FLOAT_EQ(ts.GetScaledMilliseconds(), 200.0f, 0.0001f);
}

TEST(Timestep, SlowMotion_Scale)
{
    Timestep ts(0.1f);
    ts.SetTimeScale(0.25f);    // 1/4x 慢动作

    CHECK_FLOAT_EQ(ts.GetScaledSeconds(),  0.025f, 0.0001f);
    CHECK_FLOAT_EQ(ts.GetScaledMilliseconds(), 25.0f, 0.0001f);
}

TEST(Timestep, Clamp_WithinLimit)
{
    // 正常帧时间不应被钳制
    Timestep ts(0.016f);
    Timestep clamped = Timestep::Clamp(ts);
    CHECK_FLOAT_EQ(clamped.GetSeconds(), 0.016f, 0.0001f);
}

TEST(Timestep, Clamp_ExceedsDefaultLimit)
{
    // 编辑器失焦后 5 秒的巨大 delta 应被钳制为 0.1
    Timestep ts(5.0f);
    Timestep clamped = Timestep::Clamp(ts);
    CHECK_FLOAT_EQ(clamped.GetSeconds(), 0.1f, 0.0001f);
}

TEST(Timestep, Clamp_CustomLimit)
{
    Timestep ts(0.2f);
    Timestep clamped = Timestep::Clamp(ts, 0.05f);
    CHECK_FLOAT_EQ(clamped.GetSeconds(), 0.05f, 0.0001f);
}

TEST(Timestep, MultipleLowFrames)
{
    // 连续多帧正常值
    Timestep frames[] = {
        Timestep(0.016f), Timestep(0.017f),
        Timestep(0.015f), Timestep(0.100f)  // 最后一帧刚好等于默认上限
    };

    for (int i = 0; i < 3; i++) {
        auto c = Timestep::Clamp(frames[i]);
        CHECK_FLOAT_EQ(c.GetSeconds(), frames[i].GetSeconds(), 0.0001f);
    }

    auto c4 = Timestep::Clamp(frames[3]);
    CHECK_FLOAT_EQ(c4.GetSeconds(), 0.1f, 0.0001f);
}
