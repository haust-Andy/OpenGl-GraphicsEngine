#include "PostProcess.h"
#include "core/Log.h"

// RAII 辅助类: 保存和恢复 OpenGL 状态
class GLStateSaver
{
public:
    GLStateSaver()
    {
        m_DepthTest    = glIsEnabled(GL_DEPTH_TEST);
        m_Blend        = glIsEnabled(GL_BLEND);
        m_CullFace     = glIsEnabled(GL_CULL_FACE);
        glGetIntegerv(GL_VIEWPORT, m_Viewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_PrevFBO);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &m_PrevActiveTexture);
    }

    ~GLStateSaver()
    {
        if (m_DepthTest)    glEnable(GL_DEPTH_TEST);    else glDisable(GL_DEPTH_TEST);
        if (m_Blend)        glEnable(GL_BLEND);         else glDisable(GL_BLEND);
        if (m_CullFace)     glEnable(GL_CULL_FACE);     else glDisable(GL_CULL_FACE);
        glViewport(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, m_PrevFBO);
        glActiveTexture(m_PrevActiveTexture);
    }

private:
    GLboolean m_DepthTest, m_Blend, m_CullFace;
    GLint m_Viewport[4];
    GLint m_PrevFBO;
    GLint m_PrevActiveTexture;
};

// ===== BloomPass =====
BloomPass::BloomPass(uint32_t width, uint32_t height, uint32_t blurIterations)
    : BlurIterations(blurIterations)
{
    m_BrightnessShader = Shader::Create("shader/screen.vert", "shader/brightness.frag");
    m_BlurShader       = Shader::Create("shader/screen.vert", "shader/gaussian_blur.frag");
    m_CombineShader    = Shader::Create("shader/screen.vert", "shader/bloom_combine.frag");

    FramebufferSpec spec;
    spec.Width  = width;
    spec.Height = height;
    spec.HDR    = true;  // Bloom 需要浮点格式保留 HDR 亮度

    m_BrightnessFBO = Framebuffer::Create(spec);
    m_PingFBO       = Framebuffer::Create(spec);
    m_PongFBO       = Framebuffer::Create(spec);
}

void BloomPass::Process(const std::shared_ptr<Framebuffer>& input,
                         const std::shared_ptr<Framebuffer>& output)
{
    if (!m_BrightnessShader || !m_BlurShader || !m_CombineShader) return;

    GLStateSaver stateSaver;  // M-12: 自动保存/恢复 GL 状态

    // 步骤1: 提取亮部
    glDisable(GL_DEPTH_TEST);
    m_BrightnessFBO->Bind();
    m_BrightnessShader->Bind();
    m_BrightnessShader->SetFloat("u_Threshold", Threshold);
    input->BindColorAttachment(0);
    Renderer::DrawFullscreenQuad();
    m_BrightnessFBO->Unbind();

    // 步骤2: 高斯模糊 (Ping-Pong) — 正确追踪读/写目标
    bool horizontal = true;
    std::shared_ptr<Framebuffer> currentSource = m_BrightnessFBO;

    for (uint32_t i = 0; i < BlurIterations; i++)
    {
        // 当前写目标: horizontal=true → Pong, false → Ping
        auto& currentTarget = horizontal ? m_PongFBO : m_PingFBO;
        currentTarget->Bind();

        m_BlurShader->Bind();
        m_BlurShader->SetBool("u_Horizontal", horizontal);
        currentSource->BindColorAttachment(0);

        Renderer::DrawFullscreenQuad();
        currentTarget->Unbind();

        // 下一轮: 当前写目标变成读源, 方向翻转
        currentSource = currentTarget;
        horizontal = !horizontal;
    }

    // 步骤3: 合成 — currentSource 持有最终模糊结果
    output->Bind();
    m_CombineShader->Bind();
    m_CombineShader->SetFloat("u_Intensity", Intensity);
    input->BindColorAttachment(0);
    m_CombineShader->SetInt("u_SceneTexture", 0);
    currentSource->BindColorAttachment(1);
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
    m_Shader = Shader::Create("shader/screen.vert", "shader/tonemapping.frag");
}

void ToneMappingPass::Process(const std::shared_ptr<Framebuffer>& input,
                               const std::shared_ptr<Framebuffer>& output)
{
    GLStateSaver stateSaver;  // M-12: 自动保存/恢复 GL 状态

    glDisable(GL_DEPTH_TEST);

    if (!m_Shader)
    {
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
    spec.HDR    = true;  // 管线内部 FBO 需要 HDR 支持
    m_OutputFBO  = Framebuffer::Create(spec);
    m_OutputFBO2 = Framebuffer::Create(spec);

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
    if (m_Passes.empty()) return;

    std::shared_ptr<Framebuffer> input = sceneBuffer;
    bool outputIsFBO1 = true;  // 交替使用 m_OutputFBO / m_OutputFBO2 避免同时读写同一 FBO

    for (size_t i = 0; i < m_Passes.size(); ++i)
    {
        if (!m_Passes[i]->IsEnabled()) continue;

        auto& output = outputIsFBO1 ? m_OutputFBO : m_OutputFBO2;
        m_Passes[i]->Process(input, output);
        input = output;
        outputIsFBO1 = !outputIsFBO1;
    }

    // 记录最终结果在哪个 FBO 中
    m_FinalIsFBO1 = !outputIsFBO1;
}

void PostProcessPipeline::OnResize(uint32_t width, uint32_t height)
{
    m_OutputFBO->Resize(width, height);
    m_OutputFBO2->Resize(width, height);
    for (auto& pass : m_Passes)
        pass->OnResize(width, height);
}
