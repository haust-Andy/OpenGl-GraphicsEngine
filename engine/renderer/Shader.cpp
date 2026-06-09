#include "Shader.h"

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
        std::cerr << "[Shader] Failed to read shader files: " << vertexPath << ", " << fragmentPath << std::endl;
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
        std::cerr << "[Shader] Failed to read shader files!" << std::endl;
        return false;
    }

    // 编译顶点和片段着色器
    uint32_t vertex   = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    uint32_t geometry = 0;

    if (!geometrySrc.empty())
        geometry = CompileShader(GL_GEOMETRY_SHADER, geometrySrc);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertex);
    glAttachShader(m_RendererID, fragment);
    if (geometry) glAttachShader(m_RendererID, geometry);

    glLinkProgram(m_RendererID);
    CheckCompileErrors(m_RendererID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry) glDeleteShader(geometry);

    return true;
}

bool Shader::Compile(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    uint32_t vertex   = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertex);
    glAttachShader(m_RendererID, fragment);

    glLinkProgram(m_RendererID);
    CheckCompileErrors(m_RendererID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

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

    CheckCompileErrors(shader, typeName);
    return shader;
}

std::string Shader::ReadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[Shader] Cannot open file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader::CheckCompileErrors(uint32_t shader, const std::string& type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Compilation error (" << type << "):\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Linking error:\n" << infoLog << std::endl;
        }
    }
}

int Shader::GetUniformLocation(const std::string& name)
{
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end())
        return it->second;

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}

void Shader::SetInt(const std::string& name, int value)
    { glUniform1i(GetUniformLocation(name), value); }
void Shader::SetFloat(const std::string& name, float value)
    { glUniform1f(GetUniformLocation(name), value); }
void Shader::SetBool(const std::string& name, bool value)
    { glUniform1i(GetUniformLocation(name), (int)value); }
void Shader::SetVec2(const std::string& name, const glm::vec2& value)
    { glUniform2fv(GetUniformLocation(name), 1, &value[0]); }
void Shader::SetVec3(const std::string& name, const glm::vec3& value)
    { glUniform3fv(GetUniformLocation(name), 1, &value[0]); }
void Shader::SetVec4(const std::string& name, const glm::vec4& value)
    { glUniform4fv(GetUniformLocation(name), 1, &value[0]); }
void Shader::SetMat3(const std::string& name, const glm::mat3& mat)
    { glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); }
void Shader::SetMat4(const std::string& name, const glm::mat4& mat)
    { glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); }

std::shared_ptr<Shader> Shader::Create(const std::string& vertexPath,
                                        const std::string& fragmentPath)
{
    return std::make_shared<Shader>(vertexPath, fragmentPath);
}
