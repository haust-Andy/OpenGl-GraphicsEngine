#pragma once

#include "renderer/Framebuffer.h"
#include "renderer/Shader.h"
#include "renderer/VertexArray.h"
#include "renderer/Renderer.h"
#include <memory>
#include <vector>

// 后处理 Pass 基类
class PostProcessPass
{
public:
    virtual ~PostProcessPass() = default;
    virtual void Process(const std::shared_ptr<Framebuffer>& input,
                         const std::shared_ptr<Framebuffer>& output) = 0;
    virtual void OnResize(uint32_t /*width*/, uint32_t /*height*/) {}
    virtual bool IsEnabled() const { return true; }
};

// Bloom (泛光) 后处理
class BloomPass : public PostProcessPass
{
public:
    BloomPass(uint32_t width, uint32_t height, uint32_t blurIterations = 5);
    void Process(const std::shared_ptr<Framebuffer>& input,
                 const std::shared_ptr<Framebuffer>& output) override;
    void OnResize(uint32_t width, uint32_t height) override;

    float Threshold = 0.6f;     // 亮度阈值
    float Intensity = 1.0f;     // 泛光强度
    uint32_t BlurIterations = 5;

private:
    std::shared_ptr<Shader> m_BrightnessShader;
    std::shared_ptr<Shader> m_BlurShader;
    std::shared_ptr<Shader> m_CombineShader;

    std::shared_ptr<Framebuffer> m_PingFBO;
    std::shared_ptr<Framebuffer> m_PongFBO;
    std::shared_ptr<Framebuffer> m_BrightnessFBO;
};

// 色调映射
class ToneMappingPass : public PostProcessPass
{
public:
    ToneMappingPass();
    void Process(const std::shared_ptr<Framebuffer>& input,
                 const std::shared_ptr<Framebuffer>& output) override;

    float Exposure = 1.0f;
    float Gamma    = 2.2f;

private:
    std::shared_ptr<Shader> m_Shader;
};

// 后处理管线管理器 (使用双缓冲避免同时读写同一 FBO)
class PostProcessPipeline
{
public:
    PostProcessPipeline(uint32_t width, uint32_t height);

    void AddPass(std::shared_ptr<PostProcessPass> pass);
    void Execute(const std::shared_ptr<Framebuffer>& sceneBuffer);

    void OnResize(uint32_t width, uint32_t height);

    std::shared_ptr<Framebuffer> GetFinalBuffer() const { return m_FinalIsFBO1 ? m_OutputFBO : m_OutputFBO2; }

    BloomPass& GetBloom() { return *m_BloomPass; }

private:
    std::shared_ptr<Framebuffer> m_OutputFBO;
    std::shared_ptr<Framebuffer> m_OutputFBO2;
    bool m_FinalIsFBO1 = true;
    std::shared_ptr<BloomPass>   m_BloomPass;
    std::shared_ptr<ToneMappingPass> m_ToneMappingPass;
    std::vector<std::shared_ptr<PostProcessPass>> m_Passes;
};
