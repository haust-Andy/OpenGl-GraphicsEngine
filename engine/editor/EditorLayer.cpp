#include "EditorLayer.h"
#include "renderer/Light.h"
#include "scene/SceneSerializer.h"
#include "resource/MeshLibrary.h"
#include "renderer/Shader.h"
#include "renderer/Material.h"
#include "core/Log.h"
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace fs = std::filesystem;

EditorLayer::EditorLayer()
    : Layer("EditorLayer")
{
    m_CurrentDirectory = std::filesystem::current_path().string();
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::OnAttach()
{
    RefreshContentBrowser(m_CurrentDirectory);
}

void EditorLayer::OnDetach()
{
}

void EditorLayer::OnUpdate(Timestep /*ts*/)
{
    if (m_WireframeMode)
        Renderer::SetWireframe(true);
    else
        Renderer::SetWireframe(false);
}

void EditorLayer::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::HandleKeyEvent));
}

bool EditorLayer::HandleKeyEvent(KeyPressedEvent& e)
{
    if (e.GetKeyCode() == Key::F3)
    {
        m_WireframeMode = !m_WireframeMode;
        return true;
    }
    // Gizmo 快捷键 — 仅在 ImGui 捕获键盘时（如输入框聚焦）生效，
    // 不拦截按键以避免影响相机 WASD 移动
    // Gizmo 模式可通过菜单栏 Gizmo 菜单或 Inspector 按钮切换
    return false;
}

void EditorLayer::OnImGuiRender()
{
    DrawMenuBar();

    if (m_ShowHierarchyWindow) DrawSceneHierarchy();
    if (m_ShowInspectorWindow) DrawInspector();
    if (m_ShowStatsWindow)     DrawStatsPanel();
    if (m_ShowLightEditor)     DrawLightEditor();
    if (m_ShowPostProcess)    DrawPostProcessPanel();
    if (m_ShowContentBrowser) DrawContentBrowser();
    if (m_ShowPrefabPanel)    DrawPrefabPanel();

    DrawViewport();
    if (m_ShowGizmos) DrawGizmos();
}

