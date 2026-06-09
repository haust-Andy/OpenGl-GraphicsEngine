#pragma once

#include <glm/glm.hpp>
#include "RendererAPI.h"

// 渲染命令 - 将绘制调用封装为统一接口
struct RenderCommand
{
    std::shared_ptr<class Mesh>      mesh;
    std::shared_ptr<class Material>  material;
    glm::mat4                        transform;
    float                            sortKey = 0.0f;  // 用于透明排序
};

// 渲染统计
struct RenderStats
{
    uint32_t DrawCalls      = 0;
    uint32_t TriangleCount  = 0;
    uint32_t VertexCount    = 0;

    void Reset()
    {
        DrawCalls     = 0;
        TriangleCount = 0;
        VertexCount   = 0;
    }
};
