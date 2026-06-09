#pragma once

#include "Scene.h"
#include <string>
#include <memory>

// ===== 场景序列化器 =====
// 将场景保存/加载为 .scene 文本文件（JSON-like 格式）
// 
// 使用方式:
//   SceneSerializer::SaveToFile(scene, "resources/scenes/demo.scene");
//   auto scene = SceneSerializer::LoadFromFile("resources/scenes/demo.scene");
//
// 文件格式为自定义轻量标记，无外部依赖:
//   [Scene] name="Demo"
//   [Entity] tag="Cube" id=1
//     [Transform] px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
//     [MeshRenderer] visible=1 castShadow=1
//     [Light] type=Point ...
//   [/Entity]

class SceneSerializer
{
public:
    // 保存场景到文件
    static bool SaveToFile(const Scene& scene, const std::string& filepath);

    // 从文件加载场景
    static Ref<Scene> LoadFromFile(const std::string& filepath);

    // 序列化为字符串 (可用于网络传输等)
    static std::string Serialize(const Scene& scene);

    // 从字符串反序列化
    static Ref<Scene> Deserialize(const std::string& data);

private:
    // 解析辅助
    static void ParseEntityLine(Scene& scene, const std::string& line,
                                Entity*& currentEntity);
    static void WriteEntity(std::string& out, const Entity& entity);
    static std::string Trim(const std::string& s);
    static std::string EscapeString(const std::string& s);
    static std::string UnescapeString(const std::string& s);
};
