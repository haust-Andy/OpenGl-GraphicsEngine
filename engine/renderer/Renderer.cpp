#include "Renderer.h"
#include "core/Log.h"

Renderer::SceneData Renderer::s_SceneData;
RenderStats Renderer::s_Stats;
std::unique_ptr<RendererAPI> Renderer::s_RendererAPI;
std::shared_ptr<UniformBuffer> Renderer::s_CameraUBO;

void Renderer::Init()
{
    s_RendererAPI = RendererAPI::Create();
    s_RendererAPI->Init();

    // 创建 Camera UBO (binding point = 0)
    s_CameraUBO = UniformBuffer::Create(sizeof(CameraUBOData), 0);

    CORE_INFO("[Renderer] Initialized (OpenGL ", glGetString(GL_VERSION), ")");
    CORE_INFO("[Renderer] GPU: ", glGetString(GL_RENDERER));
}

void Renderer::Shutdown()
{
    s_CameraUBO.reset();
    s_RendererAPI.reset();
}

void Renderer::BeginFrame()
{
    ResetStats();
}

void Renderer::EndFrame()
{
    Flush();
}

void Renderer::BeginScene(const Camera& camera, const glm::mat4& view, const glm::mat4& projection)
{
    s_SceneData.ViewMatrix       = view;
    s_SceneData.ProjectionMatrix = projection;
    s_SceneData.ViewProjectionMatrix = projection * view;
    s_SceneData.CameraPosition   = camera.Position;

    // 更新 Camera UBO
    if (s_CameraUBO)
    {
        CameraUBOData camData;
        camData.View           = view;
        camData.Projection     = projection;
        camData.ViewProjection = s_SceneData.ViewProjectionMatrix;
        camData.CameraPosition = glm::vec4(camera.Position, 1.0f);
        s_CameraUBO->SetData(&camData, sizeof(CameraUBOData));
    }
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const std::shared_ptr<Shader>& shader,
                       const std::shared_ptr<VertexArray>& vertexArray,
                       const glm::mat4& transform)
{
    if (!s_RendererAPI) return;
    shader->Bind();
    shader->SetMat4("u_Model", transform);
    shader->SetMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);
    shader->SetMat4("u_View", s_SceneData.ViewMatrix);
    shader->SetMat4("u_Projection", s_SceneData.ProjectionMatrix);
    shader->SetVec3("u_CameraPos", s_SceneData.CameraPosition);

    vertexArray->Bind();
    auto& ib = vertexArray->GetIndexBuffer();
    if (ib)
    {
        s_RendererAPI->DrawIndexed(vertexArray, ib->GetCount());
        s_Stats.DrawCalls++;
        s_Stats.TriangleCount += ib->GetCount() / 3;
    }
}

void Renderer::SubmitMesh(const std::shared_ptr<Shader>& shader,
                           const std::shared_ptr<VertexArray>& vertexArray,
                           uint32_t indexCount,
                           const glm::mat4& transform)
{
    if (!s_RendererAPI) return;
    shader->Bind();
    shader->SetMat4("u_Model", transform);
    shader->SetMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);

    vertexArray->Bind();
    s_RendererAPI->DrawIndexed(vertexArray, indexCount);
    s_Stats.DrawCalls++;
    s_Stats.TriangleCount += indexCount / 3;
}

void Renderer::DrawFullscreenQuad()
{
    if (!s_RendererAPI) return;
    auto quadVAO = GetFullscreenQuadVAO();
    quadVAO->Bind();
    s_RendererAPI->DrawIndexed(quadVAO, 6);
    s_Stats.DrawCalls++;
}

std::shared_ptr<VertexArray> Renderer::GetFullscreenQuadVAO()
{
    static std::shared_ptr<VertexArray> s_QuadVAO;

    if (!s_QuadVAO)
    {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,    0.0f, 1.0f,
            -1.0f, -1.0f,    0.0f, 0.0f,
             1.0f, -1.0f,    1.0f, 0.0f,
             1.0f,  1.0f,    1.0f, 1.0f,
        };
        uint32_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

        auto vbo = std::make_shared<VertexBuffer>(quadVertices, sizeof(quadVertices));
        auto ibo = std::make_shared<IndexBuffer>(quadIndices, 6);

        s_QuadVAO = std::make_shared<VertexArray>();
        s_QuadVAO->AddVertexBuffer(vbo);
        s_QuadVAO->SetIndexBuffer(ibo);

        // 设置属性布局: position(2) + texcoord(2)
        // 注意: 此处仍需直接设置，因为 VertexArray 的 AddVertexBuffer 接口需要增强
        // TODO(code-review): 在 VertexArray 中提供标准化的全屏四边形布局定义方法
        glBindVertexArray(s_QuadVAO->GetRendererID());
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    return s_QuadVAO;
}

void Renderer::ResetStats()
{
    s_Stats.Reset();
}

void Renderer::SetClearColor(const glm::vec4& color)
{
    if (!s_RendererAPI) return;
    s_RendererAPI->SetClearColor(color);
}

void Renderer::Clear()
{
    if (!s_RendererAPI) return;
    s_RendererAPI->Clear();
}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if (!s_RendererAPI) return;
    s_RendererAPI->SetViewport(x, y, width, height);
}

void Renderer::SetDepthTest(bool enabled)
{
    if (enabled) glEnable(GL_DEPTH_TEST);
    else         glDisable(GL_DEPTH_TEST);
}

void Renderer::SetCullFace(bool enabled)
{
    if (enabled) glEnable(GL_CULL_FACE);
    else         glDisable(GL_CULL_FACE);
}

void Renderer::SetWireframe(bool enabled)
{
    if (enabled) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::Flush()
{
    // 当前简单实现: 立即渲染
    // 未来可扩展为: 排序、批处理、实例化等
}
