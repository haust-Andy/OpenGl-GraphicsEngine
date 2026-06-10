#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "renderer/VertexArray.h"
#include "renderer/Material.h"
#include "renderer/Texture.h"

// 子网格 - 一个 Model 的局部 (对应一个材质)
struct SubMesh
{
    std::shared_ptr<VertexArray> VAO;
    uint32_t IndexCount = 0;
    std::shared_ptr<Material> Material;
    glm::mat4 Transform = glm::mat4(1.0f);  // 相对 Model 的局部变换
};

// 3D 模型 - 从文件加载 (Assimp)
class Model
{
public:
    Model() = default;
    ~Model() = default;

    bool LoadFromFile(const std::string& path);

    const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
    const std::string& GetFilePath() const { return m_FilePath; }
    const std::string& GetName() const { return m_Name; }

    // AABB 包围盒
    glm::vec3 GetMinExtents() const { return m_MinExtents; }
    glm::vec3 GetMaxExtents() const { return m_MaxExtents; }

    static std::shared_ptr<Model> Create(const std::string& path);

private:
    void ProcessNode(struct aiNode* node, const struct aiScene* scene,
                      const std::string& directory,
                      const std::shared_ptr<class Shader>& shader);
    SubMesh ProcessMesh(struct aiMesh* mesh, const struct aiScene* scene,
                         const std::string& directory,
                         const std::shared_ptr<class Shader>& shader);

    std::vector<std::shared_ptr<Texture2D>>
    LoadMaterialTextures(struct aiMaterial* mat, unsigned int aiType,
                          const std::string& typeName,
                          const std::string& directory);

    std::string m_FilePath;
    std::string m_Name;
    std::vector<SubMesh> m_SubMeshes;

    glm::vec3 m_MinExtents = glm::vec3(FLT_MAX);
    glm::vec3 m_MaxExtents = glm::vec3(-FLT_MAX);
};
