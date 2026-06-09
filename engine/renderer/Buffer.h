#pragma once

#include <glad/glad.h>

// 顶点缓冲区
class VertexBuffer
{
public:
    VertexBuffer(float* vertices, uint32_t size);
    VertexBuffer(uint32_t size);  // 动态 VBO
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, uint32_t size);

    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID;
};

// 索引缓冲区
class IndexBuffer
{
public:
    IndexBuffer(uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return m_Count; }
    uint32_t GetRendererID() const { return m_RendererID; }

private:
    uint32_t m_RendererID;
    uint32_t m_Count;
};
