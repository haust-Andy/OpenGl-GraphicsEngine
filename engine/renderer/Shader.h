#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

// 增强版 Shader - 支持顶点/片段/几何/计算着色器, uniform 缓存
class Shader
{
public:
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader(const std::string& vertexPath, const std::string& fragmentPath,
           const std::string& geometryPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // 从文件加载
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath,
                      const std::string& geometryPath);
    // 从字符串编译
    bool Compile(const std::string& vertexSrc, const std::string& fragmentSrc);

    uint32_t GetRendererID() const { return m_RendererID; }
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    // Uniform 设置 (带位置缓存)
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetBool(const std::string& name, bool value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& mat);
    void SetMat4(const std::string& name, const glm::mat4& mat);

    static std::shared_ptr<Shader> Create(const std::string& vertexPath,
                                          const std::string& fragmentPath);

private:
    int GetUniformLocation(const std::string& name);

    uint32_t CompileShader(GLenum type, const std::string& source);
    std::string ReadFile(const std::string& path);
    void CheckCompileErrors(uint32_t shader, const std::string& type);

    uint32_t m_RendererID = 0;
    std::string m_Name;
    std::unordered_map<std::string, int> m_UniformLocationCache;
};
