#include "PostProcess.h"
#include <iostream>

// ===== BloomPass =====
BloomPass::BloomPass(uint32_t width, uint32_t height, uint32_t blurIterations)
    : BlurIterations(blurIterations)
{
    // 此处着色器路径为示例, 实际编译从 shader/ 目录加载
    // m_BrightnessShader = Shader::Create("shader/screen.vert", "shader/brightness.frag");
    // m_BlurShader       = Shader::Create("shader/screen.vert", "shader/gaussian_blur.frag");
    // m_CombineShader    = Shader::Create("shader/screen.vert", "shader/bloom_combine.frag");

    FramebufferSpec spec;
    spec.Width  = width;
    spec.Height = height;

    m_BrightnessFBO = Framebuffer::Create(spec);
    m_PingFBO       = Framebuffer::Create(spec);
    m_PongFBO       = Framebuffer::Create(spec);
}

void BloomPass::Process(const std::shared_ptr<Framebuffer>& input,
                         const std::shared_ptr<Framebuffer>& output)
{
    if (!m_BrightnessShader || !m_BlurShader || !m_CombineShader) return;

    // 步骤1: 提取亮部
    m_BrightnessFBO->Bind();
    m_BrightnessShader->Bind();
    m_BrightnessShader->SetFloat("u_Threshold", Threshold);
    input->BindColorAttachment(0);
    Renderer::DrawFullscreenQuad();
    m_BrightnessFBO->Unbind();

    // 步骤2: 高斯模糊 (Ping-Pong)
    bool horizontal = true;
    for (uint32_t i = 0; i < BlurIterations; i++)
    {
        if (horizontal) m_PongFBO->Bind();
        else            m_PingFBO->Bind();

        m_BlurShader->Bind();
        m_BlurShader->SetBool("u_Horizontal", horizontal);

        if (i == 0)
            m_BrightnessFBO->BindColorAttachment(0);
        else if (horizontal)
            m_PingFBO->BindColorAttachment(0);
        else
            m_PongFBO->BindColorAttachment(0);

        Renderer::DrawFullscreenQuad();
        horizontal = !horizontal;

        if (horizontal) m_PongFBO->Unbind();
        else            m_PingFBO->Unbind();
    }

    // 步骤3: 合成
    output->Bind();
    m_CombineShader->Bind();
    m_CombineShader->SetFloat("u_Intensity", Intensity);
    input->BindColorAttachment(0);
    m_CombineShader->SetInt("u_SceneTexture", 0);
    m_PingFBO->BindColorAttachment(1);
    m_CombineShader->SetInt("u_BloomTexture", 1);
    Renderer::DrawFullscreenQuad();
    output->Unbind();
}

void BloomPass::OnResize(uint32_t width, uint32_t height)
{
    m_BrightnessFBO->Resize(width, height);
    m_PingFBO->Resize(width, height);
    m_PongFBO->Resize(width, height);
}

// ===== ToneMappingPass =====
ToneMappingPass::ToneMappingPass()
{
    // m_Shader = Shader::Create("shader/screen.vert", "shader/tonemapping.frag");
}

void ToneMappingPass::Process(const std::shared_ptr<Framebuffer>& input,
                               const std::shared_ptr<Framebuffer>& output)
{
    if (!m_Shader)
    {
        // 无着色器时直接将输入绘制到输出
        output->Bind();
        input->BindColorAttachment(0);
        Renderer::DrawFullscreenQuad();
        output->Unbind();
        return;
    }

    output->Bind();
    m_Shader->Bind();
    m_Shader->SetFloat("u_Exposure", Exposure);
    m_Shader->SetFloat("u_Gamma", Gamma);
    input->BindColorAttachment(0);
    m_Shader->SetInt("u_HDRTexture", 0);
    Renderer::DrawFullscreenQuad();
    output->Unbind();
}

// ===== PostProcessPipeline =====
PostProcessPipeline::PostProcessPipeline(uint32_t width, uint32_t height)
{
    FramebufferSpec spec;
    spec.Width  = width;
    spec.Height = height;
    m_OutputFBO = Framebuffer::Create(spec);

    m_BloomPass       = std::make_shared<BloomPass>(width, height);
    m_ToneMappingPass = std::make_shared<ToneMappingPass>();

    m_Passes.push_back(m_BloomPass);
    m_Passes.push_back(m_ToneMappingPass);
}

void PostProcessPipeline::AddPass(std::shared_ptr<PostProcessPass> pass)
{
    m_Passes.push_back(pass);
}

void PostProcessPipeline::Execute(const std::shared_ptr<Framebuffer>& sceneBuffer)
{
    std::shared_ptr<Framebuffer> currentInput  = sceneBuffer;
    std::shared_ptr<Framebuffer> currentOutput = m_OutputFBO;

    bool usePingPong = m_Passes.size() > 1;

    for (size_t i = 0; i < m_Passes.size(); ++i)
    {
        if (!m_Passes[i]->IsEnabled()) continue;

        if (i == m_Passes.size() - 1 || !usePingPong)
        {
            // 最后一Pass 或 单Pass: 输出到 m_OutputFBO
            m_Passes[i]->Process(currentInput, m_OutputFBO);
        }
        else
        {
            m_Passes[i]->Process(currentInput, currentOutput);
            std::swap(currentInput, currentOutput);
        }
    }
}

void PostProcessPipeline::OnResize(uint32_t width, uint32_t height)
{
    m_OutputFBO->Resize(width, height);
    for (auto& pass : m_Passes)
        pass->OnResize(width, height);
}
