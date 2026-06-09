#include "TextureLibrary.h"

std::shared_ptr<Texture2D> TextureLibrary::Load(const std::string& path)
{
    auto it = m_Textures.find(path);
    if (it != m_Textures.end())
        return it->second;

    auto texture = Texture2D::Create(path);
    m_Textures[path] = texture;
    return texture;
}

std::shared_ptr<Texture2D> TextureLibrary::Get(const std::string& path)
{
    auto it = m_Textures.find(path);
    return it != m_Textures.end() ? it->second : nullptr;
}

bool TextureLibrary::Exists(const std::string& path) const
{
    return m_Textures.find(path) != m_Textures.end();
}

std::shared_ptr<Texture2D> TextureLibrary::Create(const std::string& name, const Texture::Spec& spec)
{
    auto texture = Texture2D::Create(spec);
    m_Textures[name] = texture;
    return texture;
}

void TextureLibrary::Remove(const std::string& path)
{
    m_Textures.erase(path);
}

void TextureLibrary::Clear()
{
    m_Textures.clear();
}
