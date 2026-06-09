#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Shader.h"
#include "Texture.h"

// PBR 材质系统
// 基于 Cook-Torrance BRDF 的金属度/粗糙度工作流

struct MaterialProperties
{
    glm::vec3 Albedo    = glm::vec3(1.0f);
    float     Metallic  = 0.0f;
    float     Roughness = 0.5f;
    float     AO        = 1.0f;
    glm::vec3 Emission  = glm::vec3(0.0f);
    float     EmissionStrength = 0.0f;

    // 纹理
    std::shared_ptr<Texture2D> AlbedoMap;
    std::shared_ptr<Texture2D> NormalMap;
    std::shared_ptr<Texture2D> MetallicMap;
    std::shared_ptr<Texture2D> RoughnessMap;
    std::shared_ptr<Texture2D> AOMap;
    std::shared_ptr<Texture2D> EmissiveMap;
};

class Material
{
public:
    Material() = default;
    Material(const std::shared_ptr<Shader>& shader)
        : m_Shader(shader) {}

    void Bind() const;
    void Unbind() const;

    void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }
    std::shared_ptr<Shader> GetShader() const { return m_Shader; }

    MaterialProperties& GetProperties() { return m_Properties; }
    const MaterialProperties& GetProperties() const { return m_Properties; }

    void SetAlbedo(const glm::vec3& color)  { m_Properties.Albedo = color; }
    void SetMetallic(float value)            { m_Properties.Metallic = value; }
    void SetRoughness(float value)           { m_Properties.Roughness = value; }
    void SetAO(float value)                  { m_Properties.AO = value; }

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    static std::shared_ptr<Material> Create(const std::shared_ptr<Shader>& shader);
    static std::shared_ptr<Material> Create(const std::shared_ptr<Shader>& shader,
                                             const MaterialProperties& props);

private:
    std::string m_Name = "Unnamed Material";
    std::shared_ptr<Shader> m_Shader;
    MaterialProperties m_Properties;

    static constexpr int s_AlbedoSlot    = 0;
    static constexpr int s_NormalSlot    = 1;
    static constexpr int s_MetallicSlot  = 2;
    static constexpr int s_RoughnessSlot = 3;
    static constexpr int s_AOSlot        = 4;
    static constexpr int s_EmissiveSlot  = 5;
};
