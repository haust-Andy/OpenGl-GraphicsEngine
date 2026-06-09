#include "Test.h"
#include "engine/scene/SceneSerializer.h"
#include "engine/scene/Scene.h"
#include <sstream>
#include <fstream>
#include <cstdio>

// =====================================
//  Test: SceneSerializer (Serialize/Deserialize round-trip)
// =====================================

// ----- 辅助函数：创建简单测试场景 -----
static Ref<Scene> CreateTestScene()
{
    auto scene = CreateRef<Scene>("TestScene");

    // 添加实体
    auto* e1 = scene->CreateEntity("Cube");
    e1->GetTransform().Position = glm::vec3(1.0f, 2.0f, 3.0f);
    e1->GetTransform().Scale    = glm::vec3(2.0f, 2.0f, 2.0f);

    auto* e2 = scene->CreateEntity("Sphere");
    e2->GetTransform().Position = glm::vec3(-1.0f, 0.0f, 1.0f);

    return scene;
}

TEST(SceneSerializer, EmptyScene_Serialize)
{
    auto scene = CreateRef<Scene>("EmptyScene");
    std::string data = SceneSerializer::Serialize(*scene);

    CHECK_FALSE(data.empty());
    CHECK_TRUE(data.find("[Scene]")  != std::string::npos);
    CHECK_TRUE(data.find("EmptyScene") != std::string::npos);
    CHECK_TRUE(data.find("[/Scene]") != std::string::npos);
}

TEST(SceneSerializer, EmptyScene_RoundTrip)
{
    auto original = CreateRef<Scene>("RoundTrip");
    std::string data = SceneSerializer::Serialize(*original);

    auto restored = SceneSerializer::Deserialize(data);
    CHECK_TRUE(restored != nullptr);
    CHECK_STR_EQ(restored->GetName(), "RoundTrip");
}

TEST(SceneSerializer, SceneWithEntities_Serialize)
{
    auto scene = CreateTestScene();
    std::string data = SceneSerializer::Serialize(*scene);

    CHECK_TRUE(data.find("Cube")     != std::string::npos);
    CHECK_TRUE(data.find("Sphere")   != std::string::npos);
    CHECK_TRUE(data.find("[Transform]") != std::string::npos);
    CHECK_TRUE(data.find("[Entity]") != std::string::npos);
    CHECK_TRUE(data.find("[/Entity]") != std::string::npos);
}

TEST(SceneSerializer, SceneWithEntities_RoundTrip)
{
    auto original = CreateTestScene();
    std::string data = SceneSerializer::Serialize(*original);

    auto restored = SceneSerializer::Deserialize(data);
    CHECK_TRUE(restored != nullptr);
    CHECK_STR_EQ(restored->GetName(), "TestScene");

    // 实体数量应一致
    CHECK_EQ(restored->GetEntities().size(), original->GetEntities().size());

    // 检查第一个实体 tag
    CHECK_STR_EQ(restored->GetEntities()[0]->GetTag(), "Cube");
    CHECK_STR_EQ(restored->GetEntities()[1]->GetTag(), "Sphere");
}

TEST(SceneSerializer, TransformRoundTrip)
{
    auto scene = CreateRef<Scene>("TransformTest");
    auto* e = scene->CreateEntity("Player");

    // 设置非默认变换
    e->GetTransform().Position = glm::vec3(10.0f, 20.0f, 30.0f);
    e->GetTransform().SetEulerAngles(glm::vec3(1.57f, 0.0f, 0.0f));  // 90° X
    e->GetTransform().Scale    = glm::vec3(1.0f, 2.0f, 3.0f);

    std::string data = SceneSerializer::Serialize(*scene);
    auto restored = SceneSerializer::Deserialize(data);

    CHECK_TRUE(restored != nullptr);
    CHECK_EQ(restored->GetEntities().size(), 1u);

    auto& rt = restored->GetEntities()[0]->GetTransform();
    CHECK_FLOAT_EQ(rt.Position.x, 10.0f, 0.01f);
    CHECK_FLOAT_EQ(rt.Position.y, 20.0f, 0.01f);
    CHECK_FLOAT_EQ(rt.Position.z, 30.0f, 0.01f);
    CHECK_FLOAT_EQ(rt.Scale.x, 1.0f, 0.01f);
    CHECK_FLOAT_EQ(rt.Scale.y, 2.0f, 0.01f);
    CHECK_FLOAT_EQ(rt.Scale.z, 3.0f, 0.01f);
}

TEST(SceneSerializer, FileSaveAndLoad)
{
    auto original = CreateTestScene();
    std::string testPath = "tests_temp.scene";

    // 保存
    bool saved = SceneSerializer::SaveToFile(*original, testPath);
    CHECK_TRUE(saved);

    // 加载
    auto loaded = SceneSerializer::LoadFromFile(testPath);
    CHECK_TRUE(loaded != nullptr);
    CHECK_STR_EQ(loaded->GetName(), original->GetName());
    CHECK_EQ(loaded->GetEntities().size(), original->GetEntities().size());

    // 清理
    std::remove(testPath.c_str());
}

TEST(SceneSerializer, LoadNonExistentFile)
{
    auto loaded = SceneSerializer::LoadFromFile("nonexistent_12345.scene");
    CHECK_TRUE(loaded == nullptr);
}

TEST(SceneSerializer, EscapeString)
{
    // 通过 Serialize/Deserialize 间接测试转义
    auto scene = CreateRef<Scene>("Special\"Name");
    std::string data = SceneSerializer::Serialize(*scene);

    // 反序列化应还原原始名称
    auto restored = SceneSerializer::Deserialize(data);
    CHECK_TRUE(restored != nullptr);
    CHECK_STR_EQ(restored->GetName(), "Special\"Name");
}

TEST(SceneSerializer, MultiEntityRoundTrip)
{
    auto scene = CreateRef<Scene>("MultiTest");

    for (int i = 0; i < 10; i++) {
        auto* e = scene->CreateEntity("Entity_" + std::to_string(i));
        e->GetTransform().Position = glm::vec3(
            (float)i * 1.5f, (float)i * 0.5f, (float)i * -1.0f);
    }

    std::string data = SceneSerializer::Serialize(*scene);
    auto restored = SceneSerializer::Deserialize(data);

    CHECK_TRUE(restored != nullptr);
    CHECK_EQ(restored->GetEntities().size(), 10u);

    // 验证第 5 个实体
    auto& e4 = restored->GetEntities()[4];
    CHECK_STR_EQ(e4->GetTag(), "Entity_4");
    CHECK_FLOAT_EQ(e4->GetTransform().Position.x, 6.0f, 0.01f);
}
