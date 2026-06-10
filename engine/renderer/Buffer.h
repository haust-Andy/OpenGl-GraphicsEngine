#pragma once

#include <glad/glad.h>
#include <utility>

// 顶点缓冲区
class VertexBuffer
{
public:
    VertexBuffer(float* vertices, uint32_t size);
    VertexBuffer(uint32_t size);  // 动态 VBO
    ~VertexBuffer();

    // 禁止拷贝（防止 double-free GL 资源）
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // 移动语义
    VertexBuffer(VertexBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID)
    {
        other.m_RendererID = 0;
    }
    VertexBuffer& operator=(VertexBuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (m_RendererID) glDeleteBuffers(1, &m_RendererID);
            m_RendererID = other.m_RendererID;
            other.m_RendererID = 0;
        }
        return *this;
    }

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, uint32_t size);

    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
};

// 索引缓冲区
class IndexBuffer
{
public:
    IndexBuffer(uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    // 禁止拷贝（防止 double-free GL 资源）
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // 移动语义
    IndexBuffer(IndexBuffer&& other) noexcept
        : m_RendererID(other.m_RendererID), m_Count(other.m_Count)
    {
        other.m_RendererID = 0;
        other.m_Count = 0;
    }
    IndexBuffer& operator=(IndexBuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (m_RendererID) glDeleteBuffers(1, &m_RendererID);
            m_RendererID = other.m_RendererID;
            m_Count = other.m_Count;
            other.m_RendererID = 0;
            other.m_Count = 0;
        }
        return *this;
    }

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return m_Count; }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Count = 0;
};
