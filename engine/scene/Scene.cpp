#include "Scene.h"
#include "renderer/Renderer.h"
#include "renderer/Frustum.h"
#include "renderer/IBL.h"
#include "particle/ParticleEmitter.h"
#include <algorithm>

// Entity 的粒子发射器延迟创建
void Entity::AddParticleEmitter(const ParticleEmitterConfig& config)
{
    m_ParticleEmitter = std::make_unique<ParticleEmitter>(config);
    m_HasParticleEmitter = true;
}

// ===== Scene =====

Scene::Scene(const std::string& name)
    : m_Name(name)
{
}

Scene::~Scene()
{
    // 销毁所有实体脚本
    for (auto& entity : m_Entities)
    {
        if (entity->HasScript())
            entity->GetScript().Destroy(*entity);
    }
    m_Entities.clear();
}

Entity* Scene::CreateEntity(const std::string& tag)
{
    auto entity = std::make_unique<Entity>(m_NextEntityID++);
    entity->SetTag(tag);
    Entity* ptr = entity.get();
    m_Entities.push_back(std::move(entity));
    return ptr;
}

void Scene::DestroyEntity(Entity* entity)
{
    if (entity->HasScript())
        entity->GetScript().Destroy(*entity);

    auto it = std::find_if(m_Entities.begin(), m_Entities.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });
    if (it != m_Entities.end())
        m_Entities.erase(it);
}

Entity* Scene::GetEntity(uint32_t id)
{
    for (auto& e : m_Entities)
        if (e->GetID() == id) return e.get();
    return nullptr;
}

Entity* Scene::FindEntity(const std::string& tag)
{
    for (auto& e : m_Entities)
        if (e->GetTag() == tag) return e.get();
    return nullptr;
}

void Scene::OnUpdate(Timestep ts)
{
    float dt = ts.GetSeconds();

    for (auto& entity : m_Entities)
    {
        if (!entity->IsActive || entity->IsStatic) continue;

        // 初始化脚本 (首次 OnCreate)
        if (entity->HasScript())
        {
            auto& script = entity->GetScript();
            if (!script.m_Initialized)
                script.Initialize(*entity);
            script.Update(*entity, ts);
        }

        // 更新粒子
        if (entity->HasParticleEmitter())
        {
            auto& emitter = entity->GetParticleEmitter();
            emitter.Position = entity->GetTransform().Position;
            emitter.Update(dt);
        }

        // 音频源 3D 位置更新
        if (entity->HasAudioSource() && entity->GetAudioSource().Playing)
        {
            // TODO: 更新音频源位置到 SoLoud
        }
    }

    // 物理步进
    if (m_PhysicsWorld)
        m_PhysicsWorld->Step(dt, this);
}

