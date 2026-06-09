#pragma once

#include "renderer/Shader.h"
#include <unordered_map>
#include <memory>
#include <string>

// 着色器资源库 - 管理 Shader 的创建/缓存/热重载
class ShaderLibrary
{
public:
    void Add(const std::string& name, const std::shared_ptr<Shader>& shader);
    void Load(const std::string& name, const std::string& vertPath, const std::string& fragPath);

    std::shared_ptr<Shader> Get(const std::string& name);
    bool Exists(const std::string& name) const;

    void Remove(const std::string& name);
    void Clear();

    const std::unordered_map<std::string, std::shared_ptr<Shader>>& GetAll() const { return m_Shaders; }

    static ShaderLibrary& Instance()
    {
        static ShaderLibrary instance;
        return instance;
    }

private:
    ShaderLibrary() = default;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
};
