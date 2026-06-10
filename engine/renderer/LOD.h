#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "renderer/VertexArray.h"

// LOD 组件 - 多级细节层次
class LODComponent
{
public:
    static constexpr int MAX_LOD_LEVELS = 4;

    struct LODLevel
    {
        std::shared_ptr<VertexArray> VAO;
        uint32_t IndexCount = 0;
        float SwitchDistance = 10.0f;  // 切换距离
    };

    std::vector<LODLevel> Levels;

    // LOD 模式
    enum class Mode
    {
        Distance,       // 基于距离
        ScreenSize      // 基于屏幕占比
    };
    Mode SelectionMode = Mode::Distance;

    // 屏幕占比阈值 (当 ScreenSize 模式时使用)
    float ScreenSizeThreshold = 0.05f;  // 屏幕占比 < 5% 时降级

    // LOD 偏移 (0 = 正常, 正值 = 更早降级, 负值 = 更晚降级)
    float Bias = 0.0f;

    // 获取当前应使用的 LOD 级别
    int GetCurrentLOD(const glm::vec3& entityPos, const glm::vec3& cameraPos,
                       float entityRadius = 1.0f, const glm::mat4& viewProj = glm::mat4(1.0f)) const;

    // 便利方法: 添加 LOD 级别
    void AddLevel(const std::shared_ptr<VertexArray>& vao, uint32_t indexCount, float switchDistance);

    // 获取指定 LOD 级别的 VAO
    std::shared_ptr<VertexArray> GetVAO(int level) const;
    uint32_t GetIndexCount(int level) const;

    bool IsEmpty() const { return Levels.empty(); }
    int GetLevelCount() const { return (int)Levels.size(); }
};
