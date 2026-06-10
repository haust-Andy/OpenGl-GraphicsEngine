#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <functional>
#include <utility>

#include "Camera.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "RenderCommand.h"
#include "RendererAPI.h"

class LightEnvironment;

// ===== Camera UBO 布局 (std140, 向 shader 传递相机矩阵) =====
struct CameraUBOData
{
    glm::mat4 View;
    glm::mat4 Projection;
    glm::mat4 ViewProjection;
    glm::vec4 CameraPosition;  // .xyz = pos, .w = unused
};

// 渲染器 - 核心渲染管理类
class Renderer
{
public:
    static void Init();
    static void Shutdown();

    static void BeginFrame();
    static void EndFrame();

    // 设置当前帧的相机和视口
    static void BeginScene(const Camera& camera, const glm::mat4& view, const glm::mat4& projection);
    static void EndScene();

    // 提交绘制
    static void Submit(const std::shared_ptr<Shader>& shader,
                       const std::shared_ptr<VertexArray>& vertexArray,
                       const glm::mat4& transform = glm::mat4(1.0f));

    static void SubmitMesh(const std::shared_ptr<Shader>& shader,
                           const std::shared_ptr<VertexArray>& vertexArray,
                           uint32_t indexCount,
                           const glm::mat4& transform);

    // 全屏四边形绘制 (用于后处理)
    static void DrawFullscreenQuad();

    // 获取全屏四边形 VAO
    static std::shared_ptr<VertexArray> GetFullscreenQuadVAO();

    // 获取统计信息
    static const RenderStats& GetStats() { return s_Stats; }
    static void ResetStats();

    // 渲染配置
    static void SetClearColor(const glm::vec4& color);
    static void Clear();
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    static void SetDepthTest(bool enabled);
    static void SetCullFace(bool enabled);
    static void SetWireframe(bool enabled);

    // UBO 接口
    static std::shared_ptr<UniformBuffer> GetCameraUBO() { return s_CameraUBO; }

private:
    static void Flush();

    struct SceneData
    {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewProjectionMatrix;
        glm::vec3 CameraPosition;
    };

    static SceneData s_SceneData;
    static RenderStats s_Stats;
    static std::unique_ptr<RendererAPI> s_RendererAPI;
    static std::shared_ptr<UniformBuffer> s_CameraUBO;
};
