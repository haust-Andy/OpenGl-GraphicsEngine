#include "Prefab.h"
#include "Scene.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "LightComponent.h"
#include "ScriptComponent.h"
#include "physics/PhysicsWorld.h"
#include "renderer/Material.h"
#include "resource/MeshLibrary.h"
#include "renderer/Shader.h"
#include "renderer/Light.h"

#include <glm/gtc/quaternion.hpp>
#include <sstream>

std::shared_ptr<Prefab> Prefab::CreateFromEntity(Entity* entity)
{
    if (!entity) return nullptr;

    auto prefab = std::make_shared<Prefab>(entity->GetTag());

    // Transform
    prefab->m_Data.Position = entity->GetTransform().Position;
    prefab->m_Data.Rotation = entity->GetTransform().GetEulerAngles();
    prefab->m_Data.Scale = entity->GetTransform().Scale;

    // Mesh - 检测类型
    auto& mesh = entity->GetMesh();
    if (mesh.VertexArray)
    {
        // 简化: 默认 Cube
        prefab->m_Data.MeshType = "Cube";
    }

    // Material
    if (mesh.Material)
    {
        auto& props = mesh.Material->GetProperties();
        prefab->m_Data.Albedo = props.Albedo;
        prefab->m_Data.Metallic = props.Metallic;
        prefab->m_Data.Roughness = props.Roughness;
        prefab->m_Data.AO = props.AO;
    }

    // Light
    if (entity->HasLight())
    {
        prefab->m_Data.HasLight = true;
        auto& lc = entity->GetLight();
        prefab->m_Data.LightType = (int)lc.Type;
        if (lc.Type == LightType::Directional)
            prefab->m_Data.LightColor = lc.DirLight.Color;
        else if (lc.Type == LightType::Point)
            prefab->m_Data.LightColor = lc.PtLight.Color;
        prefab->m_Data.LightIntensity = 1.0f;
    }

    // Physics
    if (entity->HasPhysics())
    {
        prefab->m_Data.HasPhysics = true;
        prefab->m_Data.IsStaticBody = entity->GetRigidbody().IsStatic;
    }

    // Script
    if (entity->HasScript())
    {
        prefab->m_Data.HasScript = true;
        prefab->m_Data.ScriptName = entity->GetScript().ScriptName;
    }

    return prefab;
}

Entity* Prefab::Instantiate(Scene* scene, const glm::vec3& position)
{
    if (!scene) return nullptr;

    Entity* entity = scene->CreateEntity(m_Name + "_Instance");

    // Transform
    entity->GetTransform().Position = position + m_Data.Position;
    entity->GetTransform().SetEulerAngles(m_Data.Rotation);
    entity->GetTransform().Scale = m_Data.Scale;

    // Mesh + Material
    auto& mesh = entity->GetMesh();
    if (!m_Data.MeshType.empty())
    {
        auto shader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
        mesh.SetMaterial(Material::Create(shader));
        mesh.Material->GetProperties().Albedo = m_Data.Albedo;
        mesh.Material->GetProperties().Metallic = m_Data.Metallic;
        mesh.Material->GetProperties().Roughness = m_Data.Roughness;
        mesh.Material->GetProperties().AO = m_Data.AO;

        if (m_Data.MeshType == "Cube")
            mesh.SetMesh(MeshLibrary::GetCube().VAO);
        else if (m_Data.MeshType == "Sphere")
            mesh.SetMesh(MeshLibrary::GetSphere(m_Data.SphereSegments).VAO);
        else if (m_Data.MeshType == "Plane")
            mesh.SetMesh(MeshLibrary::GetPlane().VAO);
    }

    // Light
    if (m_Data.HasLight)
    {
        LightComponent lc;
        lc.Type = (LightType)m_Data.LightType;
        lc.Enabled = true;
        if (lc.Type == LightType::Directional)
            lc.DirLight.Color = m_Data.LightColor;
        else if (lc.Type == LightType::Point)
            lc.PtLight.Color = m_Data.LightColor;
        entity->SetLight(lc);
    }

    // Physics
    if (m_Data.HasPhysics)
    {
        ColliderComponent collider;
        RigidbodyComponent rb;
        rb.IsStatic = m_Data.IsStaticBody;
        entity->AddPhysics(collider, rb);
    }

    return entity;
}

std::string Prefab::Serialize() const
{
    std::ostringstream oss;
    oss << "PREFAB_BEGIN\n";
    oss << "Name=" << m_Name << "\n";
    oss << "Pos=" << m_Data.Position.x << "," << m_Data.Position.y << "," << m_Data.Position.z << "\n";
    oss << "Rot=" << m_Data.Rotation.x << "," << m_Data.Rotation.y << "," << m_Data.Rotation.z << "\n";
    oss << "Scale=" << m_Data.Scale.x << "," << m_Data.Scale.y << "," << m_Data.Scale.z << "\n";
    oss << "Mesh=" << m_Data.MeshType << "\n";
    oss << "Albedo=" << m_Data.Albedo.x << "," << m_Data.Albedo.y << "," << m_Data.Albedo.z << "\n";
    oss << "Metallic=" << m_Data.Metallic << "\n";
    oss << "Roughness=" << m_Data.Roughness << "\n";
    oss << "AO=" << m_Data.AO << "\n";
    oss << "HasLight=" << m_Data.HasLight << "\n";
    oss << "HasPhysics=" << m_Data.HasPhysics << "\n";
    oss << "PREFAB_END\n";
    return oss.str();
}

std::shared_ptr<Prefab> Prefab::Deserialize(const std::string& data)
{
    auto prefab = std::make_shared<Prefab>();
    // 简化: 仅解析关键字段
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line))
    {
        if (line.find("Name=") == 0)
            prefab->m_Name = line.substr(5);
        else if (line.find("Mesh=") == 0)
            prefab->m_Data.MeshType = line.substr(5);
    }
    return prefab;
}
