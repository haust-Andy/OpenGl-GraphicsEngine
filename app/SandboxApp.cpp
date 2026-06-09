#include "engine/core/Application.h"
#include "engine/core/EntryPoint.h"
#include "engine/core/Input.h"
#include "engine/core/KeyCodes.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Camera.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/Framebuffer.h"
#include "engine/resource/MeshLibrary.h"
#include "engine/resource/TextureLibrary.h"
#include "engine/resource/ShaderLibrary.h"
#include "engine/scene/Scene.h"
#include "engine/scene/TransformComponent.h"
#include "engine/scene/MeshComponent.h"
#include "engine/editor/EditorLayer.h"
#include "imgui.h"
#include <iostream>

// ===== Sandbox 层 - 游戏逻辑 =====
class SandboxLayer : public Layer
{
public:
    SandboxLayer()
        : Layer("Sandbox") {}

    void OnAttach() override
    {
        // 初始化渲染器
        Renderer::Init();

        // 创建场景
        m_Scene = Scene::Create("Demo Scene");

        // 加载 PBR Shader
        m_PBRShader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
        std::cout << "[Sandbox] Shader ID: " << m_PBRShader->GetRendererID() << std::endl;

        // 创建立方体实体
        {
            auto* cube = m_Scene->CreateEntity("Cube");
            cube->GetTransform().Position = glm::vec3(-1.5f, 0.0f, 0.0f);

            auto& mesh = cube->GetMesh();
            mesh.SetMesh(MeshLibrary::GetCube().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.3f, 0.5f, 0.8f);
            mesh.Material->GetProperties().Metallic  = 0.1f;
            mesh.Material->GetProperties().Roughness = 0.4f;
            std::cout << "[Sandbox] Cube VAO:" << (mesh.VertexArray ? mesh.VertexArray->GetRendererID() : 0)
                      << " IBO:" << (mesh.VertexArray && mesh.VertexArray->GetIndexBuffer() ? "yes" : "no") << std::endl;
        }

        // 创建球体实体 (占位)
        {
            auto* sphere = m_Scene->CreateEntity("Metal Sphere");
            sphere->GetTransform().Position = glm::vec3(1.5f, 0.0f, 0.0f);

            auto& mesh = sphere->GetMesh();
            mesh.SetMesh(MeshLibrary::GetCube().VAO);  // 用Cube占位
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.9f, 0.7f, 0.2f);
            mesh.Material->GetProperties().Metallic  = 0.8f;
            mesh.Material->GetProperties().Roughness = 0.2f;
        }

        // 创建地面
        {
            auto* ground = m_Scene->CreateEntity("Ground");
            ground->GetTransform().Position = glm::vec3(0.0f, -0.5f, 0.0f);

            auto& mesh = ground->GetMesh();
            mesh.SetMesh(MeshLibrary::GetPlane().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.5f, 0.5f, 0.5f);
            mesh.Material->GetProperties().Metallic  = 0.0f;
            mesh.Material->GetProperties().Roughness = 0.9f;
        }

        // 配置光照
        auto& lightEnv = m_Scene->GetLightEnvironment();
        lightEnv.SetAmbientLight(glm::vec3(1.0f), 0.15f);
        auto& dirLight = lightEnv.GetDirectionalLight();
        dirLight.Direction = glm::vec3(-0.5f, -1.0f, -0.3f);
        dirLight.Color     = glm::vec3(1.0f, 0.98f, 0.95f);
        dirLight.Intensity = 2.0f;

        // 添加点光源
        PointLight pl;
        pl.Position  = glm::vec3(0.0f, 2.0f, 2.0f);
        pl.Color     = glm::vec3(1.0f, 0.3f, 0.2f);
        pl.Intensity = 3.0f;
        pl.Range     = 10.0f;
        lightEnv.AddPointLight(pl);

        // 创建视口 Framebuffer
        FramebufferSpec fbSpec;
        fbSpec.Width  = 1600;
        fbSpec.Height = 900;
        m_ViewportFBO = Framebuffer::Create(fbSpec);
        std::cout << "[Sandbox] FBO ID:" << m_ViewportFBO->GetColorAttachmentID()
                  << " ColorTex:" << m_ViewportFBO->GetColorAttachmentID() << std::endl;

        // 设置编辑器
        m_EditorLayer = std::make_unique<EditorLayer>();
        m_EditorLayer->SetScene(m_Scene);
        m_EditorLayer->SetCamera(&m_Camera);
        m_EditorLayer->SetFramebuffer(m_ViewportFBO);
    }

    void OnDetach() override
    {
        Renderer::Shutdown();
    }

    void OnUpdate(Timestep ts) override
    {
        float dt = ts.GetSeconds();

        // 输入处理
        HandleInput(dt);

        // 场景更新
        m_Scene->OnUpdate(ts);

        // 渲染到 Framebuffer (编辑器视口显示 3D 场景)
        m_ViewportFBO->Bind();
        Renderer::SetClearColor(glm::vec4(0.05f, 0.05f, 0.08f, 1.0f));
        Renderer::SetViewport(0, 0, 1600, 900);
        Renderer::SetDepthTest(true);

        Renderer::BeginFrame();
        m_Scene->OnRender(m_Camera);
        Renderer::EndFrame();
        m_ViewportFBO->Unbind();

        // 编辑器层更新 (处理线框模式、光标切换等)
        m_EditorLayer->OnUpdate(ts);

        // GL 错误检查
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
            std::cerr << "[Sandbox] GL Error: 0x" << std::hex << err << std::dec << std::endl;
    }

    void OnImGuiRender() override
    {
        m_EditorLayer->OnImGuiRender();
    }

    void OnEvent(Event& event) override
    {
        // 编辑器层先处理事件（如 F3 线框切换）
        m_EditorLayer->OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(SandboxLayer::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(SandboxLayer::OnMouseButtonReleased));
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(SandboxLayer::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(SandboxLayer::OnMouseScrolled));
    }

