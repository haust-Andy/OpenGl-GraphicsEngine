#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <memory>

// TransformComponent - 位置/旋转/缩放 + 父子层级
class TransformComponent
{
public:
    glm::vec3 Position = glm::vec3(0.0f);
    glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // Identity
    glm::vec3 Scale    = glm::vec3(1.0f);

    // 层级关系
    TransformComponent* Parent = nullptr;
    std::vector<std::unique_ptr<TransformComponent>> Children;

    glm::mat4 GetLocalMatrix() const;
    glm::mat4 GetWorldMatrix() const;

    glm::vec3 GetEulerAngles() const { return glm::eulerAngles(Rotation); }
    void SetEulerAngles(const glm::vec3& euler) { Rotation = glm::quat(euler); }

    // 前/右/上方向
    glm::vec3 Forward() const { return Rotation * glm::vec3(0, 0, -1); }
    glm::vec3 Right()   const { return Rotation * glm::vec3(1, 0, 0); }
    glm::vec3 Up()      const { return Rotation * glm::vec3(0, 1, 0); }
};

inline glm::mat4 TransformComponent::GetLocalMatrix() const
{
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, Position);
    mat = mat * glm::toMat4(Rotation);
    mat = glm::scale(mat, Scale);
    return mat;
}

inline glm::mat4 TransformComponent::GetWorldMatrix() const
{
    glm::mat4 local = GetLocalMatrix();
    if (Parent)
        return Parent->GetWorldMatrix() * local;
    return local;
}
