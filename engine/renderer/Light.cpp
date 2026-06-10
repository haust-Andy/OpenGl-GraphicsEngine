#include "Light.h"
#include "Shader.h"
#include <string>

void LightEnvironment::AddPointLight(const PointLight& light)
{
    if (m_PointLights.size() < MAX_POINT_LIGHTS)
        m_PointLights.push_back(light);
}

void LightEnvironment::RemovePointLight(uint32_t index)
{
    if (index < m_PointLights.size())
        m_PointLights.erase(m_PointLights.begin() + index);
}

void LightEnvironment::AddSpotLight(const SpotLight& light)
{
    if (m_SpotLights.size() < MAX_SPOT_LIGHTS)
        m_SpotLights.push_back(light);
}

void LightEnvironment::BindToShader(const std::shared_ptr<Shader>& shader) const
{
    if (!shader) return;

    shader->Bind();

    // 环境光
    shader->SetVec3("u_AmbientLight", GetAmbientColor());

    // 方向光
    auto& dl = m_DirectionalLight;
    shader->SetVec3("u_DirectionalLight.Direction", dl.Direction);
    shader->SetVec3("u_DirectionalLight.Color",     dl.Color);
    shader->SetFloat("u_DirectionalLight.Intensity", dl.Intensity);

    // 点光源
    shader->SetInt("u_PointLightCount", (int)m_PointLights.size());
    for (size_t i = 0; i < m_PointLights.size(); ++i)
    {
        std::string prefix = "u_PointLights[" + std::to_string(i) + "]";
        auto& pl = m_PointLights[i];
        shader->SetVec3(prefix + ".Position",  pl.Position);
        shader->SetVec3(prefix + ".Color",     pl.Color);
        shader->SetFloat(prefix + ".Intensity", pl.Intensity);
        shader->SetFloat(prefix + ".Range",     pl.Range);
        shader->SetFloat(prefix + ".Constant",  pl.Constant);
        shader->SetFloat(prefix + ".Linear",    pl.Linear);
        shader->SetFloat(prefix + ".Quadratic", pl.Quadratic);
    }

    // 聚光灯
    shader->SetInt("u_SpotLightCount", (int)m_SpotLights.size());
    for (size_t i = 0; i < m_SpotLights.size(); ++i)
    {
        std::string prefix = "u_SpotLights[" + std::to_string(i) + "]";
        auto& sl = m_SpotLights[i];
        shader->SetVec3(prefix + ".Position",    sl.Position);
        shader->SetVec3(prefix + ".Direction",   sl.Direction);
        shader->SetVec3(prefix + ".Color",       sl.Color);
        shader->SetFloat(prefix + ".Intensity",   sl.Intensity);
        shader->SetFloat(prefix + ".Range",       sl.Range);
        shader->SetFloat(prefix + ".InnerCutOff", sl.InnerCutOff);
        shader->SetFloat(prefix + ".OuterCutOff", sl.OuterCutOff);
    }
}
