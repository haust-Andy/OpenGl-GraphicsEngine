#include "RendererAPI.h"
#include "VertexArray.h"
#include "Buffer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

// OpenGL 实现
class OpenGLRendererAPI : public RendererAPI
{
public:
    void Init() override
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override
    {
        glViewport(x, y, width, height);
    }

    void SetClearColor(const glm::vec4& color) override
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void Clear() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) override
    {
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }

    void DrawArrays(uint32_t count) override
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }
};

std::unique_ptr<RendererAPI> RendererAPI::Create()
{
    return std::make_unique<OpenGLRendererAPI>();
}
