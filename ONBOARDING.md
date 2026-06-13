# OpenGL Graphics Engine — 开发手册

> **目标读者**: 新入职开发者 & 日常开发参考  
> **项目版本**: v2.1 | 更新: 2026-06-13  
> **预计上手**: 3 天

---

## 目录

1. [项目速览](#1-项目速览)
2. [环境搭建与运行](#2-环境搭建与运行)
3. [技术架构拓扑](#3-技术架构拓扑)
4. [目录结构](#4-目录结构)
5. [推荐阅读路线](#5-推荐阅读路线)
6. [核心子系统深度解读](#6-核心子系统深度解读)
7. [渲染管线数据流](#7-渲染管线数据流)
8. [开发任务 How-To](#8-开发任务-how-to)
9. [代码规范与 Code Review](#9-代码规范与-code-review)
10. [CI/CD 与 PR 流程](#10-cicd-与-pr-流程)
11. [能力评估与路线图](#11-能力评估与路线图)
12. [调试技巧](#12-调试技巧)
13. [速查表 Cheat Sheet](#13-速查表-cheat-sheet)

---

## 1. 项目速览

从 LearnOpenGL 教学项目改造而来的分层 3D 图形引擎。C++17，OpenGL 3.3 Core Profile。

**技术栈**: GLFW 3.4 | GLAD 4.6 | GLM 0.9.9 | Assimp 6.x | stb_image | ImGui

**代码量**: ~77 源文件 + ~45 头文件，6000+ 行 C++，~11 个子系统

**构建产物**:
| 目标 | 类型 | 说明 |
|------|------|------|
| `OpenGlEngine` | 静态库 `.a` | 引擎全部代码 + ImGui + GLAD |
| `Sandbox` | 可执行文件 | 演示程序 |
| `Tests` | 可执行文件 | 单元测试 |

**启动后你能看到**: 1600×900 窗口，左侧 Hierarchy 实体树，中间 3D 视口（PBR 立方体/金属球/粒子火焰/旋转立方体），右侧 Inspector，下方 Content Browser。

---

## 2. 环境搭建与运行

### 依赖安装

| 工具 | Windows | Linux (Ubuntu) |
|------|---------|----------------|
| CMake 3.16+ | [cmake.org](https://cmake.org) | `sudo apt install cmake` |
| 编译器 | MinGW-w64 或 MSVC | `g++` |
| Linux 额外 | — | `sudo apt install libglfw3-dev libassimp-dev xorg-dev libgl1-mesa-dev` |

### 编译运行

```bash
git clone git@github.com:haust-Andy/OpenGl-GraphicsEngine.git
cd OpenGl-GraphicsEngine && git checkout dev
mkdir build && cd build

# Windows MinGW
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release -j 8
./Sandbox.exe

# Windows MSVC
cmake ..
cmake --build . --config Release
.\Release\Sandbox.exe

# Linux
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j $(nproc)
./Sandbox
```

### 运行测试

```bash
cd build && ./Tests    # (Windows: Tests.exe)
```

---

## 3. 技术架构拓扑

```
┌──────────────────────────────────────────────────────────────────────┐
│                     APPLICATION LAYER                                │
│                     app/SandboxApp.cpp                                │
├──────────────────────────────────────────────────────────────────────┤
│                     EDITOR LAYER                                      │
│  engine/editor/ — ImGui 面板: Hierarchy / Inspector / Viewport / ... │
├──────────────────────────────────────────────────────────────────────┤
│                     ENGINE CORE                                      │
│  Application | Window | Input | Event | LayerStack | Timestep | Log  │
├──────────────────────────────────────────────────────────────────────┤
│  RENDERER                          POSTPROCESS (Bloom+ToneMapping)   │
│  Shader / Texture / Framebuffer    RESOURCE (ShaderLib / Model)      │
│  VertexArray / Buffer / Material   SCENE (Entity + 10 组件)          │
│  Camera / Light / ShadowMap / IBL  SUBSYSTEMS:                       │
│  Frustum / LOD / UniformBuffer     Physics / Particle / Audio / UI   │
├──────────────────────────────────────────────────────────────────────┤
│  GLFW 3.4 | GLAD 4.6 | GLM | Assimp | stb_image | ImGui             │
└──────────────────────────────────────────────────────────────────────┘
```

**所有权链**:
```
CreateApplication() (app/SandboxApp.cpp)
  → new Application("OGL", 1600, 900)
    → Window (GLFWwindow*)  ← GLFW Callback → Event → Application::OnEvent
    → LayerStack (vector<unique_ptr<Layer>>)
        ├── SandboxLayer (Normal): 游戏逻辑/场景/渲染
        └── EditorLayer  (Overlay): ImGui 编辑器面板
```

**主循环 (Application::Run)**:
```
while (m_Running) {
    ts = Clamp(now - lastTime);
    for (layer : m_LayerStack) layer->OnUpdate(ts);     // 逻辑+渲染
    ImGui::NewFrame();
    for (layer : m_LayerStack) layer->OnImGuiRender();  // ImGui
    ImGui::Render();
    m_Window->OnUpdate();  // SwapBuffers + PollEvents
}
```

---

## 4. 目录结构

```
OpenGl-GraphicsEngine/
├── app/                        # 应用层
│   └── SandboxApp.cpp          # ★ 完整阅读 (614行, 引擎集成演示)
├── engine/                     # 引擎核心 (静态库)
│   ├── core/       (11 files)  # 基础设施: Application/Window/Event/Layer/Log/Input...
│   ├── renderer/   (22 files)  # 渲染: Shader/Texture/FBO/VAO/Buffer/Camera/Material/
│   │                           #        Light/UBO/ShadowMap(CSM)/IBL/Frustum/LOD
│   ├── resource/   (8 files)   # 资源: ShaderLibrary/TextureLibrary/MeshLibrary/Model
│   ├── scene/      (10 files)  # 场景: Scene/Entity/Prefab/SceneSerializer/Script...
│   ├── postprocess/(2 files)   # 后处理: Bloom+ToneMapping+GLStateSaver
│   ├── physics/    (2 files)   # 物理: AABB/Sphere碰撞+弹性响应+Raycast
│   ├── particle/   (2 files)   # 粒子: CPU+Billboard+颜色渐变
│   ├── audio/      (2 files)   # 音频: SoLoud接口 (Stub)
│   ├── ui/         (2 files)   # 游戏UI: SpriteBatch+Canvas+控件
│   └── editor/     (2 files)   # 编辑器: ImGui 7+面板
├── shader/                     # GLSL 着色器 (7个有效文件)
│   ├── pbr.vert/pbr.frag       # PBR + CSM阴影 + IBL (输出线性HDR)
│   ├── screen.vert              # 全屏后处理顶点
│   ├── bloom_combine.frag       # Bloom 合成
│   ├── brightness.frag          # 亮度提取
│   ├── gaussian_blur.frag       # 高斯模糊
│   └── tonemapping.frag         # ACES 色调映射
├── tests/                      # 单元测试 (TestBase/Log/LayerStack/Event/SceneSerializer)
├── resources/                  # 游戏资源 (textures/ + models/)
├── external/                   # 第三方库 (glm-master/ imgui-master/)
├── include/                    # GLAD/GLFW/stb 头文件
├── lib/                        # 预编译库 (glfw3, assimp)
└── .github/workflows/          # CI (pr-checks.yml)
```

---

## 5. 推荐阅读路线

按顺序阅读，每天 2-3 小时，3 天可上手。

### Day 1 — 核心流水线

```
engine/core/Base.h          ← Ref<>/Scope<> 别名
engine/core/Timestep.h      ← 帧时间步
engine/core/Log.h           ← CORE_TRACE/INFO/WARN/ERROR 宏
engine/renderer/Shader.h    ← 编译/Uniform缓存/热重载
engine/renderer/Buffer.h    ← VBO/IBO (禁止拷贝/移动语义)
engine/renderer/VertexArray.h ← VAO 封装
engine/renderer/Framebuffer.h ← FBO (RGBA16F HDR)
engine/renderer/Renderer.h  ← Begin/EndFrame/Submit 门面
engine/renderer/RendererAPI.h ← OpenGL 后端
```

### Day 2 — 应用框架 + 场景

```
engine/core/Event.h         ← EventDispatcher / 事件体系
engine/core/Input.h         ← 轮询式输入
engine/core/Window.h        ← GLFW→事件映射
engine/core/Layer.h         ← Layer基类 + LayerStack
engine/core/Application.h   ← 主循环 Run() / 事件分发
engine/core/EntryPoint.h    ← CreateApplication → main()
engine/scene/Scene.h        ← Entity CRUD + OnRender
engine/scene/Entity.h       ← 胖实体 + 10种组件
engine/renderer/Camera.h    ← FPS相机/动态宽高比
engine/renderer/Material.h  ← PBR材质
engine/renderer/Light.h     ← 多光源 + LightEnvironment
```

### Day 3 — 渲染管线深度 + 子系统

```
engine/renderer/ShadowMap.h  ← CSM 3级联 + Poisson PCF
engine/renderer/IBL.h        ← IBL 辐照度/预过滤/BRDF LUT
engine/renderer/Frustum.h    ← 视锥体6平面剔除
engine/renderer/LOD.h        ← 距离/屏幕占比LOD
engine/postprocess/PostProcess.h ← Bloom + ToneMapping
app/SandboxApp.cpp           ← ★ 完整阅读
engine/editor/EditorLayer.h  ← ImGui 面板架构
engine/physics/PhysicsWorld.h
engine/particle/ParticleEmitter.h
```

---

## 6. 核心子系统深度解读

### 6.1 引擎核心 (engine/core/)

**Base.h** — 基础类型别名:
```cpp
using uint32 = uint32_t;
template<typename T> using Ref   = std::shared_ptr<T>;
template<typename T> using Scope = std::unique_ptr<T>;
```

**Log** — 6 级分级日志。**禁止** `std::cout/cerr`，统一使用 `CORE_TRACE/INFO/WARN/ERROR/CRITICAL`。

**Event** — 事件体系。GLFW Callback → 构造 Event → `EventDispatcher::Dispatch<T>(handler)` 按类型分发，`e.Handled = true` 阻止冒泡。

> ⚠️ ImGui 的 GLFW 回调链会消费鼠标事件，相机旋转/平移已改为 `Input::GetMousePosition()` 轮询模式。

**LayerStack** — 2 层架构：SandboxLayer (Normal) + EditorLayer (Overlay)。Overlay 先处理事件、后渲染。所有权 `unique_ptr`。

### 6.2 渲染器 (engine/renderer/)

**Renderer 门面** 委托给 `RendererAPI`（OpenGL 实现），所有渲染状态变更经门面中转。

安全约定:
- GL 资源 ID 初始化为 0，析构前 `if (m_ID) glDelete*()`
- Buffer 禁止拷贝（`= delete`），支持移动语义
- Shader 编译/链接失败正确返回 false

**Shader** — `Shader::Create(vsPath, fsPath)` 编译 + 链接 + Uniform 缓存。`ShaderLibrary` 单例支持热重载 `ReloadAll()`。

**Material** — PBR 材质：Albedo/Metallic/Roughness/AO/Emission 属性 + 6 通道纹理。`Bind()` 自动绑定到当前 Shader。

**LightEnvironment** — 光源管理器：DirectionalLight×1 + PointLight[] + SpotLight[]。通过 UBO 传递给 Shader。

**ShadowMap (CSM)** — 3 级联 2048×2048 阴影贴图 (PSSM 分割) + Poisson Disk 32 样本 PCF + 法线偏移。

**IBL** — HDR 环境贴图 → 辐照度卷积 (32×32 CubeMap) → 预过滤 (5 级 MIP) → BRDF LUT (512×512)。

**Frustum** — 6 平面提取 + AABB/Sphere 相交测试。`Scene::OnRender()` 中自动剔除。

**LOD** — 距离/屏幕占比模式，4 级 VAO。渲染后安全恢复原始 VAO。

### 6.3 后处理 (engine/postprocess/)

```
PostProcessPipeline::Execute(sceneFBO)
  ├── BloomPass: 亮度提取 → 高斯模糊(H+V) → 合成
  │   (可调: 阈值/强度/迭代次数, HDR FBO RGBA16F)
  ├── ToneMappingPass: ACES 色调映射 + Gamma 校正 (HDR → LDR)
  └── GLStateSaver RAII: 自动保存/恢复 Depth/Blend/CullFace
```

### 6.4 资源管理 (engine/resource/)

| 库 | 职责 | 关键 API |
|----|------|---------|
| `ShaderLibrary` | Shader 按名缓存 | `Get/Add/ReloadAll` |
| `TextureLibrary` | 纹理按路径幂等加载 | `Load2D/LoadCube/LoadHDR` |
| `MeshLibrary` | 内置几何体 | `GetCube/GetSphere/GetPlane/GetScreenQuad` |
| `Model` | Assimp 导入 | `Load(.obj/.fbx/.gltf)` → SubMesh[] |

> ⚠️ ShaderLibrary/TextureLibrary 单例非线程安全。

### 6.5 场景系统 (engine/scene/)

**Entity** — "胖实体"模式（非纯 ECS，适用于 <1000 实体）：

```
Entity
├── TransformComponent        [必有] 位置/旋转/缩放/层级
├── MeshComponent             [必有] VAO + Material + CastShadow
├── LightComponent            [可选, HasLight]
├── ScriptComponent           [可选, HasScript] Lambda 回调 OnCreate/OnUpdate/OnDestroy
├── Collider + Rigidbody      [可选, HasPhysics] AABB/Sphere + 质量/速度/力/弹性
├── AudioSource/AudioListener [可选]
├── ParticleEmitter           [可选, HasParticleEmitter] unique_ptr
└── LODComponent              [可选, HasLOD] Mode + Levels[] + Bias
```

**Scene** — `CreateEntity/DestroyEntity/OnUpdate/OnRender`。`OnRender` 流程: CollectLights → SortEntities → Frustum Cull → LOD Select → Draw。

**Prefab** — `CreateFromEntity → SaveToFile/LoadFromFile → Instantiate(scene)`。

**SceneSerializer** — `.scene` 文本格式序列化。

### 6.6 物理系统 (engine/physics/)

自有实现，无第三方物理引擎。AABB/Sphere 碰撞检测 + 弹性碰撞响应 + 重力 + Raycast。碰撞回调: `OnCollisionEnter/Stay/Exit`。O(n²) 遍历，场景 >200 实体时需考虑空间分区。

### 6.7 其他子系统

| 子系统 | 文件 | 说明 |
|--------|------|------|
| Particle | `engine/particle/` | CPU 粒子池 + Billboard 渲染 + 颜色渐变 + 重力 |
| Audio | `engine/audio/` | SoLoud 接口 Stub，AudioSource/Listener 组件已定义 |
| Game UI | `engine/ui/` | SpriteBatch + UIImage/Text/Button + Canvas + 9 种锚点 |
| Editor | `engine/editor/` | ImGui 7+ 面板: Viewport/Hierarchy/Inspector/ContentBrowser/Prefab/Stats/Menu |

---

## 7. 渲染管线数据流

每帧 `SandboxLayer::OnUpdate()` 的完整流程:

```
1. HandleInput + UpdateCamera (轮询 WASD/鼠标)
2. Scene::OnUpdate(ts) → Script + Physics + Particle
3. Shadow Pass:
   ├── ShadowMap::CalculateCascades() → 3组 LightSpaceMatrix
   └── RenderShadowPass() → 深度写入 (2048×2048 3级联)
4. Scene Pass:
   ├── SceneFBO::Bind() + glClear()  [RGBA16F HDR]
   ├── Skybox (IBL 环境/程序化)
   ├── Scene::OnRender(camera):
   │   ├── CollectLights → LightEnvironment
   │   ├── SortEntities (按材质)
   │   ├── Frustum Cull (视锥体剔除)
   │   ├── LOD Select (自动LOD)
   │   ├── BeginScene(view, proj) → CameraUBO + LightUBO
   │   └── ForEach Entity:
   │       Material::Bind() → Shader::Bind() + 纹理绑定
   │       ShadowMap::Bind() (binding=2) + IBL::Bind() (slot 7/8/9)
   │       VertexArray::Bind() → Renderer::Submit() → glDrawElements()
5. PostProcess::Execute(sceneFBO) → Bloom + ToneMapping
6. Blit → ViewportFBO → EditorLayer::DrawViewport() → ImGui::Image
```

**GPU 端 (pbr.frag)**: 纹理采样 → Cook-Torrance BRDF (D-GGX + G-Smith + F-Schlick) × 多光源 → CSM Shadow (Poisson PCF) → IBL Ambient (irradiance + prefiltered × BRDF_LUT) → Emission → 输出线性 HDR。

---

## 8. 开发任务 How-To

### 添加新 Entity 组件

1. 在 `engine/scene/` 创建组件头文件
2. 在 `Entity.h` 添加成员 + `Has*` 标志
3. 在 `Scene::OnUpdate()` 处理组件逻辑
4. 在 `EditorLayer::DrawInspector()` 添加编辑器 UI

### 添加后处理效果

1. 编写 GLSL → `shader/`
2. 在 `engine/postprocess/PostProcess.cpp` 创建 Pass
3. 注册到 `PostProcessPipeline::Execute()`
4. 使用 `GLStateSaver` RAII 保护 GL 状态

### 加载自定义模型

```cpp
auto model = std::make_shared<Model>();
model->Load("resources/models/my_model.fbx");
for (auto& subMesh : model->GetSubMeshes()) {
    auto* entity = scene->CreateEntity(subMesh.Name);
    entity->GetMesh().SetMesh(subMesh.VAO);
    entity->GetMesh().SetMaterial(Material::Create(pbrShader));
}
```

### 添加热键

```cpp
// 轮询 (推荐, 持续触发):
if (Input::IsKeyPressed(Key::F6)) DoSomething();

// 事件 (单次触发):
EventDispatcher dispatcher(event);
dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
    if (e.GetKeyCode() == Key::F6 && e.GetRepeatCount() == 0) {
        DoSomething(); return true;
    }
    return false;
});
```

### 场景保存/加载

```cpp
SceneSerializer::Save(*scene, "my_scene.scene");
auto scene = Scene::Create("Loaded");
SceneSerializer::Load("my_scene.scene", *scene);
```

---

## 9. 代码规范与 Code Review

### 命名约定

| 类型 | 规则 | 示例 |
|------|------|------|
| 类/结构体 | PascalCase | `VertexArray`, `ShaderLibrary` |
| 方法 | PascalCase | `OnUpdate`, `GetViewMatrix` |
| 成员变量 | `m_` 前缀 | `m_Window`, `m_Running` |
| 静态变量 | `s_` 前缀 | `s_Instance` |
| 枚举值 | PascalCase | `AABB`, `WindowClose` |

### 关键约定

| 规则 | 说明 |
|------|------|
| 智能指针 | `Ref<T>` = shared_ptr, `Scope<T>` = unique_ptr |
| 日志 | `CORE_*` 宏，**禁止** `std::cout/cerr` |
| 断言 | `CORE_ASSERT(cond, msg)` |
| 时间 | 统一用 `Timestep` 传参 |
| GL 资源 | ID 初始化为 0，析构前 `if (m_ID) glDelete*()` |
| 拷贝控制 | 持有 GL 资源的类 `=delete` 拷贝，实现移动语义 |
| 跨平台 | `strncpy` 替代 `strcpy_s`；`#ifdef _WIN32` 条件选择 |
| 宏 | `constexpr` 替代 `#define` |
| 异常 | 禁止空 `catch(...)`，必须 `CORE_ERROR` 日志 |
| #include 顺序 | 对应 .h → 项目内 → 第三方 → 标准库，组间空行 |

### Code Review 检查清单

**P0 (致命, 必须修复)**:
- [ ] GL 资源 ID 初始化为 0，析构安全检查
- [ ] Shader 编译/链接错误正确传播 (返回 bool)
- [ ] 后处理 Pass 保存/恢复 GL 状态 (GLStateSaver)
- [ ] 场景 FBO 使用 RGBA16F (HDR)，输出 FBO 用 RGBA8
- [ ] 无平台专有 API (`strcpy_s`/`localtime_s` 裸用)
- [ ] 无成员变量与类同名 (GCC 名称遮蔽报错)

**P1 (重要, 建议修复)**:
- [ ] 无 `std::cout/cerr`，全部 CORE_* 宏
- [ ] 相机输入用轮询 (不与 ImGui 回调冲突)
- [ ] 格式化字符串类型匹配 (%ld vs %lld)
- [ ] 投影矩阵宽高比动态获取 (不硬编码 16:9)
- [ ] 函数职责单一 (SetClearColor 和 Clear 分离)

**P2 (建议, 可合并后修)**:
- [ ] 渲染循环内无字符串拼接 (预缓存 uniform 名称)
- [ ] 未绕过引擎抽象层 (避免直接调 glVertexAttribPointer)

### 代码中 TODO 标记

搜索 `// TODO(code-review)` 可找到已知技术债:
- `engine/resource/Model.cpp` — VAO 抽象层绕过
- `CMakeLists.txt` — GLOB_RECURSE → 显式文件列表
- `engine/scene/Entity.h` — 胖实体 → ECS (entt)
- `engine/renderer/` — SSAO 集成到管线

---

## 10. CI/CD 与 PR 流程

### GitHub Actions

提交 PR 到 `dev`/`main` 时自动触发 (`.github/workflows/pr-checks.yml`):

| Job | 平台 | 内容 |
|-----|------|------|
| `build-windows` | Windows 2022 | MinGW 编译 + 运行测试 |
| `format-check` | Ubuntu 22.04 | clang-format 检查 (仅报告) |

### PR 提交规范

```
1. git checkout -b feature/xxx dev
2. 编码 + 本地测试: cmake --build build && cd build && ./Tests
3. 提交 (Conventional Commits):
   feat:     新功能
   fix:      修复
   docs:     文档
   refactor: 重构
   chore:    杂项
4. git push origin feature/xxx → 创建 PR
5. 等待 CI 通过 → Code Review → 合并到 dev
```

### 第三方库版本

| 库 | 版本 | 用途 | 集成 |
|----|------|------|------|
| GLFW | 3.4 | 窗口/输入/GL上下文 | 预编译(Win) / 系统包(Linux) |
| GLAD | 4.6 | OpenGL 加载 | 源文件编译 |
| GLM | 0.9.9 | 数学库 | header-only |
| Assimp | 6.x | 模型导入 | 预编译(Win) / 系统包(Linux) |
| stb_image | — | 图片加载 | header-only |
| ImGui | master | 编辑器UI | 源文件编译 |

---

## 11. 能力评估与路线图

### 当前能力评估 (v2.1)

| 维度 | 评分 | 说明 |
|------|------|------|
| PBR 渲染质量 | ⭐⭐⭐⭐⭐ | Cook-Torrance + CSM + IBL + HDR 后处理 |
| 编辑器工具 | ⭐⭐⭐⭐ | ImGui 7+ 面板 + Gizmo + Prefab + Play/Stop |
| 代码架构 | ⭐⭐⭐⭐ | 分层清晰，组件模式，RAII 安全 |
| 阴影/光照 | ⭐⭐⭐⭐ | CSM 级联 + 多光源 + IBL |
| 物理系统 | ⭐⭐⭐ | AABB/Sphere 碰撞 + 弹性响应 + Raycast |
| 脚本系统 | ⭐⭐⭐ | C++ Lambda，缺 Lua/热重载 |
| 粒子系统 | ⭐⭐⭐ | CPU Billboard，功能完整 |
| 音频系统 | ⭐ | Stub，SoLoud 未集成 |
| 动画系统 | ⭐ | 仅静态网格，无骨骼/蒙皮 |
| 性能优化 | ⭐⭐⭐ | 视锥体剔除 + LOD，无多线程 |
| 跨平台 | ⭐⭐⭐ | Windows + Linux 验证 |

**综合: 3.2/5** — 功能完整的轻量渲染引擎。

### v2.1 已修复的关键问题

35 项代码质量问题已全部修复：
- GL 资源安全初始化 (ID=0 + 析构检查)
- Shader 错误传播 + Poisson Disk PCF
- HDR 后处理管线 (RGBA16F)
- Layer unique_ptr 所有权迁移
- 跨平台兼容 (Windows + Linux CI)
- Camera 动态宽高比 + 轮询输入
- GLStateSaver RAII
- Buffer 拷贝控制 (禁止拷贝 + 移动语义)

### v3.0+ 计划

| 优先级 | 功能 | 预估 |
|--------|------|------|
| P0 | 骨骼动画 (GPU Skinning) | 4-6 周 |
| P0 | Lua 脚本绑定 | 2-3 周 |
| P1 | 抗锯齿 (MSAA/FXAA) | 3-5 天 |
| P1 | SSAO 集成到管线 | 1 周 |
| P1 | ECS 架构迁移 (entt) | 3-4 周 |
| P1 | SoLoud 音频集成 | 1-2 周 |
| P2 | 延迟渲染管线 | 3-4 周 |
| P2 | 资源打包 (.pak) | 2-3 周 |
| P3 | 多线程渲染 | 3-4 周 |

### 技术债

| 项目 | 优先级 |
|------|--------|
| 单例线程安全 (ShaderLibrary/TextureLibrary) | 中 |
| CMake GLOB_RECURSE → 显式文件列表 | 低 |
| Event.h 按事件类型拆分 | 低 |
| 级联 uniform 名称预缓存 | 中 |
| Model.cpp 绕过 VAO 抽象层 | 中 |

### 性能基准 (参考)

| 指标 | 当前值 |
|------|--------|
| FPS (~50 实体, GTX 1060) | ~200+ |
| DrawCalls/帧 | ~80 |
| 启动内存 | ~150 MB |

---

## 12. 调试技巧

### OpenGL 错误检查

```cpp
GLenum err;
while ((err = glGetError()) != GL_NO_ERROR)
    CORE_ERROR("GL Error: 0x{:x}", err);
```

推荐工具: **RenderDoc** (抓帧分析 DrawCall) · **Nsight Graphics** · **Valgrind** (内存泄漏)

### 常见崩溃排查

| 现象 | 可能原因 | 检查 |
|------|----------|------|
| `glDrawElements` 崩溃 | VAO 未正确设置/已销毁 | VertexArray 生命周期 |
| 启动黑屏 | Shader 编译失败静默 | 控制台 Shader 日志 |
| FBO 渲染异常 | 颜色格式不匹配 | 场景 FBO 应为 RGBA16F |
| 实体不显示 | 被视锥体剔除 | Position 是否在视锥内 |
| ImGui 崩溃 | 回调冲突 | ImGui 初始化顺序 |

### Shader 调试

- 临时改输出颜色: `FragColor = vec4(albedo, 1.0);`
- 编译错误自动打印到控制台
- 热重载: `ShaderLibrary::Instance().ReloadAll();`

### 性能分析

查看 Stats 面板 (右上角: FPS/DrawCalls/Triangles) 或手动计时:
```cpp
auto start = std::chrono::high_resolution_clock::now();
// ... code ...
auto end = std::chrono::high_resolution_clock::now();
CORE_INFO("Took {}ms", duration_cast<milliseconds>(end - start).count());
```

---

## 13. 速查表 Cheat Sheet

### 常用类快速索引

| 你想... | 用这个 | 位置 |
|---------|--------|------|
| 创建窗口 | `Window` | `engine/core/Window.h` |
| 读取键盘/鼠标 | `Input` | `engine/core/Input.h` |
| 处理事件 | `EventDispatcher` | `engine/core/Event.h` |
| 打日志 | `CORE_*` 宏 | `engine/core/Log.h` |
| 编译着色器 | `Shader::Create(vs, fs)` | `engine/renderer/Shader.h` |
| 加载纹理 | `TextureLibrary::Instance().Load2D()` | `engine/resource/TextureLibrary.h` |
| 创建 FBO | `Framebuffer::Create(spec)` | `engine/renderer/Framebuffer.h` |
| 内置几何体 | `MeshLibrary::GetCube()` | `engine/resource/MeshLibrary.h` |
| 加载模型 | `Model::Load(path)` | `engine/resource/Model.h` |
| 创建材质 | `Material::Create(shader)` | `engine/renderer/Material.h` |
| 添加光源 | `LightEnvironment::AddPointLight()` | `engine/renderer/Light.h` |
| 创建实体 | `Scene::CreateEntity(tag)` | `engine/scene/Scene.h` |
| 添加脚本 | `Entity::GetScript()` | `engine/scene/ScriptComponent.h` |
| 添加物理 | `Entity::AddPhysics()` | `engine/physics/PhysicsWorld.h` |
| 添加粒子 | `Entity::AddParticleEmitter()` | `engine/particle/ParticleEmitter.h` |
| Bloom | `PostProcessPipeline` | `engine/postprocess/PostProcess.h` |
| 编辑器 UI | `EditorLayer` | `engine/editor/EditorLayer.h` |

### 快捷键

| 快捷键 | 功能 |
|--------|------|
| `右键拖拽` | 旋转相机视角 |
| `中键拖拽` | 平移相机 |
| `WASD` | 移动相机 |
| `Ctrl/Space` | 下降/上升 |
| `F3` | 切换线框模式 |
| `F5` | Play / 施加冲量 |
| `W/E/R` | Gizmo 平移/旋转/缩放 |
| `Esc` | 退出 |

### 文件路径约定

```
引擎代码: engine/<子系统>/*.h, *.cpp
应用代码: app/*.cpp
测试代码: tests/*.cpp
着色器:   shader/*.vert, *.frag
资源文件: resources/models/, resources/textures/
第三方:   external/, include/, lib/
```

---

*引擎是活的有机体，本文档随项目持续更新。*
