#pragma once

#include "renderer/Texture.h"
#include <unordered_map>
#include <memory>
#include <string>

// 纹理资源库 - 管理纹理的加载/缓存/释放
class TextureLibrary
{
public:
    std::shared_ptr<Texture2D> Load(const std::string& path);
    std::shared_ptr<Texture2D> Get(const std::string& path);
    bool Exists(const std::string& path) const;

    // 创建空纹理 (运行时生成)
    std::shared_ptr<Texture2D> Create(const std::string& name, const Texture::Spec& spec);

    void Remove(const std::string& path);
    void Clear();

    static TextureLibrary& Instance()
    {
        static TextureLibrary instance;
        return instance;
    }

private:
    TextureLibrary() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
};