void EditorLayer::DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                if (m_Scene)
                    SceneSerializer::SaveToFile(*m_Scene, "scene_save.scene");
            }
            if (ImGui::MenuItem("Load Scene"))
            {
                auto loaded = SceneSerializer::LoadFromFile("scene_save.scene");
                if (loaded) m_Scene = loaded;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
                Application::Get().Close();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowHierarchyWindow);
            ImGui::MenuItem("Inspector",       nullptr, &m_ShowInspectorWindow);
            ImGui::MenuItem("Rendering Stats",  nullptr, &m_ShowStatsWindow);
            ImGui::MenuItem("Light Editor",    nullptr, &m_ShowLightEditor);
            ImGui::MenuItem("Post Process",   nullptr, &m_ShowPostProcess);
            ImGui::MenuItem("Content Browser", nullptr, &m_ShowContentBrowser);
            ImGui::MenuItem("Prefab Panel",    nullptr, &m_ShowPrefabPanel);
            ImGui::Separator();
            ImGui::MenuItem("Gizmos", nullptr, &m_ShowGizmos);
            ImGui::MenuItem("Wireframe Mode (F3)", nullptr, &m_WireframeMode);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Play"))
        {
            if (!m_IsPlaying)
            {
                if (ImGui::MenuItem("Play (F5)"))
                    EnterPlayMode();
            }
            else
            {
                if (ImGui::MenuItem("Stop (F5)"))
                    ExitPlayMode();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Gizmo"))
        {
            if (ImGui::MenuItem("Translate (W)", nullptr, m_GizmoOperation == GizmoOperation::Translate))
                m_GizmoOperation = GizmoOperation::Translate;
            if (ImGui::MenuItem("Rotate (E)", nullptr, m_GizmoOperation == GizmoOperation::Rotate))
                m_GizmoOperation = GizmoOperation::Rotate;
            if (ImGui::MenuItem("Scale (R)", nullptr, m_GizmoOperation == GizmoOperation::Scale))
                m_GizmoOperation = GizmoOperation::Scale;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void EditorLayer::DrawViewport()
{
    if (!m_LayoutInitialized)
    {
        m_LayoutInitialized = true;
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 10, vp->WorkPos.y + 30));
        ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x * 0.72f, vp->WorkSize.y - 40));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_ViewportWidth  = viewportSize.x;
    m_ViewportHeight = viewportSize.y;

    if (m_ViewportFBO)
    {
        uint64_t texID = m_ViewportFBO->GetColorAttachmentID();
        ImGui::Image((ImTextureID)(uintptr_t)texID,
                      viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    // 操控提示
    bool isLooking = ImGui::IsMouseDragging(ImGuiMouseButton_Right);
    bool isPanning = ImGui::IsMouseDragging(ImGuiMouseButton_Middle);
    if (m_ViewportHovered && !isLooking && !isPanning)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wPos = ImGui::GetWindowPos();
        float wW = ImGui::GetWindowWidth();
        float wH = ImGui::GetWindowHeight();
        float barH = 24.0f;

        dl->AddRectFilled(ImVec2(wPos.x, wPos.y + wH - barH),
                          ImVec2(wPos.x + wW, wPos.y + wH),
                          IM_COL32(0, 0, 0, 150));

        std::string modeText = m_IsPlaying ? "[PLAYING] " : "[EDITING] ";
        modeText += "W/E/R: Gizmo | F5: Play/Stop | F3: Wireframe";

        dl->AddText(ImVec2(wPos.x + 10, wPos.y + wH - barH + 4),
                    IM_COL32(200, 200, 200, 255), modeText.c_str());
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::DrawGizmos()
{
    if (!m_SelectedEntity || !m_Camera) return;
    // Gizmo 操作已集成到 Inspector 面板
    // 完整实现需要 ImGuizmo 库
}

void EditorLayer::DrawSceneHierarchy()
{
    ImGui::Begin("Scene Hierarchy");

    if (!m_Scene)
    {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    // Play/Stop 指示器
    if (m_IsPlaying)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
        ImGui::Text("[ PLAYING ]");
        ImGui::PopStyleColor();
    }

    ImGui::Text("Scene: %s", m_Scene->GetName().c_str());

    // 添加实体按钮
    if (ImGui::Button("+ Add Entity"))
    {
        ImGui::OpenPopup("AddEntityPopup");
    }
    if (ImGui::BeginPopup("AddEntityPopup"))
    {
        if (ImGui::MenuItem("Empty Entity"))
        {
            m_SelectedEntity = m_Scene->CreateEntity("New Entity");
        }
        if (ImGui::MenuItem("Cube"))
        {
            auto* e = m_Scene->CreateEntity("Cube");
            e->GetMesh().SetMesh(MeshLibrary::GetCube().VAO);
            auto shader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
            e->GetMesh().SetMaterial(Material::Create(shader));
            m_SelectedEntity = e;
        }
        if (ImGui::MenuItem("Sphere"))
        {
            auto* e = m_Scene->CreateEntity("Sphere");
            e->GetMesh().SetMesh(MeshLibrary::GetSphere(3).VAO);
            auto shader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
            e->GetMesh().SetMaterial(Material::Create(shader));
            m_SelectedEntity = e;
        }
        if (ImGui::MenuItem("Point Light"))
        {
            auto* e = m_Scene->CreateEntity("Point Light");
            LightComponent lc;
            lc.Type = LightType::Point;
            lc.PtLight.Color = glm::vec3(1.0f);
            lc.PtLight.Intensity = 3.0f;
            lc.PtLight.Range = 10.0f;
            e->SetLight(lc);
            m_SelectedEntity = e;
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    m_Scene->ForEachEntity([this](Entity& entity) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (m_SelectedEntity == &entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.GetID(), flags,
                                         "%s", entity.GetTag().c_str());
        if (ImGui::IsItemClicked())
            m_SelectedEntity = &entity;

        // 右键菜单
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete"))
            {
                if (m_SelectedEntity == &entity)
                    m_SelectedEntity = nullptr;
                m_Scene->DestroyEntity(&entity);
            }
            if (ImGui::MenuItem("Save as Prefab"))
            {
                auto prefab = Prefab::CreateFromEntity(&entity);
                m_Prefabs.push_back(prefab);
            }
            ImGui::EndPopup();
        }

        if (opened)
            ImGui::TreePop();
    });

    ImGui::End();
}

void EditorLayer::DrawInspector()
{
    ImGui::Begin("Inspector");

    if (!m_SelectedEntity)
    {
        ImGui::Text("Select an entity to inspect.");
        ImGui::End();
        return;
    }

    Entity& entity = *m_SelectedEntity;

    // Tag
    char tagBuf[128];
    std::strncpy(tagBuf, entity.GetTag().c_str(), sizeof(tagBuf) - 1);
    tagBuf[sizeof(tagBuf) - 1] = '\0';
    if (ImGui::InputText("Tag", tagBuf, 128))
        entity.SetTag(tagBuf);

    ImGui::Separator();

    // Gizmo 操作模式切换
    if (ImGui::Button("Translate")) m_GizmoOperation = GizmoOperation::Translate;
    ImGui::SameLine();
    if (ImGui::Button("Rotate")) m_GizmoOperation = GizmoOperation::Rotate;
    ImGui::SameLine();
    if (ImGui::Button("Scale")) m_GizmoOperation = GizmoOperation::Scale;

    ImGui::Separator();

    // Transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& transform = entity.GetTransform();

        glm::vec3 pos = transform.Position;
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f))
            transform.Position = pos;

        glm::vec3 euler = transform.GetEulerAngles();
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 1.0f))
            transform.SetEulerAngles(euler);

        glm::vec3 scale = transform.Scale;
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.01f, 100.0f))
            transform.Scale = scale;
    }

    // Mesh
    if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& mesh = entity.GetMesh();
        ImGui::Text("VAO ID: %u", mesh.VertexArray ? mesh.VertexArray->GetRendererID() : 0);
        ImGui::Checkbox("Visible", &mesh.Visible);
        ImGui::Checkbox("Cast Shadow", &mesh.CastShadow);

        // Material
        if (mesh.Material)
        {
            ImGui::Separator();
            ImGui::Text("Material:");

            auto& props = mesh.Material->GetProperties();
            ImGui::ColorEdit3("Albedo", glm::value_ptr(props.Albedo));
            ImGui::SliderFloat("Metallic", &props.Metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &props.Roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("AO", &props.AO, 0.0f, 1.0f);
        }
    }

    // Light
    if (entity.HasLight())
    {
        if (ImGui::CollapsingHeader("Light"))
        {
            auto& lc = entity.GetLight();
            ImGui::Checkbox("Enabled", &lc.Enabled);

            const char* lightTypes[] = { "Directional", "Point", "Spot" };
            int typeIdx = (int)lc.Type;
            if (ImGui::Combo("Type", &typeIdx, lightTypes, 3))
                lc.Type = (LightType)typeIdx;

            if (lc.Type == LightType::Point)
            {
                ImGui::ColorEdit3("Color", glm::value_ptr(lc.PtLight.Color));
                ImGui::SliderFloat("Intensity", &lc.PtLight.Intensity, 0.0f, 10.0f);
                ImGui::SliderFloat("Range", &lc.PtLight.Range, 0.0f, 50.0f);
            }
            else if (lc.Type == LightType::Directional)
            {
                ImGui::DragFloat3("Direction", glm::value_ptr(lc.DirLight.Direction), 0.01f);
                ImGui::ColorEdit3("Color", glm::value_ptr(lc.DirLight.Color));
                ImGui::SliderFloat("Intensity", &lc.DirLight.Intensity, 0.0f, 10.0f);
            }
        }
    }

    // Physics
    if (entity.HasPhysics())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            auto& rb = entity.GetRigidbody();
            ImGui::Checkbox("Static", &rb.IsStatic);
            ImGui::SliderFloat("Mass", &rb.Mass, 0.1f, 100.0f);
            ImGui::Checkbox("Use Gravity", &rb.UseGravity);
            ImGui::SliderFloat("Restitution", &rb.Restitution, 0.0f, 1.0f);
            ImGui::SliderFloat("Friction", &rb.Friction, 0.0f, 1.0f);
        }
    }

    // Script
    if (entity.HasScript())
    {
        if (ImGui::CollapsingHeader("Script"))
        {
            auto& script = entity.GetScript();
            ImGui::Text("Name: %s", script.ScriptName.c_str());
            ImGui::Checkbox("Enabled", &script.Enabled);
        }
    }

    // LOD
    if (entity.HasLOD())
    {
        if (ImGui::CollapsingHeader("LOD"))
        {
            auto& lod = entity.GetLOD();
            ImGui::Text("Levels: %d", lod.GetLevelCount());
            for (int i = 0; i < lod.GetLevelCount(); i++)
            {
                ImGui::Text("  LOD%d: switch=%.1fm", i, lod.Levels[i].SwitchDistance);
            }
        }
    }

    // Add Component
    ImGui::Separator();
    if (ImGui::Button("+ Add Component"))
        ImGui::OpenPopup("AddComponentPopup");

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        if (!entity.HasLight() && ImGui::MenuItem("Light"))
        {
            LightComponent lc;
            lc.Type = LightType::Point;
            lc.PtLight.Intensity = 3.0f;
            entity.SetLight(lc);
        }
        if (!entity.HasPhysics() && ImGui::MenuItem("Physics"))
        {
            entity.AddPhysics();
        }
        if (!entity.HasParticleEmitter() && ImGui::MenuItem("Particle Emitter"))
        {
            entity.AddParticleEmitter();
        }
        if (!entity.HasLOD() && ImGui::MenuItem("LOD"))
        {
            LODComponent lod;
            // 默认: 用当前 mesh 做 LOD0
            if (entity.GetMesh().VertexArray)
            {
                auto& ib = entity.GetMesh().VertexArray->GetIndexBuffer();
                uint32_t count = ib ? ib->GetCount() : 0;
                lod.AddLevel(entity.GetMesh().VertexArray, count, 20.0f);
            }
            entity.SetLOD(lod);
        }
        if (!entity.HasScript() && ImGui::MenuItem("Script"))
        {
            entity.GetScript().ScriptName = "NewScript";
        }
        if (!entity.HasAudioSource() && ImGui::MenuItem("Audio Source"))
        {
            entity.AddAudioSource();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void EditorLayer::DrawStatsPanel()
{
    ImGui::Begin("Rendering Stats");

    auto& stats = Renderer::GetStats();
    ImGui::Text("Draw Calls:    %u", stats.DrawCalls);
    ImGui::Text("Triangles:     %u", stats.TriangleCount);
    ImGui::Text("Vertices:      %u", stats.VertexCount);
    ImGui::Text("FPS:           %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time:    %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

    if (m_Scene)
    {
        uint32_t entityCount = 0;
        m_Scene->ForEachEntity([&entityCount](Entity&) { entityCount++; });
        ImGui::Text("Entities:      %u", entityCount);
    }

    ImGui::End();
}

void EditorLayer::DrawLightEditor()
{
    ImGui::Begin("Light Editor");

    if (!m_Scene)
    {
        ImGui::Text("No scene loaded.");
        ImGui::End();
        return;
    }

    auto& lightEnv = m_Scene->GetLightEnvironment();

    if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& dl = lightEnv.GetDirectionalLight();
        ImGui::DragFloat3("Direction", glm::value_ptr(dl.Direction), 0.01f, -1.0f, 1.0f);
        ImGui::ColorEdit3("Color", glm::value_ptr(dl.Color));
        ImGui::SliderFloat("Intensity", &dl.Intensity, 0.0f, 10.0f);
    }

    ImGui::End();
}

void EditorLayer::DrawPostProcessPanel()
{
    ImGui::Begin("Post Process");

    if (!m_PostProcess)
    {
        ImGui::Text("No post-process pipeline.");
        ImGui::End();
        return;
    }

    auto& bloom = m_PostProcess->GetBloom();

    if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Threshold",  &bloom.Threshold,  0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Intensity",  &bloom.Intensity,  0.0f, 3.0f, "%.2f");
        int iterations = (int)bloom.BlurIterations;
        if (ImGui::SliderInt("Blur Iterations", &iterations, 1, 10))
            bloom.BlurIterations = (uint32_t)iterations;
    }

    ImGui::End();
}

