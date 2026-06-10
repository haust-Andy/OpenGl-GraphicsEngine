#include "PhysicsWorld.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"
#include <algorithm>
#include <cmath>

void PhysicsWorld::Step(float dt, Scene* scene)
{
    if (!scene) return;

    // 1. 施加重力 & 积分
    scene->ForEachEntity([dt, this](Entity& entity)
    {
        auto& rb = entity.GetRigidbody();
        if (rb.IsStatic || !entity.HasPhysics()) return;

        auto& transform = entity.GetTransform();

        // 重力
        if (rb.UseGravity)
            rb.Force += GRAVITY * rb.Mass * rb.GravityScale;

        // 加速度
        glm::vec3 acceleration = rb.Force / rb.Mass;

        // 半隐式欧拉积分
        rb.Velocity += acceleration * dt;
        rb.Velocity *= (1.0f - rb.LinearDamping);
        rb.AngularVelocity *= (1.0f - rb.AngularDamping);

        transform.Position += rb.Velocity * dt;

        // 简单角速度→旋转 (弧度)
        glm::vec3 rot = entity.GetTransform().GetEulerAngles();
        rot += rb.AngularVelocity * dt;
        transform.SetEulerAngles(rot);

        // 清零力
        rb.Force = glm::vec3(0.0f);
    });

    // 2. 碰撞检测 & 响应
    auto& entities = scene->GetEntities();
    for (size_t i = 0; i < entities.size(); ++i)
    {
        if (!entities[i]->HasPhysics()) continue;
        auto& rbA = entities[i]->GetRigidbody();
        auto& colliderA = entities[i]->GetCollider();
        auto& transformA = entities[i]->GetTransform();

        for (size_t j = i + 1; j < entities.size(); ++j)
        {
            if (!entities[j]->HasPhysics()) continue;
            auto& rbB = entities[j]->GetRigidbody();
            auto& colliderB = entities[j]->GetCollider();
            auto& transformB = entities[j]->GetTransform();

            // AABB 碰撞检测
            if (colliderA.Type == ColliderType::AABB && colliderB.Type == ColliderType::AABB)
            {
                AABB worldA = colliderA.Box.Transform(transformA.GetWorldMatrix());
                AABB worldB = colliderB.Box.Transform(transformB.GetWorldMatrix());

                if (worldA.Intersects(worldB))
                {
                    CollisionInfo info;
                    info.EntityA = entities[i].get();
                    info.EntityB = entities[j].get();

                    // 计算穿透信息
                    glm::vec3 overlap;
                    overlap.x = std::min(worldA.Max.x - worldB.Min.x, worldB.Max.x - worldA.Min.x);
                    overlap.y = std::min(worldA.Max.y - worldB.Min.y, worldB.Max.y - worldA.Min.y);
                    overlap.z = std::min(worldA.Max.z - worldB.Min.z, worldB.Max.z - worldA.Min.z);

                    // 最小穿透轴
                    if (overlap.x < overlap.y && overlap.x < overlap.z)
                    {
                        info.ContactNormal = glm::vec3(transformA.Position.x > transformB.Position.x ? 1.0f : -1.0f, 0.0f, 0.0f);
                        info.PenetrationDepth = overlap.x;
                    }
                    else if (overlap.y < overlap.z)
                    {
                        info.ContactNormal = glm::vec3(0.0f, transformA.Position.y > transformB.Position.y ? 1.0f : -1.0f, 0.0f);
                        info.PenetrationDepth = overlap.y;
                    }
                    else
                    {
                        info.ContactNormal = glm::vec3(0.0f, 0.0f, transformA.Position.z > transformB.Position.z ? 1.0f : -1.0f);
                        info.PenetrationDepth = overlap.z;
                    }

                    info.ContactPoint = (worldA.GetCenter() + worldB.GetCenter()) * 0.5f;

                    // 触发器 vs 物理
                    if (colliderA.IsTrigger || colliderB.IsTrigger)
                    {
                        if (OnTriggerEnter)
                            OnTriggerEnter(info.EntityA, info.EntityB);
                    }
                    else
                    {
                        ResolveCollision(info, &rbA, &rbB);
                        if (OnCollisionEnter)
                            OnCollisionEnter(info);
                    }
                }
            }
        }
    }
}

