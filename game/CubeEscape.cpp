#include "engine/core/Application.h"
#include "engine/core/EntryPoint.h"
#include "engine/core/Input.h"
#include "engine/core/KeyCodes.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Camera.h"
#include "engine/renderer/Shader.h"
#include "engine/resource/MeshLibrary.h"
#include "engine/resource/ShaderLibrary.h"
#include "engine/scene/Scene.h"
#include "engine/scene/TransformComponent.h"
#include "engine/scene/MeshComponent.h"
#include "engine/scene/ScriptComponent.h"
#include "engine/particle/ParticleEmitter.h"
#include "imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>

// ===== 游戏常量 =====
static constexpr float k_PlayArea      = 10.0f;
static constexpr float k_PlayerSpeed   = 6.0f;
static constexpr float k_EnemySpeed    = 2.0f;
static constexpr float k_SpawnInterval = 1.2f;
static constexpr float k_EnemySpawnInterval = 2.5f;
static constexpr float k_CollectRadius = 0.9f;
static constexpr float k_EnemyRadius   = 1.0f;
static constexpr int   k_ScorePerCollect = 10;
static constexpr int   k_ScorePerLevel   = 100;

// ===== 辅助：随机位置 =====
static glm::vec3 RandomPosition(float range = k_PlayArea)
{
    return glm::vec3(
        glm::linearRand(-range, range),
        0.5f,
        glm::linearRand(-range, range)
    );
}

// ===== Cube Escape 游戏层 =====
class CubeEscapeLayer : public Layer
{
public:
    CubeEscapeLayer()
        : Layer("CubeEscape") {}

    // ==================== 生命周期 ====================
    void OnAttach() override
    {
        Renderer::Init();

        // 创建场景
        m_Scene = Scene::Create("Cube Escape");

        // 加载 PBR Shader
        m_PBRShader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");
        ShaderLibrary::Instance().Add("pbr", m_PBRShader);

        // 初始化相机（俯视视角）
        m_Camera = Camera(glm::vec3(0.0f, 12.0f, 6.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f),
                          -90.0f, -65.0f);
        m_Camera.MovementSpeed  = 0.0f;
        m_Camera.ViewportWidth  = 1280;
        m_Camera.ViewportHeight = 720;

        // 地面
        CreateGround();

        // 边界墙
        CreateWalls();

        // 玩家
        CreatePlayer();

        // 光照（entity-based，避免被 CollectLights 重置）
        CreateLightEntities();

        // 随机种子
        std::srand((unsigned)std::time(nullptr));

        CORE_INFO("[CubeEscape] 游戏已启动! WASD 移动, 收集金色球, 躲避红色敌人");
    }

    void OnDetach() override
    {
        Renderer::Shutdown();
    }

    // ==================== 主循环 ====================
    void OnUpdate(Timestep ts) override
    {
        float dt = ts.GetSeconds();

        // ---- 游戏结束状态 ----
        if (m_GameOver)
        {
            if (Input::IsKeyPressed(Key::R))
                RestartGame();

            // 静止渲染
            RenderScene();
            return;
        }

        m_GameTime += dt;

        // ---- 输入 ----
        HandlePlayerInput(dt);

        // ---- 生成 ----
        HandleSpawning(dt);

        // ---- 敌人 AI ----
        UpdateEnemies(dt);

        // ---- 碰撞检测 ----
        CheckCollisions();

        // ---- 升级检测 ----
        if (m_Score >= m_Level * k_ScorePerLevel)
        {
            m_Level++;
            m_EnemySpeed   = 2.0f + (m_Level - 1) * 0.5f;
            m_EnemySpawnCD = std::max(0.6f, k_EnemySpawnInterval - (m_Level - 1) * 0.15f);
            CORE_INFO("[CubeEscape] 升级! Level {}", m_Level);
        }

        // ---- 场景更新（脚本 + 粒子）----
        m_Scene->OnUpdate(ts);

        // ---- 相机跟随 ----
        UpdateCamera();

        // ---- 渲染 ----
        RenderScene();
    }

