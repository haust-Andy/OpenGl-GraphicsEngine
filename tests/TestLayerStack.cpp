#include "Test.h"
#include "engine/core/Layer.h"
#include <string>
#include <vector>

// =====================================
//  Test: Layer & LayerStack
// =====================================

// Mock Layer - 记录调用以验证 LayerStack 行为
class MockLayer : public Layer
{
public:
    MockLayer(const std::string& name) : Layer(name) {}

    bool attached  = false;
    bool detached  = false;
    int  updateCount = 0;

    void OnAttach() override { attached = true; }
    void OnDetach() override { detached = true; }
    void OnUpdate(Timestep ts) override { updateCount++; }
};

TEST(Layer, Constructor_SetsName)
{
    MockLayer layer("TestLayer");
    CHECK_STR_EQ(layer.GetName(), "TestLayer");
}

TEST(Layer, DefaultName_IsLayer)
{
    Layer layer;
    CHECK_STR_EQ(layer.GetName(), "Layer");
}

TEST(LayerStack, EmptyStack)
{
    LayerStack stack;
    CHECK_EQ(std::distance(stack.begin(), stack.end()), 0);
}

TEST(LayerStack, PushLayer_CallsOnAttach)
{
    LayerStack stack;
    auto* layer = new MockLayer("L1");

    stack.PushLayer(layer);
    CHECK_TRUE(layer->attached);
    CHECK_FALSE(layer->detached);
    // ~LayerStack 负责 delete
}

TEST(LayerStack, PopLayer_CallsOnDetach)
{
    LayerStack stack;
    auto* layer = new MockLayer("L1");

    stack.PushLayer(layer);
    stack.PopLayer(layer);

    CHECK_TRUE(layer->detached);
    delete layer;  // PopLayer 已从 stack 移除，安全 delete
}

TEST(LayerStack, PushMultipleLayers_Order)
{
    LayerStack stack;
    auto* l1 = new MockLayer("L1");
    auto* l2 = new MockLayer("L2");

    stack.PushLayer(l1);
    stack.PushLayer(l2);

    // 正向迭代：先入先出（L1 → L2）
    auto it = stack.begin();
    CHECK_TRUE(static_cast<MockLayer*>(*it) == l1); it++;
    CHECK_TRUE(static_cast<MockLayer*>(*it) == l2);
    // ~LayerStack 负责 delete
}

TEST(LayerStack, PushLayerAndOverlay_Order)
{
    LayerStack stack;
    auto* layer   = new MockLayer("Layer");
    auto* overlay = new MockLayer("Overlay");

    stack.PushLayer(layer);
    stack.PushOverlay(overlay);

    // Layer 在前，Overlay 在后
    auto it = stack.begin();
    CHECK_TRUE(static_cast<MockLayer*>(*it) == layer);  it++;
    CHECK_TRUE(static_cast<MockLayer*>(*it) == overlay);
    // ~LayerStack 负责 delete
}

TEST(LayerStack, PushOverlayBeforeLayer_StillAppended)
{
    LayerStack stack;
    auto* overlay = new MockLayer("Overlay");
    auto* layer   = new MockLayer("Layer");

    stack.PushOverlay(overlay);  // 先 push overlay
    stack.PushLayer(layer);       // 再 push layer

    // Layer 应在 Overlay 之前
    auto it = stack.begin();
    CHECK_TRUE(static_cast<MockLayer*>(*it) == layer);   it++;
    CHECK_TRUE(static_cast<MockLayer*>(*it) == overlay);
    // ~LayerStack 负责 delete
}

TEST(LayerStack, PopLayer_ShiftsInsertIndex)
{
    LayerStack stack;
    auto* l1 = new MockLayer("L1");
    auto* l2 = new MockLayer("L2");
    auto* l3 = new MockLayer("L3");

    stack.PushLayer(l1);
    stack.PushLayer(l2);
    stack.PushLayer(l3);

    // 删除中间的 l2
    stack.PopLayer(l2);
    CHECK_TRUE(l2->detached);
    delete l2;  // PopLayer 已从 stack 移除

    // l1 和 l3 仍在 stack 中
    auto it = stack.begin();
    CHECK_TRUE(static_cast<MockLayer*>(*it) == l1); it++;
    CHECK_TRUE(static_cast<MockLayer*>(*it) == l3);
    // ~LayerStack 负责 delete l1, l3
}

TEST(LayerStack, ReverseIteration)
{
    LayerStack stack;
    auto* l1 = new MockLayer("L1");
    auto* l2 = new MockLayer("L2");
    auto* l3 = new MockLayer("L3");

    stack.PushLayer(l1);
    stack.PushLayer(l2);
    stack.PushLayer(l3);

    // 反向迭代：从最后一个到第一个
    auto rit = stack.rbegin();
    CHECK_TRUE(static_cast<MockLayer*>(*rit) == l3); rit++;
    CHECK_TRUE(static_cast<MockLayer*>(*rit) == l2); rit++;
    CHECK_TRUE(static_cast<MockLayer*>(*rit) == l1);
    // ~LayerStack 负责 delete
}

TEST(LayerStack, ConstIteration)
{
    LayerStack stack;
    auto* layer = new MockLayer("ConstTest");
    stack.PushLayer(layer);

    const LayerStack& constStack = stack;
    auto it = constStack.begin();
    CHECK_TRUE(it != constStack.end());
    CHECK_TRUE(*it == layer);
    // ~LayerStack 负责 delete
}
