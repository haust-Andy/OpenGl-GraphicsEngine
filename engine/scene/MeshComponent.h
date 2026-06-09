#pragma once

#include <memory>
#include "renderer/VertexArray.h"
#include "renderer/Material.h"
#include "TransformComponent.h"

// MeshComponent - 渲染网格 + 材质
class MeshComponent
{
public:
    std::shared_ptr<class VertexArray> VertexArray;
    std::shared_ptr<class Material>    Material;
    TransformComponent* Transform = nullptr;

    bool Visible = true;
    bool CastShadow = true;

    void SetMesh(std::shared_ptr<class VertexArray> va) { VertexArray = va; }
    void SetMaterial(std::shared_ptr<class Material> mat) { Material = mat; }
};