    // ==================== ImGui HUD ====================
    void OnImGuiRender() override
    {
        // HUD
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::Begin("HUD", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Score: %d", m_Score);
        ImGui::Text("Lives: %d  |  Level: %d", m_Lives, m_Level);
        ImGui::Text("Time: %.0fs", m_GameTime);
        ImGui::Text("Collectibles: %d  |  Enemies: %d",
                    (int)m_Collectibles.size(), (int)m_Enemies.size());
        ImGui::End();

        // 操作提示
        ImGui::SetNextWindowPos(ImVec2(10, 110));
        ImGui::Begin("Controls", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextDisabled("WASD - Move  |  ESC - Quit");
        ImGui::End();

        // Game Over 窗口
        if (m_GameOver)
        {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(320, 140));
            ImGui::Begin("GameOver", nullptr,
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize);

            float textWidth = ImGui::CalcTextSize("GAME OVER").x;
            ImGui::SetCursorPosX((320 - textWidth) * 0.5f);
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "GAME OVER");
            ImGui::Spacing();

            textWidth = ImGui::CalcTextSize(("Final Score: " + std::to_string(m_Score)).c_str()).x;
            ImGui::SetCursorPosX((320 - textWidth) * 0.5f);
            ImGui::Text("Final Score: %d  |  Level: %d", m_Score, m_Level);
            ImGui::Spacing();

            textWidth = ImGui::CalcTextSize("Press R to Restart").x;
            ImGui::SetCursorPosX((320 - textWidth) * 0.5f);
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Press R to Restart");

            ImGui::End();
        }
    }

    // ==================== 事件 ====================
    void OnEvent(Event& event) override
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(CubeEscapeLayer::OnKeyPressed));
    }

    bool OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == Key::Escape)
            Application::Get().Close();
        return false;
    }