void PhysicsWorld::Integrate(RigidbodyComponent& rb, TransformComponent& transform, float dt)
{
    if (rb.IsStatic) return;

    glm::vec3 acceleration = rb.Force / rb.Mass;
    rb.Velocity += acceleration * dt;
    rb.Velocity *= (1.0f - rb.LinearDamping);
    transform.Position += rb.Velocity * dt;
    rb.Force = glm::vec3(0.0f);
}

void PhysicsWorld::ResolveCollision(const CollisionInfo& info, RigidbodyComponent* rbA, RigidbodyComponent* rbB)
{
    if (!rbA || !rbB) return;
    if (rbA->IsStatic && rbB->IsStatic) return;

    // 位置修正 (推出重叠)
    float totalInvMass = (rbA->IsStatic ? 0.0f : 1.0f / rbA->Mass) +
                          (rbB->IsStatic ? 0.0f : 1.0f / rbB->Mass);
    if (totalInvMass == 0.0f) return;

    float percent = 0.8f;  // 修正比例
    float slop = 0.01f;    // 穿透容忍度
    glm::vec3 correction = info.ContactNormal *
        (std::max(info.PenetrationDepth - slop, 0.0f) / totalInvMass * percent);

    // 速度响应 (弹性碰撞)
    glm::vec3 relVel = rbA->Velocity - rbB->Velocity;
    float velAlongNormal = glm::dot(relVel, info.ContactNormal);

    // 如果已经在分离则跳过
    if (velAlongNormal > 0.0f) return;

    float e = std::min(rbA->Restitution, rbB->Restitution);
    float j = -(1.0f + e) * velAlongNormal / totalInvMass;

    glm::vec3 impulse = info.ContactNormal * j;

    if (!rbA->IsStatic)
    {
        rbA->Velocity += impulse / rbA->Mass;
        // 位置修正
        if (info.EntityA)
            info.EntityA->GetTransform().Position += correction * (1.0f / rbA->Mass) / totalInvMass;
    }
    if (!rbB->IsStatic)
    {
        rbB->Velocity -= impulse / rbB->Mass;
        if (info.EntityB)
            info.EntityB->GetTransform().Position -= correction * (1.0f / rbB->Mass) / totalInvMass;
    }
}

bool PhysicsWorld::RaycastAABB(const glm::vec3& origin, const glm::vec3& direction,
                                 const AABB& box, float& t)
{
    glm::vec3 invDir = 1.0f / direction;

    float t1 = (box.Min.x - origin.x) * invDir.x;
    float t2 = (box.Max.x - origin.x) * invDir.x;
    float t3 = (box.Min.y - origin.y) * invDir.y;
    float t4 = (box.Max.y - origin.y) * invDir.y;
    float t5 = (box.Min.z - origin.z) * invDir.z;
    float t6 = (box.Max.z - origin.z) * invDir.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    if (tmax < 0 || tmin > tmax) return false;

    t = (tmin < 0.0f) ? tmax : tmin;
    return true;
}

bool PhysicsWorld::RaycastSphere(const glm::vec3& origin, const glm::vec3& direction,
                                    const SphereCollider& sphere, float& t)
{
    glm::vec3 oc = origin - sphere.Center;
    float a = glm::dot(direction, direction);
    float b = 2.0f * glm::dot(oc, direction);
    float c = glm::dot(oc, oc) - sphere.Radius * sphere.Radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return false;

    t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0) t = (-b + std::sqrt(discriminant)) / (2.0f * a);
    return t >= 0;
}
