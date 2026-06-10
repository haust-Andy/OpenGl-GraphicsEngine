#include "Frustum.h"
#include "physics/PhysicsWorld.h"  // AABB

void Frustum::ExtractFromMatrix(const glm::mat4& vp)
{
    // 左平面
    m_Planes[0].Normal.x = vp[0][3] + vp[0][0];
    m_Planes[0].Normal.y = vp[1][3] + vp[1][0];
    m_Planes[0].Normal.z = vp[2][3] + vp[2][0];
    m_Planes[0].Distance = vp[3][3] + vp[3][0];

    // 右平面
    m_Planes[1].Normal.x = vp[0][3] - vp[0][0];
    m_Planes[1].Normal.y = vp[1][3] - vp[1][0];
    m_Planes[1].Normal.z = vp[2][3] - vp[2][0];
    m_Planes[1].Distance = vp[3][3] - vp[3][0];

    // 下平面
    m_Planes[2].Normal.x = vp[0][3] + vp[0][1];
    m_Planes[2].Normal.y = vp[1][3] + vp[1][1];
    m_Planes[2].Normal.z = vp[2][3] + vp[2][1];
    m_Planes[2].Distance = vp[3][3] + vp[3][1];

    // 上平面
    m_Planes[3].Normal.x = vp[0][3] - vp[0][1];
    m_Planes[3].Normal.y = vp[1][3] - vp[1][1];
    m_Planes[3].Normal.z = vp[2][3] - vp[2][1];
    m_Planes[3].Distance = vp[3][3] - vp[3][1];

    // 近平面
    m_Planes[4].Normal.x = vp[0][3] + vp[0][2];
    m_Planes[4].Normal.y = vp[1][3] + vp[1][2];
    m_Planes[4].Normal.z = vp[2][3] + vp[2][2];
    m_Planes[4].Distance = vp[3][3] + vp[3][2];

    // 远平面
    m_Planes[5].Normal.x = vp[0][3] - vp[0][2];
    m_Planes[5].Normal.y = vp[1][3] - vp[1][2];
    m_Planes[5].Normal.z = vp[2][3] - vp[2][2];
    m_Planes[5].Distance = vp[3][3] - vp[3][2];

    // 归一化所有平面
    for (int i = 0; i < 6; ++i)
    {
        float length = glm::length(m_Planes[i].Normal);
        if (length > 0.0001f)
        {
            m_Planes[i].Normal /= length;
            m_Planes[i].Distance /= length;
        }
    }
}

bool Frustum::ContainsPoint(const glm::vec3& point) const
{
    for (int i = 0; i < 6; ++i)
    {
        if (m_Planes[i].SignedDistance(point) < 0.0f)
            return false;
    }
    return true;
}

bool Frustum::IntersectsAABB(const AABB& aabb) const
{
    for (int i = 0; i < 6; ++i)
    {
        // 找到 AABB 上沿平面法线方向的最远点 (正极点)
        glm::vec3 positive = aabb.Min;
        if (m_Planes[i].Normal.x >= 0) positive.x = aabb.Max.x;
        if (m_Planes[i].Normal.y >= 0) positive.y = aabb.Max.y;
        if (m_Planes[i].Normal.z >= 0) positive.z = aabb.Max.z;

        // 如果正极点在平面外, 则 AABB 完全在视锥体外
        if (m_Planes[i].SignedDistance(positive) < 0.0f)
            return false;
    }
    return true;
}

bool Frustum::IntersectsSphere(const glm::vec3& center, float radius) const
{
    for (int i = 0; i < 6; ++i)
    {
        if (m_Planes[i].SignedDistance(center) < -radius)
            return false;
    }
    return true;
}
