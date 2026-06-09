#include "SceneSerializer.h"
#include "core/Log.h"
#include "core/Assert.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

// ===== 保存场景到文件 =====
bool SceneSerializer::SaveToFile(const Scene& scene, const std::string& filepath)
{
    std::string data = Serialize(scene);
    if (data.empty())
        return false;

    std::ofstream file(filepath);
    if (!file.is_open())
    {
        CORE_ERROR("SceneSerializer: Cannot open file for writing: ", filepath);
        return false;
    }

    file << data;
    file.close();

    CORE_INFO("SceneSerializer: Saved scene '", scene.GetName(), "' to ", filepath);
    return true;
}

// ===== 从文件加载场景 =====
Ref<Scene> SceneSerializer::LoadFromFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        CORE_ERROR("SceneSerializer: Cannot open file for reading: ", filepath);
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    auto scene = Deserialize(buffer.str());
    if (scene)
        CORE_INFO("SceneSerializer: Loaded scene '", scene->GetName(), "' from ", filepath);

    return scene;
}

// ===== 序列化为字符串 =====
std::string SceneSerializer::Serialize(const Scene& scene)
{
    std::string out;

    // 场景头
    out += "[Scene] name=\"" + EscapeString(scene.GetName()) + "\"\n\n";

    // 光照环境
    const auto& lightEnv = scene.GetLightEnvironment();
    out += "[LightEnvironment]\n";
    out += "  ambientColor=" + std::to_string(lightEnv.GetDirectionalLight().Color.r)
         + "," + std::to_string(lightEnv.GetDirectionalLight().Color.g)
         + "," + std::to_string(lightEnv.GetDirectionalLight().Color.b) + "\n";

    // 方向光
    {
        const auto& dl = lightEnv.GetDirectionalLight();
        out += "  [DirectionalLight] dir="
             + std::to_string(dl.Direction.x) + "," + std::to_string(dl.Direction.y) + "," + std::to_string(dl.Direction.z)
             + " color=" + std::to_string(dl.Color.r) + "," + std::to_string(dl.Color.g) + "," + std::to_string(dl.Color.b)
             + " intensity=" + std::to_string(dl.Intensity) + "\n";
    }
    out += "[/LightEnvironment]\n\n";

    // 实体列表
    for (const auto& entity : scene.GetEntities())
    {
        WriteEntity(out, *entity);
        out += "\n";
    }

    out += "[/Scene]\n";
    return out;
}

// ===== 查找第一个非转义引号 =====
static size_t FindUnescapedQuote(const std::string& s, size_t start)
{
    for (size_t i = start; i < s.size(); i++)
    {
        if (s[i] == '\\' && i + 1 < s.size())
            i++;  // 跳过转义字符
        else if (s[i] == '"')
            return i;
    }
    return std::string::npos;
}

// ===== 从字符串反序列化 =====
Ref<Scene> SceneSerializer::Deserialize(const std::string& data)
{
    Ref<Scene> scene;
    Entity* currentEntity = nullptr;

    std::istringstream stream(data);
    std::string line;

    while (std::getline(stream, line))
    {
        line = Trim(line);
        if (line.empty()) continue;

        // 解析 [Scene] 头部
        if (line.find("[Scene]") == 0 && line.find("[/Scene]") != 0)
        {
            // 提取 name
            auto pos = line.find("name=\"");
            if (pos != std::string::npos)
            {
                auto start = pos + 6;
                auto end = FindUnescapedQuote(line, start);
                std::string name = (end != std::string::npos)
                    ? UnescapeString(line.substr(start, end - start))
                    : "Untitled Scene";
                scene = CreateRef<Scene>(name);
            }
            else
            {
                scene = CreateRef<Scene>("Loaded Scene");
            }
            continue;
        }

        if (!scene) continue;

        // 解析实体
        if (line.find("[Entity]") == 0)
        {
            currentEntity = scene->CreateEntity("Entity");

            // 提取 tag
            auto pos = line.find("tag=\"");
            if (pos != std::string::npos)
            {
                auto start = pos + 5;
                auto end = FindUnescapedQuote(line, start);
                if (end != std::string::npos)
                    currentEntity->SetTag(UnescapeString(line.substr(start, end - start)));
            }
            continue;
        }

        // 结束实体
        if (line == "[/Entity]")
        {
            currentEntity = nullptr;
            continue;
        }

        if (currentEntity)
        {
            ParseEntityLine(*scene, line, currentEntity);
        }
    }

    return scene;
}

