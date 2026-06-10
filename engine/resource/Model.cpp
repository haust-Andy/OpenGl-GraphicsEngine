#include "Model.h"
#include "ShaderLibrary.h"
#include "TextureLibrary.h"
#include "core/Log.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>

bool Model::LoadFromFile(const std::string& path)
{
    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate |
                          aiProcess_GenSmoothNormals |
                          aiProcess_CalcTangentSpace |
                          aiProcess_JoinIdenticalVertices |
                          aiProcess_FlipUVs;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        CORE_ERROR("[Model] Assimp error: ", importer.GetErrorString());
        return false;
    }

    m_FilePath = path;

    // 从路径提取名称
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of('.');
    m_Name = path.substr(lastSlash + 1, lastDot - lastSlash - 1);

    // 提取目录 (用于相对纹理路径)
    std::string directory = path.substr(0, lastSlash + 1);

    // 获取默认 PBR Shader
    auto shader = ShaderLibrary::Instance().Get("pbr");
    if (!shader)
        shader = Shader::Create("shader/pbr.vert", "shader/pbr.frag");

    ProcessNode(scene->mRootNode, scene, directory, shader);

    CORE_INFO("[Model] Loaded: ", m_Name, " (", m_SubMeshes.size(), " submeshes)");
    return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene,
                          const std::string& directory,
                          const std::shared_ptr<Shader>& shader)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_SubMeshes.push_back(ProcessMesh(mesh, scene, directory, shader));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene, directory, shader);
    }
}

SubMesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene,
                              const std::string& directory,
                              const std::shared_ptr<Shader>& shader)
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // 位置(3) + 法线(3) + UV(2) + 切线(3) + 副切线(3) = 14 floats
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        // Position
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Normal
        if (mesh->HasNormals())
        {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }
        else
        {
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
        }

        // TexCoords
        if (mesh->mTextureCoords[0])
        {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertices.push_back(0.0f); vertices.push_back(0.0f);
        }

        // Tangent + Bitangent
        if (mesh->HasTangentsAndBitangents())
        {
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);
            vertices.push_back(mesh->mBitangents[i].x);
            vertices.push_back(mesh->mBitangents[i].y);
            vertices.push_back(mesh->mBitangents[i].z);
        }
        else
        {
            vertices.insert(vertices.end(), 6, 0.0f);
        }

        // 更新 AABB
        m_MinExtents.x = std::min(m_MinExtents.x, mesh->mVertices[i].x);
        m_MinExtents.y = std::min(m_MinExtents.y, mesh->mVertices[i].y);
        m_MinExtents.z = std::min(m_MinExtents.z, mesh->mVertices[i].z);
        m_MaxExtents.x = std::max(m_MaxExtents.x, mesh->mVertices[i].x);
        m_MaxExtents.y = std::max(m_MaxExtents.y, mesh->mVertices[i].y);
        m_MaxExtents.z = std::max(m_MaxExtents.z, mesh->mVertices[i].z);
    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // 创建 VAO
    auto vbo = std::make_shared<VertexBuffer>(vertices.data(), (uint32_t)(vertices.size() * sizeof(float)));
    auto ibo = std::make_shared<IndexBuffer>(indices.data(), (uint32_t)indices.size());
    auto vao = std::make_shared<VertexArray>();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);

    // 设置顶点属性: pos(3) + normal(3) + uv(2) + tangent(3) + bitangent(3) = 14 floats
    // TODO(code-review): 绕过 VertexArray 抽象层直接调用 OpenGL API，应在 VertexArray 中提供标准化 PBR 布局方法 (M-11)
    glBindVertexArray(vao->GetRendererID());
    // location 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    // location 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    // location 2: TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    // location 3: Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    // location 4: Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glBindVertexArray(0);

    // 材质
    auto material = Material::Create(shader);
    if (mesh->mMaterialIndex < scene->mNumMaterials)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        // Albedo
        auto albedoMaps = LoadMaterialTextures(mat, aiTextureType_DIFFUSE, "albedo", directory);
        if (!albedoMaps.empty()) material->GetProperties().AlbedoMap = albedoMaps[0];

        // Normal
        auto normalMaps = LoadMaterialTextures(mat, aiTextureType_NORMALS, "normal", directory);
        if (!normalMaps.empty()) material->GetProperties().NormalMap = normalMaps[0];

        // Metallic
        auto metallicMaps = LoadMaterialTextures(mat, aiTextureType_METALNESS, "metallic", directory);
        if (!metallicMaps.empty()) material->GetProperties().MetallicMap = metallicMaps[0];

        // Roughness
        auto roughnessMaps = LoadMaterialTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", directory);
        if (!roughnessMaps.empty()) material->GetProperties().RoughnessMap = roughnessMaps[0];

        // AO
        auto aoMaps = LoadMaterialTextures(mat, aiTextureType_AMBIENT_OCCLUSION, "ao", directory);
        if (!aoMaps.empty()) material->GetProperties().AOMap = aoMaps[0];

        // Emissive
        auto emissiveMaps = LoadMaterialTextures(mat, aiTextureType_EMISSIVE, "emissive", directory);
        if (!emissiveMaps.empty()) material->GetProperties().EmissiveMap = emissiveMaps[0];

        // 从 Assimp 读取 PBR 属性
        aiColor3D color;
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            material->GetProperties().Albedo = glm::vec3(color.r, color.g, color.b);

        float val;
        if (mat->Get(AI_MATKEY_METALLIC_FACTOR, val) == AI_SUCCESS)
            material->GetProperties().Metallic = val;
        if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, val) == AI_SUCCESS)
            material->GetProperties().Roughness = val;
    }

    SubMesh subMesh;
    subMesh.VAO = vao;
    subMesh.IndexCount = (uint32_t)indices.size();
    subMesh.Material = material;
    return subMesh;
}

std::vector<std::shared_ptr<Texture2D>>
Model::LoadMaterialTextures(aiMaterial* mat, unsigned int aiType,
                              const std::string& /*typeName*/,
                              const std::string& directory)
{
    std::vector<std::shared_ptr<Texture2D>> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount((aiTextureType)aiType); ++i)
    {
        aiString str;
        mat->GetTexture((aiTextureType)aiType, i, &str);
        std::string fullPath = directory + str.C_Str();

        // 幂等加载
        if (TextureLibrary::Instance().Exists(fullPath))
        {
            textures.push_back(TextureLibrary::Instance().Get(fullPath));
        }
        else
        {
            auto tex = TextureLibrary::Instance().Load(fullPath);
            if (tex) textures.push_back(tex);
        }
    }

    return textures;
}

std::shared_ptr<Model> Model::Create(const std::string& path)
{
    auto model = std::make_shared<Model>();
    if (model->LoadFromFile(path))
        return model;
    return nullptr;
}
