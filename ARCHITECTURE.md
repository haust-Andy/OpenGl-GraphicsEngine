# OpenGL Graphics Engine — 项目架构文档

> 基于 OpenGL 3.3 Core Profile 的轻量游戏图形引擎  
> 当前版本：v2.1  
> 最后更新：2026-06-12

---

## 一、项目概览

### 1.1 项目定位

**OpenGL Graphics Engine** 是一个从 LearnOpenGL 教学项目改造而来的分层轻量图形引擎，兼具教学价值与实用扩展性。

| 适用场景 | 说明 |
|----------|------|
| 学习引擎架构 | 理解分层引擎、事件系统、资源管理、ECS 模式 |
| 图形学实验 | 在 PBR+CSM+IBL 管线基础上实验新算法 |
| 渲染器原型 | 快速验证 Shadow Mapping、SSAO、延迟渲染等 |
| 编辑器工具 | 通过 ImGui 面板理解场景编辑器工作原理 |
| 小型 3D 应用 | 加载模型 + PBR 材质 + 多光源 + 物理碰撞 |

### 1.2 功能清单

#### 渲染系统
| 功能 | 状态 | 说明 |
|------|------|------|
| **PBR 渲染管线** | ✅ 完成 | Cook-Torrance BRDF，Albedo/Normal/Metallic/Roughness/AO/Emission 6 通道纹理 |
| **级联阴影 (CSM)** | ✅ 完成 | 3 级联阴影 + PSSM 分割 + Poisson Disk PCF 软阴影 + 法线偏移 |
| **IBL 环境光照** | ✅ 完成 | HDR 加载 + 辐照度卷积 + 预过滤 5 级 MIP + BRDF LUT |
| **HDR 后处理管线** | ✅ 完成 | RGBA16F FBO + Bloom + ACES ToneMapping + GLStateSaver RAII |
| **多光源** | ✅ 完成 | 方向光 + 点光源 + 聚光灯，UBO 高效传递 |
| **Uniform Buffer** | ✅ 完成 | CameraUBO + LightUBO |
| **视锥体剔除** | ✅ 完成 | 6 平面提取 + AABB/Sphere 测试 + Scene 自动剔除 |
| **LOD 系统** | ✅ 完成 | 距离/屏幕占比模式 + 4 级 LOD + VAO 安全恢复 |
| **线框模式** | ✅ 完成 | F3 切换 Wireframe/Solid |
| **SSAO** | 🚧 着色器就绪 | 64 采样 + 双边模糊 + 合成（待集成到管线） |

#### 场景管理
| 功能 | 状态 | 说明 |
|------|------|------|
| **Entity-Component** | ✅ 完成 | Transform/Mesh/Light/Script/Physics/Audio/Particle/LOD 组件 |
| **脚本系统** | ✅ 完成 | ScriptComponent：OnCreate/OnUpdate/OnDestroy，Lambda 回调 |
| **Prefab 预制体** | ✅ 完成 | Entity 模板创建/实例化/序列化 |
| **层级变换** | ✅ 完成 | 父子节点，世界矩阵自动计算 |
| **场景序列化** | ✅ 完成 | `.scene` 文本格式保存/加载 |
| **光照收集** | ✅ 完成 | CollectLights/SortEntities 自动优化渲染顺序 |

#### 子系统
| 功能 | 状态 | 说明 |
|------|------|------|
| **物理碰撞** | ✅ 完成 | AABB/Sphere 碰撞 + 弹性响应 + 重力 + Raycast |
| **粒子系统** | ✅ 完成 | CPU 粒子 + Billboard + 颜色渐变 + 重力 |
| **音频系统** | ✅ Stub | AudioSource/Listener 组件 + SoLoud 接口 |
| **游戏 UI** | ✅ 完成 | SpriteBatch + UIImage/Text/Button + Canvas + 锚点 |

#### 编辑器
| 功能 | 状态 | 说明 |
|------|------|------|
| **Viewport** | ✅ 完成 | FBO 渲染 + 焦点检测 + 动态宽高比 |
| **Scene Hierarchy** | ✅ 完成 | 实体树 + 添加/删除 + 右键保存 Prefab |
| **Inspector** | ✅ 完成 | Transform/Material/Light/Physics/Script/LOD 属性编辑 |
| **Content Browser** | ✅ 完成 | 文件系统浏览 + 目录导航 + 文件类型图标 |
| **Prefab Panel** | ✅ 完成 | Prefab 列表 + 点击实例化 |
| **Rendering Stats** | ✅ 完成 | FPS/DrawCalls/Triangles/Vertices/Entity Count |
| **Gizmo** | ✅ 完成 | W/E/R 平移/旋转/缩放 |
| **Play/Stop** | ✅ 完成 | F5 Play 模式 |
| **Menu Bar** | ✅ 完成 | File/View/Play 菜单 |

