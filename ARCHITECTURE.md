# OpenGL Graphics Engine — 项目架构文档

> 基于 OpenGL 3.3 的轻量游戏图形引擎  
> 最后更新：2026-06-08

---

## 一、项目概览

### 1.1 项目用途

**OpenGL Graphics Engine** 是一个面向学习的轻量级实时 3D 图形引擎，基于 OpenGL 3.3 Core Profile 构建。

**核心定位**：从 LearnOpenGL 教学项目改造而来的 **分层轻量图形引擎**，兼具教学价值与实用扩展性。适合以下场景：

| 场景 | 说明 |
|------|------|
| 🎓 **学习引擎架构** | 从过程式渲染代码过渡到分层引擎架构，理解 ECS / 事件系统 / 资源管理 |
| 🧪 **图形学实验** | 在 PBR 管线基础上实验新算法（Shadow Mapping、SSAO、延迟渲染） |
| 🔧 **渲染器原型** | 作为自定义渲染器的起点，快速验证想法 |
| 🖥️ **编辑器工具** | 通过 ImGui 面板理解场景编辑器、属性检查器的工作原理 |
| 📦 **小型游戏/可视化** | 加载 3D 模型 + PBR 材质 + 多光源，做小型 3D 应用 |

### 1.2 功能清单

#### 渲染系统
| 功能 | 状态 | 说明 |
|------|------|------|
| **PBR 渲染管线** | ✅ 完成 | Cook-Torrance BRDF，支持 Albedo/Normal/Metallic/Roughness/AO/Emission 6 通道纹理 |
| **方向光** | ✅ 完成 | 方向/颜色/强度可调 |
| **点光源** | ✅ 完成 | 支持多盏，衰减/范围可配 |
| **聚光灯** | ✅ 完成 | 内外锥角 + 衰减 |
| **Uniform Buffer** | ✅ 完成 | CameraUBO + LightUBO，减少 uniform 调用 |
| **Shadow Mapping** | 🚧 P0 | Cascaded Shadow Maps 计划中 |
| **Skybox / IBL** | 🚧 P0 | 天空盒 + 基于图像的照明计划中 |

#### 后处理
| 功能 | 状态 | 说明 |
|------|------|------|
| **Bloom** | ✅ 完成 | 亮度提取 → 高斯模糊 → 合成，阈值/强度/迭代次数可调 |
| **ToneMapping** | ✅ 完成 | ACES / Reinhard 色调映射 |
| **FXAA** | 🚧 待定 | 快速近似抗锯齿 |
| **SSAO** | 🚧 P1 | 屏幕空间环境光遮蔽 |

#### 场景管理
| 功能 | 状态 | 说明 |
|------|------|------|
| **Entity-Component** | ✅ 完成 | Transform / Mesh / Light 组件，轻量 ECS |
| **场景容器** | ✅ 完成 | Entity CRUD + 渲染遍历 + 光源自动收集 |
| **场景序列化** | ✅ 完成 | `.scene` 文本格式保存/加载 |
| **层级变换** | ✅ 完成 | 父子节点，世界矩阵自动计算 |

#### 资源管理
| 功能 | 状态 | 说明 |
|------|------|------|
| **Shader 缓存** | ✅ 完成 | 按名索引，Uniform 位置缓存，支持热重载 |
| **纹理缓存** | ✅ 完成 | 2D/CubeMap 按路径幂等加载 |
| **内置几何体** | ✅ 完成 | Cube/Sphere/Plane/ScreenQuad |
| **3D 模型导入** | ✅ 完成 | 通过 Assimp 加载 .obj/.fbx/.gltf |

