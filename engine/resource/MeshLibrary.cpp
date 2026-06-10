#include "MeshLibrary.h"
#include <cmath>
#include <unordered_map>
#include <glm/gtc/constants.hpp>

bool MeshLibrary::s_Initialized = false;
MeshLibrary::Primitive MeshLibrary::s_Cube;
MeshLibrary::Primitive MeshLibrary::s_Plane;
MeshLibrary::Primitive MeshLibrary::s_Quad;

void MeshLibrary::InitDefaults()
{
    if (s_Initialized) return;

    // ===== Cube =====
    // position(3) + normal(3) + texcoord(2) = 8 floats per vertex
    float cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    };

    auto cubeVBO = std::make_shared<VertexBuffer>(cubeVertices, sizeof(cubeVertices));
    auto cubeVAO = std::make_shared<VertexArray>();
    cubeVAO->AddVertexBuffer(cubeVBO);

    glBindVertexArray(cubeVAO->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    uint32_t indices[36];
    for (uint32_t i = 0; i < 36; ++i) indices[i] = i;
    auto ibo = std::make_shared<IndexBuffer>(indices, 36);
    cubeVAO->SetIndexBuffer(ibo);

    s_Cube.VAO = cubeVAO;
    s_Cube.IndexCount = 36;

    // ===== Plane =====
    float planeVertices[] = {
         5.0f, -0.5f,  5.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.0f,
         5.0f, -0.5f,  5.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,   2.0f, 2.0f,
    };

    auto planeVBO = std::make_shared<VertexBuffer>(planeVertices, sizeof(planeVertices));
    auto planeVAO = std::make_shared<VertexArray>();
    planeVAO->AddVertexBuffer(planeVBO);

    glBindVertexArray(planeVAO->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    uint32_t planeIndices[] = { 0, 1, 2, 3, 4, 5 };
    auto planeIBO = std::make_shared<IndexBuffer>(planeIndices, 6);
    planeVAO->SetIndexBuffer(planeIBO);

    s_Plane.VAO = planeVAO;
    s_Plane.IndexCount = 6;

    // ===== Quad (屏幕四边形, position+texcoord only) =====
    float quadVertices[] = {
        // positions       // texcoords
        -1.0f,  1.0f,      0.0f, 1.0f,
        -1.0f, -1.0f,      0.0f, 0.0f,
         1.0f, -1.0f,      1.0f, 0.0f,
         1.0f,  1.0f,      1.0f, 1.0f,
    };
    uint32_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

    auto quadVBO = std::make_shared<VertexBuffer>(quadVertices, sizeof(quadVertices));
    auto quadVAO = std::make_shared<VertexArray>();
    quadVAO->AddVertexBuffer(quadVBO);
    auto quadIBO = std::make_shared<IndexBuffer>(quadIndices, 6);
    quadVAO->SetIndexBuffer(quadIBO);

    glBindVertexArray(quadVAO->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    s_Quad.VAO = quadVAO;
    s_Quad.IndexCount = 6;

    s_Initialized = true;
}

MeshLibrary::Primitive MeshLibrary::GetCube()
{
    InitDefaults();
    return s_Cube;
}

MeshLibrary::Primitive MeshLibrary::GetPlane()
{
    InitDefaults();
    return s_Plane;
}

MeshLibrary::Primitive MeshLibrary::GetSphere(uint32_t segments)
{
    InitDefaults();

    // === Icosphere 实现 ===
    static std::unordered_map<uint32_t, Primitive> s_SphereCache;
    auto it = s_SphereCache.find(segments);
    if (it != s_SphereCache.end())
        return it->second;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 初始二十面体 (12个顶点, 20个面)
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    auto addVertex = [&](float x, float y, float z) -> uint32_t {
        glm::vec3 p = glm::normalize(glm::vec3(x, y, z));
        vertices.push_back({ p, p, glm::vec2(std::atan2(p.z, p.x) / (2.0f * glm::pi<float>()) + 0.5f,
                                              std::asin(p.y) / glm::pi<float>() + 0.5f) });
        return (uint32_t)vertices.size() - 1;
    };

    addVertex(-1,  t,  0);
    addVertex( 1,  t,  0);
    addVertex(-1, -t,  0);
    addVertex( 1, -t,  0);
    addVertex( 0, -1,  t);
    addVertex( 0,  1,  t);
    addVertex( 0, -1, -t);
    addVertex( 0,  1, -t);
    addVertex( t,  0, -1);
    addVertex( t,  0,  1);
    addVertex(-t,  0, -1);
    addVertex(-t,  0,  1);

    // 二十面体三角形索引
    uint32_t icoIndices[] = {
        0, 11, 5,  0,  5, 1,  0,  1, 7,  0, 7, 10,  0, 10, 11,
        1,  5, 9,  5, 11, 4,  11, 10, 2,  10, 7, 6,  7,  1, 8,
        3,  9, 4,  3,  4, 2,  3,  2, 6,  3,  6, 8,  3,  8, 9,
        4,  9, 5,  2,  4,11,  6,  2,10,  8,  6, 7,  9,  8, 1,
    };

    for (int i = 0; i < 60; i++) indices.push_back(icoIndices[i]);

    // 细分
    std::unordered_map<uint64_t, uint32_t> midPointCache;
    auto getMidPoint = [&](uint32_t p1, uint32_t p2) -> uint32_t {
        uint64_t key = (uint64_t)std::min(p1, p2) << 32 | std::max(p1, p2);
        auto it = midPointCache.find(key);
        if (it != midPointCache.end()) return it->second;

        glm::vec3 mid = glm::normalize(glm::vec3(
            (vertices[p1].Position.x + vertices[p2].Position.x) * 0.5f,
            (vertices[p1].Position.y + vertices[p2].Position.y) * 0.5f,
            (vertices[p1].Position.z + vertices[p2].Position.z) * 0.5f));

        uint32_t idx = (uint32_t)vertices.size();
        vertices.push_back({ mid, mid, glm::vec2(
            std::atan2(mid.z, mid.x) / (2.0f * glm::pi<float>()) + 0.5f,
            std::asin(mid.y) / glm::pi<float>() + 0.5f) });
        midPointCache[key] = idx;
        return idx;
    };

    for (uint32_t s = 0; s < segments; s++)
    {
        std::vector<uint32_t> newIndices;
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t a = indices[i], b = indices[i + 1], c = indices[i + 2];
            uint32_t ab = getMidPoint(a, b);
            uint32_t bc = getMidPoint(b, c);
            uint32_t ca = getMidPoint(c, a);

            newIndices.insert(newIndices.end(), { a, ab, ca });
            newIndices.insert(newIndices.end(), { b, bc, ab });
            newIndices.insert(newIndices.end(), { c, ca, bc });
            newIndices.insert(newIndices.end(), { ab, bc, ca });
        }
        indices = std::move(newIndices);
    }

    // 构建缓冲区
    std::vector<float> rawData;
    for (auto& v : vertices) {
        rawData.insert(rawData.end(), { v.Position.x, v.Position.y, v.Position.z,
                                        v.Normal.x, v.Normal.y, v.Normal.z,
                                        v.TexCoords.x, v.TexCoords.y });
    }

    auto vbo = std::make_shared<VertexBuffer>(rawData.data(), (uint32_t)(rawData.size() * sizeof(float)));
    auto vao = std::make_shared<VertexArray>();
    vao->AddVertexBuffer(vbo);
    auto ibo = std::make_shared<IndexBuffer>(indices.data(), (uint32_t)indices.size());
    vao->SetIndexBuffer(ibo);

    glBindVertexArray(vao->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    Primitive result = { vao, (uint32_t)indices.size() };
    s_SphereCache[segments] = result;
    return result;
}

MeshLibrary::Primitive MeshLibrary::GetQuad()
{
    return s_Quad;
}

void MeshLibrary::Register(const std::string& /*name*/, const Primitive& /*primitive*/)
{
    // 存储到静态表
}

MeshLibrary::Primitive MeshLibrary::Get(const std::string& /*name*/)
{
    return GetCube();
}
