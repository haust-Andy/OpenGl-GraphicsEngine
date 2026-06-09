#pragma once

#include "core/Layer.h"
#include "core/Input.h"
#include "core/KeyCodes.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "renderer/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Framebuffer.h"
#include "postprocess/PostProcess.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// 编辑器层 - 提供 ImGui 可视调试面板
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

    // 查询 Viewport 聚焦状态 (供 Sandbox 层判断是否旋转相机)
    bool IsViewportFocused() const { return m_ViewportFocused; }
    bool IsViewportHovered() const { return m_ViewportHovered; }

private:
    void DrawViewport();
    void DrawSceneHierarchy();
    void DrawInspector();
    void DrawStatsPanel();
    void DrawLightEditor();

    bool HandleKeyEvent(KeyPressedEvent& e);

    std::shared_ptr<Scene> m_Scene;
    Camera* m_Camera = nullptr;
    std::shared_ptr<Framebuffer> m_ViewportFBO;

    // 编辑器状态
    Entity* m_SelectedEntity = nullptr;
    bool m_ShowStatsWindow    = true;
    bool m_ShowHierarchyWindow = true;
    bool m_ShowInspectorWindow = true;
    bool m_ShowLightEditor     = false;
    bool m_ViewportFocused     = false;
    bool m_ViewportHovered     = false;
    bool m_WireframeMode       = false;

    float m_ViewportWidth  = 1280;
    float m_ViewportHeight = 720;
    bool m_LayoutInitialized = false;
};
