#pragma once

// ===== 时间步 =====
// 封装 deltaTime，支持不同单位获取
// 目的:
//   1. 帧率无关的更新逻辑
//   2. 锁帧 / 时间缩放
//   3. 物理步进与渲染步进分离

class Timestep
{
public:
    Timestep(float time = 0.0f)
        : m_Time(time) {}

    operator float() const { return m_Time; }

    // 以秒为单位 (默认)
    float GetSeconds() const { return m_Time; }

    // 以毫秒为单位
    float GetMilliseconds() const { return m_Time * 1000.0f; }

    // 时间缩放 (可用于慢动作/快进)
    void  SetTimeScale(float scale) { m_TimeScale = scale; }
    float GetTimeScale() const      { return m_TimeScale; }

    // 应用时间缩放后的时间
    float GetScaledSeconds()      const { return m_Time * m_TimeScale; }
    float GetScaledMilliseconds() const { return m_Time * m_TimeScale * 1000.0f; }

    // 钳制最大帧间隔 (防止编辑器失焦后巨大 deltaTime)
    static Timestep Clamp(Timestep ts, float maxSeconds = 0.1f)
    {
        if (ts.m_Time > maxSeconds)
            return Timestep(maxSeconds);
        return ts;
    }

private:
    float m_Time      = 0.0f;
    float m_TimeScale = 1.0f;
};