#### 基础设施
| 功能 | 状态 | 说明 |
|------|------|------|
| **CMake 构建** | ✅ 完成 | 引擎静态库 + Sandbox + Tests |
| **GitHub Actions CI** | ✅ 完成 | Ubuntu + Windows 构建 + 格式检查 |
| **跨平台** | ✅ 完成 | Windows + Linux 已验证 |
| **单元测试** | ✅ 完成 | tests/ 覆盖核心模块 |
| **C++17** | ✅ 完成 | fold expression/constexpr/auto 推导 |

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────────────────┐
│                       APPLICATION LAYER (应用层)                      │
│                       app/SandboxApp.cpp                              │
│            游戏逻辑 / 场景初始化 / 输入处理 / 物理演示                    │
├──────────────────────────────────────────────────────────────────────┤
│                        EDITOR LAYER (编辑器层)                         │
│                  engine/editor/EditorLayer                            │
│  ImGui 面板：Hierarchy / Inspector / Stats / ContentBrowser / Prefab │
│  Gizmo (W/E/R) / Play-Stop (F5) / Save-Load Scene                   │
├──────────────────────────────────────────────────────────────────────┤
│                        ENGINE CORE (引擎核心)                          │
│  ┌────────────┬────────────┬────────────┬──────────────────────────┐│
│  │Application │   Window   │   Input    │     Event System         ││
│  │ 主循环      │ GLFW 窗口   │  键盘/鼠标  │     事件分发              ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │Layer Stack │  Timestep  │    Log     │       Assert             ││
│  │ 分层架构     │  帧时间步   │  分级日志   │     断言系统              ││
│  │ unique_ptr │            │            │                          ││
│  └────────────┴────────────┴────────────┴──────────────────────────┘│
├──────────────────────────────────────────────────────────────────────┤
│                         RENDERER (渲染子系统)                          │
│  ┌────────────┬────────────┬────────────┬──────────────────────────┐│
│  │ Renderer   │RendererAPI │  Shader    │    Framebuffer           ││
│  │ Begin/End  │ OpenGL 抽象 │  着色器编译  │   HDR FBO + RGBA16F     ││
│  │ SetClearColor│           │  错误传播    │    多附件 FBO            ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │ Texture    │VertexArray │  Buffer    │      Material            ││
│  │ 2D/CubeMap │ VAO 封装    │VBO/IBO    │    PBR 材质系统           ││
│  │ 安全ID初始化 │            │ 拷贝控制    │                          ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │  Light     │  Camera    │ UniformBuf │    MeshLibrary           ││
│  │ 多光源系统   │ 自由相机    │  UBO 管理  │    内置几何体库           ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │ ShadowMap  │   IBL      │  Frustum   │       LOD               ││
│  │ CSM 级联阴影│ IBL 环境光照│ 视锥体剔除  │  距离/屏幕占比LOD        ││
│  │ Poisson PCF│            │            │     VAO 安全恢复          ││
│  └────────────┴────────────┴────────────┴──────────────────────────┘│
├──────────────────────────────────────────────────────────────────────┤
│                      POSTPROCESS (后处理管线)                          │
│    BloomPass → ToneMappingPass → Pipeline                             │
│    HDR FBO + GLStateSaver RAII 状态管理                                │
│    (SSAO 着色器已就绪，待集成到管线)                                     │
├──────────────────────────────────────────────────────────────────────┤
│                        RESOURCE (资源管理)                             │
│  ShaderLibrary / TextureLibrary / Model (.obj/.fbx/.gltf 导入)       │
├──────────────────────────────────────────────────────────────────────┤
│                         SCENE (场景系统)                               │
│  Scene → Entity → [Transform | Mesh | Light | Script | Physics       │
│                | Audio | Particle | LOD | Collider | Rigidbody]      │
│  Prefab 系统 · SceneSerializer (.scene) · CollectLights · SortEntities│
├──────────────────────────────────────────────────────────────────────┤
│                     SUBSYSTEMS (子系统)                                │
│  ┌──────────────┬──────────────┬──────────────┬─────────────────────┐│
│  │   Physics    │    Audio     │   Particle   │      Game UI        ││
│  │ AABB碰撞     │ SoLoud接口   │ CPU粒子系统   │ SpriteBatch         ││
│  │ 弹性响应      │ 3D空间化音频  │ Billboard渲染 │ UIImage/Text/Button ││
│  │ 射线检测      │ 音频组件     │ 颜色渐变/重力  │ Canvas + 锚点       ││
│  └──────────────┴──────────────┴──────────────┴─────────────────────┘│
├──────────────────────────────────────────────────────────────────────┤
│                     PLATFORM / MATH (基础层)                           │
│   GLFW 3.4 | GLAD 4.6 | GLM 0.9.9 | Assimp 6.x | stb_image | ImGui │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 三、目录结构