#### 引擎核心
| 功能 | 状态 | 说明 |
|------|------|------|
| **应用框架** | ✅ 完成 | Application::Run() 主循环 + 生命周期管理 |
| **窗口抽象** | ✅ 完成 | GLFW 窗口封装，VSync/宽高比/事件回调 |
| **分层架构** | ✅ 完成 | Layer + LayerStack，支持 Overlay 叠加层 |
| **事件系统** | ✅ 完成 | Key/Mouse/Window 事件 + EventDispatcher 类型分发 |
| **输入轮询** | ✅ 完成 | Input 静态类，查询按键/鼠标状态 |
| **时间步** | ✅ 完成 | Timestep 类，支持缩放 + 钳制防止跳帧 |
| **日志系统** | ✅ 完成 | CORE_TRACE~CRITICAL 6 级日志 + 文件/行号 |
| **断言系统** | ✅ 完成 | CORE_ASSERT / CORE_ASSERT_DEBUG / CORE_STATIC_ASSERT |
| **基础类型** | ✅ 完成 | Ref<T>/Scope<T> 别名，跨平台整数类型 |
| **相机系统** | ✅ 完成 | FPS 风格自由相机，欧拉角旋转，视角可调 |

#### 编辑器
| 功能 | 状态 | 说明 |
|------|------|------|
| **Viewport 面板** | ✅ 完成 | FBO → ImGui::Image 渲染，焦点检测 + 自由相机 |
| **层级面板** | ✅ 完成 | 实体树形列表，点击选中 |
| **属性检查器** | ✅ 完成 | Tag / Transform / Mesh 属性实时编辑 |
| **渲染统计** | ✅ 完成 | FPS / DrawCalls / Triangles / Vertices / Entity Count |
| **光源编辑器** | ✅ 完成 | 方向光参数实时调节 |
| **菜单栏** | ✅ 完成 | View 菜单控制面板显隐 |
| **线框模式** | ✅ 完成 | F3 切换 wireframe / solid |
| **场景加载/保存** | ✅ 完成 | 编辑器内打开/保存 .scene 文件 |

#### 基础设施
| 功能 | 状态 | 说明 |
|------|------|------|
| **CMake 构建** | ✅ 完成 | 引擎静态库 + Sandbox 可执行文件，资源自动复制 |
| **C++17** | ✅ 完成 | fold expression / constexpr / auto 推导 |
| **跨平台** | ⚠️ 部分 | Windows 已验证，Linux/macOS 理论支持 |
| **单元测试** | ✅ 新增 | tests/ 目录，覆盖核心模块 |

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER (应用层)                      │
│                        app/SandboxApp.cpp                              │
│              SandboxLayer : public Layer                              │
│              游戏逻辑 / 场景初始化 / 输入处理                            │
├──────────────────────────────────────────────────────────────────────┤
│                        EDITOR LAYER (编辑器层)                         │
│                     engine/editor/EditorLayer                          │
│          ImGui 面板：Hierarchy / Inspector / Stats / Light Editor      │
├──────────────────────────────────────────────────────────────────────┤
│                        ENGINE CORE (引擎核心)                          │
│   ┌─────────────┬──────────────┬───────────────┬───────────────────┐ │
│   │ Application │   Window     │    Input      │  Event System     │ │
│   │ 主循环/Life │  GLFW 窗口   │  键盘/鼠标     │  事件分发/处理     │ │
│   ├─────────────┼──────────────┼───────────────┼───────────────────┤ │
│   │ Layer Stack │   Timestep   │    Log        │    Assert         │ │
│   │ 分层架构    │  帧时间步    │  分级日志      │   断言系统         │ │
│   └─────────────┴──────────────┴───────────────┴───────────────────┘ │
├──────────────────────────────────────────────────────────────────────┤
│                        RENDERER (渲染子系统)                           │
│   ┌─────────────┬──────────────┬───────────────┬───────────────────┐ │
│   │  Renderer   │  RendererAPI │  RenderCommand │  Framebuffer     │ │
│   │ Begin/End   │  OpenGL 抽象  │  glClear等封装  │  多附件FBO       │ │
│   ├─────────────┼──────────────┼───────────────┼───────────────────┤ │
│   │ Shader      │  Texture     │  VertexArray   │  Buffer          │ │
│   │ 着色器编译  │  2D/CubeMap  │  VAO 封装      │  VBO / IBO       │ │
│   ├─────────────┼──────────────┼───────────────┼───────────────────┤ │
│   │  Material   │  Light       │  Camera        │  UniformBuffer   │ │
│   │ PBR 材质   │ 光源系统      │  自由相机      │  UBO 管理         │ │
│   └─────────────┴──────────────┴───────────────┴───────────────────┘ │
├──────────────────────────────────────────────────────────────────────┤
│                     POSTPROCESS (后处理管线)                           │
│       BloomPass → ToneMappingPass → PostProcessPipeline               │
├──────────────────────────────────────────────────────────────────────┤
│                     RESOURCE (资源管理)                                │
│    ShaderLibrary / TextureLibrary / MeshLibrary (RefCache + 热重载)    │
├──────────────────────────────────────────────────────────────────────┤
│                        SCENE (场景系统)                                │
│       Scene → Entity → [Transform | Mesh | Light]Component            │
│                SceneSerializer (.scene 文本文件)                       │
├──────────────────────────────────────────────────────────────────────┤
│                     PLATFORM / MATH (基础层)                           │
│   GLFW 3.4 | GLAD 4.6 | GLM 0.9.9 | Assimp | stb_image | ImGui       │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 三、目录结构

