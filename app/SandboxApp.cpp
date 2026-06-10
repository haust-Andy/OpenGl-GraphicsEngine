#include "engine/core/Application.h"
#include "engine/core/EntryPoint.h"
#include "engine/core/Input.h"
#include "engine/core/KeyCodes.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Camera.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/Framebuffer.h"
#include "engine/renderer/ShadowMap.h"
#include "engine/renderer/IBL.h"
#include "engine/resource/MeshLibrary.h"
#include "engine/resource/TextureLibrary.h"
#include "engine/resource/ShaderLibrary.h"
#include "engine/scene/Scene.h"
#include "engine/scene/TransformComponent.h"
#include "engine/scene/MeshComponent.h"
#include "engine/postprocess/PostProcess.h"
#include "engine/editor/EditorLayer.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/audio/AudioSystem.h"
#include "engine/particle/ParticleEmitter.h"
#include "imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <vector>

// 轻量立方体贴图封装 (绕过 TextureCube 方便程序化生成)
struct SimpleCubeTex
{
    uint32_t ID = 0;
    void Bind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    }
};

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

        // 初始化阴影渲染器
        ShadowRenderer::Init();

        // 初始化音频
        AudioEngine::Init();

        // 创建场景
        m_Scene = Scene::Create("Demo Scene");

        // 创建物理世界
        auto physicsWorld = std::make_unique<PhysicsWorld>();
        physicsWorld->OnCollisionEnter = [](const CollisionInfo& info) {
            std::cout << "[Physics] Collision: " << info.EntityA->GetTag()
                      << " <-> " << info.EntityB->GetTag() << std::endl;
        };
        m_Scene->SetPhysicsWorld(std::move(physicsWorld));

        // 创建阴影贴图
        auto shadowMap = std::make_shared<ShadowMap>(2048, 2048, 3);
        m_Scene->SetShadowMap(shadowMap);

        // 加载 PBR Shader
        m_PBRShader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
        ShaderLibrary::Instance().Add("pbr", m_PBRShader);

        // ===== 创建实体 =====

        // 立方体
        {
            auto* cube = m_Scene->CreateEntity("Cube");
            cube->GetTransform().Position = glm::vec3(-1.5f, 0.0f, 0.0f);

            auto& mesh = cube->GetMesh();
            mesh.SetMesh(MeshLibrary::GetCube().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.3f, 0.5f, 0.8f);
            mesh.Material->GetProperties().Metallic  = 0.1f;
            mesh.Material->GetProperties().Roughness = 0.4f;

            // 添加物理: 静态碰撞体
            ColliderComponent collider;
            collider.Type = ColliderType::AABB;
            collider.Box = { glm::vec3(-0.5f), glm::vec3(0.5f) };
            RigidbodyComponent rb;
            rb.IsStatic = true;
            cube->AddPhysics(collider, rb);
        }

        // 金属球
        {
            auto* sphere = m_Scene->CreateEntity("Metal Sphere");
            sphere->GetTransform().Position = glm::vec3(1.5f, 0.5f, 0.0f);

            auto& mesh = sphere->GetMesh();
            mesh.SetMesh(MeshLibrary::GetSphere(3).VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.9f, 0.7f, 0.2f);
            mesh.Material->GetProperties().Metallic  = 0.8f;
            mesh.Material->GetProperties().Roughness = 0.2f;

            // 添加物理: 动态球体
            ColliderComponent collider;
            collider.Type = ColliderType::AABB;
            collider.Box = { glm::vec3(-0.5f), glm::vec3(0.5f) };
            RigidbodyComponent rb;
            rb.Mass = 1.0f;
            rb.UseGravity = true;
            sphere->AddPhysics(collider, rb);
        }

        // PBR 球体阵列
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 2; c++)
            {
                auto* ball = m_Scene->CreateEntity("LightBall_" + std::to_string(r * 2 + c));
                ball->GetTransform().Position = glm::vec3(-2.0f + r * 2.0f, 0.3f, -1.5f + c * 3.0f);

                auto& mesh = ball->GetMesh();
                mesh.SetMesh(MeshLibrary::GetSphere(2).VAO);
                mesh.SetMaterial(Material::Create(m_PBRShader));
                mesh.Material->GetProperties().Albedo    = glm::vec3(0.2f + r * 0.3f, 0.3f + c * 0.3f, 0.8f);
                mesh.Material->GetProperties().Metallic  = 0.1f + r * 0.3f;
                mesh.Material->GetProperties().Roughness = 0.1f + c * 0.4f;
            }
        }

        // 地面
        {
            auto* ground = m_Scene->CreateEntity("Ground");
            ground->GetTransform().Position = glm::vec3(0.0f, -0.5f, 0.0f);
            ground->IsStatic = true;

            auto& mesh = ground->GetMesh();
            mesh.SetMesh(MeshLibrary::GetPlane().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.5f, 0.5f, 0.5f);
            mesh.Material->GetProperties().Metallic  = 0.0f;
            mesh.Material->GetProperties().Roughness = 0.9f;

            // 地面碰撞体
            ColliderComponent collider;
            collider.Type = ColliderType::AABB;
            collider.Box = { glm::vec3(-5.0f, -0.5f, -5.0f), glm::vec3(5.0f, 0.5f, 5.0f) };
            RigidbodyComponent rb;
            rb.IsStatic = true;
            ground->AddPhysics(collider, rb);
        }

        // 粒子效果实体
        {
            auto* fireEntity = m_Scene->CreateEntity("Fire Particles");
            fireEntity->GetTransform().Position = glm::vec3(0.0f, 0.5f, 2.0f);

            ParticleEmitterConfig config;
            config.EmitRate = 30;
            config.MaxParticles = 200;
            config.MinLife = 0.5f;
            config.MaxLife = 2.0f;
            config.MinSpeed = 0.5f;
            config.MaxSpeed = 2.0f;
            config.StartSize = 0.3f;
            config.EndSize = 0.0f;
            config.Direction = glm::vec3(0.0f, 1.0f, 0.0f);
            config.SpreadAngle = 15.0f;
            config.StartColor = glm::vec4(1.0f, 0.6f, 0.1f, 1.0f);
            config.EndColor = glm::vec4(0.8f, 0.1f, 0.0f, 0.0f);
            config.Gravity = glm::vec3(0.0f, 0.5f, 0.0f);  // 火焰向上
            config.EmitRadius = 0.3f;
            fireEntity->AddParticleEmitter(config);
        }

        // 脚本实体演示
        {
            auto* scriptEntity = m_Scene->CreateEntity("Rotating Cube");
            scriptEntity->GetTransform().Position = glm::vec3(3.0f, 1.0f, -2.0f);

            auto& mesh = scriptEntity->GetMesh();
            mesh.SetMesh(MeshLibrary::GetCube().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo = glm::vec3(0.8f, 0.2f, 0.3f);
            mesh.Material->GetProperties().Metallic = 0.5f;
            mesh.Material->GetProperties().Roughness = 0.3f;

            // 添加旋转脚本
            auto& script = scriptEntity->GetScript();
            script.ScriptName = "RotateScript";
            script.OnUpdate = [](Entity& e, Timestep ts) {
                float dt = ts.GetSeconds();
                glm::vec3 euler = e.GetTransform().GetEulerAngles();
                euler.y += 45.0f * dt;  // 每秒旋转 45 度
                e.GetTransform().SetEulerAngles(euler);
            };
        }

        // ===== 光照 =====
        auto& lightEnv = m_Scene->GetLightEnvironment();
        lightEnv.SetAmbientLight(glm::vec3(1.0f), 0.15f);
        auto& dirLight = lightEnv.GetDirectionalLight();
        dirLight.Direction = glm::vec3(-0.5f, -1.0f, -0.3f);
        dirLight.Color     = glm::vec3(1.0f, 0.98f, 0.95f);
        dirLight.Intensity = 2.0f;

        // 点光源
        PointLight pl;
        pl.Position  = glm::vec3(0.0f, 2.0f, 2.0f);
        pl.Color     = glm::vec3(1.0f, 0.3f, 0.2f);
        pl.Intensity = 3.0f;
        pl.Range     = 10.0f;
        lightEnv.AddPointLight(pl);

        pl.Position  = glm::vec3(-3.0f, 1.0f, -1.0f);
        pl.Color     = glm::vec3(0.2f, 0.4f, 1.0f);
        pl.Intensity = 2.5f;
        pl.Range     = 8.0f;
        lightEnv.AddPointLight(pl);

        // 聚光灯
        SpotLight sl;
        sl.Position    = glm::vec3(0.0f, 3.0f, 0.0f);
        sl.Direction   = glm::vec3(0.0f, -1.0f, 0.0f);
        sl.Color       = glm::vec3(1.0f, 0.9f, 0.6f);
        sl.Intensity   = 4.0f;
        sl.Range       = 15.0f;
        sl.InnerCutOff = 20.0f;
        sl.OuterCutOff = 35.0f;
        lightEnv.AddSpotLight(sl);

        // ===== IBL 环境光照 =====
        m_IBL = std::make_shared<IBL>();
        // 尝试加载 HDR 文件, 如果不存在则用程序化天空
        if (!m_IBL->LoadFromHDR("resources/environment.hdr"))
        {
            m_IBL->GenerateFromProcedural(
                glm::vec3(0.3f, 0.5f, 1.0f),   // 天顶色
                glm::vec3(0.05f, 0.05f, 0.15f)   // 地平线色
            );
        }
        m_Scene->SetIBL(m_IBL);

        // ===== Skybox =====
        m_SkyboxShader = Shader::Create("shader/skybox.vert", "shader/skybox.frag");
        GenerateProceduralSkybox();

        // ===== Framebuffers =====
        FramebufferSpec fbSpec;
        fbSpec.Width  = 1600;
        fbSpec.Height = 900;
        m_ViewportFBO = Framebuffer::Create(fbSpec);
        m_SceneFBO = Framebuffer::Create(fbSpec);

        // ===== 后处理 =====
        m_PostProcess = std::make_unique<PostProcessPipeline>(1600, 900);

        // ===== 编辑器 =====
        m_EditorLayer = std::make_unique<EditorLayer>();
        m_EditorLayer->SetScene(m_Scene);
        m_EditorLayer->SetCamera(&m_Camera);
        m_EditorLayer->SetFramebuffer(m_ViewportFBO);
        m_EditorLayer->SetPostProcessPipeline(m_PostProcess.get());

        std::cout << "[Sandbox] All systems initialized (Shadows + Physics + Scripting + Particles + Audio)" << std::endl;
    }

    void OnDetach() override
    {
        AudioEngine::Shutdown();
        ShadowRenderer::Shutdown();
        Renderer::Shutdown();
    }

    void OnUpdate(Timestep ts) override
    {
        float dt = ts.GetSeconds();

        // 输入处理
        HandleInput(dt);

        // 相机旋转/平移（通过轮询，不依赖事件回调）
        UpdateCameraRotation();
        UpdateCameraPan();

        // 场景更新 (脚本 + 物理 + 粒子)
        m_Scene->OnUpdate(ts);

        // ===== 渲染阴影 Pass =====
        if (m_Scene->GetShadowMap())
        {
            auto& shadowMap = *m_Scene->GetShadowMap();
            auto& lightEnv = m_Scene->GetLightEnvironment();
            auto& dirLight = lightEnv.GetDirectionalLight();

            // 计算级联 VP 矩阵
            shadowMap.CalculateCascades(
                dirLight.Direction,
                m_Camera.GetViewMatrix(),
                m_Camera.GetProjectionMatrix(16.0f / 9.0f),
                m_Camera.NearPlane,
                m_Camera.FarPlane
            );

            // 收集可投射阴影的实体
            std::vector<std::pair<std::shared_ptr<VertexArray>, glm::mat4>> shadowCasters;
            m_Scene->ForEachEntity([&](Entity& entity) {
                auto& mesh = entity.GetMesh();
                if (mesh.VertexArray && mesh.CastShadow && mesh.Visible)
                {
                    shadowCasters.emplace_back(
                        mesh.VertexArray,
                        entity.GetTransform().GetWorldMatrix()
                    );
                }
            });

            ShadowRenderer::RenderShadowPass(shadowMap, shadowCasters);
        }

        // ===== 渲染场景 =====
        m_SceneFBO->Bind();
        Renderer::SetClearColor(glm::vec4(0.05f, 0.05f, 0.08f, 1.0f));
        Renderer::SetViewport(0, 0, 1600, 900);
        Renderer::SetDepthTest(true);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer::BeginFrame();

        // 天空盒
        if (m_SkyboxShader && m_SkyboxCube)
        {
            glDepthFunc(GL_LEQUAL);
            m_SkyboxShader->Bind();
            glm::mat4 viewNoTrans = glm::mat4(glm::mat3(m_Camera.GetViewMatrix()));
            glm::mat4 proj = m_Camera.GetProjectionMatrix(16.0f / 9.0f);
            m_SkyboxShader->SetMat4("u_View", viewNoTrans);
            m_SkyboxShader->SetMat4("u_Projection", proj);

            // 优先使用 IBL 环境贴图
            if (m_IBL && m_IBL->IsLoaded())
            {
                m_IBL->BindEnvironmentMap(0);
            }
            else
            {
                m_SkyboxCube->Bind(0);
            }
            m_SkyboxShader->SetInt("u_Skybox", 0);

            auto skyboxVAO = MeshLibrary::GetCube().VAO;
            skyboxVAO->Bind();
            auto& ib = skyboxVAO->GetIndexBuffer();
            if (ib) glDrawElements(GL_TRIANGLES, (GLsizei)ib->GetCount(), GL_UNSIGNED_INT, 0);
            glDepthFunc(GL_LESS);
        }

        m_Scene->OnRender(m_Camera);
        Renderer::EndFrame();
        m_SceneFBO->Unbind();

        // ===== 后处理 =====
        m_PostProcess->Execute(m_SceneFBO);

        // Blit 到 Viewport FBO
        auto finalBuf = m_PostProcess->GetFinalBuffer();
        m_ViewportFBO->Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, finalBuf->GetRendererID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ViewportFBO->GetRendererID());
        glBlitFramebuffer(0, 0, 1600, 900, 0, 0, 1600, 900, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        m_ViewportFBO->Unbind();

        // 编辑器层更新
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

        // 相机移动: 右键按住时或视口聚焦时均可WASD移动
        bool canMoveCamera = m_RightMouseHeld || m_EditorLayer->IsViewportFocused();
        if (!canMoveCamera)
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

        // F5: 给球体一个向上的冲量
        if (Input::IsKeyPressed(Key::F5))
        {
            auto* sphere = m_Scene->FindEntity("Metal Sphere");
            if (sphere && sphere->HasPhysics())
                sphere->GetRigidbody().AddImpulse(glm::vec3(0.0f, 5.0f, 0.0f));
        }
    }

    // 每帧通过轮询处理相机视角旋转，不依赖事件回调
    void UpdateCameraRotation()
    {
        if (!m_RightMouseHeld)
        {
            m_FirstMouse = true;
            return;
        }

        auto [mouseX, mouseY] = Input::GetMousePosition();

        if (m_FirstMouse)
        {
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
            m_FirstMouse = false;
            return;
        }

        float xoffset = mouseX - m_LastMouseX;
        float yoffset = m_LastMouseY - mouseY;
        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;

        m_Camera.ProcessMouseMovement(xoffset, yoffset);
    }

    // 每帧通过轮询处理中键平移
    void UpdateCameraPan()
    {
        if (!m_MiddleMouseHeld)
            return;

        auto [mouseX, mouseY] = Input::GetMousePosition();

        if (m_FirstMouse)
        {
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
            m_FirstMouse = false;
            return;
        }

        float xoffset = mouseX - m_LastMouseX;
        float yoffset = m_LastMouseY - mouseY;
        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;

            float panSpeed = 0.01f;
            m_Camera.Position -= m_Camera.Right * xoffset * panSpeed;
            m_Camera.Position += glm::vec3(0.0f, 1.0f, 0.0f) * yoffset * panSpeed;
    }

    bool OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonRight && m_EditorLayer->IsViewportHovered())
        {
            m_RightMouseHeld = true;
            m_FirstMouse = true;
            return true;
        }
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
        // 鼠标移动改用轮询方式处理（UpdateCameraRotation/UpdateCameraPan），
        // 不再依赖事件回调，避免与 ImGui 的 GLFW 回调冲突
        return false;
    }

    bool OnMouseScrolled(MouseScrolledEvent& e)
    {
        if (!m_EditorLayer->IsViewportHovered())
            return false;

        m_Camera.ProcessMouseScroll(e.GetYOffset());
        return true;
    }

    void GenerateProceduralSkybox()
    {
        glGenTextures(1, &m_SkyboxTexID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexID);

        const int faceSize = 64;
        std::vector<unsigned char> faceData(faceSize * faceSize * 3);

        for (int face = 0; face < 6; face++)
        {
            for (int y = 0; y < faceSize; y++)
            {
                for (int x = 0; x < faceSize; x++)
                {
                    float u = (float)x / faceSize;
                    float v = (float)y / faceSize;
                    glm::vec3 bottomColor(0.05f, 0.05f, 0.15f);
                    glm::vec3 topColor(0.1f, 0.2f, 0.5f);
                    glm::vec3 color = glm::mix(bottomColor, topColor, v);

                    float horizonGlow = std::exp(-std::abs(v - 0.4f) * 8.0f) * 0.3f;
                    color += glm::vec3(horizonGlow * 0.4f, horizonGlow * 0.3f, horizonGlow * 0.2f);

                    int idx = (y * faceSize + x) * 3;
                    faceData[idx + 0] = (unsigned char)(glm::clamp(color.r, 0.0f, 1.0f) * 255);
                    faceData[idx + 1] = (unsigned char)(glm::clamp(color.g, 0.0f, 1.0f) * 255);
                    faceData[idx + 2] = (unsigned char)(glm::clamp(color.b, 0.0f, 1.0f) * 255);
                }
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB,
                         faceSize, faceSize, 0, GL_RGB, GL_UNSIGNED_BYTE, faceData.data());
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        m_SkyboxCube = std::make_shared<SimpleCubeTex>();
        m_SkyboxCube->ID = m_SkyboxTexID;
    }

    std::shared_ptr<Scene> m_Scene;
    std::shared_ptr<Shader> m_PBRShader;
    std::shared_ptr<Shader> m_SkyboxShader;
    std::shared_ptr<Framebuffer> m_SceneFBO;
    std::shared_ptr<Framebuffer> m_ViewportFBO;
    std::unique_ptr<PostProcessPipeline> m_PostProcess;

    uint32_t m_SkyboxTexID = 0;
    std::shared_ptr<SimpleCubeTex> m_SkyboxCube;
    std::shared_ptr<IBL> m_IBL;

    Camera m_Camera;
    std::unique_ptr<EditorLayer> m_EditorLayer;

    bool m_RightMouseHeld  = false;
    bool m_MiddleMouseHeld = false;
    bool m_FirstMouse = true;
    float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
};

// ===== 应用入口 =====
Application* CreateApplication()
{
    Application* app = new Application("OpenGL Graphics Engine", 1600, 900);
    app->PushLayer(std::make_unique<SandboxLayer>());
    return app;
}
