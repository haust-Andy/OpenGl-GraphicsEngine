#include "Layer.h"

Layer::Layer(const std::string& name)
    : m_DebugName(name) {}

LayerStack::~LayerStack()
{
    for (auto& layer : m_Layers)
    {
        layer->OnDetach();
        // unique_ptr 自动释放，无需手动 delete
    }
    m_Layers.clear();
}

void LayerStack::PushLayer(std::unique_ptr<Layer> layer)
{
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
    m_LayerInsertIndex++;
    m_Layers[m_LayerInsertIndex - 1]->OnAttach();
}

void LayerStack::PushOverlay(std::unique_ptr<Layer> overlay)
{
    m_Layers.emplace_back(std::move(overlay));
    m_Layers.back()->OnAttach();
}

void LayerStack::PopLayer(Layer* layer)
{
    auto it = std::find_if(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex,
        [layer](const std::unique_ptr<Layer>& l) { return l.get() == layer; });
    if (it != m_Layers.begin() + m_LayerInsertIndex)
    {
        (*it)->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer* overlay)
{
    auto it = std::find_if(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(),
        [overlay](const std::unique_ptr<Layer>& l) { return l.get() == overlay; });
    if (it != m_Layers.end())
    {
        (*it)->OnDetach();
        m_Layers.erase(it);
    }
}