```
OpenGl-GraphicsEngine/
├── ARCHITECTURE.md                   # ★ 本架构文档
├── README.md                         # 项目简介
├── CMakeLists.txt                    # 构建配置（引擎静态库 + Sandbox 可执行文件）
│
├── app/                              # 应用层 (游戏/沙盒)
│   └── SandboxApp.cpp                # 示例应用 (PBR 场景 + 编辑器)
│
├── tests/                            # 单元测试
│   ├── Test.h                        #   轻量测试框架 (header-only)
│   ├── TestMain.cpp                  #   测试入口
│   ├── TestBase.cpp                  #   基础类型 + 智能指针别名
│   ├── TestTimestep.cpp              #   Timestep 时间步
│   ├── TestLog.cpp                   #   日志系统 (FormatLog / Level)
│   ├── TestLayerStack.cpp            #   Layer + LayerStack
│   ├── TestEvent.cpp                 #   事件系统 + EventDispatcher
│   └── TestSceneSerializer.cpp       #   场景序列化
│
├── engine/                           # ★ 引擎核心 (静态库 OpenGlEngine)
│   ├── core/                         # 引擎基础设施
│   │   ├── Base.h                    #   基础类型 (int32/uint32/...) + Ref<T>/Scope<T> 别名
│   │   ├── Log.h / .cpp              #   分级日志系统 (CORE_TRACE~CRITICAL 宏)
│   │   ├── Assert.h                  #   断言系统 (CORE_ASSERT / CORE_ASSERT_DEBUG)
│   │   ├── Timestep.h                #   时间步 (帧率无关/Timestep::Clamp)
│   │   ├── KeyCodes.h               #   键盘按键枚举
│   │   ├── MouseCodes.h             #   鼠标按键枚举
│   │   ├── Event.h                  #   事件系统 (Key/Mouse/Window 事件 + EventDispatcher)
│   │   ├── Window.h / .cpp          #   GLFW 窗口抽象 (创建/回调/VSync)
│   │   ├── Input.h / .cpp           #   输入系统 (轮询按键/鼠标状态)
│   │   ├── Layer.h / .cpp           #   Layer 基类 + LayerStack (分层架构)
│   │   ├── Application.h / .cpp     #   应用基类 (Run循环/事件分发)
│   │   └── EntryPoint.h             #   外部入口宏 (CreateApplication → main)
│   │
│   ├── renderer/                     # 渲染子系统
│   │   ├── RendererAPI.h / .cpp      #   渲染API抽象 (Init/Clear/Draw)
│   │   ├── Renderer.h / .cpp         #   渲染器主类 (BeginScene/Submit/EndScene)
│   │   ├── RenderCommand.h           #   渲染命令封装 (glClear/glDraw)
│   │   ├── Shader.h / .cpp           #   着色器 (编译/Uniform缓存/热重载)
│   │   ├── Texture.h / .cpp          #   纹理抽象 (2D/CubeMap/Mipmap)
│   │   ├── Framebuffer.h / .cpp      #   帧缓冲 (多颜色附件+深度/模板)
│   │   ├── VertexArray.h / .cpp      #   VAO 封装 (动态顶点布局)
│   │   ├── Buffer.h / .cpp           #   VBO + IndexBuffer 抽象
│   │   ├── Camera.h / .cpp           #   自由相机 (FPS风格+欧拉角旋转)
│   │   ├── Material.h / .cpp         #   PBR材质 (Albedo/Metallic/Roughness/AO/Emission)
│   │   ├── Light.h / .cpp            #   光源系统 (Directional/Point/Spot + LightEnvironment)
│   │   └── UniformBuffer.h           #   UBO 管理 (CameraUBO/LightUBO)
│   │
│   ├── resource/                     # 资源管理
│   │   ├── ShaderLibrary.h           #   Shader 缓存池 (按名索引)
│   │   ├── TextureLibrary.h / .cpp   #   纹理缓存池 (含 Skybox/CubeMap 加载)
│   │   └── MeshLibrary.h / .cpp      #   内置几何体库 (Cube/Sphere/Plane/ScreenQuad)
│   │
│   ├── scene/                        # 场景系统
│   │   ├── Scene.h / .cpp            #   场景容器 (Entity CRUD/渲染遍历)
│   │   ├── TransformComponent.h      #   变换组件 (Position/Rotation/Scale/父子层级)
│   │   ├── MeshComponent.h           #   网格组件 (VAO引用 + Material引用)
│   │   ├── LightComponent.h          #   光源组件 (灯类型+参数)
│   │   ├── SceneSerializer.h / .cpp  #   场景序列化 (.scene 文本格式)
│   │
│   ├── postprocess/                  # 后处理管线
│   │   ├── PostProcess.h / .cpp      #   BloomPass + ToneMappingPass + Pipeline
│   │   └── (FXAA / SSAO 待扩展)
│   │
│   └── editor/                       # 编辑器
│       ├── EditorLayer.h / .cpp      #   编辑器主层 (ImGui 面板集合)
│       └── (SceneHierarchyPanel / InspectorPanel / ViewportPanel 内聚在 EditorLayer)
│
├── shader/                           # GLSL 着色器
│   ├── pbr.vert / pbr.frag           #   PBR 着色器 (Cook-Torrance BRDF)
│   ├── screen.vert / screen.frag      #   全屏后处理着色器
│   ├── skybox.vert / skybox.frag      #   天空盒着色器
│   └── *.vs / *.fs                   #   原有学习用着色器 (保留)
│
├── resources/                        # 资源文件
│   ├── textures/                     #   纹理贴图
│   └── models/                       #   3D 模型
│
├── external/                         # 第三方库 (header-only)
│   ├── glm-master/                   #   数学库
│   ├── imgui-master/                 #   ImGui 调试UI
│   └── ...
│
├── include/                          # GLAD / GLFW / stb 头文件
├── lib/                              # 预编译库 (glfw3, assimp)
└── src/                              # 旧代码 (glad, 学习demo，保留参考)
```

