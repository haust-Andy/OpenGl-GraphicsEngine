#pragma once

#include "Event.h"
#include "Window.h"
#include "Timestep.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

// 引擎分层架构 - 每一层代表一个独立的功能模块
// 例如: 游戏逻辑层, 编辑器UI层, 调试层

class Layer
{
public:
    Layer(const std::string& name = "Layer");
    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(Timestep /*ts*/) {}
    virtual void OnEvent(Event& /*event*/) {}
    virtual void OnImGuiRender() {}

    const std::string& GetName() const { return m_DebugName; }

protected:
    std::string m_DebugName;
};

// LayerStack - 管理 Layer 的栈结构
// 后加入的 Layer 先处理事件、后渲染 (Overlay 模式)
// 使用 unique_ptr 明确所有权，防止 double-free
class LayerStack
{
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(std::unique_ptr<Layer> layer);
    void PushOverlay(std::unique_ptr<Layer> overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    // 迭代器支持 (返回裸指针供外部使用，不转移所有权)
    // 注意: 外部不得 delete 返回的指针
    class Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Layer*;
        using difference_type = std::ptrdiff_t;
        using pointer = Layer**;
        using reference = Layer*&;

        Iterator(std::vector<std::unique_ptr<Layer>>::iterator it) : m_It(it) {}
        Layer* operator*() { return m_It->get(); }
        Iterator& operator++() { ++m_It; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++m_It; return tmp; }
        Iterator& operator--() { --m_It; return *this; }
        bool operator!=(const Iterator& other) const { return m_It != other.m_It; }
        bool operator==(const Iterator& other) const { return m_It == other.m_It; }
    private:
        std::vector<std::unique_ptr<Layer>>::iterator m_It;
    };

    class ConstIterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const Layer*;
        using difference_type = std::ptrdiff_t;
        using pointer = const Layer**;
        using reference = const Layer*&;

        ConstIterator(std::vector<std::unique_ptr<Layer>>::const_iterator it) : m_It(it) {}
        const Layer* operator*() const { return m_It->get(); }
        ConstIterator& operator++() { ++m_It; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++m_It; return tmp; }
        ConstIterator& operator--() { --m_It; return *this; }
        bool operator!=(const ConstIterator& other) const { return m_It != other.m_It; }
        bool operator==(const ConstIterator& other) const { return m_It == other.m_It; }
    private:
        std::vector<std::unique_ptr<Layer>>::const_iterator m_It;
    };

    class ReverseIterator
    {
    public:
        ReverseIterator(std::vector<std::unique_ptr<Layer>>::reverse_iterator it) : m_It(it) {}
        Layer* operator*() { return m_It->get(); }
        ReverseIterator& operator++() { ++m_It; return *this; }
        ReverseIterator operator++(int) { ReverseIterator tmp = *this; ++m_It; return tmp; }
        bool operator!=(const ReverseIterator& other) const { return m_It != other.m_It; }
    private:
        std::vector<std::unique_ptr<Layer>>::reverse_iterator m_It;
    };

    Iterator begin() { return Iterator(m_Layers.begin()); }
    Iterator end()   { return Iterator(m_Layers.end()); }
    ConstIterator begin() const { return ConstIterator(m_Layers.begin()); }
    ConstIterator end() const   { return ConstIterator(m_Layers.end()); }
    ReverseIterator rbegin() { return ReverseIterator(m_Layers.rbegin()); }
    ReverseIterator rend()   { return ReverseIterator(m_Layers.rend()); }

private:
    std::vector<std::unique_ptr<Layer>> m_Layers;
    unsigned int m_LayerInsertIndex = 0;   // 普通 Layer 插入位置
};
