#pragma once

#include "renderer/Light.h"

// LightComponent - 光源组件
struct LightComponent
{
    LightType Type = LightType::Point;

    DirectionalLight DirLight;
    PointLight       PtLight;
    SpotLight        SpLight;

    bool Enabled = true;
};