---

## 四、核心子系统详解

### 4.1 引擎核心 (engine/core/)

#### Base — 基础类型与指针别名

```cpp
// 跨平台整数类型
using uint32 = uint32_t;

// 智能指针别名（可无缝切换内存分配器）
template<typename T> using Ref   = std::shared_ptr<T>;
template<typename T> using Scope = std::unique_ptr<T>;
template<typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) { return std::make_shared<T>(...); }
```

#### Log — 分级日志

```cpp
CORE_TRACE("Loading texture: ", path);
CORE_INFO("Scene loaded with ", entityCount, " entities.");
CORE_WARN("Shader compilation warning: ", msg);
CORE_ERROR("Failed to open file: ", filepath);
CORE_CRITICAL("Out of GPU memory!");
```

- 默认级别 `Trace`，所有日志均输出
- Debug/Release 均可用，Error+ 级别输出到 stderr
- 自动捕获 `__FILE__:__LINE__`，带彩色时间戳

#### Assert — 断言

```cpp
CORE_ASSERT(shader, "Shader is null!");
CORE_ASSERT_DEBUG(camera != nullptr, "Camera pointer is null");
CORE_STATIC_ASSERT(sizeof(TransformComponent) <= 128, "Transform too large");
```

- `CORE_ASSERT` 在所有构建中有效（致命）
- `CORE_ASSERT_DEBUG` 仅 Debug 构建有效
- 可通过 `#define CORE_DISABLE_ASSERTS` 全局关闭

#### Timestep — 帧时间步