private:
    void HandleInput(float deltaTime)
    {
        if (Input::IsKeyPressed(Key::Escape))
            Application::Get().Close();

        // WASD 飞行：仅当 Viewport 已激活 (左键点过 Viewport)
        if (!m_EditorLayer->IsViewportFocused())
            return;

        if (Input::IsKeyPressed(Key::W))
            m_Camera.ProcessKeyboard(FORWARD, deltaTime);
        if (Input::IsKeyPressed(Key::S))
            m_Camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (Input::IsKeyPressed(Key::A))
            m_Camera.ProcessKeyboard(LEFT, deltaTime);
        if (Input::IsKeyPressed(Key::D))
            m_Camera.ProcessKeyboard(RIGHT, deltaTime);
        if (Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl))
            m_Camera.ProcessKeyboard(DOWN_WORLD, deltaTime);
        if (Input::IsKeyPressed(Key::Space))
            m_Camera.ProcessKeyboard(UP_WORLD, deltaTime);
    }

    bool OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        // 右键：在 Viewport 内按住 → 旋转视角
        if (e.GetMouseButton() == Mouse::ButtonRight && m_EditorLayer->IsViewportHovered())
        {
            m_RightMouseHeld = true;
            m_FirstMouse = true;
            return true;   // 阻止 ImGui 右键菜单
        }
        // 中键：在 Viewport 内按住 → 平移视角
        if (e.GetMouseButton() == Mouse::ButtonMiddle && m_EditorLayer->IsViewportHovered())
        {
            m_MiddleMouseHeld = true;
            m_FirstMouse = true;
            return true;
        }
        return false;
    }

    bool OnMouseButtonReleased(MouseButtonReleasedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonRight)
        {
            m_RightMouseHeld = false;
            m_FirstMouse = true;
            return false;
        }
        if (e.GetMouseButton() == Mouse::ButtonMiddle)
        {
            m_MiddleMouseHeld = false;
            m_FirstMouse = true;
            return false;
        }
        return false;
    }

    bool OnMouseMoved(MouseMovedEvent& e)
    {
        // === 中键平移 ===
        if (m_MiddleMouseHeld)
        {
            if (m_FirstMouse)
            {
                m_LastMouseX = e.GetX();
                m_LastMouseY = e.GetY();
                m_FirstMouse = false;
                return false;
            }
            float xoffset = e.GetX() - m_LastMouseX;
            float yoffset = m_LastMouseY - e.GetY();
            m_LastMouseX = e.GetX();
            m_LastMouseY = e.GetY();

            // 沿相机本地轴平移，速度随距离缩放
            float panSpeed = 0.002f;
            m_Camera.Position -= m_Camera.Right * xoffset * panSpeed;
            m_Camera.Position += m_Camera.Up    * yoffset * panSpeed;
            return true;
        }

        // === 右键旋转 ===
        if (!m_RightMouseHeld)
        {
            m_FirstMouse = true;
            return false;
        }

        if (m_FirstMouse)
        {
            m_LastMouseX = e.GetX();
            m_LastMouseY = e.GetY();
            m_FirstMouse = false;
            return false;
        }

        float xoffset = e.GetX() - m_LastMouseX;
        float yoffset = m_LastMouseY - e.GetY();  // Y 轴反转
        m_LastMouseX = e.GetX();
        m_LastMouseY = e.GetY();

        m_Camera.ProcessMouseMovement(xoffset, yoffset);
        return true;
    }

    bool OnMouseScrolled(MouseScrolledEvent& e)
    {
        // 鼠标悬停在 Viewport 上即可缩放 (不需要按住右键)
        if (!m_EditorLayer->IsViewportHovered() || ImGui::GetIO().WantCaptureMouse)
            return false;

        m_Camera.ProcessMouseScroll(e.GetYOffset());
        return true;
    }

    std::shared_ptr<Scene> m_Scene;
    std::shared_ptr<Shader> m_PBRShader;
    std::shared_ptr<Framebuffer> m_ViewportFBO;

    Camera m_Camera;
    std::unique_ptr<EditorLayer> m_EditorLayer;

    // 鼠标旋转状态
    bool m_RightMouseHeld  = false;
    bool m_MiddleMouseHeld = false;
    bool m_FirstMouse = true;
    float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
};

// ===== 应用入口 =====
Application* CreateApplication()
{
    Application* app = new Application("OpenGL Graphics Engine", 1600, 900);
    app->PushLayer(new SandboxLayer());
    return app;
}
