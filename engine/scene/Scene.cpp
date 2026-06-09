#include "Scene.h"
#include "renderer/Renderer.h"
#include <algorithm>

Scene::Scene(const std::string& name)
    : m_Name(name)
{
}

Scene::~Scene()
{
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

void Scene::OnUpdate(Timestep /*ts*/)
{
    // 更新实体逻辑 (动画, 脚本等)
    // 时间步通过 ts.GetSeconds() / ts.GetMilliseconds() 获取
}

void Scene::OnRender(const Camera& camera)
{
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix(16.0f / 9.0f);  // 默认16:9

    Renderer::BeginScene(camera, view, projection);

    // 渲染各实体 (光照在 RenderEntities 中绑定到实际 Shader)
    RenderEntities(camera);

    Renderer::EndScene();
}

void Scene::RenderEntities(const Camera& /*camera*/)
{
    for (auto& entity : m_Entities)
    {
        auto& mesh = entity->GetMesh();
        if (!mesh.Material || !mesh.VertexArray) continue;

        mesh.Material->Bind();

        // 绑定光照数据到当前实体的 Shader
        m_LightEnv.BindToShader(mesh.Material->GetShader());

        glm::mat4 transform = entity->GetTransform().GetWorldMatrix();

        entity->GetMesh().VertexArray->Bind();
        auto& ib = entity->GetMesh().VertexArray->GetIndexBuffer();

        if (ib)
        {
            Renderer::Submit(mesh.Material->GetShader(),
                             mesh.VertexArray,
                             transform);
        }
    }
}

void Scene::CollectLights()
{
    m_LightEnv = LightEnvironment();  // Reset
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
    // 按材质排序以减少状态切换
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
