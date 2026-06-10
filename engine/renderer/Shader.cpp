#include "Shader.h"
#include "core/Log.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    LoadFromFile(vertexPath, fragmentPath);
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath,
               const std::string& geometryPath)
{
    LoadFromFile(vertexPath, fragmentPath, geometryPath);
}

Shader::~Shader()
{
    if (m_RendererID)
        glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexSrc   = ReadFile(vertexPath);
    std::string fragmentSrc = ReadFile(fragmentPath);

    if (vertexSrc.empty() || fragmentSrc.empty())
    {
        CORE_ERROR("[Shader] Failed to read shader files: ", vertexPath, ", ", fragmentPath);
        return false;
    }

    return Compile(vertexSrc, fragmentSrc);
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath,
                          const std::string& geometryPath)
{
    std::string vertexSrc   = ReadFile(vertexPath);
    std::string fragmentSrc = ReadFile(fragmentPath);
    std::string geometrySrc = ReadFile(geometryPath);

    if (vertexSrc.empty() || fragmentSrc.empty())
    {
        CORE_ERROR("[Shader] Failed to read shader files: ", vertexPath, ", ", fragmentPath);
        return false;
    }

    // 先编译基础顶点+片段着色器
    if (!Compile(vertexSrc, fragmentSrc))
        return false;

    // 如果有几何着色器，附加并重新链接
    if (!geometrySrc.empty())
    {
        uint32_t geometry = CompileShader(GL_GEOMETRY_SHADER, geometrySrc);
        if (!geometry) return false;

        glAttachShader(m_RendererID, geometry);
        glLinkProgram(m_RendererID);

        bool linkOK = CheckCompileErrors(m_RendererID, "PROGRAM");
        glDeleteShader(geometry);

        if (!linkOK)
        {
            glDeleteProgram(m_RendererID);
            m_RendererID = 0;
            return false;
        }
    }

    return true;
}

bool Shader::Compile(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    uint32_t vertex   = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    if (!vertex || !fragment)
    {
        if (vertex)   glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertex);
    glAttachShader(m_RendererID, fragment);

    glLinkProgram(m_RendererID);
    bool linkOK = CheckCompileErrors(m_RendererID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!linkOK)
    {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
        return false;
    }

    return true;
}

uint32_t Shader::CompileShader(GLenum type, const std::string& source)
{
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    std::string typeName;
    switch (type)
    {
    case GL_VERTEX_SHADER:   typeName = "VERTEX";   break;
    case GL_FRAGMENT_SHADER: typeName = "FRAGMENT"; break;
    case GL_GEOMETRY_SHADER: typeName = "GEOMETRY"; break;
#ifndef __APPLE__
    case GL_COMPUTE_SHADER:  typeName = "COMPUTE";  break;
#endif
    default: typeName = "UNKNOWN"; break;
    }

    if (!CheckCompileErrors(shader, typeName))
    {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

std::string Shader::ReadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        CORE_ERROR("[Shader] Cannot open file: ", path);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Shader::CheckCompileErrors(uint32_t shader, const std::string& type)
{
    int success;
    char infoLog[1024];

    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            CORE_ERROR("[Shader] Compilation error (", type, "):\n", infoLog);
            return false;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            CORE_ERROR("[Shader] Linking error:\n", infoLog);
            return false;
        }
    }
    return true;
}

int Shader::GetUniformLocation(const std::string& name)
{
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end())
        return it->second;

    int location = glGetUniformLocation(m_RendererID, name.c_str());

    // 缓存 -1 会导致每帧对无效 uniform 调用 glUniform*，浪费 CPU
    // 改为：-1 不缓存，直接返回（后续 Set* 调用会安全地 no-op）
    if (location != -1)
        m_UniformLocationCache[name] = location;
    else
        CORE_WARN("[Shader] Uniform '", name, "' not found (shader ID: ", m_RendererID, ")");

    return location;
}

void Shader::SetInt(const std::string& name, int value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform1i(loc, value); }
void Shader::SetFloat(const std::string& name, float value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform1f(loc, value); }
void Shader::SetBool(const std::string& name, bool value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform1i(loc, (int)value); }
void Shader::SetVec2(const std::string& name, const glm::vec2& value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform2fv(loc, 1, &value[0]); }
void Shader::SetVec3(const std::string& name, const glm::vec3& value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform3fv(loc, 1, &value[0]); }
void Shader::SetVec4(const std::string& name, const glm::vec4& value)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniform4fv(loc, 1, &value[0]); }
void Shader::SetMat3(const std::string& name, const glm::mat3& mat)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat)); }
void Shader::SetMat4(const std::string& name, const glm::mat4& mat)
    { int loc = GetUniformLocation(name); if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat)); }

std::shared_ptr<Shader> Shader::Create(const std::string& vertexPath,
                                        const std::string& fragmentPath)
{
    return std::make_shared<Shader>(vertexPath, fragmentPath);
}