private:
    // ==================== 场景构建 ====================

    void CreateGround()
    {
        auto* ground = m_Scene->CreateEntity("Ground");
        ground->GetTransform().Position = glm::vec3(0.0f, -0.5f, 0.0f);
        ground->GetTransform().Scale    = glm::vec3(k_PlayArea * 2.2f, 1.0f, k_PlayArea * 2.2f);
        ground->IsStatic = true;

        auto& mesh = ground->GetMesh();
        mesh.SetMesh(MeshLibrary::GetPlane().VAO);
        mesh.SetMaterial(Material::Create(m_PBRShader));
        mesh.Material->GetProperties().Albedo    = glm::vec3(0.25f, 0.25f, 0.3f);
        mesh.Material->GetProperties().Metallic  = 0.05f;
        mesh.Material->GetProperties().Roughness = 0.85f;
        mesh.CastShadow = false;
    }

    void CreateWalls()
    {
        auto createWall = [&](const std::string& name,
                              const glm::vec3& pos,
                              const glm::vec3& scale)
        {
            auto* wall = m_Scene->CreateEntity(name);
            wall->GetTransform().Position = pos;
            wall->GetTransform().Scale    = scale;
            wall->IsStatic = true;

            auto& mesh = wall->GetMesh();
            mesh.SetMesh(MeshLibrary::GetCube().VAO);
            mesh.SetMaterial(Material::Create(m_PBRShader));
            mesh.Material->GetProperties().Albedo    = glm::vec3(0.15f, 0.4f, 0.6f);
            mesh.Material->GetProperties().Metallic  = 0.3f;
            mesh.Material->GetProperties().Roughness = 0.4f;
            mesh.CastShadow = true;
        };

        float half = k_PlayArea + 1.0f;
        float wallLen = half * 2.0f;

        createWall("Wall_North", glm::vec3(0.0f, 0.5f, -half), glm::vec3(wallLen, 1.0f, 0.3f));
        createWall("Wall_South", glm::vec3(0.0f, 0.5f,  half), glm::vec3(wallLen, 1.0f, 0.3f));
        createWall("Wall_West",  glm::vec3(-half, 0.5f, 0.0f), glm::vec3(0.3f, 1.0f, wallLen));
        createWall("Wall_East",  glm::vec3( half, 0.5f, 0.0f), glm::vec3(0.3f, 1.0f, wallLen));
    }

    void CreatePlayer()
    {
        m_Player = m_Scene->CreateEntity("Player");
        m_Player->GetTransform().Position = glm::vec3(0.0f, 0.5f, 0.0f);
        m_Player->GetTransform().Scale    = glm::vec3(0.8f);

        auto& mesh = m_Player->GetMesh();
        mesh.SetMesh(MeshLibrary::GetCube().VAO);
        mesh.SetMaterial(Material::Create(m_PBRShader));
        mesh.Material->GetProperties().Albedo    = glm::vec3(0.2f, 0.5f, 0.9f);
        mesh.Material->GetProperties().Metallic  = 0.2f;
        mesh.Material->GetProperties().Roughness = 0.3f;
        mesh.Material->GetProperties().Emission  = glm::vec3(0.1f, 0.3f, 0.7f);
        mesh.Material->GetProperties().EmissionStrength = 0.4f;

        // 玩家光环粒子
        ParticleEmitterConfig ringConfig;
        ringConfig.EmitRate     = 15;
        ringConfig.MaxParticles = 60;
        ringConfig.MinLife      = 0.6f;
        ringConfig.MaxLife      = 1.2f;
        ringConfig.MinSpeed     = 0.3f;
        ringConfig.MaxSpeed     = 0.8f;
        ringConfig.StartSize    = 0.12f;
        ringConfig.EndSize      = 0.0f;
        ringConfig.Direction    = glm::vec3(0.0f, 0.5f, 0.0f);
        ringConfig.SpreadAngle  = 60.0f;
        ringConfig.StartColor   = glm::vec4(0.3f, 0.5f, 1.0f, 0.8f);
        ringConfig.EndColor     = glm::vec4(0.1f, 0.2f, 0.6f, 0.0f);
        ringConfig.Gravity      = glm::vec3(0.0f, 0.3f, 0.0f);
        ringConfig.EmitRadius   = 0.4f;
        m_Player->AddParticleEmitter(ringConfig);

        // 旋转脚本
        auto& script = m_Player->GetScript();
        script.ScriptName = "PlayerSpin";
        script.OnUpdate = [](Entity& e, Timestep ts) {
            float dt = ts.GetSeconds();
            glm::vec3 euler = e.GetTransform().GetEulerAngles();
            euler.y += 60.0f * dt;
            euler.x += 30.0f * dt;
            e.GetTransform().SetEulerAngles(euler);
        };
    }

    // ========== 关键修复：使用 Entity-based 灯光 ==========
    // 问题：Scene::OnRender() → CollectLights() 执行
    //   m_LightEnv = LightEnvironment() 会清除直接设置的环境光。
    // 解决方案：创建带 LightComponent 的实体，
    //   CollectLights 会自动从实体中收集灯光数据。
    void CreateLightEntities()
    {
        // 方向光（模拟太阳）
        {
            auto* e = m_Scene->CreateEntity("Light_Directional");
            e->IsStatic = true;
            LightComponent lc;
            lc.Type = LightType::Directional;
            lc.Enabled = true;
            lc.DirLight.Direction = glm::vec3(-0.5f, -1.0f, -0.3f);
            lc.DirLight.Color     = glm::vec3(1.0f, 0.98f, 0.92f);
            lc.DirLight.Intensity = 1.8f;
            e->SetLight(lc);
        }

        // 顶光源（照亮场景上方）
        {
            auto* e = m_Scene->CreateEntity("Light_Point1");
            e->IsStatic = true;
            LightComponent lc;
            lc.Type = LightType::Point;
            lc.Enabled = true;
            lc.PtLight.Position  = glm::vec3(0.0f, 8.0f, 0.0f);
            lc.PtLight.Color     = glm::vec3(1.0f, 1.0f, 1.0f);
            lc.PtLight.Intensity = 0.6f;
            lc.PtLight.Range     = 20.0f;
            e->SetLight(lc);
        }

        // 暖色点光源（靠近地面，提供温暖氛围）
        {
            auto* e = m_Scene->CreateEntity("Light_Point2");
            e->IsStatic = true;
            LightComponent lc;
            lc.Type = LightType::Point;
            lc.Enabled = true;
            lc.PtLight.Position  = glm::vec3(0.0f, 2.0f, 0.0f);
            lc.PtLight.Color     = glm::vec3(0.95f, 0.65f, 0.3f);
            lc.PtLight.Intensity = 1.2f;
            lc.PtLight.Range     = 15.0f;
            e->SetLight(lc);
        }

        // 设置环境光（会被 CollectLights 重置为默认值 0.1，
        // 但在 RenderScene 后重新设置）
        m_Scene->GetLightEnvironment().SetAmbientLight(
            glm::vec3(1.0f, 0.95f, 0.9f), 0.25f);
    }

    // ==================== 输入 ====================

    void HandlePlayerInput(float dt)
    {
        glm::vec3 moveDir(0.0f);

        if (Input::IsKeyPressed(Key::W))  moveDir.z -= 1.0f;
        if (Input::IsKeyPressed(Key::S))  moveDir.z += 1.0f;
        if (Input::IsKeyPressed(Key::A))  moveDir.x -= 1.0f;
        if (Input::IsKeyPressed(Key::D))  moveDir.x += 1.0f;

        if (glm::length(moveDir) > 0.01f)
        {
            moveDir = glm::normalize(moveDir);
            glm::vec3& pos = m_Player->GetTransform().Position;
            pos += moveDir * k_PlayerSpeed * dt;

            // 边界限制
            pos.x = glm::clamp(pos.x, -k_PlayArea, k_PlayArea);
            pos.z = glm::clamp(pos.z, -k_PlayArea, k_PlayArea);
        }
    }

    // ==================== 生成 ====================

    void HandleSpawning(float dt)
    {
        m_SpawnTimer      += dt;
        m_EnemySpawnTimer += dt;

        if (m_SpawnTimer >= m_SpawnCD)
        {
            m_SpawnTimer = 0.0f;
            SpawnCollectible();
        }

        if (m_EnemySpawnTimer >= m_EnemySpawnCD)
        {
            m_EnemySpawnTimer = 0.0f;
            SpawnEnemy();
        }
    }

    void SpawnCollectible()
    {
        auto* gem = m_Scene->CreateEntity("Collectible_" + std::to_string(m_NextCollectibleID++));
        gem->GetTransform().Position = RandomPosition(k_PlayArea * 0.85f);
        gem->GetTransform().Scale    = glm::vec3(0.35f);

        auto& mesh = gem->GetMesh();
        mesh.SetMesh(MeshLibrary::GetSphere(16).VAO);
        mesh.SetMaterial(Material::Create(m_PBRShader));
        mesh.Material->GetProperties().Albedo    = glm::vec3(1.0f, 0.85f, 0.1f);
        mesh.Material->GetProperties().Metallic  = 0.6f;
        mesh.Material->GetProperties().Roughness = 0.25f;
        mesh.Material->GetProperties().Emission  = glm::vec3(1.0f, 0.7f, 0.1f);
        mesh.Material->GetProperties().EmissionStrength = 0.6f;

        // 浮动旋转脚本
        auto& script = gem->GetScript();
        script.ScriptName = "GemFloat";
        script.OnUpdate = [](Entity& e, Timestep ts) {
            float dt = ts.GetSeconds();
            glm::vec3 euler = e.GetTransform().GetEulerAngles();
            euler.y += 90.0f * dt;
            e.GetTransform().SetEulerAngles(euler);
        };

        m_Collectibles.push_back(gem);
    }

    void SpawnEnemy()
    {
        auto* enemy = m_Scene->CreateEntity("Enemy_" + std::to_string(m_NextEnemyID++));
        enemy->GetTransform().Scale = glm::vec3(0.7f);

        // 从边界外生成
        int edge = std::rand() % 4;
        float pos = glm::linearRand(-k_PlayArea, k_PlayArea);
        float outside = k_PlayArea + 1.5f;
        switch (edge)
        {
            case 0: enemy->GetTransform().Position = glm::vec3(pos, 0.5f, -outside); break;
            case 1: enemy->GetTransform().Position = glm::vec3(pos, 0.5f,  outside); break;
            case 2: enemy->GetTransform().Position = glm::vec3(-outside, 0.5f, pos); break;
            case 3: enemy->GetTransform().Position = glm::vec3( outside, 0.5f, pos); break;
        }

        auto& mesh = enemy->GetMesh();
        mesh.SetMesh(MeshLibrary::GetSphere(12).VAO);
        mesh.SetMaterial(Material::Create(m_PBRShader));
        mesh.Material->GetProperties().Albedo    = glm::vec3(0.9f, 0.2f, 0.15f);
        mesh.Material->GetProperties().Metallic  = 0.1f;
        mesh.Material->GetProperties().Roughness = 0.5f;
        mesh.Material->GetProperties().Emission  = glm::vec3(0.8f, 0.1f, 0.0f);
        mesh.Material->GetProperties().EmissionStrength = 0.4f;

        // 敌意粒子效果
        ParticleEmitterConfig enemyFx;
        enemyFx.EmitRate    = 10;
        enemyFx.MaxParticles = 30;
        enemyFx.MinLife     = 0.3f;
        enemyFx.MaxLife     = 0.8f;
        enemyFx.MinSpeed    = 0.2f;
        enemyFx.MaxSpeed    = 0.5f;
        enemyFx.StartSize   = 0.08f;
        enemyFx.EndSize     = 0.0f;
        enemyFx.StartColor  = glm::vec4(0.9f, 0.2f, 0.1f, 0.6f);
        enemyFx.EndColor    = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);
        enemyFx.Gravity     = glm::vec3(0.0f, 0.1f, 0.0f);
        enemyFx.EmitRadius  = 0.35f;
        enemy->AddParticleEmitter(enemyFx);

        m_Enemies.push_back(enemy);
    }

    // ==================== 敌人 AI ====================

    void UpdateEnemies(float dt)
    {
        glm::vec3 playerPos = m_Player->GetTransform().Position;

        for (auto* enemy : m_Enemies)
        {
            if (!enemy->IsActive) continue;

            glm::vec3& ePos = enemy->GetTransform().Position;
            glm::vec3 dir   = glm::normalize(playerPos - ePos);
            ePos += dir * m_EnemySpeed * dt;

            ePos.x = glm::clamp(ePos.x, -k_PlayArea, k_PlayArea);
            ePos.z = glm::clamp(ePos.z, -k_PlayArea, k_PlayArea);
        }
    }

    // ==================== 碰撞 ====================

    void CheckCollisions()
    {
        glm::vec3 playerPos = m_Player->GetTransform().Position;

        // 收集物碰撞
        for (auto it = m_Collectibles.begin(); it != m_Collectibles.end();)
        {
            Entity* gem = *it;
            if (!gem->IsActive)
            {
                it = m_Collectibles.erase(it);
                continue;
            }

            float dist = glm::distance(playerPos, gem->GetTransform().Position);
            if (dist < k_CollectRadius)
            {
                m_Score += k_ScorePerCollect;
                SpawnBurstParticles(gem->GetTransform().Position,
                    glm::vec4(1.0f, 0.85f, 0.1f, 1.0f));

                gem->IsActive = false;
                it = m_Collectibles.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // 敌人碰撞
        for (auto it = m_Enemies.begin(); it != m_Enemies.end();)
        {
            Entity* enemy = *it;
            if (!enemy->IsActive)
            {
                it = m_Enemies.erase(it);
                continue;
            }

            float dist = glm::distance(playerPos, enemy->GetTransform().Position);
            if (dist < k_EnemyRadius)
            {
                m_Lives--;
                SpawnBurstParticles(enemy->GetTransform().Position,
                    glm::vec4(0.9f, 0.2f, 0.1f, 1.0f));

                enemy->IsActive = false;
                it = m_Enemies.erase(it);

                if (m_Lives <= 0)
                {
                    m_GameOver = true;
                    SpawnBurstParticles(playerPos,
                        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                    CORE_INFO("[CubeEscape] Game Over! Score: {}", m_Score);
                }
            }
            else
            {
                ++it;
            }
        }
    }

    // ==================== 粒子特效 ====================

    void SpawnBurstParticles(const glm::vec3& pos, const glm::vec4& color)
    {
        auto* fx = m_Scene->CreateEntity("FX_Burst_" + std::to_string(m_FxCounter++));
        fx->GetTransform().Position = pos;
        fx->IsStatic = true;  // 不参与脚本更新

        ParticleEmitterConfig config;
        config.EmitRate    = 0;
        config.BurstCount  = 20;
        config.MaxParticles = 30;
        config.MinLife     = 0.3f;
        config.MaxLife     = 1.0f;
        config.MinSpeed    = 1.0f;
        config.MaxSpeed    = 3.0f;
        config.StartSize   = 0.15f;
        config.EndSize     = 0.0f;
        config.Direction   = glm::vec3(0.0f, 1.0f, 0.0f);
        config.SpreadAngle = 180.0f;
        config.StartColor  = color;
        config.EndColor    = glm::vec4(color.r, color.g, color.b, 0.0f);
        config.Gravity     = glm::vec3(0.0f, -2.0f, 0.0f);
        config.EmitRadius  = 0.0f;
        fx->AddParticleEmitter(config);

        m_Effects.push_back(fx);
    }

    // ==================== 相机 ====================

    void UpdateCamera()
    {
        glm::vec3 playerPos  = m_Player->GetTransform().Position;
        glm::vec3 targetPos  = playerPos + glm::vec3(0.0f, 12.0f, 6.0f);

        // 平滑跟随
        m_Camera.Position = glm::mix(m_Camera.Position, targetPos, 0.08f);

        // 看向玩家位置
        glm::vec3 lookTarget = playerPos + glm::vec3(0.0f, 0.5f, 0.0f);
        m_Camera.Front = glm::normalize(lookTarget - m_Camera.Position);

        // 同时更新 Yaw/Pitch 保持一致（便于编辑器等使用）
        m_Camera.Yaw   = glm::degrees(std::atan2(m_Camera.Front.z, m_Camera.Front.x)) - 90.0f;
        m_Camera.Pitch = glm::degrees(std::asin(-m_Camera.Front.y));
    }

    // ==================== 渲染 ====================

    void RenderScene()
    {
        Renderer::SetClearColor(glm::vec4(0.05f, 0.06f, 0.12f, 1.0f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::SetViewport(0, 0, 1280, 720);
        Renderer::SetDepthTest(true);

        Renderer::BeginFrame();
        m_Scene->OnRender(m_Camera);
        Renderer::EndFrame();

        // 渲染后重新设置环境光（因为 CollectLights 会重置它）
        m_Scene->GetLightEnvironment().SetAmbientLight(
            glm::vec3(1.0f, 0.95f, 0.9f), 0.25f);

        // 清理已完成的特效
        CleanupEffects();
    }

    void CleanupEffects()
    {
        for (auto it = m_Effects.begin(); it != m_Effects.end();)
        {
            Entity* fx = *it;
            bool finished = false;

            if (fx->HasParticleEmitter() && fx->GetParticleEmitter().GetActiveCount() == 0)
                finished = true;
            else if (!fx->HasParticleEmitter())
                finished = true;

            if (finished)
            {
                fx->IsActive = false;
                it = m_Effects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // ==================== 重启 ====================

    void RestartGame()
    {
        // 收集需要删除的实体（不包括灯光和固定物体）
        std::vector<uint32_t> toRemove;
        for (auto& entityPtr : m_Scene->GetEntities())
        {
            const std::string& tag = entityPtr->GetTag();
            if (tag.find("Collectible_")  == 0 ||
                tag.find("Enemy_")        == 0 ||
                tag.find("FX_Burst_")     == 0)
            {
                toRemove.push_back(entityPtr->GetID());
            }
        }

        // 用 ID 查找并删除
        for (uint32_t id : toRemove)
        {
            Entity* e = m_Scene->GetEntity(id);
            if (e) m_Scene->DestroyEntity(e);
        }

        m_Collectibles.clear();
        m_Enemies.clear();
        m_Effects.clear();

        m_Score     = 0;
        m_Lives     = 3;
        m_Level     = 1;
        m_GameTime  = 0.0f;
        m_GameOver  = false;

        m_SpawnTimer      = 0.0f;
        m_EnemySpawnTimer = 0.0f;
        m_EnemySpeed      = k_EnemySpeed;
        m_EnemySpawnCD    = k_EnemySpawnInterval;

        m_FxCounter         = 0;
        m_NextCollectibleID = 0;
        m_NextEnemyID       = 0;

        // 玩家归位
        m_Player->GetTransform().Position = glm::vec3(0.0f, 0.5f, 0.0f);

        CORE_INFO("[CubeEscape] 游戏已重新开始!");
    }

    // ==================== 成员变量 ====================

    // 引擎资源
    std::shared_ptr<Scene>  m_Scene;
    std::shared_ptr<Shader> m_PBRShader;
    Camera m_Camera;
    Entity* m_Player = nullptr;

    // 实体追踪
    std::vector<Entity*> m_Collectibles;
    std::vector<Entity*> m_Enemies;
    std::vector<Entity*> m_Effects;

    // ID 计数器
    int m_NextCollectibleID = 0;
    int m_NextEnemyID       = 0;
    int m_FxCounter         = 0;

    // 游戏状态
    int   m_Score     = 0;
    int   m_Lives     = 3;
    int   m_Level     = 1;
    float m_GameTime  = 0.0f;
    bool  m_GameOver  = false;

    // 计时器
    float m_SpawnTimer     = 0.0f;
    float m_SpawnCD        = k_SpawnInterval;
    float m_EnemySpawnTimer = 0.0f;
    float m_EnemySpawnCD   = k_EnemySpawnInterval;

    // 难度
    float m_EnemySpeed = k_EnemySpeed;
};

// ===== 应用入口 =====
Application* CreateApplication()
{
    Application* app = new Application("Cube Escape - Engine Game Demo", 1280, 720);
    app->PushLayer(std::make_unique<CubeEscapeLayer>());
    return app;
}
