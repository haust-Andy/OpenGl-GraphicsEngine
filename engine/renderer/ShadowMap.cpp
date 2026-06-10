#include "ShadowMap.h"
#include "Shader.h"
#include "VertexArray.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ===== ShadowMap =====

ShadowMap::ShadowMap(uint32_t width, uint32_t height, uint32_t cascadeCount)
    : m_Width(width), m_Height(height), m_CascadeCount(cascadeCount)
{
    m_Cascades.resize(cascadeCount);
    m_SplitDepths.resize(cascadeCount);

    // 创建深度纹理数组 (每个级联一层)
    glGenFramebuffers(1, &m_FBO);
    glGenTextures(1, &m_DepthArrayTexture);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthArrayTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 m_Width, m_Height, m_CascadeCount,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D_ARRAY, m_DepthArrayTexture, 0);
    // 不需要颜色输出
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[ShadowMap] Framebuffer incomplete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_DepthArrayTexture);
}

void ShadowMap::BindForWriting(uint32_t cascadeIndex)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                m_DepthArrayTexture, 0, cascadeIndex);
    glViewport(0, 0, m_Width, m_Height);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::UnbindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::BindForReading(uint32_t startSlot) const
{
    glActiveTexture(GL_TEXTURE0 + startSlot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthArrayTexture);
}

void ShadowMap::Resize(uint32_t width, uint32_t height)
{
    if (width == m_Width && height == m_Height) return;
    m_Width = width;
    m_Height = height;

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthArrayTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 m_Width, m_Height, m_CascadeCount,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
}

void ShadowMap::CalculateCascades(const glm::vec3& lightDir,
                                   const glm::mat4& cameraView,
                                   const glm::mat4& cameraProjection,
                                   float nearPlane, float farPlane,
                                   const glm::vec3& sceneCenter,
                                   float sceneRadius)
{
    // 计算级联分割距离 (PSSM 混合)
    for (uint32_t i = 0; i < m_CascadeCount; ++i)
    {
        float p = (i + 1) / (float)m_CascadeCount;
        float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
        float uniSplit = nearPlane + (farPlane - nearPlane) * p;
        m_SplitDepths[i] = CascadeSplitLambda * logSplit + (1.0f - CascadeSplitLambda) * uniSplit;
    }

    // 提取视锥体角点
    glm::mat4 invVP = glm::inverse(cameraProjection * cameraView);
    glm::vec3 frustumCorners[8];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 ndc(
            (i & 1) ? 1.0f : -1.0f,
            (i & 2) ? 1.0f : -1.0f,
            (i & 4) ? 1.0f : -1.0f,
            1.0f
        );
        glm::vec4 world = invVP * ndc;
        frustumCorners[i] = glm::vec3(world) / world.w;
    }

    // 对每个级联计算光空间矩阵
    glm::vec3 lightDirNorm = glm::normalize(lightDir);

    for (uint32_t i = 0; i < m_CascadeCount; ++i)
    {
        float prevSplit = (i == 0) ? nearPlane : m_SplitDepths[i - 1];
        float currSplit = m_SplitDepths[i];

        // 插值视锥体角点到当前级联范围
        glm::vec3 cascadeCorners[8];
        for (int j = 0; j < 4; ++j)
        {
            glm::vec3 ray = frustumCorners[j + 4] - frustumCorners[j];
            cascadeCorners[j]     = frustumCorners[j] + ray * prevSplit;
            cascadeCorners[j + 4] = frustumCorners[j] + ray * currSplit;
        }

        // 计算级联的中心和半径
        glm::vec3 center(0.0f);
        for (int j = 0; j < 8; ++j)
            center += cascadeCorners[j];
        center /= 8.0f;

        float radius = 0.0f;
        for (int j = 0; j < 8; ++j)
            radius = glm::max(radius, glm::length(cascadeCorners[j] - center));
        radius = std::ceil(radius * 16.0f) / 16.0f;  // 对齐到像素

        // 构建光空间视图投影矩阵
        glm::vec3 eye = center - lightDirNorm * radius;
        glm::mat4 lightView = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius,
                                          0.0f, radius * 3.0f);

        m_Cascades[i].LightViewProjection = lightProj * lightView;
        m_Cascades[i].SplitDepth = currSplit;
    }
}

// ===== ShadowRenderer =====

std::shared_ptr<Shader> ShadowRenderer::s_ShadowShader;
bool ShadowRenderer::s_Initialized = false;

void ShadowRenderer::Init()
{
    if (s_Initialized) return;

    // 深度 Pass 着色器 - 只写深度
    const char* shadowVertSrc = R"(
#version 330 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_LightViewProjection;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightViewProjection * u_Model * vec4(a_Position, 1.0);
}
)";
    const char* shadowFragSrc = R"(
#version 330 core
void main()
{
    // 深度自动写入
}
)";
    s_ShadowShader = std::make_shared<Shader>();
    s_ShadowShader->Compile(shadowVertSrc, shadowFragSrc);
    s_ShadowShader->SetName("ShadowDepth");

    s_Initialized = true;
    std::cout << "[ShadowRenderer] Initialized" << std::endl;
}

void ShadowRenderer::Shutdown()
{
    s_ShadowShader.reset();
    s_Initialized = false;
}

void ShadowRenderer::RenderShadowPass(ShadowMap& shadowMap,
                                        const std::vector<std::pair<std::shared_ptr<class VertexArray>, glm::mat4>>& renderList)
{
    if (!s_ShadowShader) return;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);  // Peter Panning 补偿: 剔除正面减少自阴影

    for (uint32_t cascade = 0; cascade < shadowMap.GetCascadeCount(); ++cascade)
    {
        shadowMap.BindForWriting(cascade);

        s_ShadowShader->Bind();
        s_ShadowShader->SetMat4("u_LightViewProjection",
                                 shadowMap.GetCascades()[cascade].LightViewProjection);

        for (auto& [vao, transform] : renderList)
        {
            s_ShadowShader->SetMat4("u_Model", transform);
            vao->Bind();
            auto& ib = vao->GetIndexBuffer();
            if (ib)
                glDrawElements(GL_TRIANGLES, (GLsizei)ib->GetCount(), GL_UNSIGNED_INT, 0);
        }

        shadowMap.UnbindForWriting();
    }

    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
}
