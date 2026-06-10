#include "ShaderLibrary.h"

void ShaderLibrary::Add(const std::string& name, const std::shared_ptr<Shader>& shader)
{
    if (m_Shaders.find(name) != m_Shaders.end())
    {
        std::cerr << "[ShaderLibrary] Shader '" << name << "' already exists, overwriting." << std::endl;
    }
    m_Shaders[name] = shader;
}

void ShaderLibrary::Load(const std::string& name, const std::string& vertPath, const std::string& fragPath)
{
    auto shader = Shader::Create(vertPath, fragPath);
    if (shader)
    {
        shader->SetName(name);
        m_Shaders[name] = shader;
    }
}

std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name)
{
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end())
        return it->second;
    return nullptr;
}

bool ShaderLibrary::Exists(const std::string& name) const
{
    return m_Shaders.find(name) != m_Shaders.end();
}

void ShaderLibrary::Remove(const std::string& name)
{
    m_Shaders.erase(name);
}

void ShaderLibrary::Clear()
{
    m_Shaders.clear();
}
