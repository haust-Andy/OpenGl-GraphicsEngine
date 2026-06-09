#include "Material.h"

void Material::Bind() const
{
    if (!m_Shader) return;

    m_Shader->Bind();

    m_Shader->SetVec3("u_Material.Albedo", m_Properties.Albedo);
    m_Shader->SetFloat("u_Material.Metallic", m_Properties.Metallic);
    m_Shader->SetFloat("u_Material.Roughness", m_Properties.Roughness);
    m_Shader->SetFloat("u_Material.AO", m_Properties.AO);
    m_Shader->SetVec3("u_Material.Emission", m_Properties.Emission * m_Properties.EmissionStrength);

    // 绑定纹理 (如果存在)
    bool hasAlbedo    = m_Properties.AlbedoMap    != nullptr;
    bool hasNormal    = m_Properties.NormalMap    != nullptr;
    bool hasMetallic  = m_Properties.MetallicMap  != nullptr;
    bool hasRoughness = m_Properties.RoughnessMap != nullptr;
    bool hasAO        = m_Properties.AOMap        != nullptr;
    bool hasEmissive  = m_Properties.EmissiveMap  != nullptr;

    m_Shader->SetBool("u_HasAlbedoMap",    hasAlbedo);
    m_Shader->SetBool("u_HasNormalMap",    hasNormal);
    m_Shader->SetBool("u_HasMetallicMap",  hasMetallic);
    m_Shader->SetBool("u_HasRoughnessMap", hasRoughness);
    m_Shader->SetBool("u_HasAOMap",        hasAO);
    m_Shader->SetBool("u_HasEmissiveMap",  hasEmissive);

    if (hasAlbedo)
    {
        m_Shader->SetInt("u_AlbedoMap", s_AlbedoSlot);
        m_Properties.AlbedoMap->Bind(s_AlbedoSlot);
    }
    if (hasNormal)
    {
        m_Shader->SetInt("u_NormalMap", s_NormalSlot);
        m_Properties.NormalMap->Bind(s_NormalSlot);
    }
    if (hasMetallic)
    {
        m_Shader->SetInt("u_MetallicMap", s_MetallicSlot);
        m_Properties.MetallicMap->Bind(s_MetallicSlot);
    }
    if (hasRoughness)
    {
        m_Shader->SetInt("u_RoughnessMap", s_RoughnessSlot);
        m_Properties.RoughnessMap->Bind(s_RoughnessSlot);
    }
    if (hasAO)
    {
        m_Shader->SetInt("u_AOMap", s_AOSlot);
        m_Properties.AOMap->Bind(s_AOSlot);
    }
    if (hasEmissive)
    {
        m_Shader->SetInt("u_EmissiveMap", s_EmissiveSlot);
        m_Properties.EmissiveMap->Bind(s_EmissiveSlot);
    }
}

void Material::Unbind() const
{
    // 纹理由调用者管理解绑
}

std::shared_ptr<Material> Material::Create(const std::shared_ptr<Shader>& shader)
{
    return std::make_shared<Material>(shader);
}

std::shared_ptr<Material> Material::Create(const std::shared_ptr<Shader>& shader,
                                            const MaterialProperties& props)
{
    auto mat = std::make_shared<Material>(shader);
    mat->GetProperties() = props;
    return mat;
}