void EditorLayer::DrawContentBrowser()
{
    ImGui::Begin("Content Browser");

    // 路径导航
    if (ImGui::Button(".."))
    {
        fs::path parent = fs::path(m_CurrentDirectory).parent_path();
        if (fs::exists(parent))
        {
            m_CurrentDirectory = parent.string();
            RefreshContentBrowser(m_CurrentDirectory);
        }
    }

    ImGui::SameLine();
    ImGui::Text("Path: %s", m_CurrentDirectory.c_str());
    ImGui::Separator();

    // 显示文件/目录
    for (auto& entry : m_DirectoryEntries)
    {
        fs::path p(entry);
        std::string name = p.filename().string();

        if (fs::is_directory(p))
        {
            if (ImGui::Selectable(("[DIR] " + name).c_str()))
            {
                m_CurrentDirectory = entry;
                RefreshContentBrowser(m_CurrentDirectory);
            }
        }
        else
        {
            // 文件图标
            std::string ext = p.extension().string();
            std::string icon = "[FILE]";
            if (ext == ".scene")   icon = "[SCENE]";
            else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") icon = "[MESH]";
            else if (ext == ".png" || ext == ".jpg" || ext == ".hdr") icon = "[TEX]";
            else if (ext == ".frag" || ext == ".vert") icon = "[SHADER]";

            if (ImGui::Selectable((icon + " " + name).c_str()))
            {
                // 点击文件: 如果是场景文件则加载
                if (ext == ".scene")
                {
                    auto loaded = SceneSerializer::LoadFromFile(entry);
                    if (loaded) m_Scene = loaded;
                }
            }
        }
    }

    ImGui::End();
}