```cpp
Timestep ts(deltaTime);          // 秒
float dt  = ts.GetSeconds();      // 秒
float ms  = ts.GetMilliseconds(); // 毫秒
ts.SetTimeScale(0.5f);           // 慢动作
ts = Timestep::Clamp(ts, 0.1f);  // 钳制最大间隔（防编辑器失焦）
```

#### Event — 事件系统

```
Event (基类)
├── KeyPressedEvent    (按键按下)
├── KeyReleasedEvent   (按键释放)
├── MouseMovedEvent    (鼠标移动)
├── MouseScrolledEvent (滚轮)
├── MouseButtonPressedEvent
├── MouseButtonReleasedEvent
├── WindowResizeEvent
└── WindowCloseEvent
```

`EventDispatcher::Dispatch<T>(handler)` 按类型分发，`e.Handled = true` 阻止冒泡。

#### Layer Stack — 分层架构

```
LayerStack (in Application::Run)
  ┌──────────────────┐  ← render last / handle event first
  │   Overlay Layer   │     (ImGui EditorLayer)
  ├──────────────────┤
  │   Normal Layer    │     (SandboxLayer / GameLayer)
  └──────────────────┘  ← render first / handle event last
```

每层有 5 个生命周期回调：
- `OnAttach()` / `OnDetach()` — 层的挂载与卸载
- `OnUpdate(Timestep ts)` — 每帧逻辑更新
- `OnEvent(Event& e)` — 事件处理
- `OnImGuiRender()` — ImGui 绘制（Application::Run 自动调用）

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

#### 渲染流程

```
Scene::OnRender(camera)
  ├── CollectLights()          # 收集光源数据 → LightEnvironment
  ├── SortEntities()           # 按材质排序减少状态切换
  ├── Renderer::BeginScene()   # 设置 VP 矩阵
  │   ├── 绑定 LightUBO
  │   └── 绑定 CameraUBO
  ├── For each Entity:
  │   ├── Material::Bind()
  │   ├── VertexArray::Bind()
  │   └── Renderer::Submit()
  └── Renderer::EndScene()
```

---

### 4.3 后处理管线 (engine/postprocess/)

```
PostProcessPipeline
  ├── BloomPass
  │   ├──亮度提取 → 水平模糊 → 垂直模糊 → 合成
  │   └──可调参数: 阈值 / 强度 / 模糊迭代次数
  ├── ToneMappingPass
  │   └──ACES / Reinhard 色调映射
  └── Execute(hdrFBO) → outputFBO
```

---

### 4.4 资源管理 (engine/resource/)

| 库 | 职责 | 关键API |
|----|------|---------|
| `ShaderLibrary` | Shader 按名缓存 | `Get(name)` `Load(name, vs, fs)` `ReloadAll()` |
| `TextureLibrary` | 纹理按路径缓存 | `Load2D(path)` `LoadCube(faces)` |
| `MeshLibrary` | 内置几何体 | `GetCube()` `GetPlane()` `GetScreenQuad()` |

- `ShaderLibrary` 支持热重载：`ReloadAll()` 重新编译所有 Shader
- `TextureLibrary` 自动幂等加载，避免重复创建 GPU 纹理

---

### 4.5 场景系统 (engine/scene/)

#### Entity-Component（轻量 ECS）

```
Entity (uint32_t ID)
├── TransformComponent    (Position/Rotation/Scale/EulerAngles)
├── MeshComponent         (VertexArray引用 + Material引用 + Shadow标志)
└── LightComponent        (光源类型 + 参数)
```

Scene 提供：
- `CreateEntity(tag)` / `DestroyEntity(entity)`
- `OnUpdate(Timestep ts)` / `OnRender(camera)`
- `ForEachEntity(callback)` 遍历所有实体
- 自动收集光源数据 → `LightEnvironment`

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
[/Entity]

[Entity] tag="Point Light" id=2
  [Transform] px=0 py=2 pz=2 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1
  [Light] type=Point r=1 g=0.3 b=0.2 intensity=3 range=10
[/Entity]

