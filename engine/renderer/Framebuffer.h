#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include "Texture.h"

// Framebuffer 封装 - 支持多颜色附件 + 深度/模板附件
struct FramebufferSpec
{
    uint32_t Width  = 1280;
    uint32_t Height = 720;
    uint32_t Samples = 1;  // MSAA 采样数

    bool SwapChainTarget = false;  // 是否直接渲染到屏幕
    bool HDR = false;              // 是否使用浮点颜色格式 (GL_RGBA16F)
};

class Framebuffer
{
public:
    Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void Bind();
    void Unbind();
    void Resize(uint32_t width, uint32_t height);

    void BindColorAttachment(uint32_t slot = 0) const;
    void BindDepthAttachment(uint32_t slot = 0) const;

    uint32_t GetRendererID()       const { return m_RendererID; }
    uint32_t GetColorAttachmentID() const { return m_ColorAttachment; }
    uint32_t GetDepthAttachmentID() const { return m_DepthAttachment; }
    const FramebufferSpec& GetSpec() const { return m_Spec; }

    static std::shared_ptr<Framebuffer> Create(const FramebufferSpec& spec);

private:
    void Invalidate();

    FramebufferSpec m_Spec;
    uint32_t m_RendererID = 0;
    uint32_t m_ColorAttachment = 0;
    uint32_t m_DepthAttachment = 0;
};
