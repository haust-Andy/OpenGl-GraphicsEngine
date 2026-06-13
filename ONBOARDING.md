# OpenGL Graphics Engine — 新员工交接手册

> **目标读者**: 新加入项目的中级/高级 C++ 图形程序员  
> **预计上手时间**: 3~5 天  
> **项目版本**: v2.1 | 更新日期: 2026-06-12  

---

## 目录

1. [第一天：环境搭建与第一次运行](#第一天环境搭建与第一次运行)
2. [项目全貌速览](#项目全貌速览)
3. [推荐阅读路线](#推荐阅读路线)
4. [核心子系统深度解读](#核心子系统深度解读)
5. [常见开发任务 (How-To)](#常见开发任务-how-to)
6. [调试技巧](#调试技巧)
7. [代码风格与团队规范](#代码风格与团队规范)
8. [CI/CD 与 PR 流程](#cicd-与-pr-流程)
9. [已知技术债与注意事项](#已知技术债与注意事项)
10. [速查表 (Cheat Sheet)](#速查表-cheat-sheet)

---

## 第一天：环境搭建与第一次运行

### 1.1 你需要安装什么

| 工具 | Windows | Linux (Ubuntu) |
|------|---------|----------------|
| CMake 3.16+ | [cmake.org](https://cmake.org) | `sudo apt install cmake` |
| 编译器 | MinGW-w64 或 MSVC | `g++` (系统自带) |
| Git | [git-scm.com](https://git-scm.com) | `sudo apt install git` |
| GPU 驱动 | 厂商驱动 | 开源 Mesa 或厂商驱动 |
| Linux 额外依赖 | — | `sudo apt install libglfw3-dev libassimp-dev xorg-dev libgl1-mesa-dev` |

### 1.2 克隆并编译

```bash
# 克隆项目
git clone git@github.com:haust-Andy/OpenGl-GraphicsEngine.git
cd OpenGl-GraphicsEngine
git checkout dev          # 开发分支

# 创建构建目录
mkdir build && cd build

# === Windows MinGW ===
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release -j 8
./Sandbox.exe

# === Windows MSVC ===
cmake ..
cmake --build . --config Release
.\Release\Sandbox.exe

# === Linux ===
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j $(nproc)
./Sandbox
```

### 1.3 运行测试

```bash
cd build
./Tests      # Linux
Tests.exe    # Windows
```

所有测试应全部通过。测试覆盖：Base 基础类型、Timestep、Log、LayerStack、Event、SceneSerializer。

### 1.4 跑起来之后你应该看到什么

一个窗口（1600×900），包含：
- **中间视口** — 左边两个立方体 + 右边金属球 + 6 个 PBR 球体阵列 + 地面 + 火焰粒子 + 旋转立方体
- **左侧面板** — Hierarchy 实体树
- **右侧面板** — Inspector 属性面板
- **下方面板** — Content Browser 文件管理
- **右上角** — Stats 面板 (FPS/DrawCalls)

**试试这些操作**：
- 右键拖拽 → 旋转视角
- WASD → 移动相机
- 滚轮 → 缩放
- `F5` → 给"金属球"一个向上冲量
- `F3` → 切换线框模式

---

## 项目全貌速览

### 一句话描述

这是一个**从 LearnOpenGL 教学项目改造而来的分层 3D 图形引擎**，C++17，OpenGL 3.3 Core Profile，具备 PBR 渲染、CSM 阴影、IBL 环境光、后处理管线、物理碰撞、粒子、脚本系统、游戏 UI 和 ImGui 编辑器。

### 技术栈

```
┌──────────────────────────────────────────────────────┐
│                    YOUR CODE                          │
│              app/SandboxApp.cpp                        │
├──────────────────────────────────────────────────────┤
│          OpenGlEngine (STATIC LIBRARY)                 │
│  ┌─────────┬──────────┬─────────┬──────────────────┐ │
│  │  core   │ renderer │  scene  │ postprocess       │ │
│  │ 编辑: editor │ 资源: resource │ physics/particle/  │ │
│  │          audio/ui │          │                   │ │
│  └─────────┴──────────┴─────────┴──────────────────┘ │
├──────────────────────────────────────────────────────┤
│  GLFW 3.4  │ GLAD 4.6  │ Assimp 6.x  │ ImGui        │
│  GLM 0.9.9 │ stb_image │ OpenGL 3.3  │              │
└──────────────────────────────────────────────────────┘
```

### 构建产物

| 目标 | 类型 | 说明 |
|------|------|------|
| `OpenGlEngine` | 静态库 `.a` | 引擎全部代码 + ImGui + GLAD |
| `Sandbox` | 可执行文件 | 演示程序，链接 OpenGlEngine |
| `Tests` | 可执行文件 | 单元测试，链接 OpenGlEngine |

### 关键数字

- **代码量**: ~77 个源文件, ~45 个头文件, ~6000+ 行 C++
- **Shader**: 17 个 GLSL 文件
- **子系统**: 11 个功能模块 (core / renderer / resource / scene / postprocess / editor / physics / particle / audio / ui / tests)
- **组件类型**: Transform / Mesh / Light / Script / Collider / Rigidbody / AudioSource / AudioListener / Particle / LOD — 共 10 种

---

## 推荐阅读路线

**不要试图一次性理解所有文件。** 按以下顺序阅读，每天 2-3 小时，3 天可上手。

### Day 1 (下午): 核心流水线

阅读顺序从底层到上层：

```
1. engine/core/Base.h              ← 了解 Ref<>/Scope<> 别名
2. engine/core/Timestep.h          ← 帧时间步 (GetSeconds/Clamp)
3. engine/core/Log.h               ← CORE_TRACE/INFO/WARN/ERROR 宏
4. engine/renderer/Shader.h        ← Shader 编译/缓存 Uniform/热重载
5. engine/renderer/Buffer.h        ← VBO/IBO (禁止拷贝/移动语义)
6. engine/renderer/VertexArray.h   ← VAO 封装 (AddVertexBuffer)
7. engine/renderer/Framebuffer.h   ← FBO 封装 (RGBA16F)
8. engine/renderer/Renderer.h      ← Begin/EndFrame/Submit
9. engine/renderer/RendererAPI.h   ← OpenGL 后端
```

**关键理解**: `Renderer` 是一个门面 (Facade)，委托给 `RendererAPI`（当前只有 OpenGL 实现）。

### Day 2 (上午): 应用框架

```
10. engine/core/Event.h            ← EventDispatcher / 事件类型体系
11. engine/core/Input.h            ← 轮询式输入 (GetMousePosition)
12. engine/core/Window.h           ← GLFW 窗口 → 引擎事件映射
13. engine/core/Layer.h            ← Layer 基类 + LayerStack
14. engine/core/Application.h      ← 主循环 Run() / 事件分发
15. engine/core/EntryPoint.h       ← CreateApplication → main()
```

**关键理解**: Application 是控制反转 (IoC) 核心。你实现 `CreateApplication()`，Application 在其 `Run()` 中驱动一切。

### Day 2 (下午): 场景系统 & 渲染

```
16. engine/scene/TransformComponent.h   ← 位置/旋转/缩放/世界矩阵
17. engine/scene/MeshComponent.h        ← VAO + Material 引用
18. engine/scene/ScriptComponent.h      ← 脚本组件
19. engine/scene/Scene.h                ← Entity CRUD + OnRender/OnUpdate
20. engine/renderer/Camera.h            ← FPS 相机/欧拉角
21. engine/renderer/Material.h          ← PBR 材质属性
22. engine/renderer/Light.h             ← 多光源 + LightEnvironment
```

**关键理解**: Entity 是"胖实体"模式 (非纯 ECS)。每个 Entity 包含所有组件的联合体 + `Has*` 标志位。这在 <1000 实体的场景下工作良好。

### Day 3 (上午): 渲染管线深度

```
23. engine/renderer/PBR流程            ← shader/pbr.vert + pbr.frag
24. engine/renderer/ShadowMap.h        ← CSM 级联阴影 + PCF
25. engine/renderer/IBL.h              ← IBL 辐照度/预过滤/BRDF LUT
26. engine/renderer/Frustum.h          ← 视锥体 6 平面提取
27. engine/renderer/LOD.h              ← 距离/屏幕占比 LOD
28. engine/postprocess/PostProcess.h   ← Bloom + ToneMapping
29. app/SandboxApp.cpp                 ← ★ 完整阅读 (614 行)
```

**读完 SandboxApp.cpp 你就对一切有了整体理解。** 它是引擎所有功能的"集成测试"。

### Day 3 (下午): 编辑器 & 其他子系统

```
30. engine/editor/EditorLayer.h        ← ImGui 面板架构
31. engine/physics/PhysicsWorld.h      ← AABB 碰撞检测
32. engine/particle/ParticleEmitter.h  ← CPU 粒子系统
33. engine/ui/UIElements.h             ← SpriteBatch UI
```

---

## 核心子系统深度解读

### 1. 事件系统 (engine/core/Event.h)

**设计**: 静态多态 (CRTP 变体)，非虚函数分发。

```
Event (基类, EventType 枚举)
├── KeyEvent       ← KeyPressed / KeyReleased / KeyTyped
├── MouseEvent     ← ButtonPressed / ButtonReleased / Moved / Scrolled
└── WindowEvent    ← Resize / Close / Focus / LostFocus
```

**事件分发的控制流**:

```
GLFW Callback (Window.cpp)
    → 构造引擎 Event 对象
    → m_Data.EventCallback(event)
        → Application::OnEvent(event)
            → EventDispatcher(event).Dispatch<T>(handler)
                → 按 EventType 匹配，调用 handler(T&)
            → LayerStack (从顶层到下层反向遍历)
                → EditorLayer::OnEvent(event)  [先处理]
                → SandboxLayer::OnEvent(event)  [后处理]
```

**⚠️ 重要陷阱**: ImGui 的 `ImGui_ImplGlfw_InitForOpenGL(window, true)` 第二个参数 `true` 会安装 GLFW 回调链，**消费鼠标事件**。因此相机旋转/平移不能依赖 `MouseMovedEvent` 回调，改用 `Input::GetMousePosition()` 轮询。详见 `SandboxApp::UpdateCameraRotation()`。

### 2. Layer Stack 与 Application 生命周期

```
main()
  → CreateApplication()           ← 由 app/SandboxApp.cpp 实现
    → new Application(name, W, H) ← 构造: 创建 Window + 初始化 ImGui
    → PushLayer(make_unique<SandboxLayer>())
    → Run()                       ← ★ 主循环
```

**Run() 内部** (每帧):
```cpp
while (m_Running) {
    ts = Clamp(now - lastTime);           // 1. 计算时间步
  
    for (layer : m_LayerStack)
        layer->OnUpdate(ts);             // 2. 逻辑更新
  
    ImGui_ImplOpenGL3_NewFrame();         // 3. ImGui 帧开始
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  
    for (layer : m_LayerStack)
        layer->OnImGuiRender();           // 4. ImGui 绘制
  
    ImGui::Render();                      // 5. ImGui 提交
    ImGui_ImplOpenGL3_RenderDrawData();
  
    m_Window->OnUpdate();                 // 6. SwapBuffers + PollEvents
}
```

**Layer 层级关系**:
- `SandboxLayer` (Normal): 游戏逻辑、场景初始化、渲染管线
- `EditorLayer` (Overlay): ImGui 编辑器面板

Overlay 在 Normal 之上渲染，事件优先处理。

### 3. PBR 渲染管线

**数据流** (从 CPU 到 GPU):

```
CPU 侧:
  Material::Bind()
    ├── Shader::Bind()
    ├── Shader::SetInt("u_AlbedoMap", 0)
    ├── Texture::Bind(slot) ← AlbedoMap, NormalMap 等
    ├── Shader::SetFloat3("u_Albedo", ...)
    └── Shader::SetFloat("u_Metallic", ...)

  Renderer::BeginScene(view, proj)
    ├── CameraUBO → binding point 0
    └── LightUBO → binding point 1

  ShadowMap::Bind()
    └── CSM 级联矩阵 + 阴影贴图数组 → binding point 2

  IBL::Bind()
    ├── Irradiance Map → slot 7
    ├── Prefiltered Map → slot 8
    └── BRDF LUT → slot 9

GPU 侧 (pbr.frag):
  1. 材质参数采样 (Albedo/Normal/Metallic/Roughness/AO/Emission)
  2. Direct Lighting:
     - Directional Light: Cook-Torrance BRDF
     - Point/Spot Light: 带衰减的 BRDF
     - Shadow: 从 CSM 级联贴图采样 × Poisson Disk PCF
  3. IBL:
     - Diffuse: texture(u_IrradianceMap, N)
     - Specular: textureLod(u_PrefilteredMap, R, roughness*4) × BRDF_LUT
  4. 输出: 线性 HDR 值 (不做 ToneMapping，留给后处理)
```

### 4. CSM 级联阴影

```
ShadowMap (2048×2048, 3 cascades)
├── cascade[0]: 近处 (0%~33% camera range) → 512×2048 区域
├── cascade[1]: 中距 (33%~66%)              → 512×2048 区域
└── cascade[2]: 远处 (66%~100%)             → 1024×2048 区域

每帧:
  CalculateCascades(lightDir, view, proj, near, far)
    → 3 组 LightSpace VP 矩阵
    → CameraUBO 中传递 u_CascadeSplits + u_LightSpaceMatrices[3]

  RenderShadowPass(shadowCasters)
    → 对每个 cascade:
      - glViewport(cascade 区域)
      - 绑定 ShadowMap::GetDepthFBO()
      - 渲染所有 CastShadow=true 的实体

  PBR Shader 中:
    - 根据片元深度选择 cascade
    - Poisson Disk 32 样本 PCF
    - 法线偏移防止 shadow acne
```

### 5. 资源管理系统

```
ShaderLibrary (单例)
  └── unordered_map<string, shared_ptr<Shader>>
      ├── Add("pbr", pbrShader)
      ├── Get("pbr")
      └── ReloadAll()  ← 运行时热重载

TextureLibrary (单例)
  └── unordered_map<string, shared_ptr<Texture>>
      ├── Load2D("path.png")
      ├── LoadCube({6 faces})
      └── LoadHDR("path.hdr")

MeshLibrary (静态工厂)
  └── 内置几何体:
      ├── GetCube()         → 1×1×1 立方体 VAO
      ├── GetSphere(seg)    → 经纬球 VAO
      ├── GetPlane()        → 10×10 平面 VAO
      └── GetScreenQuad()   → NDC 全屏四边形 VAO

Model (Assimp 导入)
  ├── Load("model.fbx") → vector<SubMesh>
  └── SubMesh: VAO + MaterialPtr + name
```

**⚠️ 已知问题**: ShaderLibrary/TextureLibrary 单例非线程安全，当前在单线程中使用无问题，未来多线程加载需加锁。

### 6. Scene 与 Entity

```
Scene
├── vector<Entity> m_Entities
├── unordered_map<uint32_t, uint32_t> m_EntityMap  (ID → index)
│
├── LightEnvironment m_LightEnv        ← 光照数据
├── shared_ptr<ShadowMap>               ← 阴影系统
├── shared_ptr<IBL>                     ← 环境光照
├── unique_ptr<PhysicsWorld>            ← 物理世界
│
├── CreateEntity(tag) → Entity*
├── DestroyEntity(entity)
├── FindEntity(tag) → Entity*
├── ForEachEntity(callback)
│
├── OnUpdate(ts)             ← 脚本 + 物理 + 粒子
└── OnRender(camera)         ← 收集光源 → 排序 → 剔除 → LOD → 渲染

Entity
├── uint32_t ID + string Tag
├── TransformComponent         [必有]
├── MeshComponent              [必有]
├── LightComponent             [可选, HasLight]
├── ScriptComponent            [可选, HasScript]
├── ColliderComponent + RigidbodyComponent  [可选, HasPhysics]
├── AudioSource/AudioListener  [可选, HasAudioSource/HasAudioListener]
├── ParticleEmitter (unique_ptr) [可选, HasParticleEmitter]
├── LODComponent               [可选, HasLOD]
└── uint32_t ParentID          [可选, 层级]
```

### 7. 后处理管线

```
PostProcessPipeline(width, height)
  ├── BloomPass
  │   ├── Step 1: BrightnessPass  (提取 HDR 亮部 → downsampled FBO)
  │   ├── Step 2: BlurH           (水平高斯模糊)
  │   ├── Step 3: BlurV           (垂直高斯模糊)
  │   └── Step 4: Combine         (原图 + Bloom)
  ├── ToneMappingPass
  │   └── ACES 色调映射 + Gamma 校正 (HDR → LDR)
  ├── GLStateSaver (RAII)
  │   └── 每个 Pass 自动保存/恢复: Depth Test / Blend / CullFace
  └── Execute(inputFBO) → finalFBO
```

**数据格式**:
- 场景 FBO: `GL_RGBA16F` (HDR 线性空间)
- 后处理中间 FBO: 同格式
- 输出: `GL_RGBA8` (LDR sRGB)

### 8. 编辑器的面板架构

EditorLayer 的 `OnImGuiRender()` 是一个巨大的 ImGui 函数，按以下顺序绘制：

```
1. DrawMenuBar()             ← File | View | Play | Gizmo
2. DockSpace::Begin()         ← ImGui Docking 布局
3. DrawViewport()             ← 中间视口 (FBO→ImGui::Image)
4. DrawSceneHierarchy()       ← 左侧实体树
5. DrawInspector()            ← 右侧属性
6. DrawContentBrowser()       ← 下方文件浏览器
7. DrawPrefabPanel()          ← Prefab 面板
8. DrawStatsPanel()           ← 渲染统计
```

### 9. 物理系统

自有实现 (非第三方物理引擎):

```
PhysicsWorld::Step(dt)
  ├── 遍历所有 HasPhysics 实体
  ├── 每个 Rigidbody::Update(dt):
  │   ├── 应用重力: velocity += gravity * dt
  │   ├── 应用外力: velocity += force / mass * dt
  │   └── 更新位置: position += velocity * dt
  ├── 碰撞检测 (O(n²), 简单场景足够):
  │   ├── AABB-AABB: 分离轴测试
  │   ├── Sphere-Sphere: 圆心距离
  │   └── AABB-Sphere: 最近点距离
  └── 碰撞响应:
      ├── 沿法线分离
      ├── 速度反射 × Restitution
      └── 触发 OnCollisionEnter/Stay/Exit 回调
```

**⚠️ 注意**: 未使用空间分区 (BVH/Octree)，场景实体超过 200 时需优化。

---

## 常见开发任务 (How-To)

### 添加新光源类型

1. 在 `engine/renderer/Light.h` 中定义 `LightUBO` 结构体
2. 在 PBR Shader 中声明对应 uniform
3. 在 `LightEnvironment::Bind()` 中更新 UBO
4. 在 `pbr.frag` 中实现 BRDF 计算

### 添加新的后处理效果

1. 编写 GLSL 着色器 → 放入 `shader/` 目录
2. 在 `engine/postprocess/PostProcess.cpp` 中创建新 Pass
3. 注册到 `PostProcessPipeline::Execute()` 管线
4. 确保使用 `GLStateSaver` RAII 保护 GL 状态

### 添加新 Entity 组件

1. 在 `engine/scene/` 中创建新组件头文件
2. 在 `Entity.h` 中添加组件成员 + `Has*` 标志位
3. 在 `Scene::OnUpdate()` 中处理组件逻辑
4. 在 `EditorLayer::DrawInspector()` 中添加编辑器 UI

### 加载自定义 3D 模型

```cpp
// 方法 1: 使用 Model (Assimp)
auto model = std::make_shared<Model>();
model->Load("resources/models/my_model.fbx");
for (auto& subMesh : model->GetSubMeshes()) {
    auto* entity = scene->CreateEntity(subMesh.Name);
    entity->GetMesh().SetMesh(subMesh.VAO);
    entity->GetMesh().SetMaterial(Material::Create(pbrShader));
}

// 方法 2: 使用内置几何体 + 自定义顶点
auto cubeVAO = MeshLibrary::GetCube().VAO;
auto* entity = scene->CreateEntity("MyCube");
entity->GetMesh().SetMesh(cubeVAO);
entity->GetMesh().SetMaterial(material);
```

### 添加热键

```cpp
// 在 SandboxLayer::HandleInput() 中:
if (Input::IsKeyPressed(Key::F6))
    DoSomething();

// 或通过事件 (适合单次触发):
EventDispatcher dispatcher(event);
dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
    if (e.GetKeyCode() == Key::F6 && e.GetRepeatCount() == 0) {
        DoSomething();
        return true;
    }
    return false;
});
```

### 保存/加载场景

```cpp
// 保存
SceneSerializer::Save(*scene, "my_scene.scene");

// 加载
auto scene = Scene::Create("Loaded");
SceneSerializer::Load("my_scene.scene", *scene);
```

---

## 调试技巧

### 1. OpenGL 错误检查

```cpp
// 方法 1: 手动检查 (已有模板在 SandboxApp.cpp 末尾)
GLenum err;
while ((err = glGetError()) != GL_NO_ERROR)
    CORE_ERROR("GL Error: 0x{:x}", err);

// 方法 2: 条件断点
// 在 RendererAPI::DrawIndexed 中设置条件: glGetError() != GL_NO_ERROR

// 方法 3: RenderDoc 抓帧
// 启动 Sandbox 后，用 RenderDoc 注入，捕获一帧分析 DrawCall
```

### 2. 性能分析

```cpp
// 查看 Stats 面板 (右上角)
// FPS / DrawCalls / Triangles / Vertices

// 手动计时代码
auto start = std::chrono::high_resolution_clock::now();
// ... your code ...
auto end = std::chrono::high_resolution_clock::now();
CORE_INFO("Operation took {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
```

### 3. Shader 调试

```cpp
// 方法 1: 输出颜色值观察
// 在 pbr.frag 中临时改: FragColor = vec4(albedo, 1.0);

// 方法 2: 着色器编译错误位置
// Shader::Create() 会打印编译错误日志到控制台

// 方法 3: 热重载
// 修改 shader 文件后，程序内调用:
ShaderLibrary::Instance().ReloadAll();
```

### 4. 常见崩溃原因

| 现象 | 可能原因 | 检查 |
|------|----------|------|
| `glDrawElements` 崩溃 | VAO 未正确设置或已被销毁 | 检查 VertexArray 生命周期 |
| 启动黑屏 | Shader 编译失败静默 | 检查控制台 Shader 编译日志 |
| FBO 渲染异常 | 颜色格式不匹配 | 场景 FBO 应使用 `GL_RGBA16F` |
| 实体不显示 | 被视锥体剔除 | 检查 Entity Position 是否在相机视锥内 |
| ImGui 崩溃 | 版本不兼容或回调冲突 | 检查 ImGui 初始化顺序 |

### 5. 推荐的调试工具

- **RenderDoc**: 图形调试利器，抓帧分析 DrawCall 和纹理
- **Nsight Graphics**: NVIDIA GPU 调试
- **GDB / Visual Studio Debugger**: 标准 C++ 调试
- **Valgrind** (Linux): 内存泄漏检测

---

## 代码风格与团队规范

### 命名约定

```
类/结构体:    PascalCase    Application, VertexArray, ShaderLibrary
函数/方法:    PascalCase    OnUpdate, GetViewMatrix, CreateEntity
成员变量:     m_前缀         m_Window, m_Running, m_Entities
静态变量:     s_前缀         s_Instance
全局常量:     k_前缀或全大写  kMaxLights / MAX_CASCADES
枚举值:       直接 PascalCase  AABB, Forward, WindowClose
类型别名:     CamelCase     uint32, RendererID
```

### 代码组织

```
#include 顺序:
  1. 对应的 .h (如果当前是 .cpp)
  2. 项目内头文件 (engine/... , app/...)
  3. 第三方库 (imgui.h, glm/..., glad/...)
  4. 标准库 (<memory>, <vector>, ...)

每组之间空行分隔
```

### 关键约定

| 规则 | 说明 |
|------|------|
| 智能指针 | `Ref<T>` = shared_ptr, `Scope<T>` = unique_ptr, 对应 `CreateRef` / `CreateScope` |
| 日志 | 用 `CORE_TRACE/INFO/WARN/ERROR/CRITICAL` 宏, **禁止** `std::cout/cerr` |
| 断言 | `CORE_ASSERT(cond, msg)` 检查前置条件 |
| 时间 | 统一用 `Timestep` 传参，不用裸 `float` |
| GL 资源 | ID 初始化为 `0`，析构前 `if (m_ID) glDelete*()` |
| 拷贝控制 | 持有 GL 资源的类必须 `=delete` 拷贝构造/赋值 |
| 跨平台 | JSON style: `strncpy` 替代 `strcpy_s`; 时间: `#ifdef _WIN32` 条件选择 |
| 宏最小化 | 用 `constexpr` 替代 `#define` 常量 |

### 完整 Code Review 检查清单

详见 `CODE_REVIEW_SPEC.md`。快速摘要：

**P0 (致命, 必须修复)**:
- [ ] GL 资源 ID 初始化为 0, 析构安全检查
- [ ] Shader 编译/链接错误正确传播
- [ ] 无平台专有 API
- [ ] 无成员变量与类同名

**P1 (重要, 建议修复)**:
- [ ] 无 std::cout/cerr, 全部用 CORE_* 宏
- [ ] 相机输入轮询 (不与 ImGui 回调冲突)
- [ ] 格式化字符串类型匹配 (%ld vs %lld)

**P2 (建议, 可合并后修)**:
- [ ] 渲染循环无字符串拼接
- [ ] 未绕过引擎抽象层 (如直接调 glVertexAttribPointer)

---

## CI/CD 与 PR 流程

### GitHub Actions

提交 PR 到 `dev` 或 `main` 分支时自动触发 (`.github/workflows/pr-checks.yml`):

| Job | 平台 | 内容 |
|-----|------|------|
| `build-linux` | Ubuntu 22.04 | 安装依赖 → CMake 配置 → 编译 → 运行测试 |
| `build-windows` | Windows 2022 | MinGW CMake 配置 → 编译 → 运行测试 |
| `format-check` | Ubuntu 22.04 | clang-format 检查 (仅报告, 不阻断) |

### PR 提交规范

```
1. 从 dev 分支创建 feature/fix 分支
   git checkout -b feature/my-xxx dev

2. 编码 + 本地编译测试
   cmake --build build --config Release
   cd build && ./Tests

3. 提交 (遵循 Conventional Commits)
   feat: 添加 SSAO 后处理集成
   fix: 修复 LOD 切换时 VAO 状态泄漏
   docs: 更新 README 构建说明
   refactor: 将 Model 加载改为流式

4. 推送到 GitHub 并创建 PR
   git push origin feature/my-xxx

5. 等待 CI 全部通过

6. Code Review → 合并到 dev
```

### PR 模板要素

每个 PR 描述应包含：
- **做了什么** (What)
- **为什么这么做** (Why)
- **测试方法** (How to test)
- **风险点** (Risks)
- 如果有 UI 变更，附带截图

---

## 已知技术债与注意事项

### 需要了解的工程设计决策

| # | 决策 | 原因 | 未来计划 |
|---|------|------|----------|
| 1 | Entity 是胖实体 (非纯 ECS) | 简单直观，适合 <1000 实体的场景 | 可迁移到 entt |
| 2 | CMake 用 GLOB_RECURSE | 方便快速添加文件 | 应改为显式源文件列表 |
| 3 | Model.cpp 绕过 VAO 抽象层 | 简化 Assimp 导入流程 | 应重构为重入 VAO 创建流程 |
| 4 | ShaderLibrary 单例非线程安全 | 当前无多线程加载需求 | 需加锁或改为 DI |
| 5 | 物理系统无空间分区 | 场景实体少，O(n²) 足够 | 超过 200 实体需 BVH |
| 6 | 相机输入使用轮询 | ImGui 回调链会消费鼠标事件 | 考虑改用无冲突方案 |
| 7 | 音频模块是 Stub | SoLoud 未实际集成 | 需要时可快速集成 |

### 代码中的 TODO 标记

搜索 `TODO` 关键字可找到所有待处理项。关键的有：
- `engine/resource/Model.cpp`: VAO 抽象层绕过
- `engine/renderer/`: SSAO 集成到管线
- `CMakeLists.txt`: GLOB_RECURSE → 显式文件列表

### 性能基准 (参考)

| 指标 | 当前值 | 说明 |
|------|--------|------|
| Benchmark 场景 FPS | ~200+ | 桌面 GTX 1060 级别 GPU, ~50 实体 |
| DrawCalls/帧 | ~80 | 含阴影 Pass 的额外 DrawCall |
| 启动内存 | ~150 MB | 含 ImGui + 纹理加载 |

---

## 速查表 (Cheat Sheet)

### 常用类快速索引

| 你想... | 用这个类 | 文件位置 |
|---------|---------|---------|
| 创建窗口 | `Window` | `engine/core/Window.h` |
| 读取键盘/鼠标 | `Input` | `engine/core/Input.h` |
| 处理事件 | `EventDispatcher` | `engine/core/Event.h` |
| 打日志 | `CORE_*` 宏 | `engine/core/Log.h` |
| 编译着色器 | `Shader::Create(vs, fs)` | `engine/renderer/Shader.h` |
| 加载纹理 | `TextureLibrary::Instance().Load2D()` | `engine/resource/TextureLibrary.h` |
| 创建 FBO | `Framebuffer::Create(spec)` | `engine/renderer/Framebuffer.h` |
| 创建几何体 | `MeshLibrary::GetCube()` | `engine/resource/MeshLibrary.h` |
| 加载模型 | `Model::Load(path)` | `engine/resource/Model.h` |
| 创建材质 | `Material::Create(shader)` | `engine/renderer/Material.h` |
| 添加光源 | `LightEnvironment::AddPointLight()` | `engine/renderer/Light.h` |
| 创建实体 | `Scene::CreateEntity(tag)` | `engine/scene/Scene.h` |
| 添加脚本 | `Entity::GetScript()` | `engine/scene/ScriptComponent.h` |
| 添加物理 | `Entity::AddPhysics(collider, rb)` | `engine/physics/PhysicsWorld.h` |
| 添加粒子 | `Entity::AddParticleEmitter(config)` | `engine/particle/ParticleEmitter.h` |
| Bloom 效果 | `PostProcessPipeline` | `engine/postprocess/PostProcess.h` |
| 编辑器 UI | `EditorLayer` | `engine/editor/EditorLayer.h` |

### 常用快捷键

| 快捷键 | 功能 |
|--------|------|
| `右键拖拽` | 旋转相机视角 |
| `中键拖拽` | 平移相机 |
| `WASD` | 移动相机 (需视口聚焦或右键按住) |
| `Ctrl/Space` | 下降/上升 |
| `F3` | 切换线框模式 |
| `F5` | Play 模式 / 给金属球冲量 |
| `W` (Gizmo) | 平移模式 |
| `E` (Gizmo) | 旋转模式 |
| `R` (Gizmo) | 缩放模式 |
| `Esc` | 退出程序 |

### 文件路径约定

```
引擎代码:    engine/<子系统>/*.h, *.cpp
应用代码:    app/*.cpp
测试代码:    tests/*.cpp
着色器:      shader/*.vert, *.frag
模型/纹理:   resources/models/, resources/textures/
构建产物:    build/
第三方库:    external/, include/, lib/
```

---

## 附录：进一步学习

- **ARCHITECTURE.md** — 完整架构文档，含子系统详解和数据流图
- **CODE_REVIEW_SPEC.md** — Code Review 规范与检查清单
- **FIX_REPORT.md** — 代码质量修复历史
- **todo.md** — 开发路线图与技术债
- **CMakeLists.txt** — 构建系统配置
- **.github/workflows/pr-checks.yml** — CI/CD 工作流

---

*如果有任何问题，优先搜索代码中的 TODO 注释和架构文档。*
*引擎是活的有机体，本文档会随项目持续更新。*

> — 总架构师, 2026-06-12
