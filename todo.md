# 游戏引擎开发路线图

> 当前引擎版本 v2.1，已实现 PBR+CSM+IBL 渲染管线、物理碰撞、脚本系统、粒子系统、游戏 UI 及全功能 ImGui 编辑器。

---

## 已实现功能总览

### 🔴 P0 — 游戏性基础设施 (全部完成 ✅)

| 功能 | 文件 | 状态 |
|------|------|------|
| **阴影系统 (CSM)** | `engine/renderer/ShadowMap.h/.cpp`, `shader/pbr.frag` | ✅ 完成 |
| | 3 级联阴影贴图 + PSSM 分割 + Poisson Disk PCF 软阴影 + 法线偏移 | |
| **模型导入** | `engine/resource/Model.h/.cpp` | ✅ 完成 |
| | Assimp 加载 .obj/.fbx/.gltf + PBR 纹理自动映射 + 子网格 | |
| **脚本系统** | `engine/scene/ScriptComponent.h` | ✅ 完成 |
| | OnCreate/OnUpdate/OnDestroy 生命周期 + Lambda 回调 | |
| **碰撞/物理** | `engine/physics/PhysicsWorld.h/.cpp` | ✅ 完成 |
| | AABB/Sphere 碰撞 + 弹性碰撞响应 + 重力 + Raycast + 碰撞回调 | |

### 🟡 P1 — 画面质量与开发效率 (全部完成 ✅)

| 功能 | 文件 | 状态 |
|------|------|------|
| **视锥体剔除** | `engine/renderer/Frustum.h/.cpp` | ✅ 完成 |
| | 6 平面提取 + AABB/球体相交测试 + Scene 自动剔除 | |
| **粒子系统** | `engine/particle/ParticleEmitter.h/.cpp` | ✅ 完成 |
| | CPU 粒子 + Billboard 渲染 + 发射/生命周期/颜色渐变/重力 | |
| **SSAO** | `shader/ssao.frag`, `shader/ssao_blur.frag`, `shader/ssao_combine.frag` | ✅ 着色器完成 |
| | 64 采样核心 + 双边模糊 + 合成 (需集成到后处理管线) | |
| **IBL 环境光照** | `engine/renderer/IBL.h/.cpp`, `shader/pbr.frag` | ✅ 完成 |
| | HDR 加载 + 辐照度卷积 + 预过滤 5 级 MIP + BRDF LUT + PBR 集成 | |
| **游戏 UI** | `engine/ui/UIElements.h/.cpp` | ✅ 完成 |
| | SpriteBatch + UIImage/UIText/UIButton + Canvas + 锚点 | |
| **抗锯齿** | — | ❌ 未实现 |
| | MSAA/FXAA/TAA 均未实现，FramebufferSpec::Samples 已预留 | |

### 🟢 P2 — 渲染架构与性能 (完成 ✅)

| 功能 | 文件 | 状态 |
|------|------|------|
| **LOD 系统** | `engine/renderer/LOD.h/.cpp` | ✅ 完成 |
| | 距离/屏幕占比模式 + Entity LOD 组件 + Scene 自动选择 | |

### 🔵 P3 — 编辑器与工具 (完成 ✅)

| 功能 | 文件 | 状态 |
|------|------|------|
| **编辑器增强** | `engine/editor/EditorLayer.h/.cpp` | ✅ 完成 |
| | Gizmo 操作 (W/E/R) + ContentBrowser + Play/Stop (F5) + Prefab Panel | |
| **Prefab 系统** | `engine/scene/Prefab.h/.cpp` | ✅ 完成 |
| | 从 Entity 创建模板 + 实例化 + 序列化 | |

### 🔧 v2.1 代码质量改进 (全部完成 ✅)

| 改进 | 状态 |
|------|------|
| HDR 后处理管线 (RGBA16F) | ✅ |
| Shader 错误传播 | ✅ |
| GL 资源安全初始化 | ✅ |
| LOD VAO 安全恢复 | ✅ |
| Poisson Disk PCF | ✅ |
| GLStateSaver RAII | ✅ |
| Layer unique_ptr 迁移 | ✅ |
| Camera 动态宽高比 | ✅ |
| 跨平台兼容 (Windows + Linux) | ✅ |
| GitHub Actions CI | ✅ |