[/Scene]
```

使用方式：
```cpp
SceneSerializer::SaveToFile(*scene, "resources/scenes/demo.scene");
auto loaded = SceneSerializer::LoadFromFile("resources/scenes/demo.scene");
```

---

### 4.6 编辑器 (engine/editor/)

EditorLayer 提供 5 个 ImGui 面板（可在 View 菜单开关）：

| 面板 | 功能 |
|------|------|
| **Viewport** | 场景渲染视图（FBO → ImGui::Image），支持焦点检测 |
| **Scene Hierarchy** | 实体树形列表，点击选中 |
| **Inspector** | 选中实体的 Tag / Transform / Mesh 属性编辑 |
| **Rendering Stats** | FPS / DrawCalls / Triangles / Vertices / Entity Count |
| **Light Editor** | 方向光参数调节（Direction / Color / Intensity） |

编辑器快捷键：
- `F3` — 切换线框模式
- 鼠标聚焦 Viewport 时启用自由相机控制

---

## 五、数据流图（一帧的完整流程）

```
main()
  └── CreateApplication() → new Application(...)
      └── app->PushLayer(new SandboxLayer())
          └── app->Run()
              │
              ▼
  ┌────────────────────────────────────────────────────────────┐
  │                    每帧循环 (Application::Run)             │
  │                                                            │
  │  1. Timestep ts = Clamp(glfwGetTime - m_LastFrameTime)    │
  │                                                            │
  │  2. Layer::OnUpdate(ts)                                    │
  │     ├── SandboxLayer::HandleInput(dt)                      │
  │     │   └── Camera::ProcessKeyboard / ProcessMouse         │
  │     ├── Scene::OnUpdate(ts)                                │
  │     ├── ViewportFBO::Bind()                                │
  │     │   └── Scene::OnRender(camera)                        │
  │     │       ├── CollectLights() → LightEnvironment         │
  │     │       ├── BeginScene(view, proj)                     │
  │     │       │   └── 绑定 CameraUBO + LightUBO              │
  │     │       └── ForEach Entity:                            │
  │     │           └── Material::Bind()                       │
  │     │               └── VertexArray::Bind()                │
  │     │                   └── Renderer::Submit()             │
  │     │                       └── glDrawElements()           │
  │     ├── EditorLayer::OnUpdate(ts)                          │
  │     │   └── 线框模式 / 鼠标捕获状态切换                    │
  │     └── ViewportFBO::Unbind()                              │
  │                                                            │
  │  3. Layer::OnImGuiRender()                                 │
  │     ├── ImGui_ImplOpenGL3_NewFrame()                       │
  │     └── EditorLayer::OnImGuiRender()                       │
  │         ├── DrawMenuBar() → 菜单栏                         │
  │         ├── DrawViewport() → FBO渲染到ImGui::Image         │
  │         ├── DrawSceneHierarchy() → 实体树                  │
  │         ├── DrawInspector() → 属性面板                     │
  │         ├── DrawStatsPanel() → 渲染统计                    │
  │         └── DrawLightEditor() → 光源参数                   │
  │                                                            │
  │  4. Window::OnUpdate()                                     │
  │     └── glfwSwapBuffers | glfwPollEvents                   │
  │         └── GLFW Callback → Event → Application::OnEvent   │
  │             ├── WindowCloseEvent  → m_Running = false      │
  │             ├── WindowResizeEvent → glViewport             │
  │             └── LayerStack::OnEvent (reverse iteration)    │
  └────────────────────────────────────────────────────────────┘
```

---

## 六、改造前后对比

| 维度 | 改造前 | 改造后 |
|------|--------|--------|
| **架构模式** | 单文件过程式 (all in main.cpp) | 分层面向对象 + 轻量 ECS |
| **源文件数** | ~8 个 | **50+ 个** |
| **核心类** | Shader/Camera/Mesh/Model (4个) | **30+ 个** 引擎类 |
| **渲染管线** | 裸 Forward 调用 | PBR + 后处理 + RenderPass 架构 |
| **材质系统** | 无 | Cook-Torrance PBR (6 种纹理通道) |
| **光照系统** | 无 (代码写了但未使用) | Directional/Point/Spot + UBO |
| **后处理** | 无 | Bloom + ToneMapping |
| **资源管理** | 手动加载/释放 | RefCache 缓存池 + Shader 热重载 |
| **场景管理** | 硬编码顶点数据 | Scene + Entity + Component + 序列化 |
| **输入系统** | glfwGetKey 裸调 | Input 静态轮询 + Event 事件驱动 |
| **日志/断言** | std::cout | CORE_TRACE~CRITICAL + CORE_ASSERT |
| **时间步** | 裸 float | Timestep 类 (缩放/钳制) |
| **编辑器** | 无 | ImGui 5 面板编辑器 |
| **代码规范** | using namespace std 在头文件 | Ref<T>/Scope<T> 统一别名 |
| **构建系统** | 单可执行文件 | 引擎静态库 + Sandbox 可执行文件 |

---

## 七、依赖关系

```
┌──────────────────────────────────────────────────────┐
│                     SandboxApp                        │
│          (链接 OpenGlEngine 静态库)                    │
└──────────────────────┬───────────────────────────────┘
                       │ 依赖
