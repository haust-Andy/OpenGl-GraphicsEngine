#pragma once

#include <functional>
#include <string>
#include <memory>

#include "core/Base.h"
#include "core/Timestep.h"

// 脚本组件 - 实体行为逻辑 (C++ 回调模式)
// 用法:
//   auto* entity = scene->CreateEntity("Player");
//   auto& script = entity->GetScript();
//   script.OnCreate = [](Entity& e) { ... };
//   script.OnUpdate = [](Entity& e, Timestep ts) { ... };

class Entity;

class ScriptComponent
{
public:
    // 生命周期回调
    std::function<void(Entity&)> OnCreate;
    std::function<void(Entity&, Timestep)> OnUpdate;
    std::function<void(Entity&)> OnDestroy;

    // 启用/禁用
    bool Enabled = true;

    // 脚本标识 (用于调试/热重载)
    std::string ScriptName = "UnnamedScript";

    // 是否已调用 OnCreate
    bool m_Initialized = false;

    void Initialize(Entity& entity)
    {
        if (m_Initialized) return;
        if (OnCreate) OnCreate(entity);
        m_Initialized = true;
    }

    void Update(Entity& entity, Timestep ts)
    {
        if (!Enabled || !m_Initialized) return;
        if (OnUpdate) OnUpdate(entity, ts);
    }

    void Destroy(Entity& entity)
    {
        if (OnDestroy) OnDestroy(entity);
    }
};
