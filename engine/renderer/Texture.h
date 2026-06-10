#pragma once

#include <glad/glad.h>
#include <string>
#include <memory>
#include <vector>

// 纹理抽象 - 支持 2D 纹理和立方体贴图
class Texture
{
public:
    enum class Format
    {
        RGB, RGBA, RED, Depth
    };

    enum class Filter
    {
        Nearest, Linear, LinearMipmapLinear
    };

    enum class Wrap
    {
        Repeat, ClampToEdge, ClampToBorder
    };

    struct Spec
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
        Format   ImageFormat = Format::RGBA;
        Filter   MinFilter = Filter::LinearMipmapLinear;
        Filter   MagFilter = Filter::Linear;
        Wrap     WrapS = Wrap::Repeat;
        Wrap     WrapT = Wrap::Repeat;
    };

    virtual ~Texture() = default;

    virtual void Bind(uint32_t slot = 0) const = 0;
    virtual void Unbind() const = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetRendererID() const = 0;

    virtual const Spec& GetSpecification() const = 0;
};

// 2D 纹理
class Texture2D : public Texture
{
public:
    Texture2D(const Spec& spec);
    Texture2D(const std::string& path);
    ~Texture2D();

    void Bind(uint32_t slot = 0) const override;
    void Unbind() const override;

    uint32_t GetWidth() const override  { return m_Spec.Width; }
    uint32_t GetHeight() const override { return m_Spec.Height; }
    uint32_t GetRendererID() const override { return m_RendererID; }
    const Spec& GetSpecification() const override { return m_Spec; }

    void SetData(void* data, uint32_t size);

    static std::shared_ptr<Texture2D> Create(const Spec& spec);
    static std::shared_ptr<Texture2D> Create(const std::string& path);

private:
    Spec m_Spec;
    uint32_t m_RendererID = 0;
    GLenum m_InternalFormat = GL_RGBA, m_DataFormat = GL_RGBA;
};

// CubeMap 纹理
class TextureCube : public Texture
{
public:
    TextureCube(const std::vector<std::string>& faces);
    ~TextureCube();

    void Bind(uint32_t slot = 0) const override;
    void Unbind() const override;

    uint32_t GetWidth() const override  { return m_Width; }
    uint32_t GetHeight() const override { return m_Height; }
    uint32_t GetRendererID() const override { return m_RendererID; }
    const Spec& GetSpecification() const override { return m_Spec; }

private:
    Spec m_Spec;
    uint32_t m_RendererID = 0;
    uint32_t m_Width = 0, m_Height = 0;
};
