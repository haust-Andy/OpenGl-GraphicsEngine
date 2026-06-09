#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "core/Base.h"
#include "core/Timestep.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "LightComponent.h"
#include "renderer/Camera.h"
#include "renderer/Light.h"
#include "renderer/Shader.h"

// 简单 Entity - 不使用第三方 ECS 库时的轻量实现
// 使用 Tag + Components 模式

class Entity
{
public:
    Entity() = default;
    Entity(uint32_t id) : m_ID(id) {}

    uint32_t GetID() const { return m_ID; }
    const std::string& GetTag() const { return m_Tag; }
    void SetTag(const std::string& tag) { m_Tag = tag; }

    // 组件
    TransformComponent& GetTransform() { return m_Transform; }
    const TransformComponent& GetTransform() const { return m_Transform; }

    MeshComponent& GetMesh() { return m_Mesh; }
    const MeshComponent& GetMesh() const { return m_Mesh; }

    bool HasLight() const { return m_HasLight; }
    LightComponent& GetLight() { return m_Light; }
    const LightComponent& GetLight() const { return m_Light; }
    void SetLight(const LightComponent& light) { m_Light = light; m_HasLight = true; }

private:
    uint32_t m_ID = 0;
    std::string m_Tag = "Entity";

    TransformComponent m_Transform;
    MeshComponent      m_Mesh;
    LightComponent     m_Light;
    bool               m_HasLight = false;
};

// Scene - 场景管理
class Scene
{
public:
    Scene(const std::string& name = "Untitled Scene");
    ~Scene();

    Entity* CreateEntity(const std::string& tag = "Entity");
    void DestroyEntity(Entity* entity);

    Entity* GetEntity(uint32_t id);
    Entity* FindEntity(const std::string& tag);

    const std::vector<std::unique_ptr<Entity>>& GetEntities() const { return m_Entities; }

    // 每帧更新
    void OnUpdate(Timestep ts);

    // 渲染
    void OnRender(const Camera& camera);

    // 光照
    LightEnvironment& GetLightEnvironment() { return m_LightEnv; }
    const LightEnvironment& GetLightEnvironment() const { return m_LightEnv; }

    // 场景属性
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    void SetSkybox(const Ref<class TextureCube>& skybox) { m_Skybox = skybox; }

    // 遍历所有 Entity
    void ForEachEntity(std::function<void(Entity&)> callback);

    static std::shared_ptr<Scene> Create(const std::string& name = "Untitled Scene");

private:
    std::string m_Name;
    std::vector<std::unique_ptr<Entity>> m_Entities;
    uint32_t m_NextEntityID = 1;

    LightEnvironment m_LightEnv;
    std::shared_ptr<class TextureCube> m_Skybox;

    // 用于收集渲染数据
    void CollectLights();
    void SortEntities();
    void RenderEntities(const Camera& camera);
};