```
OpenGl-GraphicsEngine/
├── ARCHITECTURE.md               # ★ 本架构文档
├── README.md                     # 项目简介与快速上手
├── CODE_REVIEW_SPEC.md           # Code Review 规范
├── FIX_REPORT.md                 # 代码质量修复报告
├── ai_advice.md                  # 原始改造建议 (归档)
├── todo.md                       # 开发路线图
├── CMakeLists.txt                # 构建配置
├── .github/workflows/            # GitHub Actions CI
│   └── pr-checks.yml             # PR 自动化检查 (Linux + Windows + 格式)
│
├── app/                          # 应用层
│   └── SandboxApp.cpp            # 演示程序 (PBR + CSM + 物理 + 粒子 + 脚本)
│
├── tests/                        # 单元测试
│   ├── Test.h                    #   轻量测试框架 (header-only)
│   ├── TestMain.cpp              #   测试入口
│   ├── TestBase.cpp              #   基础类型 + 智能指针别名
│   ├── TestTimestep.cpp          #   Timestep 时间步
│   ├── TestLog.cpp               #   日志系统
│   ├── TestLayerStack.cpp        #   Layer + LayerStack
│   ├── TestEvent.cpp             #   事件系统 + EventDispatcher
│   └── TestSceneSerializer.cpp   #   场景序列化
│
├── engine/                       # ★ 引擎核心 (静态库 OpenGlEngine)
│   ├── core/                     # 引擎基础设施
│   │   ├── Base.h                #   基础类型 + Ref<T>/Scope<T> 别名
│   │   ├── Log.h / .cpp          #   分级日志 (CORE_TRACE~CRITICAL)
│   │   ├── Assert.h              #   断言系统 (CORE_ASSERT / _DEBUG / _STATIC)
│   │   ├── Timestep.h            #   时间步 (帧率无关/缩放/钳制)
│   │   ├── KeyCodes.h            #   键盘按键枚举
│   │   ├── MouseCodes.h          #   鼠标按键枚举
│   │   ├── Event.h               #   事件系统 + EventDispatcher
│   │   ├── Window.h / .cpp       #   GLFW 窗口抽象
│   │   ├── Input.h / .cpp        #   输入系统 (轮询按键/鼠标状态)
│   │   ├── Layer.h / .cpp        #   Layer 基类 + LayerStack (unique_ptr)
│   │   ├── Application.h / .cpp  #   应用基类 (Run循环/事件分发)
│   │   └── EntryPoint.h           #   入口宏 (CreateApplication → main)
│   │
│   ├── renderer/                 # 渲染子系统
│   │   ├── RendererAPI.h / .cpp   #   渲染API抽象
│   │   ├── Renderer.h / .cpp      #   渲染器主类
│   │   ├── RenderCommand.h        #   渲染命令封装
│   │   ├── Shader.h / .cpp        #   着色器 (编译/Uniform缓存/热重载)
│   │   ├── Texture.h / .cpp       #   纹理 (2D/CubeMap/Mipmap)
│   │   ├── Framebuffer.h / .cpp   #   帧缓冲 (HDR RGBA16F + 多附件)
│   │   ├── VertexArray.h / .cpp   #   VAO 封装
│   │   ├── Buffer.h / .cpp        #   VBO/IBO (禁止拷贝/移动语义)
│   │   ├── Camera.h / .cpp        #   自由相机 (FPS风格/欧拉角/动态宽高比)
│   │   ├── Material.h / .cpp      #   PBR材质 (6通道纹理 + 属性)
│   │   ├── Light.h / .cpp         #   光源系统 (Dir/Point/Spot + LightEnvironment)
│   │   ├── UniformBuffer.h        #   UBO (CameraUBO/LightUBO)
│   │   ├── ShadowMap.h / .cpp     #   CSM 级联阴影 (3级联 + PCF)
│   │   ├── IBL.h / .cpp           #   IBL 环境光照 (辐照度/预过滤/BRDF LUT)
│   │   ├── Frustum.h / .cpp       #   视锥体剔除 (6平面 + AABB/Sphere)
│   │   └── LOD.h / .cpp           #   LOD 系统 (距离/屏幕占比)
│   │
│   ├── resource/                  # 资源管理
│   │   ├── ShaderLibrary.h / .cpp #   Shader 缓存池 (按名索引/热重载)
│   │   ├── TextureLibrary.h / .cpp #   纹理缓存池 (Skybox/CubeMap)
│   │   ├── MeshLibrary.h / .cpp   #   内置几何体 (Cube/Sphere/Plane/ScreenQuad)
│   │   └── Model.h / .cpp         #   Assimp 模型导入 (.obj/.fbx/.gltf)
│   │
│   ├── scene/                     # 场景系统
│   │   ├── Scene.h / .cpp         #   场景容器 (Entity CRUD/渲染/剔除)
│   │   ├── TransformComponent.h   #   变换组件 (Position/Rotation/Scale/层级)
│   │   ├── MeshComponent.h        #   网格组件 (VAO + Material + CastShadow)
│   │   ├── LightComponent.h       #   光源组件
│   │   ├── ScriptComponent.h      #   脚本组件 (OnCreate/OnUpdate/OnDestroy)
│   │   ├── Prefab.h / .cpp        #   预制体系统 (模板/实例化/序列化)
│   │   └── SceneSerializer.h/.cpp #   场景序列化 (.scene 文本格式)
│   │
│   ├── postprocess/               # 后处理管线
│   │   └── PostProcess.h / .cpp   #   Bloom + ToneMapping + Pipeline
│   │
│   ├── physics/                   # 物理系统
│   │   └── PhysicsWorld.h / .cpp  #   AABB/Sphere碰撞 + 弹性响应 + 重力 + Raycast
│   │
│   ├── particle/                  # 粒子系统
│   │   └── ParticleEmitter.h/.cpp#   CPU粒子 + Billboard + 颜色渐变 + 重力
│   │
│   ├── audio/                     # 音频系统
│   │   └── AudioSystem.h / .cpp   #   SoLoud 接口 (Stub)
│   │
│   ├── ui/                        # 游戏 UI
│   │   └── UIElements.h / .cpp    #   SpriteBatch + UIImage/Text/Button + Canvas
│   │
│   └── editor/                    # 编辑器
│       └── EditorLayer.h / .cpp    #   ImGui 编辑器 (7+ 面板)
│
├── shader/                        # GLSL 着色器 (17 个文件)
│   ├── pbr.vert / pbr.frag        #   PBR + CSM阴影 + IBL (线性 HDR 输出)
│   ├── skybox.vert / skybox.frag   #   天空盒
│   ├── screen.vert / screen.frag   #   全屏后处理
│   ├── bloom_combine.frag         #   泛光合成
│   ├── brightness.frag            #   亮度提取
│   ├── gaussian_blur.frag         #   高斯模糊
│   ├── tonemapping.frag            #   ACES 色调映射
│   ├── ssao.frag                   #   SSAO 采样
│   ├── ssao_blur.frag              #   SSAO 双边模糊
│   ├── ssao_combine.frag           #   SSAO 合成
│   ├── depth_testing.vs / .fs      #   深度测试 (学习用)
│   └── stencil_testing.vs / .fs    #   模板测试 (学习用)
│
├── resources/                     # 资源文件
│   ├── textures/                  #   纹理贴图
│   └── models/                    #   3D 模型
│
├── external/                      # 第三方库 (header-only)
│   ├── glm-master/                #   数学库
│   └── imgui-master/              #   ImGui 调试UI
│
├── include/                       # GLAD / GLFW / stb 头文件
├── lib/                           # 预编译库 (glfw3, assimp)
└── src/                           # 旧代码 (glad, 学习demo)
```