┌──────────────────────▼───────────────────────────────┐
│                  OpenGlEngine.lib                      │
│  engine/core + engine/renderer + engine/scene + ...   │
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
| GLFW | 3.4 | 窗口/输入/OpenGL 上下文 | 预编译 `.a` |
| GLAD | 4.6 | OpenGL 函数加载 | 源文件编译 |
| GLM | 0.9.9 | 数学库 (vec3/mat4/quat) | header-only |
| Assimp | 6.x | 3D 模型导入 | 预编译 `.dll.a` |
| stb_image | — | 图片加载 | header-only |
| ImGui | master | 调试 UI | 源文件编译 |

---

## 八、扩展路线图

### ✅ 已完成 (v1.0)

- [x] 引擎核心基础设施 (Application/Window/Input/Event/Layer/Log/Assert/Timestep/Base)
- [x] 渲染器抽象 (RendererAPI/Shader/Texture/Framebuffer/VertexArray/Buffer/Camera)
- [x] PBR 材质系统 (Cook-Torrance + 6 纹理通道)
- [x] 光照系统 (DirectionalLight/PointLight/SpotLight + UBO)
- [x] 资源管理器 (ShaderLibrary/TextureLibrary/MeshLibrary + 热重载)
- [x] 场景系统 (Entity + Transform/Mesh/Light Component)
- [x] 场景序列化 (SceneSerializer → `.scene` 文本)
- [x] 后处理管线 (BloomPass + ToneMappingPass)
- [x] ImGui 编辑器 (Hierarchy/Inspector/Stats/Light Editor/Viewport)
- [x] CMake 多目标构建 (引擎静态库 + Sandbox)

### 🚧 计划中 (v1.1+)

| 优先级 | 功能 | 说明 |
|--------|------|------|
| P0 | Shadow Mapping | 方向光 Cascaded Shadow Maps |
| P0 | Skybox / IBL | 天空盒 + 基于图像的照明 |
| P1 | 骨骼动画 | 接入 Assimp 动画通道，GPU Skinning |
| P1 | SSAO | 屏幕空间环境光遮蔽 |
| P2 | 延迟渲染 | Deferred Shading G-Buffer 管线 |
| P2 | 粒子系统 | GPU 粒子 + Compute Shader |
| P2 | 批量渲染 | DrawIndirect + 合批优化 |
| P3 | spdlog 替换 | 当前自实现日志 → spdlog |
| P3 | entt 集成 | 替换轻量 Entity → 成熟 ECS |
| P3 | yaml-cpp | 替换 `.scene` 格式 → YAML |
| P4 | Vulkan/DX12 后端 | Platform 抽象层 |

---

## 九、编译与运行

```bash
# 配置
cd OpenGl-GraphicsEngine
mkdir build && cd build
cmake ..

# 编译
cmake --build . --config Release

# 运行
./Sandbox     # Linux/macOS
Sandbox.exe   # Windows

# 运行测试
./Tests       # Linux/macOS
Tests.exe     # Windows
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
| 日志 | 使用 `CORE_TRACE/INFO/WARN/ERROR` 宏，不直接使用 `std::cout` |
| 断言 | 使用 `CORE_ASSERT` 检查前置条件 |
| 时间 | 使用 `Timestep` 类型传递，不用裸 `float` |
| 命名空间 | 无全局 using namespace (已在旧代码中移除) |

---

*本文档随项目同步更新，反映最新架构状态。*