// ===== 解析实体属性行 =====
void SceneSerializer::ParseEntityLine(Scene& /*scene*/, const std::string& line, Entity*& currentEntity)
{
    if (!currentEntity) return;

    std::string trimmed = Trim(line);

    // [Transform]
    if (trimmed.find("[Transform]") == 0)
    {
        auto& t = currentEntity->GetTransform();
        auto getVal = [&](const char* key, float def) -> float {
            auto pos = trimmed.find(std::string(key) + "=");
            if (pos != std::string::npos)
            {
                try { return std::stof(trimmed.substr(pos + strlen(key) + 1)); }
                catch (...) {}
            }
            return def;
        };
        t.Position.x = getVal("px", 0.0f);
        t.Position.y = getVal("py", 0.0f);
        t.Position.z = getVal("pz", 0.0f);
        // Rotation stored as euler, restore via quat
        float rx = getVal("rx", 0.0f);
        float ry = getVal("ry", 0.0f);
        float rz = getVal("rz", 0.0f);
        t.SetEulerAngles(glm::vec3(rx, ry, rz));
        t.Scale.x = getVal("sx", 1.0f);
        t.Scale.y = getVal("sy", 1.0f);
        t.Scale.z = getVal("sz", 1.0f);
        return;
    }

    // [MeshRenderer]
    if (trimmed.find("[MeshRenderer]") == 0)
    {
        auto& mesh = currentEntity->GetMesh();
        auto getBool = [&](const char* key, bool def) -> bool {
            auto pos = trimmed.find(std::string(key) + "=");
            if (pos != std::string::npos)
            {
                try { return std::stoi(trimmed.substr(pos + strlen(key) + 1)) != 0; }
                catch (...) {}
            }
            return def;
        };
        mesh.Visible     = getBool("visible", true);
        mesh.CastShadow  = getBool("castShadow", true);
        return;
    }

    // [Light] - 简单版本
    if (trimmed.find("[Light]") == 0)
    {
        LightComponent lc;
        // 默认点光源
        lc.Type = LightType::Point;
        lc.PtLight.Position = currentEntity->GetTransform().Position;

        auto getVal = [&](const char* key, float def) -> float {
            auto pos = trimmed.find(std::string(key) + "=");
            if (pos != std::string::npos)
            {
                try { return std::stof(trimmed.substr(pos + strlen(key) + 1)); }
                catch (...) {}
            }
            return def;
        };

        lc.PtLight.Color.r = getVal("r", 1.0f);
        lc.PtLight.Color.g = getVal("g", 1.0f);
        lc.PtLight.Color.b = getVal("b", 1.0f);
        lc.PtLight.Intensity = getVal("intensity", 1.0f);
        lc.PtLight.Range     = getVal("range", 5.0f);

        currentEntity->SetLight(lc);
        return;
    }
}

// ===== 写入实体 =====
void SceneSerializer::WriteEntity(std::string& out, const Entity& entity)
{
    out += "[Entity] tag=\"" + EscapeString(entity.GetTag())
         + "\" id=" + std::to_string(entity.GetID()) + "\n";

    // Transform
    const auto& t = entity.GetTransform();
    out += "  [Transform] px=" + std::to_string(t.Position.x)
         + " py=" + std::to_string(t.Position.y)
         + " pz=" + std::to_string(t.Position.z);

    glm::vec3 euler = t.GetEulerAngles();
    out += " rx=" + std::to_string(euler.x)
         + " ry=" + std::to_string(euler.y)
         + " rz=" + std::to_string(euler.z)
         + " sx=" + std::to_string(t.Scale.x)
         + " sy=" + std::to_string(t.Scale.y)
         + " sz=" + std::to_string(t.Scale.z) + "\n";

    // Mesh
    const auto& mesh = entity.GetMesh();
    if (mesh.VertexArray)  // 有 mesh 数据的实体
    {
        out += "  [MeshRenderer] visible=" + std::to_string(mesh.Visible ? 1 : 0)
             + " castShadow=" + std::to_string(mesh.CastShadow ? 1 : 0) + "\n";
    }

    // Light
    if (entity.HasLight())
    {
        const auto& lc = entity.GetLight();
        if (lc.Type == LightType::Point)
        {
            out += "  [Light] type=Point r=" + std::to_string(lc.PtLight.Color.r)
                 + " g=" + std::to_string(lc.PtLight.Color.g)
                 + " b=" + std::to_string(lc.PtLight.Color.b)
                 + " intensity=" + std::to_string(lc.PtLight.Intensity)
                 + " range=" + std::to_string(lc.PtLight.Range) + "\n";
        }
    }

    out += "[/Entity]\n";
}

// ===== 字符串工具 =====
std::string SceneSerializer::Trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string SceneSerializer::EscapeString(const std::string& s)
{
    std::string out;
    for (char c : s)
    {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

std::string SceneSerializer::UnescapeString(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            switch (s[i + 1])
            {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 't':  out += '\t'; i++; break;
                default:   out += s[i]; break;
            }
        }
        else
        {
            out += s[i];
        }
    }
    return out;
}
