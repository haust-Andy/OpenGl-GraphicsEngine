#include "EditorLayer.h"
#include "renderer/Light.h"
#include <glm/gtc/type_ptr.hpp>

EditorLayer::EditorLayer()
    : Layer("EditorLayer")
{
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::OnAttach()
{
    // ImGui 已在 Application 层初始化
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

    // 鼠标光标始终自由，按住右键时由 Sandbox 层控制旋转
}

void EditorLayer::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::HandleKeyEvent));
}

bool EditorLayer::HandleKeyEvent(KeyPressedEvent& e)
{
    // 切换线框模式
    if (e.GetKeyCode() == Key::F3)
    {
        m_WireframeMode = !m_WireframeMode;
        return true;
    }
    return false;
}

void EditorLayer::OnImGuiRender()
{
    // === 菜单栏 ===
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowHierarchyWindow);
            ImGui::MenuItem("Inspector",       nullptr, &m_ShowInspectorWindow);
            ImGui::MenuItem("Rendering Stats",  nullptr, &m_ShowStatsWindow);
            ImGui::MenuItem("Light Editor",    nullptr, &m_ShowLightEditor);
            ImGui::Separator();
            ImGui::MenuItem("Wireframe Mode (F3)", nullptr, &m_WireframeMode);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // === 可拖拽浮动画板 ===
    if (m_ShowHierarchyWindow) DrawSceneHierarchy();
    if (m_ShowInspectorWindow) DrawInspector();
    if (m_ShowStatsWindow)     DrawStatsPanel();
    if (m_ShowLightEditor)     DrawLightEditor();

    DrawViewport();
}

void EditorLayer::DrawViewport()
{
    // 首次运行：设置 Viewport 初始位置和大小 (紧靠 Begin 之前)
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

    // 渲染 Framebuffer 到 ImGui Image
    if (m_ViewportFBO)
    {
        uint64_t texID = m_ViewportFBO->GetColorAttachmentID();
        ImGui::Image((ImTextureID)(uintptr_t)texID,
                      viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    // 操控提示：悬停时显示，右键/中键拖拽时隐藏以免遮挡视线
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
        dl->AddText(ImVec2(wPos.x + 10, wPos.y + wH - barH + 4),
                    IM_COL32(200, 200, 200, 255),
                    "Right-drag: Rotate  |  Middle-drag: Pan  |  Scroll: Zoom  |  Left-click: Focus (WASD / Ctrl-down / Space-up)");
    }

    ImGui::End();
    ImGui::PopStyleVar();
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

    // 场景名称
    ImGui::Text("Scene: %s", m_Scene->GetName().c_str());
    ImGui::Separator();

    // 实体列表
    m_Scene->ForEachEntity([this](Entity& entity) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (m_SelectedEntity == &entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.GetID(), flags,
                                         "%s", entity.GetTag().c_str());
        if (ImGui::IsItemClicked())
            m_SelectedEntity = &entity;

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
    strcpy_s(tagBuf, entity.GetTag().c_str());
    if (ImGui::InputText("Tag", tagBuf, 128))
        entity.SetTag(tagBuf);

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