void Scene::OnRender(const Camera& camera)
{
    // M-08: 收集光源并排序实体
    CollectLights();
    SortEntities();

    // M-07: 从窗口实际宽高比计算投影矩阵
    float aspectRatio = (float)camera.ViewportWidth / (float)camera.ViewportHeight;
    if (aspectRatio <= 0.0f) aspectRatio = 16.0f / 9.0f;  // 回退默认值
    glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 view = camera.GetViewMatrix();

    Renderer::BeginScene(camera, view, projection);

    // 视锥体剔除
    Frustum frustum;
    frustum.ExtractFromMatrix(projection * view);

    // 渲染各实体
    for (auto& entity : m_Entities)
    {
        if (!entity->IsActive) continue;

        auto& mesh = entity->GetMesh();
        if (!mesh.Material || !mesh.VertexArray) continue;
        if (!mesh.Visible) continue;

        // 视锥体剔除
        if (m_FrustumCullingEnabled)
        {
            // 简单球体剔除: 用 Transform 位置 + 缩放半径
            glm::vec3 pos = entity->GetTransform().Position;
            float maxScale = glm::compMax(entity->GetTransform().Scale);
            if (!frustum.IntersectsSphere(pos, maxScale * 2.0f))
                continue;
        }

        mesh.Material->Bind();
        m_LightEnv.BindToShader(mesh.Material->GetShader());

        glm::mat4 transform = entity->GetTransform().GetWorldMatrix();

        // IBL 绑定
        if (m_IBL)
        {
            auto shader = mesh.Material->GetShader();
            m_IBL->BindIrradianceMap(7);
            shader->SetInt("u_IrradianceMap", 7);
            m_IBL->BindPrefilterMap(8);
            shader->SetInt("u_PrefilterMap", 8);
            m_IBL->BindBRDFLUT(9);
            shader->SetInt("u_BRDFLUT", 9);
            shader->SetBool("u_IBL_Enabled", true);
        }
        else
        {
            mesh.Material->GetShader()->SetBool("u_IBL_Enabled", false);
        }

        // LOD 选择
        std::shared_ptr<VertexArray> origVAO;  // H-03: 用于恢复原始 VAO
        if (entity->HasLOD())
        {
            auto& lod = entity->GetLOD();
            int level = lod.GetCurrentLOD(
                entity->GetTransform().Position,
                camera.Position,
                glm::compMax(entity->GetTransform().Scale),
                projection * view
            );
            auto lodVAO = lod.GetVAO(level);
            uint32_t lodIndexCount = lod.GetIndexCount(level);
            if (lodVAO && lodIndexCount > 0)
            {
                origVAO = mesh.VertexArray;
                mesh.VertexArray = lodVAO;
            }
        }

        // 绑定阴影贴图
        if (m_ShadowMap)
        {
            auto shader = mesh.Material->GetShader();
            m_ShadowMap->BindForReading(6);
            shader->SetInt("u_ShadowMap", 6);
            shader->SetBool("u_ShadowsEnabled", true);
            shader->SetFloat("u_ShadowBias", m_ShadowMap->ShadowBias);
            shader->SetFloat("u_ShadowNormalBias", m_ShadowMap->NormalBias);
            shader->SetBool("u_SoftShadows", m_ShadowMap->SoftShadows);
            shader->SetInt("u_PCFSamples", m_ShadowMap->PCFSamples);
            shader->SetFloat("u_PCFRadius", m_ShadowMap->PCFRadius);

            // 级联信息
            int cascadeCount = (int)m_ShadowMap->GetCascadeCount();
            shader->SetInt("u_CascadeCount", cascadeCount);
            for (int i = 0; i < cascadeCount; ++i)
            {
                std::string prefix = "u_Cascades[" + std::to_string(i) + "]";
                shader->SetMat4(prefix + ".LightViewProjection",
                                 m_ShadowMap->GetCascades()[i].LightViewProjection);
                shader->SetFloat(prefix + ".SplitDepth",
                                   m_ShadowMap->GetCascades()[i].SplitDepth);
            }
        }
        else
        {
            mesh.Material->GetShader()->SetBool("u_ShadowsEnabled", false);
        }

        entity->GetMesh().VertexArray->Bind();
        auto& ib = entity->GetMesh().VertexArray->GetIndexBuffer();

        if (ib)
        {
            Renderer::Submit(mesh.Material->GetShader(),
                             mesh.VertexArray,
                             transform);
        }

        // H-03: 渲染后恢复原始 VAO
        if (origVAO)
            mesh.VertexArray = origVAO;
    }

    // 渲染粒子
    for (auto& entity : m_Entities)
    {
        if (entity->HasParticleEmitter() && entity->IsActive)
            entity->GetParticleEmitter().Render(camera);
    }

    Renderer::EndScene();
}

void Scene::CollectLights()
{
    m_LightEnv = LightEnvironment();
    for (auto& entity : m_Entities)
    {
        if (entity->HasLight() && entity->GetLight().Enabled)
        {
            auto& lc = entity->GetLight();
            switch (lc.Type)
            {
            case LightType::Directional:
                m_LightEnv.GetDirectionalLight() = lc.DirLight;
                break;
            case LightType::Point:
                m_LightEnv.AddPointLight(lc.PtLight);
                break;
            case LightType::Spot:
                m_LightEnv.AddSpotLight(lc.SpLight);
                break;
            }
        }
    }
}

void Scene::SortEntities()
{
    std::sort(m_Entities.begin(), m_Entities.end(),
        [](const std::unique_ptr<Entity>& a, const std::unique_ptr<Entity>& b) {
            auto matA = a->GetMesh().Material ? a->GetMesh().Material->GetShader()->GetRendererID() : 0;
            auto matB = b->GetMesh().Material ? b->GetMesh().Material->GetShader()->GetRendererID() : 0;
            return matA < matB;
        });
}

void Scene::ForEachEntity(std::function<void(Entity&)> callback)
{
    for (auto& entity : m_Entities)
        callback(*entity);
}

std::shared_ptr<Scene> Scene::Create(const std::string& name)
{
    return std::make_shared<Scene>(name);
}