void EditorLayer::DrawPrefabPanel()
{
    ImGui::Begin("Prefabs");

    if (m_Prefabs.empty())
    {
        ImGui::Text("No prefabs. Right-click entity -> 'Save as Prefab'");
    }

    for (size_t i = 0; i < m_Prefabs.size(); i++)
    {
        auto& prefab = m_Prefabs[i];
        if (ImGui::Selectable(prefab->GetName().c_str()))
        {
            // 点击: 实例化到场景
            if (m_Scene)
                prefab->Instantiate(m_Scene.get());
        }

        // 右键删除
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete"))
            {
                m_Prefabs.erase(m_Prefabs.begin() + i);
                break;
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void EditorLayer::RefreshContentBrowser(const std::string& path)
{
    m_DirectoryEntries.clear();
    try
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            m_DirectoryEntries.push_back(entry.path().string());
        }
        std::sort(m_DirectoryEntries.begin(), m_DirectoryEntries.end());
    }
    catch (const std::exception& e)
    {
        CORE_ERROR("[ContentBrowser] Error reading directory: ", e.what());
    }
    catch (...)
    {
        CORE_ERROR("[ContentBrowser] Unknown error reading directory: ", path);
    }
}

void EditorLayer::EnterPlayMode()
{
    if (m_IsPlaying) return;

    m_IsPlaying = true;
    CORE_INFO("[Editor] Entering Play Mode");
    // 保存场景状态以便恢复
    if (m_Scene)
        m_SavedSceneState = SceneSerializer::Serialize(*m_Scene);
}

void EditorLayer::ExitPlayMode()
{
    if (!m_IsPlaying) return;

    m_IsPlaying = false;
    CORE_INFO("[Editor] Exiting Play Mode");
    // 恢复场景状态
    if (!m_SavedSceneState.empty())
    {
        auto restored = SceneSerializer::Deserialize(m_SavedSceneState);
        if (restored) m_Scene = restored;
        m_SelectedEntity = nullptr;
    }
}
