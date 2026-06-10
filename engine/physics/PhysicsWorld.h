#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <memory>

// 前向声明
class TransformComponent;
class Entity;

// ===== 碰撞体 =====

// AABB 轴对齐包围盒
struct AABB
{
    glm::vec3 Min = glm::vec3(-0.5f);
    glm::vec3 Max = glm::vec3(0.5f);

    glm::vec3 GetCenter() const { return (Min + Max) * 0.5f; }
    glm::vec3 GetExtents() const { return (Max - Min) * 0.5f; }
    float GetRadius() const { return glm::length(GetExtents()); }

    bool Contains(const glm::vec3& point) const
    {
        return point.x >= Min.x && point.x <= Max.x &&
               point.y >= Min.y && point.y <= Max.y &&
               point.z >= Min.z && point.z <= Max.z;
    }

    bool Intersects(const AABB& other) const
    {
        return (Min.x <= other.Max.x && Max.x >= other.Min.x) &&
               (Min.y <= other.Max.y && Max.y >= other.Min.y) &&
               (Min.z <= other.Max.z && Max.z >= other.Min.z);
    }

    // 转换到世界空间 (考虑 Transform)
    AABB Transform(const glm::mat4& mat) const
    {
        glm::vec3 corners[8] = {
            {Min.x, Min.y, Min.z}, {Max.x, Min.y, Min.z},
            {Min.x, Max.y, Min.z}, {Max.x, Max.y, Min.z},
            {Min.x, Min.y, Max.z}, {Max.x, Min.y, Max.z},
            {Min.x, Max.y, Max.z}, {Max.x, Max.y, Max.z},
        };

        AABB result;
        result.Min = glm::vec3(FLT_MAX);
        result.Max = glm::vec3(-FLT_MAX);

        for (int i = 0; i < 8; ++i)
        {
            glm::vec3 transformed = glm::vec3(mat * glm::vec4(corners[i], 1.0f));
            result.Min = glm::min(result.Min, transformed);
            result.Max = glm::max(result.Max, transformed);
        }
        return result;
    }
};

// 球体碰撞体
struct SphereCollider
{
    glm::vec3 Center = glm::vec3(0.0f);
    float Radius = 0.5f;
};

// 碰撞体类型
enum class ColliderType
{
    None = 0,
    AABB,
    Sphere
};

// 碰撞体组件
struct ColliderComponent
{
    ColliderType Type = ColliderType::AABB;
    AABB Box = { glm::vec3(-0.5f), glm::vec3(0.5f) };
    SphereCollider Sphere = { glm::vec3(0.0f), 0.5f };

    bool IsTrigger = false;  // 触发器 (不产生物理响应, 只发送回调)
};

// ===== 刚体组件 =====

struct RigidbodyComponent
{
    // 质量
    float Mass = 1.0f;
    bool  IsStatic = false;

    // 速度
    glm::vec3 Velocity = glm::vec3(0.0f);
    glm::vec3 AngularVelocity = glm::vec3(0.0f);

    // 力
    glm::vec3 Force = glm::vec3(0.0f);

    // 阻尼
    float LinearDamping  = 0.01f;
    float AngularDamping = 0.05f;

    // 重力
    bool  UseGravity = true;
    float GravityScale = 1.0f;

    // 弹性系数
    float Restitution = 0.5f;
    float Friction    = 0.5f;

    // 方法
    void AddForce(const glm::vec3& force) { Force += force; }
    void AddImpulse(const glm::vec3& impulse) { Velocity += impulse / Mass; }
};

// ===== 碰撞信息 =====

struct CollisionInfo
{
    class Entity* EntityA = nullptr;
    class Entity* EntityB = nullptr;
    glm::vec3 ContactPoint = glm::vec3(0.0f);
    glm::vec3 ContactNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    float PenetrationDepth = 0.0f;
};

// ===== 物理世界 =====

class PhysicsWorld
{
public:
    static constexpr glm::vec3 GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);

    void Step(float dt, class Scene* scene);
    void DebugDraw(class Scene* scene);

    // 射线检测
    struct RaycastHit
    {
        class Entity* HitEntity = nullptr;
        glm::vec3 Point = glm::vec3(0.0f);
        glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        float Distance = 0.0f;
    };

    static bool RaycastAABB(const glm::vec3& origin, const glm::vec3& direction,
                              const AABB& box, float& t);
    static bool RaycastSphere(const glm::vec3& origin, const glm::vec3& direction,
                               const SphereCollider& sphere, float& t);

    // 碰撞回调
    std::function<void(const CollisionInfo&)> OnCollisionEnter;
    std::function<void(const CollisionInfo&)> OnCollisionStay;
    std::function<void(const CollisionInfo&)> OnCollisionExit;
    std::function<void(class Entity*, class Entity*)> OnTriggerEnter;
    std::function<void(class Entity*, class Entity*)> OnTriggerExit;

private:
    void Integrate(RigidbodyComponent& rb, class TransformComponent& transform, float dt);
    void ResolveCollision(const CollisionInfo& info, RigidbodyComponent* rbA, RigidbodyComponent* rbB);
};
