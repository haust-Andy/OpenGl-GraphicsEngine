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
#include "ScriptComponent.h"
#include "physics/PhysicsWorld.h"
#include "audio/AudioSystem.h"
#include "particle/ParticleEmitter.h"
#include "renderer/Camera.h"
#include "renderer/Light.h"
#include "renderer/Shader.h"
#include "renderer/ShadowMap.h"
#include "renderer/LOD.h"

// 前向声明
class PhysicsWorld;
class ShadowMap;
class IBL;

// Entity - 场景实体 (组件模式)
class Entity
{
public:
    Entity() = default;
    Entity(uint32_t id) : m_ID(id) {}

    uint32_t GetID() const { return m_ID; }
    const std::string& GetTag() const { return m_Tag; }
    void SetTag(const std::string& tag) { m_Tag = tag; }

    // ===== 核心组件 (总是存在) =====
    TransformComponent& GetTransform() { return m_Transform; }
    const TransformComponent& GetTransform() const { return m_Transform; }

    MeshComponent& GetMesh() { return m_Mesh; }
    const MeshComponent& GetMesh() const { return m_Mesh; }

    // ===== 光源组件 =====
    bool HasLight() const { return m_HasLight; }
    LightComponent& GetLight() { return m_Light; }
    const LightComponent& GetLight() const { return m_Light; }
    void SetLight(const LightComponent& light) { m_Light = light; m_HasLight = true; }
    void RemoveLight() { m_HasLight = false; }

    // ===== 脚本组件 =====
    bool HasScript() const { return m_HasScript; }
    ScriptComponent& GetScript() { return m_Script; }
    const ScriptComponent& GetScript() const { return m_Script; }
    void SetScript(const ScriptComponent& script) { m_Script = script; m_HasScript = true; }
    void RemoveScript() { m_HasScript = false; }

    // ===== 物理组件 =====
    bool HasPhysics() const { return m_HasPhysics; }
    ColliderComponent& GetCollider() { return m_Collider; }
    const ColliderComponent& GetCollider() const { return m_Collider; }
    RigidbodyComponent& GetRigidbody() { return m_Rigidbody; }
    const RigidbodyComponent& GetRigidbody() const { return m_Rigidbody; }
    void AddPhysics(const ColliderComponent& collider = ColliderComponent(),
                     const RigidbodyComponent& rb = RigidbodyComponent())
    {
        m_Collider = collider; m_Rigidbody = rb; m_HasPhysics = true;
    }
    void RemovePhysics() { m_HasPhysics = false; }

    // ===== 音频组件 =====
    bool HasAudioSource() const { return m_HasAudioSource; }
    AudioSourceComponent& GetAudioSource() { return m_AudioSource; }
    void AddAudioSource(const AudioSourceComponent& src = AudioSourceComponent())
    {
        m_AudioSource = src; m_HasAudioSource = true;
    }
    void RemoveAudioSource() { m_HasAudioSource = false; }

    bool HasAudioListener() const { return m_HasAudioListener; }
    AudioListenerComponent& GetAudioListener() { return m_AudioListener; }
    void AddAudioListener(const AudioListenerComponent& listener = AudioListenerComponent())
    {
        m_AudioListener = listener; m_HasAudioListener = true;
    }
    void RemoveAudioListener() { m_HasAudioListener = false; }

    // ===== 粒子组件 =====
    bool HasParticleEmitter() const { return m_HasParticleEmitter; }
    ParticleEmitter& GetParticleEmitter() { return *m_ParticleEmitter; }
    void AddParticleEmitter(const struct ParticleEmitterConfig& config = ParticleEmitterConfig());
    void RemoveParticleEmitter() { m_ParticleEmitter.reset(); m_HasParticleEmitter = false; }

    // ===== LOD 组件 =====
    bool HasLOD() const { return m_HasLOD; }
    LODComponent& GetLOD() { return m_LOD; }
    const LODComponent& GetLOD() const { return m_LOD; }
    void SetLOD(const LODComponent& lod) { m_LOD = lod; m_HasLOD = true; }
    void RemoveLOD() { m_HasLOD = false; }

    // ===== 通用标记 =====
    bool IsActive = true;      // 是否激活
    bool IsStatic = false;      // 是否静态 (不参与物理/脚本更新)

private:
    uint32_t m_ID = 0;
    std::string m_Tag = "Entity";

    // 核心组件
    TransformComponent m_Transform;
    MeshComponent      m_Mesh;

    // 可选组件
    LightComponent      m_Light;
    bool                m_HasLight = false;

    ScriptComponent     m_Script;
    bool                m_HasScript = false;

    ColliderComponent   m_Collider;
    RigidbodyComponent  m_Rigidbody;
    bool                m_HasPhysics = false;

    AudioSourceComponent   m_AudioSource;
    bool                    m_HasAudioSource = false;

    AudioListenerComponent m_AudioListener;
    bool                   m_HasAudioListener = false;

    std::unique_ptr<ParticleEmitter> m_ParticleEmitter;
    bool m_HasParticleEmitter = false;

    LODComponent m_LOD;
    bool m_HasLOD = false;
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

    // ===== 阴影 =====
    void SetShadowMap(std::shared_ptr<ShadowMap> shadowMap) { m_ShadowMap = shadowMap; }
    std::shared_ptr<ShadowMap> GetShadowMap() const { return m_ShadowMap; }

    // ===== 物理 =====
    void SetPhysicsWorld(std::unique_ptr<PhysicsWorld> world) { m_PhysicsWorld = std::move(world); }
    PhysicsWorld* GetPhysicsWorld() const { return m_PhysicsWorld.get(); }

    // ===== 视锥体剔除 =====
    void SetFrustumCulling(bool enabled) { m_FrustumCullingEnabled = enabled; }
    bool IsFrustumCullingEnabled() const { return m_FrustumCullingEnabled; }

    // ===== IBL 环境光照 =====
    void SetIBL(std::shared_ptr<class IBL> ibl) { m_IBL = ibl; }
    std::shared_ptr<class IBL> GetIBL() const { return m_IBL; }

    static std::shared_ptr<Scene> Create(const std::string& name = "Untitled Scene");

private:
    std::string m_Name;
    std::vector<std::unique_ptr<Entity>> m_Entities;
    uint32_t m_NextEntityID = 1;

    LightEnvironment m_LightEnv;
    std::shared_ptr<class TextureCube> m_Skybox;

    // 阴影
    std::shared_ptr<ShadowMap> m_ShadowMap;

    // 物理
    std::unique_ptr<PhysicsWorld> m_PhysicsWorld;

    // 视锥体剔除
    bool m_FrustumCullingEnabled = true;

    // IBL 环境光照
    std::shared_ptr<class IBL> m_IBL;

    // 用于收集渲染数据
    void CollectLights();
    void SortEntities();
    void RenderEntities(const Camera& camera);
};
