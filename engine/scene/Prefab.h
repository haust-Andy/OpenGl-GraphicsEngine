#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cstdint>

// Prefab - 可复用的实体模板
class Prefab
{
public:
    Prefab() = default;
    Prefab(const std::string& name) : m_Name(name) {}

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    // 从一个现有 Entity 创建 Prefab
    static std::shared_ptr<Prefab> CreateFromEntity(class Entity* entity);

    // 从 Prefab 实例化一个新 Entity
    Entity* Instantiate(class Scene* scene, const glm::vec3& position = glm::vec3(0.0f));

    // 序列化/反序列化
    std::string Serialize() const;
    static std::shared_ptr<Prefab> Deserialize(const std::string& data);

private:
    std::string m_Name = "Unnamed Prefab";

    // 存储模板数据
    struct PrefabData
    {
        // Transform
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Rotation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);

        // Mesh
        std::string MeshType;  // "Cube", "Sphere", "Plane", "Custom"
        uint32_t SphereSegments = 3;

        // Material
        glm::vec3 Albedo = glm::vec3(0.8f);
        float Metallic = 0.0f;
        float Roughness = 0.5f;
        float AO = 1.0f;

        // Light
        bool HasLight = false;
        int LightType = 0;  // 0=Dir, 1=Point, 2=Spot
        glm::vec3 LightColor = glm::vec3(1.0f);
        float LightIntensity = 1.0f;

        // Physics
        bool HasPhysics = false;
        bool IsStaticBody = false;

        // Script
        bool HasScript = false;
        std::string ScriptName;
    };

    PrefabData m_Data;
};
