#pragma once

#include "Event.h"
#include "Window.h"
#include "Timestep.h"
#include <string>
#include <vector>
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
class LayerStack
{
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
    std::vector<Layer*>::iterator end()   { return m_Layers.end(); }
    std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend()   { return m_Layers.rend(); }

    std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
    std::vector<Layer*>::const_iterator end()   const { return m_Layers.end(); }

private:
    std::vector<Layer*> m_Layers;
    unsigned int m_LayerInsertIndex = 0;   // 普通 Layer 插入位置
};