---

## 四、核心子系统详解

### 4.1 引擎核心 (engine/core/)

#### Base — 基础类型与指针别名

```cpp
// 跨平台整数类型
using uint32 = uint32_t;

// 智能指针别名
template<typename T> using Ref   = std::shared_ptr<T>;
template<typename T> using Scope = std::unique_ptr<T>;
template<typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
template<typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
```

#### Log — 分级日志

```cpp
CORE_TRACE("Loading texture: {}", path);
CORE_INFO("Scene loaded with {} entities.", entityCount);
CORE_WARN("Shader compilation warning: {}", msg);
CORE_ERROR("Failed to open file: {}", filepath);
CORE_CRITICAL("Out of GPU memory!");
```

- 6 级日志：Trace / Debug / Info / Warn / Error / Critical
- 自动捕获 `__FILE__:__LINE__`，带彩色时间戳
- 跨平台时间格式（`localtime_s` Windows / `localtime_r` Linux）

#### Assert — 断言

```cpp
CORE_ASSERT(shader, "Shader is null!");
CORE_ASSERT_DEBUG(camera != nullptr, "Camera pointer is null");
CORE_STATIC_ASSERT(sizeof(TransformComponent) <= 128, "Transform too large");
```

- `CORE_ASSERT` — 所有构建中有效（致命）
- `CORE_ASSERT_DEBUG` — 仅 Debug 构建有效
- 可通过 `CORE_DISABLE_ASSERTS` 全局关闭

#### Timestep — 帧时间步

```cpp
Timestep ts(deltaTime);          // 秒
float dt  = ts.GetSeconds();      // 秒
float ms  = ts.GetMilliseconds(); // 毫秒
ts = Timestep::Clamp(ts, 0.1f);   // 钳制最大间隔（防编辑器失焦跳帧）
```

#### Event — 事件系统

```
Event (基类)
├── KeyPressedEvent    (按键按下，含重复计数)
├── KeyReleasedEvent   (按键释放)
├── KeyTypedEvent      (字符输入)
├── MouseMovedEvent    (鼠标移动)
├── MouseScrolledEvent (滚轮)
├── MouseButtonPressedEvent
├── MouseButtonReleasedEvent
├── WindowResizeEvent
├── WindowCloseEvent
├── WindowFocusEvent
└── WindowLostFocusEvent
```

`EventDispatcher::Dispatch<T>(handler)` 按类型分发，`e.Handled = true` 阻止冒泡。

> **注意**：当前 ImGui 的 GLFW 回调链会消费鼠标事件，相机旋转/平移已改为轮询模式 (`Input::GetMousePosition()`)。

#### Layer Stack — 分层架构

```
LayerStack (in Application::Run)
  ┌──────────────────┐  ← render last / handle event first
  │   Overlay Layer   │     (ImGui EditorLayer)
  ├──────────────────┤
  │   Normal Layer    │     (SandboxLayer / GameLayer)
  └──────────────────┘  ← render first / handle event last
```

每层 5 个生命周期回调：
- `OnAttach()` / `OnDetach()` — 层的挂载与卸载
- `OnUpdate(Timestep ts)` — 每帧逻辑更新
- `OnEvent(Event& e)` — 事件处理
- `OnImGuiRender()` — ImGui 绘制

Layer 所有权通过 `unique_ptr` 管理，LayerStack 提供自定义迭代器（含 `iterator_traits` 支持）。

---

### 4.2 渲染器 (engine/renderer/)

#### 渲染 API 抽象

```
Renderer (门面)
    ↓
RendererAPI (OpenGL 实现)
    ↓ 封装以下 OpenGL 对象:
    Shader / Texture / Framebuffer / VertexArray / Buffer
```

每个图形对象遵循 RAII 模式：

```cpp
auto vao = VertexArray::Create();
auto vbo = VertexBuffer::Create(vertices, size);
vao->AddVertexBuffer(vbo, { {Float3, "aPos"}, {Float3, "aNormal"}, {Float2, "aTexCoord"} });
auto ibo = IndexBuffer::Create(indices, count);
vao->SetIndexBuffer(ibo);
```

**安全保证**：
- 所有 GL 资源 ID 初始化为 0
- 析构前检查 ID 有效性
- Buffer 禁止拷贝，支持移动语义
- Shader 编译/链接失败正确返回 false

#### PBR 材质系统

```
Material
├── Shader 引用
├── MaterialProperties
│   ├── Albedo    (vec3)    基础色
│   ├── Metallic  (float)   金属度
│   ├── Roughness (float)   粗糙度
│   ├── AO        (float)   环境光遮蔽
│   └── Emission  (vec3)    自发光
├── 纹理贴图 (可选)
│   ├── AlbedoMap / NormalMap / MetallicMap
│   ├── RoughnessMap / AOMap / EmissiveMap
└── Bind() / Unbind() — 绑定到当前 Shader
```

