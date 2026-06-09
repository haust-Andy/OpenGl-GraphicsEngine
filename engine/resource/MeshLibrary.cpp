#include "MeshLibrary.h"
#include <cmath>

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

    // IndexBuffer (顺序索引 0..35)
    {
        uint32_t indices[36];
        for (uint32_t i = 0; i < 36; ++i) indices[i] = i;
        auto ibo = std::make_shared<IndexBuffer>(indices, 36);
        cubeVAO->SetIndexBuffer(ibo);
    }

    s_Cube.VAO = cubeVAO;
    s_Cube.IndexCount = 36;

    // ===== Plane =====
    float planeVertices[] = {
        // positions            // normals         // texcoords
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

    // IndexBuffer (顺序索引 0..5)
    {
        uint32_t indices[] = { 0, 1, 2, 3, 4, 5 };
        auto ibo = std::make_shared<IndexBuffer>(indices, 6);
        planeVAO->SetIndexBuffer(ibo);
    }

    s_Plane.VAO = planeVAO;
    s_Plane.IndexCount = 6;

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

MeshLibrary::Primitive MeshLibrary::GetSphere(uint32_t /*segments*/)
{
    // 简单实现: 使用 icosphere
    // 详细实现略, 此处使用 Cube 作为占位
    return GetCube();
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
