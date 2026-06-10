#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// 阴影贴图封装 - 支持级联阴影 (CSM)
class ShadowMap
{
public:
    // 单个级联层
    struct CascadeInfo
    {
        glm::mat4 LightViewProjection;
        float SplitDepth;  // 该级联的远裁面 (灯光空间)
    };

    ShadowMap(uint32_t width = 2048, uint32_t height = 2048, uint32_t cascadeCount = 3);
    ~ShadowMap();

    void BindForWriting(uint32_t cascadeIndex = 0);
    void UnbindForWriting();

    void BindForReading(uint32_t startSlot = 6) const;  // 从 slot 6 开始避免与 PBR 纹理冲突

    void Resize(uint32_t width, uint32_t height);

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetCascadeCount() const { return m_CascadeCount; }

    // 计算级联分割和 VP 矩阵
    void CalculateCascades(const glm::vec3& lightDir,
                           const glm::mat4& cameraView,
                           const glm::mat4& cameraProjection,
                           float nearPlane, float farPlane,
                           const glm::vec3& sceneCenter = glm::vec3(0.0f),
                           float sceneRadius = 20.0f);

    const std::vector<CascadeInfo>& GetCascades() const { return m_Cascades; }

    // 调节参数
    float ShadowBias     = 0.005f;
    float NormalBias     = 0.02f;
    float CascadeSplitLambda = 0.95f;
    float MaxShadowDistance = 50.0f;
    bool  SoftShadows    = true;
    int   PCFSamples     = 16;
    float PCFRadius      = 2.0f;

private:
    uint32_t m_Width, m_Height;
    uint32_t m_CascadeCount;

    uint32_t m_FBO = 0;
    uint32_t m_DepthArrayTexture = 0;  // GL_TEXTURE_2D_ARRAY

    std::vector<CascadeInfo> m_Cascades;

    // 级联分割比例
    std::vector<float> m_SplitDepths;
};

// 阴影渲染器 - 深度 Pass
class ShadowRenderer
{
public:
    static void Init();
    static void Shutdown();

    // 渲染场景深度到 ShadowMap
    static void RenderShadowPass(ShadowMap& shadowMap,
                                  const std::vector<std::pair<std::shared_ptr<class VertexArray>, glm::mat4>>& renderList);

    static std::shared_ptr<class Shader> GetShadowShader() { return s_ShadowShader; }

private:
    static std::shared_ptr<class Shader> s_ShadowShader;
    static bool s_Initialized;
};