#### 光源系统

```
LightEnvironment (场景光照管理器)
├── DirectionalLight   ×1  (方向/颜色/强度)
├── PointLight[]       ×N  (位置/颜色/强度/衰减/范围)
└── SpotLight[]        ×N  (位置/方向/内外锥角/衰减)
```

光源数据通过 UBO 传递给 Shader，减少 `glUniform` 调用。

#### CSM 级联阴影 (ShadowMap)

```
ShadowMap
├── 3 级联阴影贴图 (2048×2048 分 3 块)
├── PSSM 分割策略
├── Poisson Disk PCF 软阴影 (32 预计算偏移)
├── 法线偏移防阴影粉刺
└── ShadowRenderer::RenderShadowPass() 深度渲染

ShadowMap::CalculateCascades()
  输入: 光方向 / 相机 View / Proj / Near / Far
  输出: 3 组 LightSpaceMatrix → UBO 传递给 PBR Shader
```

#### IBL 环境光照

```
IBL
├── LoadFromHDR(path)           HDR 环境贴图加载
├── GenerateFromProcedural(...)  程序化天空盒生成
├── Irradiance Map               辐照度卷积 (32×32 CubeMap)
├── Prefiltered Env Map          预过滤环境图 (5 级 MIP)
├── BRDF LUT                     BRDF 积分查找表 (512×512)
└── BindEnvironmentMap(slot)      绑定到 PBR Shader
```

PBR Shader 中 IBL 合成：
- 漫反射：`texture(u_IrradianceMap, N)` → 环境漫反射
- 镜面反射：`textureLod(u_PrefilteredMap, R, roughness * 4)` + BRDF LUT
- 最终：`Lo = DirectPBR + IBL_Diffuse + IBL_Specular`

#### 视锥体剔除 (Frustum)

```
Frustum
├── 6 平面提取 (从 VP 矩阵)
├── AABB 相交测试
├── Sphere 相交测试
└── Scene::OnRender() 中自动剔除不可见实体
```

#### LOD 系统

```
LODComponent
├── Mode: Distance / ScreenSize
├── LODLevels[] (最多 4 级: VA0~VA3 + 距离阈值)
├── Bias (调整 LOD 切换距离)
└── Scene::OnRender() 中自动选择 VAO

安全机制: 渲染后恢复原始 VAO，防止状态污染
```

#### 渲染流程

```
SandboxLayer::OnUpdate()
  ├── HandleInput(dt)             # WASD + 鼠标
  ├── UpdateCameraRotation()      # 轮询式视角旋转
  ├── UpdateCameraPan()           # 轮询式中键平移
  ├── Scene::OnUpdate(ts)         # 脚本 + 物理 + 粒子
  │
  ├── ShadowMap::CalculateCascades()    # CSM 级联矩阵
  ├── ShadowRenderer::RenderShadowPass() # 深度渲染 Pass
  │
  ├── SceneFBO::Bind() + glClear()     # HDR FBO
  ├── Skybox Rendering                   # IBL 环境/程序化天空
  ├── Scene::OnRender(camera)            # PBR 渲染
  │   ├── CollectLights()                # 收集光源 → LightEnvironment
  │   ├── SortEntities()                 # 按材质排序
  │   ├── Frustum Culling                # 视锥体剔除
  │   ├── LOD Selection                  # 自动 LOD
  │   ├── BeginScene(view, proj)         # 绑定 CameraUBO + LightUBO
  │   └── ForEach Entity:               # 遍历可见实体
  │       ├── Material::Bind()
  │       ├── ShadowMap::Bind()           # CSM 阴影贴图
  │       ├── IBL::Bind()                 # IBL 纹理
  │       ├── VertexArray::Bind()
  │       └── Renderer::Submit()         # glDrawElements()
  │
  ├── PostProcess::Execute()            # Bloom + ToneMapping
  └── Blit to ViewportFBO               # 输出到编辑器视口
```

---

### 4.3 后处理管线 (engine/postprocess/)

```
PostProcessPipeline
  ├── BloomPass
  │   ├── 亮度提取 → 水平模糊 → 垂直模糊 → 合成
  │   ├── 可调: 阈值 / 强度 / 模糊迭代次数
  │   └── HDR FBO (RGBA16F)
  ├── ToneMappingPass
  │   └── ACES 色调映射 + Gamma 校正
  ├── GLStateSaver RAII
  │   └── 自动保存/恢复: Depth/Blend/CullFace/绑定对象
  └── Execute(hdrFBO) → outputFBO
```

**SSAO (待集成)**：
- `shader/ssao.frag` — 64 采样核心
- `shader/ssao_blur.frag` — 双边模糊
- `shader/ssao_combine.frag` — 合成

---

### 4.4 资源管理 (engine/resource/)

| 库 | 职责 | 关键API |
|----|------|---------|
| `ShaderLibrary` | Shader 按名缓存 | `Get(name)` `Load(name, vs, fs)` `ReloadAll()` |
| `TextureLibrary` | 纹理按路径缓存 | `Load2D(path)` `LoadCube(faces)` `LoadHDR(path)` |
| `MeshLibrary` | 内置几何体 | `GetCube()` `GetSphere(seg)` `GetPlane()` `GetScreenQuad()` |
| `Model` | Assimp 模型导入 | `Load(path)` → SubMesh[] + MaterialPtr |

- `ShaderLibrary` / `TextureLibrary` 为单例，支持热重载
- `Model` 支持 `.obj` / `.fbx` / `.gltf`，PBR 纹理自动映射