---

## 当前引擎能力评估 (v2.1)

| 维度 | 评分 (1-5) | 说明 |
|------|-----------|------|
| PBR 渲染质量 | ⭐⭐⭐⭐⭐ | Cook-Torrance + CSM + IBL + HDR 后处理 |
| 编辑器工具 | ⭐⭐⭐⭐ | ImGui 7+ 面板，Gizmo + Prefab + Play/Stop |
| 代码架构 | ⭐⭐⭐⭐ | 分层清晰，组件模式，RAII 安全 |
| 阴影/光照 | ⭐⭐⭐⭐ | CSM 级联阴影 + 多光源 + IBL |
| 物理系统 | ⭐⭐⭐ | AABB/Sphere 碰撞 + 弹性响应 + Raycast |
| 脚本系统 | ⭐⭐⭐ | C++ Lambda 回调，缺 Lua/热重载 |
| 粒子系统 | ⭐⭐⭐ | CPU Billboard，功能完整 |
| 音频系统 | ⭐ | 仅 Stub，SoLoud 未集成 |
| 动画系统 | ⭐ | 仅 Assimp 静态网格，无骨骼/蒙皮 |
| 性能优化 | ⭐⭐⭐ | 视锥体剔除 + LOD，无多线程 |
| 跨平台 | ⭐⭐⭐ | Windows + Linux CI 验证 |

**综合评分: 3.2/5** — 功能完整的轻量渲染引擎，可开发有限游戏，距工业级引擎还需骨骼动画、Lua 脚本、音频集成等。

---

## 计划中功能 (v3.0+)

| 优先级 | 功能 | 说明 | 预估工作量 |
|--------|------|------|------------|
| P0 | 骨骼动画 (GPU Skinning) | Assimp 骨骼导入 + 蒙皮矩阵 UBO + 动画状态机 | 4-6 周 |
| P0 | Lua 脚本绑定 | sol3 集成 + C++ API 导出 + 热重载 | 2-3 周 |
| P1 | 抗锯齿 (MSAA/FXAA) | FramebufferSpec::Samples 已预留 | 3-5 天 |
| P1 | SSAO 集成 | 着色器已就绪，需接入后处理管线 | 1 周 |
| P1 | ECS 架构迁移 | Entity 胖实体 → 真正 Entity-Component-System (entt) | 3-4 周 |
| P1 | SoLoud 音频集成 | Stub → 实际音频播放 + 3D 空间化 | 1-2 周 |
| P2 | 延迟渲染管线 | G-Buffer + Tile-Based 光照 | 3-4 周 |
| P2 | 资源打包 (.pak) | 虚拟文件系统 + AssetHandle | 2-3 周 |
| P2 | VAO 抽象层增强 | 为 PBR 布局提供标准化方法 | 1 周 |
| P3 | 多线程渲染 | 渲染线程分离 + 异步资源加载 | 3-4 周 |
| P3 | 网络系统 | Client-Server + Entity 同步 | 6-8 周 |
| P3 | GPU 粒子 | Compute Shader 更新粒子 | 2-3 周 |

---

## 技术债

| 项目 | 说明 | 优先级 |
|------|------|--------|
| 单例线程安全 | ShaderLibrary / TextureLibrary 非线程安全 | 中 |
| CMake GLOB_RECURSE | 源文件列表不会自动检测新增文件 | 低 |
| Event.h 文件拆分 | 单文件过大，按事件类型拆分 | 低 |
| 级联 uniform 缓存 | 级联矩阵 uniform 名称运行时字符串拼接 | 中 |
| Model.cpp 绕过 VAO 抽象 | 直接调用 glVertexAttribPointer | 中 |
| SandboxApp 硬编码宽高 | 1600×900 硬编码，应从 Window 获取 | 低 |
