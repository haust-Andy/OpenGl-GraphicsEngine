#include "LOD.h"
#include <cmath>
#include <algorithm>

int LODComponent::GetCurrentLOD(const glm::vec3& entityPos, const glm::vec3& cameraPos,
                                  float entityRadius, const glm::mat4& viewProj) const
{
    if (Levels.empty()) return -1;

    float distance = glm::length(entityPos - cameraPos);
    distance = std::max(distance - Bias, 0.0f);

    if (SelectionMode == Mode::Distance)
    {
        // 基于距离选择 LOD
        for (int i = 0; i < (int)Levels.size() - 1; i++)
        {
            if (distance < Levels[i].SwitchDistance)
                return i;
        }
        return (int)Levels.size() - 1;
    }
    else // ScreenSize
    {
        // 基于屏幕占比选择 LOD
        // 将包围球投影到屏幕空间, 计算占比
        glm::vec4 clipPos = viewProj * glm::vec4(entityPos, 1.0f);
        if (clipPos.w <= 0.0f) return (int)Levels.size() - 1;

        float screenRadius = entityRadius / clipPos.w;
        float screenSize = screenRadius * 0.5f;  // 归一化到 0-1

        if (screenSize > ScreenSizeThreshold)
            return 0;
        else if (screenSize > ScreenSizeThreshold * 0.5f)
            return std::min(1, (int)Levels.size() - 1);
        else if (screenSize > ScreenSizeThreshold * 0.25f)
            return std::min(2, (int)Levels.size() - 1);
        else
            return (int)Levels.size() - 1;
    }
}

void LODComponent::AddLevel(const std::shared_ptr<VertexArray>& vao, uint32_t indexCount, float switchDistance)
{
    Levels.push_back({vao, indexCount, switchDistance});
}

std::shared_ptr<VertexArray> LODComponent::GetVAO(int level) const
{
    if (level < 0 || level >= (int)Levels.size()) return nullptr;
    return Levels[level].VAO;
}

uint32_t LODComponent::GetIndexCount(int level) const
{
    if (level < 0 || level >= (int)Levels.size()) return 0;
    return Levels[level].IndexCount;
}