---

### 4.5 场景系统 (engine/scene/)

#### Entity-Component 模式

```
Entity (uint32_t ID)
├── Tag (string)                          实体名称
├── IsActive (bool)                       激活状态
├── IsStatic (bool)                       静态标志 (跳过物理/脚本)
│
├── TransformComponent [核心]             位置/旋转/缩放/层级
├── MeshComponent [核心]                  VAO + Material + CastShadow
│
├── LightComponent [可选]                 光源类型 + 参数
│   └── HasLight 标志
├── ScriptComponent [可选]                OnCreate/OnUpdate/OnDestroy
│   └── HasScript 标志
├── ColliderComponent + RigidbodyComponent [可选]
│   └── HasPhysics 标志
│   ├── ColliderType: AABB / Sphere
│   ├── IsTrigger 触发器模式
│   ├── Mass / Velocity / Force / Gravity / Restitution / Friction
│   └── AddForce() / AddImpulse()
├── AudioSourceComponent / AudioListenerComponent [可选]
│   └── HasAudioSource / HasAudioListener 标志
├── ParticleEmitter [可选] (unique_ptr)
│   └── HasParticleEmitter 标志
│   └── EmitRate / MaxParticles / Life / Speed / Size / Color / Gravity
└── LODComponent [可选]
    └── HasLOD 标志
    └── Mode / LODLevels[] / Bias
```

Scene 提供：
- `CreateEntity(tag)` / `DestroyEntity(entity)`
- `OnUpdate(Timestep)` — 更新脚本 + 物理 + 粒子
- `OnRender(Camera&)` — 收集光源 + 排序 + 剔除 + LOD + 渲染
- `ForEachEntity(callback)` — 遍历所有实体
- `SetShadowMap()` / `SetPhysicsWorld()` / `SetIBL()` / `SetFrustumCulling()`

#### ScriptComponent — 脚本系统

```cpp
auto& script = entity->GetScript();
script.ScriptName = "RotateScript";
script.OnCreate = [](Entity& e) { /* 初始化 */ };
script.OnUpdate = [](Entity& e, Timestep ts) {
    float dt = ts.GetSeconds();
    auto euler = e.GetTransform().GetEulerAngles();
    euler.y += 45.0f * dt;  // 每秒旋转 45 度
    e.GetTransform().SetEulerAngles(euler);
};
script.OnDestroy = [](Entity& e) { /* 清理 */ };
```

#### Prefab — 预制体系统

```cpp
// 从已有 Entity 创建 Prefab
auto prefab = Prefab::CreateFromEntity(entity, "MyPrefab");

// 实例化到场景
auto* instance = prefab->Instantiate(scene);

// 序列化/反序列化
Prefab::SaveToFile(*prefab, "prefabs/my_prefab.prefab");
auto loaded = Prefab::LoadFromFile("prefabs/my_prefab.prefab");
```

#### SceneSerializer — 场景序列化

```scene
[Scene] name="Demo Scene"

[LightEnvironment]
  ambientColor=1,1,1
  [DirectionalLight] dir=-0.5,-1,-0.3 color=1,0.98,0.95 intensity=2
[/LightEnvironment]

[Entity] tag="Cube" id=1
  [Transform] px=-1.5 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
  [MeshRenderer] visible=1 castShadow=1
  [Physics] colliderType=AABB boxMin=-0.5,-0.5,-0.5 boxMax=0.5,0.5,0.5 isStatic=1
[/Entity]

[/Scene]
```

---

### 4.6 物理系统 (engine/physics/)

```
PhysicsWorld
├── AABB 碰撞检测
│   ├── Transform(mat4) 8 角点变换到世界空间
│   ├── Contains(point) 点包含测试
│   └── Intersects(other) AABB 相交测试
├── Sphere 碰撞检测
├── 弹性碰撞响应
│   └── Restitution 弹性系数 + 半隐式欧拉积分
├── 重力模拟
│   └── GravityScale + UseGravity
├── Raycast
│   ├── RayAABB 相交测试
│   └── RaySphere 相交测试
├── 碰撞回调
│   ├── OnCollisionEnter
│   ├── OnCollisionStay
│   └── OnCollisionExit
└── 固定时间步更新
```

---

### 4.7 粒子系统 (engine/particle/)

```
ParticleEmitter
├── 配置 (ParticleEmitterConfig)
│   ├── EmitRate / MaxParticles
│   ├── MinLife / MaxLife
│   ├── MinSpeed / MaxSpeed
│   ├── StartSize / EndSize
│   ├── Direction / SpreadAngle
│   ├── StartColor / EndColor (渐变)
│   ├── Gravity
│   └── EmitRadius
├── CPU 粒子池 (预分配，避免运行时 new/delete)
├── Billboard 渲染 (始终面向相机)
└── 粒子生命周期管理
```

---

### 4.8 音频系统 (engine/audio/)

```
AudioEngine (SoLoud Stub)
├── Init() / Shutdown()
├── AudioSourceComponent
│   ├── ClipPath / Volume / Pitch / Loop
│   └── Is3D / MinDistance / MaxDistance
└── AudioListenerComponent
    └── 跟随相机位置/朝向
```

> 当前为 Stub 实现，接口已定义但 SoLoud 未实际集成。

---

### 4.9 游戏 UI (engine/ui/)

