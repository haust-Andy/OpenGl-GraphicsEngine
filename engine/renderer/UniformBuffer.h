#pragma once

#include <glad/glad.h>
#include <memory>
#include <cstring>

// Uniform Buffer Object (UBO) 封装 — 高效传递批量 uniform 数据
class UniformBuffer
{
public:
    UniformBuffer(uint32_t size, uint32_t bindingPoint)
        : m_Size(size), m_BindingPoint(bindingPoint)
    {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_RendererID, 0, size);
    }

    ~UniformBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void SetData(const void* data, uint32_t size, uint32_t offset = 0)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Bind() const
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID, 0, m_Size);
    }

    uint32_t GetRendererID() const { return m_RendererID; }
    uint32_t GetBindingPoint() const { return m_BindingPoint; }

    static std::shared_ptr<UniformBuffer> Create(uint32_t size, uint32_t bindingPoint)
    {
        return std::make_shared<UniformBuffer>(size, bindingPoint);
    }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Size = 0;
    uint32_t m_BindingPoint = 0;
};
