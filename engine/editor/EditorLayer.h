#pragma once

#include "core/Layer.h"
#include "core/Input.h"
#include "core/KeyCodes.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "scene/Prefab.h"
#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Framebuffer.h"
#include "postprocess/PostProcess.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <string>

// 编辑器层 - 提供 ImGui 可视调试面板 + Gizmos + ContentBrowser + Play/Stop + Prefab
class EditorLayer : public Layer
{
public:
    EditorLayer();
    ~EditorLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnEvent(Event& event) override;
    void OnImGuiRender() override;

    void SetScene(std::shared_ptr<Scene> scene) { m_Scene = scene; }
    void SetCamera(Camera* camera) { m_Camera = camera; }
    void SetFramebuffer(std::shared_ptr<Framebuffer> fb) { m_ViewportFBO = fb; }
    void SetPostProcessPipeline(PostProcessPipeline* pipeline) { m_PostProcess = pipeline; }

    bool IsViewportFocused() const { return m_ViewportFocused; }
    bool IsViewportHovered() const { return m_ViewportHovered; }

    // Play/Stop 状态
    bool IsPlaying() const { return m_IsPlaying; }

private:
    void DrawViewport();
    void DrawSceneHierarchy();
    void DrawInspector();
    void DrawStatsPanel();
    void DrawLightEditor();
    void DrawPostProcessPanel();
    void DrawMenuBar();
    void DrawGizmos();
    void DrawContentBrowser();
    void DrawPrefabPanel();

    // Gizmo 操作模式
    enum class GizmoOperation
    {
        Translate, Rotate, Scale
    };
    GizmoOperation m_GizmoOperation = GizmoOperation::Translate;

    bool HandleKeyEvent(KeyPressedEvent& e);

    std::shared_ptr<Scene> m_Scene;
    Camera* m_Camera = nullptr;
    std::shared_ptr<Framebuffer> m_ViewportFBO;
    PostProcessPipeline* m_PostProcess = nullptr;

    // 编辑器状态
    Entity* m_SelectedEntity = nullptr;
    bool m_ShowStatsWindow    = true;
    bool m_ShowHierarchyWindow = true;
    bool m_ShowInspectorWindow = true;
    bool m_ShowLightEditor     = false;
    bool m_ShowPostProcess     = false;
    bool m_ShowContentBrowser  = false;
    bool m_ShowPrefabPanel     = false;
    bool m_ShowGizmos          = true;
    bool m_ViewportFocused     = false;
    bool m_ViewportHovered     = false;
    bool m_WireframeMode       = false;

    float m_ViewportWidth  = 1280;
    float m_ViewportHeight = 720;
    bool m_LayoutInitialized = false;

    // Play/Stop 状态
    bool m_IsPlaying = false;
    std::string m_SavedSceneState;  // Play 前保存场景状态

    // Content Browser
    std::string m_CurrentDirectory;
    std::vector<std::string> m_DirectoryEntries;

    // Prefab 库
    std::vector<std::shared_ptr<Prefab>> m_Prefabs;

    // Gizmo 显示
    bool m_GizmoEnabled = true;

    void RefreshContentBrowser(const std::string& path);
    void EnterPlayMode();
    void ExitPlayMode();
};