```
SpriteBatch
├── 2D 批量渲染 (正交投影 + Alpha 混合)
├── DrawCall 合并
└── Begin() / Draw() / End()

UI 控件体系
├── UIElement (基类: Position/Size/Color/Visible)
├── UIImage (纹理 + UV)
├── UIText (文本 + 字体 + 对齐)
├── UIButton (OnClick/OnHover 事件)
└── UICanvas
    ├── 设计分辨率
    ├── 9 种锚点 (TopLeft/Center/BottomRight/Stretch...)
    └── 缩放适配
```

---

### 4.10 编辑器 (engine/editor/)

EditorLayer 提供 7+ 个 ImGui 面板（View 菜单开关）：

| 面板 | 功能 |
|------|------|
| **Viewport** | 场景渲染视图 (FBO→ImGui::Image)，焦点检测 + 动态宽高比 |
| **Scene Hierarchy** | 实体树形列表，添加/删除实体，右键保存为 Prefab |
| **Inspector** | Transform/Material/Light/Physics/Script/LOD 属性编辑，添加组件 |
| **Content Browser** | 文件系统浏览器，目录导航，文件类型图标，异常日志 |
| **Prefab Panel** | Prefab 列表，点击实例化到场景 |
| **Rendering Stats** | FPS / DrawCalls / Triangles / Vertices / Entity Count |
| **Menu Bar** | File (Save/Load Scene) · View (面板切换) · Play (F5) · Gizmo (W/E/R) |

编辑器快捷键：
- `F3` — 切换线框模式
- `F5` — Play 模式
- `W/E/R` — Gizmo 平移/旋转/缩放
- 右键拖拽 — 旋转视角
- 中键拖拽 — 平移视角
- 滚轮 — 缩放

---

## 五、数据流图（一帧的完整流程）

```
main()
  └── CreateApplication() → new Application("OpenGL Graphics Engine", 1600, 900)
      └── app->PushLayer(std::make_unique<SandboxLayer>())
          └── app->Run()
              │
              ▼
  ┌────────────────────────────────────────────────────────────────┐
  │                    每帧循环 (Application::Run)                  │
  │                                                                │
  │  1. Timestep ts = Clamp(glfwGetTime - m_LastFrameTime)         │
  │                                                                │
  │  2. Layer::OnUpdate(ts)                                        │
  │     ├── SandboxLayer::HandleInput(dt)                           │
  │     │   └── WASD + Ctrl/Space → Camera::ProcessKeyboard        │
  │     ├── UpdateCameraRotation() [轮询]                           │
  │     │   └── Input::GetMousePosition() → Camera::ProcessMouse   │
  │     ├── UpdateCameraPan() [轮询]                                │
  │     │   └── Input::GetMousePosition() → Camera.Position        │
  │     ├── Scene::OnUpdate(ts)                                     │
  │     │   ├── ScriptComponent::OnUpdate()                         │
  │     │   ├── PhysicsWorld::Step() (碰撞检测 + 弹性响应)           │
  │     │   └── ParticleEmitter::Update()                           │
  │     │                                                           │
  │     ├── ShadowMap::CalculateCascades()                          │
  │     │   └── 3 组 LightSpaceMatrix → ShadowMap UBO               │
  │     ├── ShadowRenderer::RenderShadowPass()                      │
  │     │   └── 收集 CastShadow 实体 → 深度渲染                     │
  │     │                                                           │
  │     ├── SceneFBO::Bind() + glClear()                            │
  │     ├── Skybox Rendering (IBL 环境/程序化天空)                    │
  │     ├── Scene::OnRender(camera)                                  │
  │     │   ├── CollectLights() → LightEnvironment                 │
  │     │   ├── SortEntities() → 按材质排序                          │
  │     │   ├── Frustum Culling → 剔除不可见实体                     │
  │     │   ├── LOD Selection → 按距离/屏幕占比选择 VAO             │
  │     │   ├── BeginScene(view, proj)                              │
  │     │   │   └── 绑定 CameraUBO + LightUBO + ShadowMap + IBL     │
  │     │   └── ForEach Visible Entity:                             │
  │     │       └── Material::Bind()                                │
  │     │           └── VertexArray::Bind()                          │
  │     │               └── Renderer::Submit() → glDrawElements()   │
  │     ├── SceneFBO::Unbind()                                      │
  │     │                                                           │
  │     ├── PostProcess::Execute(SceneFBO)                         │
  │     │   └── Bloom → ToneMapping → OutputFBO                    │
  │     └── Blit to ViewportFBO                                     │
  │                                                                │
  │  3. Layer::OnImGuiRender()                                      │
  │     └── EditorLayer::OnImGuiRender()                             │
  │         ├── DrawMenuBar() → File/View/Play                      │
  │         ├── DrawViewport() → ViewportFBO→ImGui::Image           │
  │         ├── DrawSceneHierarchy() → 实体树                       │
  │         ├── DrawInspector() → 属性面板                          │
  │         ├── DrawContentBrowser() → 文件浏览                     │
  │         ├── DrawPrefabPanel() → Prefab 实例化                   │
  │         └── DrawStatsPanel() → 渲染统计                          │
  │                                                                │
  │  4. Window::OnUpdate()                                          │
  │     └── glfwSwapBuffers | glfwPollEvents                        │
  │         └── GLFW Callback → Event → Application::OnEvent        │
  │             ├── WindowCloseEvent  → m_Running = false           │
  │             ├── WindowResizeEvent → glViewport                  │
  │             └── LayerStack::OnEvent (reverse iteration)         │
  └────────────────────────────────────────────────────────────────┘
```

---

## 六、依赖关系

