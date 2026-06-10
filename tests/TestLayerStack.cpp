#include "Test.h"
#include "engine/core/Layer.h"
#include <string>
#include <vector>
#include <memory>

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
    int count = 0;
    for (auto it = stack.begin(); it != stack.end(); ++it)
        count++;
    CHECK_EQ(count, 0);
}

TEST(LayerStack, PushLayer_CallsOnAttach)
{
    LayerStack stack;
    auto layer = std::make_unique<MockLayer>("L1");
    MockLayer* rawPtr = layer.get();

    stack.PushLayer(std::move(layer));
    CHECK_TRUE(rawPtr->attached);
    CHECK_FALSE(rawPtr->detached);
}

TEST(LayerStack, PopLayer_CallsOnDetach)
{
    LayerStack stack;
    auto layer = std::make_unique<MockLayer>("L1");
    MockLayer* rawPtr = layer.get();

    stack.PushLayer(std::move(layer));
    stack.PopLayer(rawPtr);

    CHECK_TRUE(rawPtr->detached);
}

TEST(LayerStack, PushMultipleLayers_Order)
{
    LayerStack stack;
    auto l1 = std::make_unique<MockLayer>("L1");
    auto l2 = std::make_unique<MockLayer>("L2");
    MockLayer* rawL1 = l1.get();
    MockLayer* rawL2 = l2.get();

    stack.PushLayer(std::move(l1));
    stack.PushLayer(std::move(l2));

    // 正向迭代：先入先出（L1 → L2）
    auto it = stack.begin();
    CHECK_TRUE(*it == rawL1); ++it;
    CHECK_TRUE(*it == rawL2);
}

TEST(LayerStack, PushLayerAndOverlay_Order)
{
    LayerStack stack;
    auto layer   = std::make_unique<MockLayer>("Layer");
    auto overlay = std::make_unique<MockLayer>("Overlay");
    MockLayer* rawLayer = layer.get();
    MockLayer* rawOverlay = overlay.get();

    stack.PushLayer(std::move(layer));
    stack.PushOverlay(std::move(overlay));

    // Layer 在前，Overlay 在后
    auto it = stack.begin();
    CHECK_TRUE(*it == rawLayer);  ++it;
    CHECK_TRUE(*it == rawOverlay);
}

TEST(LayerStack, PushOverlayBeforeLayer_StillAppended)
{
    LayerStack stack;
    auto overlay = std::make_unique<MockLayer>("Overlay");
    auto layer   = std::make_unique<MockLayer>("Layer");
    MockLayer* rawOverlay = overlay.get();
    MockLayer* rawLayer = layer.get();

    stack.PushOverlay(std::move(overlay));  // 先 push overlay
    stack.PushLayer(std::move(layer));       // 再 push layer

    // Layer 应在 Overlay 之前
    auto it = stack.begin();
    CHECK_TRUE(*it == rawLayer);   ++it;
    CHECK_TRUE(*it == rawOverlay);
}

TEST(LayerStack, PopLayer_ShiftsInsertIndex)
{
    LayerStack stack;
    auto l1 = std::make_unique<MockLayer>("L1");
    auto l2 = std::make_unique<MockLayer>("L2");
    auto l3 = std::make_unique<MockLayer>("L3");
    MockLayer* rawL1 = l1.get();
    MockLayer* rawL2 = l2.get();
    MockLayer* rawL3 = l3.get();

    stack.PushLayer(std::move(l1));
    stack.PushLayer(std::move(l2));
    stack.PushLayer(std::move(l3));

    // 删除中间的 l2
    stack.PopLayer(rawL2);
    CHECK_TRUE(rawL2->detached);

    // l1 和 l3 仍在 stack 中
    auto it = stack.begin();
    CHECK_TRUE(*it == rawL1); ++it;
    CHECK_TRUE(*it == rawL3);
}

TEST(LayerStack, ReverseIteration)
{
    LayerStack stack;
    auto l1 = std::make_unique<MockLayer>("L1");
    auto l2 = std::make_unique<MockLayer>("L2");
    auto l3 = std::make_unique<MockLayer>("L3");
    MockLayer* rawL1 = l1.get();
    MockLayer* rawL2 = l2.get();
    MockLayer* rawL3 = l3.get();

    stack.PushLayer(std::move(l1));
    stack.PushLayer(std::move(l2));
    stack.PushLayer(std::move(l3));

    // 反向迭代：从最后一个到第一个
    auto rit = stack.rbegin();
    CHECK_TRUE(*rit == rawL3); ++rit;
    CHECK_TRUE(*rit == rawL2); ++rit;
    CHECK_TRUE(*rit == rawL1);
}

TEST(LayerStack, ConstIteration)
{
    LayerStack stack;
    auto layer = std::make_unique<MockLayer>("ConstTest");
    MockLayer* rawLayer = layer.get();
    stack.PushLayer(std::move(layer));

    const LayerStack& constStack = stack;
    auto it = constStack.begin();
    CHECK_TRUE(it != constStack.end());
    CHECK_TRUE(*it == rawLayer);
}
