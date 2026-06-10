#include "Texture.h"
#include "core/Log.h"

// stb_image 实现 (整个引擎只在此处定义一次)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <vector>

// ===== Texture2D =====

static GLenum TextureFormatToGL(Texture::Format format)
{
    switch (format)
    {
    case Texture::Format::RGB:   return GL_RGB;
    case Texture::Format::RGBA:  return GL_RGBA;
    case Texture::Format::RED:   return GL_RED;
    case Texture::Format::Depth: return GL_DEPTH_COMPONENT;
    }
    return GL_RGBA;
}

static GLenum TextureFilterToGL(Texture::Filter filter)
{
    switch (filter)
    {
    case Texture::Filter::Nearest:            return GL_NEAREST;
    case Texture::Filter::Linear:             return GL_LINEAR;
    case Texture::Filter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

static GLenum TextureWrapToGL(Texture::Wrap wrap)
{
    switch (wrap)
    {
    case Texture::Wrap::Repeat:         return GL_REPEAT;
    case Texture::Wrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
    case Texture::Wrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

static int TextureFormatToChannels(Texture::Format format)
{
    switch (format)
    {
    case Texture::Format::RGB:   return 3;
    case Texture::Format::RGBA:  return 4;
    case Texture::Format::RED:   return 1;
    case Texture::Format::Depth: return 1;
    }
    return 4;
}

Texture2D::Texture2D(const Spec& spec)
    : m_Spec(spec)
{
    m_InternalFormat = TextureFormatToGL(spec.ImageFormat);
    m_DataFormat     = TextureFormatToGL(spec.ImageFormat);

    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGL(spec.MinFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGL(spec.MagFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGL(spec.WrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGL(spec.WrapT));

    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat,
                 spec.Width, spec.Height, 0,
                 m_DataFormat, GL_UNSIGNED_BYTE, nullptr);
}

Texture2D::Texture2D(const std::string& path)
    : m_RendererID(0), m_InternalFormat(GL_RGBA), m_DataFormat(GL_RGBA)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        CORE_ERROR("[Texture2D] Failed to load: ", path);
        return;
    }

    m_Spec.Width  = width;
    m_Spec.Height = height;

    if (channels == 4)
    {
        m_Spec.ImageFormat  = Format::RGBA;
        m_InternalFormat = GL_RGBA;
        m_DataFormat     = GL_RGBA;
    }
    else if (channels == 3)
    {
        m_Spec.ImageFormat  = Format::RGB;
        m_InternalFormat = GL_RGB;
        m_DataFormat     = GL_RGB;
    }
    else if (channels == 2)
    {
        m_Spec.ImageFormat  = Format::RGBA;
        m_InternalFormat = GL_RG;
        m_DataFormat     = GL_RG;
    }
    else if (channels == 1)
    {
        m_Spec.ImageFormat  = Format::RED;
        m_InternalFormat = GL_RED;
        m_DataFormat     = GL_RED;
    }

    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat,
                 width, height, 0,
                 m_DataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    if (m_RendererID)
        glDeleteTextures(1, &m_RendererID);
}

void Texture2D::Bind(uint32_t slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture2D::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::SetData(void* data, uint32_t /*size*/)
{
    uint32_t bpp = TextureFormatToChannels(m_Spec.ImageFormat);
    (void)bpp;
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    m_Spec.Width, m_Spec.Height,
                    m_DataFormat, GL_UNSIGNED_BYTE, data);
}

std::shared_ptr<Texture2D> Texture2D::Create(const Spec& spec)
{
    return std::make_shared<Texture2D>(spec);
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path)
{
    return std::make_shared<Texture2D>(path);
}

// ===== TextureCube =====
TextureCube::TextureCube(const std::vector<std::string>& faces)
    : m_RendererID(0), m_Width(0), m_Height(0)
{
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    int loadedFaces = 0;

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                         width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            loadedFaces++;
            if (i == 0) { m_Width = width; m_Height = height; }
        }
        else
        {
            CORE_WARN("[TextureCube] Failed to load face ", i, ": ", faces[i]);
        }
    }

    if (loadedFaces < (int)faces.size())
        CORE_ERROR("[TextureCube] Only ", loadedFaces, "/", faces.size(), " faces loaded successfully");

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

TextureCube::~TextureCube()
{
    if (m_RendererID)
        glDeleteTextures(1, &m_RendererID);
}

void TextureCube::Bind(uint32_t slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
}

void TextureCube::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