```
┌──────────────────────────────────────────────────────┐
│                     SandboxApp                        │
│          (链接 OpenGlEngine 静态库)                    │
└──────────────────────┬───────────────────────────────┘
                       │ 依赖
┌──────────────────────▼───────────────────────────────┐
│                  OpenGlEngine.lib                      │
│  engine/core + renderer + scene + resource + ...      │
│  + imgui + glad                                       │
└──────────────────────┬───────────────────────────────┘
                       │ 依赖
┌──────────────────────▼───────────────────────────────┐
│              第三方库 (external / include / lib)       │
│  GLFW 3.4 | GLAD 4.6 | GLM 0.9.9 | Assimp | stb      │
└──────────────────────────────────────────────────────┘
```

| 库 | 版本 | 用途 | 集成方式 |
|----|------|------|----------|
| GLFW | 3.4 | 窗口/输入/OpenGL 上下文 | 预编译 `.a` (Win) / 系统包 (Linux) |
| GLAD | 4.6 | OpenGL 函数加载 | 源文件编译 |
| GLM | 0.9.9 | 数学库 (vec3/mat4/quat) | header-only |
| Assimp | 6.x | 3D 模型导入 (.obj/.fbx/.gltf) | 预编译 (Win) / 系统包 (Linux) |
| stb_image | — | 图片加载 (LDR + HDR) | header-only |
| ImGui | master | 编辑器 UI | 源文件编译 |

---

## 七、CI/CD

项目已配置 GitHub Actions 自动化检查 (`.github/workflows/pr-checks.yml`)：

| Job | 平台 | 内容 |
|-----|------|------|
| `build-linux` | Ubuntu 22.04 | CMake 配置 + 编译 + 运行测试 |
| `build-windows` | Windows 2022 | CMake MinGW 配置 + 编译 + 运行测试 |
| `format-check` | Ubuntu 22.04 | clang-format 格式检查 (仅报告) |

触发条件：向 `dev` 或 `main` 分支提交 PR 时自动运行。

---

## 八、版本历史

### v2.1 — 代码质量改进

- HDR 后处理管线 (RGBA16F)
- Shader 错误传播 (编译/链接失败返回 false)
- GL 资源安全初始化 (ID=0 + 析构检查)
- LOD VAO 安全恢复
- Poisson Disk PCF 软阴影
- GLStateSaver RAII 状态保护
- Layer 所有权 unique_ptr 迁移
- Camera 动态宽高比
- 跨平台兼容 (Windows + Linux)
- Buffer 拷贝控制 (禁止拷贝 + 移动语义)

### v2.0 — 功能扩展

- CSM 级联阴影映射
- IBL 环境光照
- Assimp 模型导入
- 脚本系统 (ScriptComponent)
- 物理碰撞 (AABB/Sphere + 弹性响应 + Raycast)
- 音频系统 (SoLoud Stub)
- 视锥体剔除
- 粒子系统 (CPU + Billboard)
- 游戏 UI (SpriteBatch + Canvas + 锚点)
- LOD 系统
- Prefab 预制体
- 编辑器增强 (ContentBrowser + Prefab + Gizmo + Play/Stop)
- SSAO 着色器

### v1.0 — 初始架构

- 引擎核心基础设施
- PBR 渲染管线
- 多光源系统 + UBO
- 后处理管线 (Bloom + ToneMapping)
- 场景系统 (Entity-Component + 序列化)
- 资源管理 (Shader/Texture/Mesh 缓存)
- ImGui 编辑器 (5 面板)
- CMake 多目标构建

---

## 九、编译与运行

```bash
# 配置
cd OpenGl-GraphicsEngine
mkdir build && cd build

# Windows (MinGW)
cmake .. -G "MinGW Makefiles"
# Windows (MSVC)
cmake ..
# Linux
cmake ..

# 编译
cmake --build . --config Release

# 运行
./Sandbox      # Linux/macOS
Sandbox.exe    # Windows

# 运行测试
./Tests        # Linux/macOS
Tests.exe      # Windows
```

### CMake 构建产物

| 目标 | 类型 | 输出 |
|------|------|------|
| `OpenGlEngine` | STATIC 库 | `libOpenGlEngine.a` |
| `Sandbox` | 可执行 | `Sandbox(.exe)` → 自动复制 resources/ shader/ |
| `Tests` | 可执行 | `Tests(.exe)` → 自动复制 resources/ shader/ |

---

## 十、代码规范约定

| 类别 | 规则 |
|------|------|
| 文件命名 | PascalCase: `ShaderLibrary.h`, `SceneSerializer.cpp` |
| 类命名 | PascalCase: `Application`, `VertexArray` |
| 方法命名 | PascalCase: `OnUpdate`, `GetViewMatrix` |
| 成员变量 | `m_` 前缀: `m_Window`, `m_Running` |
| 静态变量 | `s_` 前缀: `s_Instance` |
| 指针别名 | `Ref<T>` = shared_ptr, `Scope<T>` = unique_ptr |
| 日志 | `CORE_TRACE/INFO/WARN/ERROR` 宏，禁止 `std::cout/cerr` |
| 断言 | `CORE_ASSERT` 检查前置条件 |
| 时间 | `Timestep` 类型传递，不用裸 `float` |
| GL 资源 | ID 初始化为 0，析构前检查，禁止拷贝 |
| 编译期常量 | `constexpr` 替代预处理器宏 |
| 异常处理 | 禁止空 `catch(...)`，必须记录日志 |
| 跨平台 | `strncpy` 替代 `strcpy_s`，`#ifdef _WIN32` 条件编译 |

---

*本文档随项目同步更新，反映 v2.1 最新架构状态。*
