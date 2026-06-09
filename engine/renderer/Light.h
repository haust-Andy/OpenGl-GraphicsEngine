#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

// 光源类型
enum class LightType
{
    Directional = 0,
    Point       = 1,
    Spot        = 2
};

// 方向光
struct DirectionalLight
{
    glm::vec3 Direction  = glm::vec3(-0.5f, -1.0f, -0.3f);
    glm::vec3 Color      = glm::vec3(1.0f);
    float     Intensity  = 1.0f;
};

// 点光源
struct PointLight
{
    glm::vec3 Position     = glm::vec3(0.0f);
    glm::vec3 Color        = glm::vec3(1.0f);
    float     Intensity    = 1.0f;
    float     Range        = 10.0f;
    float     Constant     = 1.0f;
    float     Linear       = 0.09f;
    float     Quadratic    = 0.032f;
};

// 聚光灯
struct SpotLight
{
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 Color     = glm::vec3(1.0f);
    float     Intensity = 1.0f;
    float     Range     = 20.0f;
    float     InnerCutOff  = 12.5f;  // 内锥角 (度)
    float     OuterCutOff  = 17.5f;  // 外锥角 (度)
};

// 光照环境 - 管理场景中所有光源
class LightEnvironment
{
public:
    static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 1;
    static constexpr uint32_t MAX_POINT_LIGHTS       = 16;
    static constexpr uint32_t MAX_SPOT_LIGHTS        = 4;

    void SetAmbientLight(const glm::vec3& color, float intensity = 0.1f)
    {
        m_AmbientColor = color;
        m_AmbientIntensity = intensity;
    }

    DirectionalLight& GetDirectionalLight() { return m_DirectionalLight; }
    const DirectionalLight& GetDirectionalLight() const { return m_DirectionalLight; }

    void AddPointLight(const PointLight& light);
    void RemovePointLight(uint32_t index);
    PointLight& GetPointLight(uint32_t index) { return m_PointLights[index]; }
    const std::vector<PointLight>& GetPointLights() const { return m_PointLights; }

    void AddSpotLight(const SpotLight& light);
    const std::vector<SpotLight>& GetSpotLights() const { return m_SpotLights; }

    // 将光源数据绑定到 Shader
    void BindToShader(const std::shared_ptr<class Shader>& shader) const;

    glm::vec3 GetAmbientColor() const { return m_AmbientColor * m_AmbientIntensity; }

private:
    glm::vec3 m_AmbientColor     = glm::vec3(1.0f);
    float     m_AmbientIntensity = 0.1f;

    DirectionalLight m_DirectionalLight;
    std::vector<PointLight> m_PointLights;
    std::vector<SpotLight>  m_SpotLights;
};
