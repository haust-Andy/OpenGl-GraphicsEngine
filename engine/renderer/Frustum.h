#pragma once

#include <glm/glm.hpp>
#include <vector>

// 视锥体 - 用于视锥体剔除
class Frustum
{
public:
    // 从视图投影矩阵提取 6 个平面
    void ExtractFromMatrix(const glm::mat4& viewProjection);

    // 检测点是否在视锥体内
    bool ContainsPoint(const glm::vec3& point) const;

    // 检测 AABB 是否与视锥体相交
    bool IntersectsAABB(const struct AABB& aabb) const;

    // 检测球体是否与视锥体相交
    bool IntersectsSphere(const glm::vec3& center, float radius) const;

    // 获取平面 (用于调试)
    struct Plane {
        glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        float Distance = 0.0f;

        float SignedDistance(const glm::vec3& point) const
        {
            return glm::dot(Normal, point) + Distance;
        }
    };

    const Plane& GetPlane(int index) const { return m_Planes[index]; }

private:
    // 顺序: Left, Right, Bottom, Top, Near, Far
    Plane m_Planes[6];
};
