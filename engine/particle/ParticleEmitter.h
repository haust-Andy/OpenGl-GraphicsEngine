#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <random>

#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/VertexArray.h"
#include "renderer/Camera.h"

// 粒子
struct Particle
{
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Velocity  = glm::vec3(0.0f);
    glm::vec4 Color     = glm::vec4(1.0f);
    float     Size      = 1.0f;
    float     Life      = 0.0f;
    float     MaxLife   = 1.0f;
    float     Rotation  = 0.0f;

    bool IsAlive() const { return Life > 0.0f; }
    float GetLifeRatio() const { return Life / MaxLife; }
};

// 粒子发射器配置
struct ParticleEmitterConfig
{
    // 发射
    int   EmitRate         = 50;        // 每秒发射粒子数
    int   BurstCount       = 0;        // 单次爆发数量 (0 = 无爆发)
    int   MaxParticles     = 500;

    // 生命周期
    float MinLife          = 1.0f;
    float MaxLife          = 3.0f;

    // 速度
    float MinSpeed         = 1.0f;
    float MaxSpeed         = 5.0f;

    // 大小
    float StartSize        = 0.5f;
    float EndSize          = 0.0f;

    // 方向 (世界空间)
    glm::vec3 Direction    = glm::vec3(0.0f, 1.0f, 0.0f);
    float SpreadAngle      = 30.0f;    // 扩散角度 (度)

    // 颜色
    glm::vec4 StartColor  = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
    glm::vec4 EndColor    = glm::vec4(1.0f, 0.2f, 0.0f, 0.0f);

    // 重力
    glm::vec3 Gravity     = glm::vec3(0.0f, -2.0f, 0.0f);

    // 纹理
    std::shared_ptr<Texture2D> ParticleTexture;

    // 空间
    float EmitRadius      = 0.0f;      // 发射半径 (0 = 点发射)
};

// 粒子发射器组件
class ParticleEmitter
{
public:
    ParticleEmitter() = default;
    ParticleEmitter(const ParticleEmitterConfig& config) : Config(config) {}

    void Update(float dt);
    void Render(const Camera& camera);

    // 手动触发一次爆发
    void Burst(int count);

    // 启用/禁用
    bool Enabled = true;

    // 发射器位置
    glm::vec3 Position = glm::vec3(0.0f);

    // 配置
    ParticleEmitterConfig Config;

    // 统计
    uint32_t GetActiveCount() const { return (uint32_t)m_ActiveCount; }

private:
    void Emit(int count);
    void UpdateParticle(Particle& p, float dt);

    std::vector<Particle> m_Particles;
    float m_EmitAccumulator = 0.0f;
    uint32_t m_ActiveCount = 0;

    // 渲染资源 (延迟初始化)
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<VertexArray> m_QuadVAO;
    bool m_Initialized = false;

    void InitRenderer();

    // 随机数
    std::mt19937 m_RNG{ std::random_device{}() };
};
