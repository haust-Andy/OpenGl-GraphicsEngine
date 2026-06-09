#pragma once

#include "renderer/VertexArray.h"
#include "renderer/Buffer.h"
#include "renderer/Shader.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

// 静态网格几何体库 - 提供常用基本几何体 (Cube, Sphere, Plane, Quad 等)
class MeshLibrary
{
public:
    struct Primitive
    {
        std::shared_ptr<VertexArray> VAO;
        uint32_t IndexCount = 0;
    };

    // 获取预定义几何体
    static Primitive GetCube();
    static Primitive GetPlane();
    static Primitive GetSphere(uint32_t segments = 32);
    static Primitive GetQuad();

    // 自定义 Mesh 注册
    static void Register(const std::string& name, const Primitive& primitive);
    static Primitive Get(const std::string& name);

private:
    static void InitDefaults();

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    static bool s_Initialized;
    static Primitive s_Cube;
    static Primitive s_Plane;
    static Primitive s_Quad;
};
