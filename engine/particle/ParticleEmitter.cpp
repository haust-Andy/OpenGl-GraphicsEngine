#include "ParticleEmitter.h"
#include "renderer/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void ParticleEmitter::Update(float dt)
{
    if (!Enabled) return;

    // 发射新粒子
    m_EmitAccumulator += Config.EmitRate * dt;
    int toEmit = (int)m_EmitAccumulator;
    if (toEmit > 0)
    {
        m_EmitAccumulator -= toEmit;
        Emit(toEmit);
    }

    // 更新所有粒子
    m_ActiveCount = 0;
    for (auto& p : m_Particles)
    {
        if (!p.IsAlive()) continue;
        UpdateParticle(p, dt);
        m_ActiveCount++;
    }
}

void ParticleEmitter::Render(const Camera& camera)
{
    if (m_ActiveCount == 0) return;

    if (!m_Initialized) InitRenderer();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_Shader->Bind();
    m_Shader->SetMat4("u_View", camera.GetViewMatrix());
    m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix(16.0f / 9.0f));

    if (Config.ParticleTexture)
    {
        Config.ParticleTexture->Bind(0);
        m_Shader->SetInt("u_ParticleTexture", 0);
        m_Shader->SetBool("u_HasTexture", true);
    }
    else
    {
        m_Shader->SetBool("u_HasTexture", false);
    }

    m_QuadVAO->Bind();

    for (auto& p : m_Particles)
    {
        if (!p.IsAlive()) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.Position);
        model = glm::rotate(model, p.Rotation, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(p.Size));

        // Billboard: 取视图旋转但保持位置
        glm::mat4 view = camera.GetViewMatrix();
        model[0] = glm::vec4(view[0][0], view[1][0], view[2][0], 0.0f) * p.Size;
        model[1] = glm::vec4(view[0][1], view[1][1], view[2][1], 0.0f) * p.Size;
        model[2] = glm::vec4(view[0][2], view[1][2], view[2][2], 0.0f) * p.Size;
        model[3] = glm::vec4(p.Position, 1.0f);

        m_Shader->SetMat4("u_Model", model);
        m_Shader->SetVec4("u_Color", p.Color);

        auto& ib = m_QuadVAO->GetIndexBuffer();
        if (ib)
            glDrawElements(GL_TRIANGLES, (GLsizei)ib->GetCount(), GL_UNSIGNED_INT, 0);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void ParticleEmitter::Burst(int count)
{
    Emit(count);
}

void ParticleEmitter::Emit(int count)
{
    // 确保有足够的粒子槽位
    if (m_Particles.size() < (size_t)Config.MaxParticles)
        m_Particles.resize(Config.MaxParticles);

    std::uniform_real_distribution<float> distLife(Config.MinLife, Config.MaxLife);
    std::uniform_real_distribution<float> distSpeed(Config.MinSpeed, Config.MaxSpeed);
    std::uniform_real_distribution<float> distAngle(-glm::radians(Config.SpreadAngle),
                                                      glm::radians(Config.SpreadAngle));
    std::uniform_real_distribution<float> distRadius(0.0f, Config.EmitRadius);
    std::uniform_real_distribution<float> distOne(-1.0f, 1.0f);

    int emitted = 0;
    for (auto& p : m_Particles)
    {
        if (p.IsAlive()) continue;
        if (emitted >= count) break;

        p.Position = Position;
        if (Config.EmitRadius > 0.0f)
        {
            p.Position += glm::vec3(distOne(m_RNG), distOne(m_RNG), distOne(m_RNG)) * distRadius(m_RNG);
        }

        p.Life    = distLife(m_RNG);
        p.MaxLife  = p.Life;

        float speed = distSpeed(m_RNG);
        // 在方向锥体内随机
        glm::vec3 dir = Config.Direction;
        dir = glm::normalize(dir + glm::vec3(distAngle(m_RNG), distAngle(m_RNG), distAngle(m_RNG)));
        p.Velocity = dir * speed;

        p.Size     = Config.StartSize;
        p.Color    = Config.StartColor;
        p.Rotation = 0.0f;

        emitted++;
    }
}

void ParticleEmitter::UpdateParticle(Particle& p, float dt)
{
    p.Life -= dt;
    if (p.Life <= 0.0f) { p.Life = 0.0f; return; }

    p.Velocity += Config.Gravity * dt;
    p.Position += p.Velocity * dt;

    float ratio = p.GetLifeRatio();
    p.Size   = glm::mix(Config.EndSize,   Config.StartSize,  ratio);
    p.Color  = glm::mix(Config.EndColor,   Config.StartColor, ratio);
    p.Rotation += dt * 0.5f;
}

void ParticleEmitter::InitRenderer()
{
    if (m_Initialized) return;

    const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 0.0, 1.0);
}
)";

    const char* fragSrc = R"(
#version 330 core
in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform bool u_HasTexture;
uniform sampler2D u_ParticleTexture;

out vec4 FragColor;

void main()
{
    if (u_HasTexture)
        FragColor = texture(u_ParticleTexture, v_TexCoord) * u_Color;
    else
        FragColor = u_Color;
}
)";

    m_Shader = std::make_shared<Shader>();
    m_Shader->Compile(vertSrc, fragSrc);
    m_Shader->SetName("Particle");

    // 创建 Quad VAO
    float quadVertices[] = {
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f,
    };
    uint32_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

    auto vbo = std::make_shared<VertexBuffer>(quadVertices, sizeof(quadVertices));
    auto ibo = std::make_shared<IndexBuffer>(quadIndices, 6);
    m_QuadVAO = std::make_shared<VertexArray>();
    m_QuadVAO->AddVertexBuffer(vbo);
    m_QuadVAO->SetIndexBuffer(ibo);

    glBindVertexArray(m_QuadVAO->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    m_Initialized = true;
}
