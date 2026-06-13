# OpenGL Graphics Engine — 代码结构梳理图

> 项目总架构师绘制 | v2.1 | 2026-06-12  
> 配合 `ONBOARDING.md` 使用，本文档侧重 **视觉化架构概览**。

---

## 一、项目全景拓扑图

```
                         ┌──────────────────────────┐
                         │     CreateApplication()    │
                         │     (app/SandboxApp.cpp)   │
                         │                            │
                         │ new Application("OGL",     │
                         │                 1600,900)  │
                         │ → PushLayer(SandboxLayer)  │
                         └────────────┬───────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    Application::Run()  主循环                        │
│                                                                     │
│  while (m_Running) {                                                │
│     1. Timestep ts = ...                                            │
│     2. SandboxLayer::OnUpdate(ts)     ←─── 所有逻辑/渲染            │
│     3. EditorLayer::OnImGuiRender()   ←─── 编辑器面板               │
│     4. Window::OnUpdate()             ←─── Swap + Poll              │
│  }                                                                  │
└─────────────────────────────────────────────────────────────────────┘
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
┌──────────────────┐ ┌──────────┐ ┌──────────────────┐
│   SandboxLayer    │ │ Editor   │ │    Window         │
│   (游戏逻辑层)     │ │ Layer    │ │    (GLFW窗口)     │
│                  │ │          │ │                   │
│ HandleInput()    │ │ Hierarchy│ │ glfwPollEvents()  │
│ UpdateCamera()   │ │ Inspector│ │    → Event系统     │
│ Scene::OnUpdate()│ │ Viewport │ │ glfwSwapBuffers() │
│ Shadow Pass      │ │ Stats    │ │                   │
│ Scene::OnRender()│ │ Gizmo    │ │                   │
│ PostProcess      │ │          │ │                   │
└───────┬──────────┘ └──────────┘ └──────────────────┘
        │
        │ 驱动
        ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        Scene (场景容器)                               │
│                                                                     │
│  m_Entities: [Entity₀, Entity₁, ..., Entityₙ]                       │
│  m_LightEnv: LightEnvironment                                       │
│  m_ShadowMap: shared_ptr<ShadowMap>                                 │
│  m_IBL: shared_ptr<IBL>                                             │
│  m_Physics: unique_ptr<PhysicsWorld>                                │
│                                                                     │
│  OnUpdate(ts):                                                      │
│    ├── ScriptComponent::OnUpdate()    for each entity               │
│    ├── PhysicsWorld::Step(ts)         碰撞检测+响应                  │
│    └── ParticleEmitter::Update(ts)    粒子更新                       │
│                                                                     │
│  OnRender(camera):                                                  │
│    ├── CollectLights()  → LightEnvironment                          │
│    ├── SortEntities()   按材质排序                                   │
│    ├── Frustum::Cull()  视锥体剔除                                   │
│    ├── LOD::Select()    LOD 级别选择                                 │
│    └── ForEach visible entity → Material::Bind() → Submit()         │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 二、完整目录树

```
OpenGl-GraphicsEngine/
│
├── 📄 CMakeLists.txt                  # CMake 构建: 静态库 + Sandbox + Tests
├── 📄 README.md                       # 项目简介 / 快速开始 / CI 信息
├── 📄 ARCHITECTURE.md                 # ★ 完整架构文档
├── 📄 ONBOARDING.md                   # ★ 新员工交接手册
├── 📄 CODE_STRUCTURE_DIAGRAM.md       # ★ 本文档
├── 📄 CODE_REVIEW_SPEC.md             # Code Review 规范
├── 📄 FIX_REPORT.md                   # 代码质量修复报告
├── 📄 todo.md                         # 开发路线图
├── 📄 ai_advice.md                    # 原始改造建议 (归档)
│
├── 📁 .github/
│   └── 📁 workflows/
│       └── 📄 pr-checks.yml           # CI: Ubuntu + Windows 编译测试
│
├── 📁 app/                            # ── 应用层 (游戏逻辑) ──
│   └── 📄 SandboxApp.cpp              # 614行, 演示: PBR/CSM/Physics/Particles/Script
│
├── 📁 tests/                          # ── 测试层 ──
│   ├── 📄 Test.h                      # 轻量测试框架 (header-only)
│   ├── 📄 TestMain.cpp                # 测试入口
│   ├── 📄 TestBase.cpp                # Base 类型 / 智能指针别名
│   ├── 📄 TestTimestep.cpp            # Timestep
│   ├── 📄 TestLog.cpp                 # 日志系统
│   ├── 📄 TestLayerStack.cpp          # Layer + LayerStack
│   ├── 📄 TestEvent.cpp               # Event + EventDispatcher
│   └── 📄 TestSceneSerializer.cpp     # 场景序列化
│
├── 📁 engine/                         # ── 引擎核心 (静态库 OpenGlEngine) ──
│   │
│   ├── 📁 core/                       # ◆ 引擎基础设施 (11 files)
│   │   ├── Base.h                     #    Ref/Scope/CreateRef/CreateScope
│   │   ├── Log.h / .cpp               #    CORE_TRACE~CRITICAL 分级日志
│   │   ├── Assert.h                   #    CORE_ASSERT / _DEBUG / _STATIC
│   │   ├── Timestep.h                 #    时间步 (秒/毫秒/钳制)
│   │   ├── KeyCodes.h                 #    键盘按键枚举
│   │   ├── MouseCodes.h               #    鼠标按键枚举
│   │   ├── Event.h                    #    Event 体系 + EventDispatcher
│   │   ├── Window.h / .cpp            #    GLFW 窗口 → 引擎事件映射
│   │   ├── Input.h / .cpp             #    输入轮询 (键盘/鼠标状态)
│   │   ├── Layer.h / .cpp             #    Layer 基类 + LayerStack
│   │   ├── Application.h / .cpp       #    主循环 / 事件分发 / ImGui 初始化
│   │   └── EntryPoint.h               #    入口宏 → main()
│   │
│   ├── 📁 renderer/                   # ◆ 渲染子系统 (22 files)
│   │   ├── RendererAPI.h / .cpp       #    RendererAPI (OpenGL 后端)
│   │   ├── Renderer.h / .cpp          #    Renderer 门面 (Begin/End/Submit)
│   │   ├── RenderCommand.h            #    渲染命令封装
│   │   ├── Shader.h / .cpp            #    Shader 编译/Uniform缓存/热重载
│   │   ├── Texture.h / .cpp           #    Texture2D / TextureCubeMap
│   │   ├── Framebuffer.h / .cpp       #    FBO (HDR RGBA16F / 多附件)
│   │   ├── VertexArray.h / .cpp       #    VAO 封装 + 布局描述
│   │   ├── Buffer.h / .cpp            #    VBO / IBO (禁止拷贝/移动语义)
│   │   ├── Camera.h / .cpp            #    自由相机 (FPS风格/动态宽高比)
│   │   ├── Material.h / .cpp          #    PBR 材质 (6通道纹理)
│   │   ├── Light.h / .cpp             #    多光源 + LightEnvironment
│   │   ├── UniformBuffer.h            #    UBO (CameraUBO / LightUBO)
│   │   ├── ShadowMap.h / .cpp         #    CSM 级联阴影 (3级联/Poisson PCF)
│   │   ├── IBL.h / .cpp               #    IBL (辐照度/预过滤/BRDF LUT)
│   │   ├── Frustum.h / .cpp           #    视锥体剔除 (6平面提取)
│   │   └── LOD.h / .cpp               #    LOD (距离/屏幕占比模式)
│   │
│   ├── 📁 resource/                   # ◆ 资源管理 (8 files)
│   │   ├── ShaderLibrary.h / .cpp     #    Shader 缓存池 (单例/热重载)
│   │   ├── TextureLibrary.h / .cpp    #    纹理缓存池 (单例)
│   │   ├── MeshLibrary.h / .cpp       #    内置几何体 (Cube/Sphere/Plane)
│   │   └── Model.h / .cpp             #    Assimp 模型导入 (.obj/.fbx/.gltf)
│   │
│   ├── 📁 scene/                      # ◆ 场景系统 (10 files)
│   │   ├── Scene.h / .cpp             #    场景容器 (Entity CRUD/剔除/渲染)
│   │   ├── TransformComponent.h       #    变换 (位置/旋转/缩放/层级)
│   │   ├── MeshComponent.h            #    网格 (VAO + Material)
│   │   ├── LightComponent.h           #    光源组件
│   │   ├── ScriptComponent.h          #    脚本组件 (Lambda 回调)
│   │   ├── Prefab.h / .cpp            #    预制体 (模板/实例化/序列化)
│   │   └── SceneSerializer.h / .cpp   #    场景序列化 (.scene 文本)
│   │
│   ├── 📁 postprocess/                # ◆ 后处理 (2 files)
│   │   └── PostProcess.h / .cpp       #    Bloom + ToneMapping + Pipeline
│   │
│   ├── 📁 physics/                    # ◆ 物理系统 (2 files)
│   │   └── PhysicsWorld.h / .cpp      #    AABB/Sphere碰撞 + 弹性响应 + Raycast
│   │
│   ├── 📁 particle/                   # ◆ 粒子系统 (2 files)
│   │   └── ParticleEmitter.h / .cpp   #    CPU粒子 + Billboard
│   │
│   ├── 📁 audio/                      # ◆ 音频系统 (2 files)
│   │   └── AudioSystem.h / .cpp       #    SoLoud 接口 (Stub)
│   │
│   ├── 📁 ui/                         # ◆ 游戏UI (2 files)
│   │   └── UIElements.h / .cpp        #    SpriteBatch + Canvas + 控件
│   │
│   └── 📁 editor/                     # ◆ 编辑器 (2 files)
│       └── EditorLayer.h / .cpp       #    ImGui 7+ 面板编辑器
│
├── 📁 shader/                         # ── GLSL 着色器 (17 files) ──
│   ├── 📄 pbr.vert                    # PBR 顶点着色器
│   ├── 📄 pbr.frag                    # PBR 片元 (Cook-Torrance + CSM + IBL)
│   ├── 📄 skybox.vert                 # 天空盒顶点
│   ├── 📄 skybox.frag                 # 天空盒片元
│   ├── 📄 screen.vert                 # 全屏后处理顶点
│   ├── 📄 screen.frag                 # 全屏后处理片元
│   ├── 📄 bloom_combine.frag          # Bloom 合成
│   ├── 📄 brightness.frag             # 亮度提取
│   ├── 📄 gaussian_blur.frag          # 高斯模糊
│   ├── 📄 tonemapping.frag            # ACES 色调映射
│   ├── 📄 ssao.frag                   # SSAO 采样 (待集成)
│   ├── 📄 ssao_blur.frag              # SSAO 双边模糊 (待集成)
│   ├── 📄 ssao_combine.frag           # SSAO 合成 (待集成)
│   ├── 📄 depth_testing.vs            # 深度测试 (学习用)
│   ├── 📄 depth_testing.fs            # 深度测试 (学习用)
│   ├── 📄 stencil_testing.vs          # 模板测试 (学习用)
│   └── 📄 stencil_testing.fs          # 模板测试 (学习用)
│
├── 📁 resources/                      # ── 游戏资源 ──
│   ├── 📁 textures/                   # 纹理贴图
│   └── 📁 models/                     # 3D 模型文件
│
├── 📁 external/                       # ── 第三方库 (header-only) ──
│   ├── 📁 glm-master/                 # GLM 数学库
│   └── 📁 imgui-master/               # Dear ImGui
│
├── 📁 include/                        # GLAD / GLFW / stb 头文件
└── 📁 lib/                            # 预编译 .a / .dll (glfw3, assimp)
```

---

## 三、核心类继承与依赖关系图

### 3.1 引擎核心 — 所有权链

```
EntryPoint.h
 └── int main()
      └── CreateApplication()  ← 由用户实现
           └── Application (m_Window + m_LayerStack)
                │
                ├── Window (m_Window: GLFWwindow*)
                │    └── GLFW Callback → Event → Application::OnEvent()
                │
                └── LayerStack (vector<unique_ptr<Layer>>)
                     ├── SandboxLayer : Layer
                     │    ├── HandleInput()
                     │    ├── UpdateCameraRotation()
                     │    ├── Scene::OnUpdate(ts)
                     │    ├── Shadow Pass
                     │    ├── Scene::OnRender(camera)
                     │    ├── PostProcess::Execute()
                     │    └── EditorLayer::OnUpdate()
                     │
                     └── EditorLayer : Layer
                          └── OnImGuiRender()
                               ├── DrawMenuBar()
                               ├── DrawViewport()
                               ├── DrawSceneHierarchy()
                               ├── DrawInspector()
                               ├── DrawContentBrowser()
                               ├── DrawPrefabPanel()
                               └── DrawStatsPanel()
```

### 3.2 事件系统 — 类型体系

```
Event (基类, EventType 枚举区分)
 │
 ├── KeyEvent
 │    ├── KeyPressedEvent   (keyCode, repeatCount)
 │    ├── KeyReleasedEvent  (keyCode)
 │    └── KeyTypedEvent     (keyCode)
 │
 ├── MouseEvent
 │    ├── MouseMovedEvent        (x, y)
 │    ├── MouseScrolledEvent     (xOffset, yOffset)
 │    ├── MouseButtonPressedEvent (button)
 │    └── MouseButtonReleasedEvent(button)
 │
 └── WindowEvent
      ├── WindowResizeEvent    (width, height)
      ├── WindowCloseEvent
      ├── WindowFocusEvent
      └── WindowLostFocusEvent

EventDispatcher
 └── Dispatch<T>(handler)
      └── if (event.GetEventType() == T::GetStaticType())
           → handler(static_cast<T&>(event))
           → event.Handled = true  (阻止继续冒泡)
```

### 3.3 渲染器 — OpenGL 对象封装

```
Renderer (门面)
 ├── static Init() / Shutdown()
 ├── static BeginFrame() / EndFrame()
 ├── static Submit(vao)
 ├── static SetClearColor() / SetViewport()
 └── 委托 →
      RendererAPI (OpenGL 实现)
           ├── Init()          → gladLoadGLLoader()
           ├── Clear()         → glClear()
           ├── DrawIndexed()   → glDrawElements()
           └── SetViewport()   → glViewport()

OpenGL 资源对象层次:
 ┌──────────────────────────────────────────────┐
 │               VertexArray (VAO)               │
 │  ┌─────────────────────────────────────────┐ │
 │  │   VertexBuffer (VBO)                    │ │
 │  │   ├── BufferLayout { Float3, Float2 }   │ │
 │  │   └── void* data + size                 │ │
 │  ├─────────────────────────────────────────┤ │
 │  │   IndexBuffer (IBO)                     │ │
 │  │   └── uint32_t* indices + count         │ │
 │  └─────────────────────────────────────────┘ │
 └──────────────────────────────────────────────┘

 ┌──────────────────────────────────────────────┐
 │               Framebuffer (FBO)               │
 │  ├── ColorAttachment  (GL_RGBA16F)  [可选多] │
 │  ├── DepthAttachment  (GL_DEPTH_COMPONENT)   │
 │  └── Spec: Width, Height, Samples            │
 └──────────────────────────────────────────────┘

 ┌──────────────────────────────────────────────┐
 │               Texture                         │
 │  ├── Texture2D     (GL_TEXTURE_2D)            │
 │  │   ├── Load(path) → stb_image              │
 │  │   └── LoadHDR(path) → float* data         │
 │  └── TextureCubeMap (GL_TEXTURE_CUBE_MAP)    │
 │      └── LoadCubeMap({6 face paths})         │
 └──────────────────────────────────────────────┘

 ┌──────────────────────────────────────────────┐
 │               Shader                          │
 │  ├── Compile(GL_VERTEX_SHADER, src)           │
 │  ├── Compile(GL_FRAGMENT_SHADER, src)         │
 │  ├── Link() → 错误传播                        │
 │  ├── Uniform Cache (map<string,int>)          │
 │  ├── Bind() → glUseProgram()                  │
 │  └── SetInt/SetFloat/SetMat4(...)             │
 └──────────────────────────────────────────────┘
```

### 3.4 场景系统 — Entity 胖实体结构

```
Entity
 ├── uint32_t    ID
 ├── string      Tag
 ├── bool        IsActive / IsStatic
 │
 ├── TransformComponent                    ← [核心, 必有]
 │    ├── vec3 Position / Rotation / Scale
 │    ├── uint32_t ParentID
 │    └── mat4 GetWorldMatrix()    (含层级)
 │
 ├── MeshComponent                         ← [核心, 必有]
 │    ├── shared_ptr<VertexArray> VertexArray
 │    ├── shared_ptr<Material>    Material
 │    ├── bool Visible / CastShadow
 │    └── RendererID  (临时渲染 ID)
 │
 ├── LightComponent          [可选, HasLight]
 │    ├── LightType: Directional / Point / Spot
 │    └── 对应光源参数
 │
 ├── ScriptComponent         [可选, HasScript]
 │    ├── string ScriptName
 │    ├── lambda OnCreate(Entity&)
 │    ├── lambda OnUpdate(Entity&, Timestep)
 │    └── lambda OnDestroy(Entity&)
 │
 ├── ColliderComponent       [可选, HasPhysics]
 │    ├── ColliderType: AABB / Sphere
 │    ├── AABB Box / float Radius
 │    └── bool IsTrigger
 │
 ├── RigidbodyComponent      [可选, HasPhysics]
 │    ├── float Mass / Restitution / Friction
 │    ├── vec3 Velocity / Force
 │    ├── bool IsStatic / UseGravity
 │    └── AddForce() / AddImpulse()
 │
 ├── AudioSourceComponent    [可选, HasAudioSource]
 │    ├── string ClipPath
 │    ├── float Volume / Pitch
 │    └── bool Loop / Is3D
 │
 ├── AudioListenerComponent  [可选, HasAudioListener]
 │
 ├── ParticleEmitter*        [可选, HasParticleEmitter]
 │    └── unique_ptr<ParticleEmitter>
 │
 └── LODComponent            [可选, HasLOD]
      ├── LODMode: Distance / ScreenSize
      ├── vector<LODLevel> Levels[4]
      └── float Bias

Scene 管理:
 ├── vector<Entity>            m_Entities
 ├── map<uint32_t, uint32_t>   m_EntityMap   (ID→index)
 ├── LightEnvironment          m_LightEnv
 ├── shared_ptr<ShadowMap>     m_ShadowMap
 ├── shared_ptr<IBL>           m_IBL
 ├── unique_ptr<PhysicsWorld>  m_PhysicsWorld
 ├── bool m_FrustumCulling     = true
 └── int  m_DrawCalls / m_TriangleCount / m_VisibleEntities
```

---

## 四、渲染管线数据流图

```
                    ┌─────────────────────────────┐
                    │     Camera (CPU 侧)           │
                    │  ├── ViewMatrix              │
                    │  ├── ProjectionMatrix         │
                    │  └── Position                │
                    └─────────────┬───────────────┘
                                  │
                    ┌─────────────▼───────────────┐
                    │    CameraUBO (binding=0)      │
                    │    { View, Proj, ViewProj,    │
                    │      CameraPos }              │
                    └─────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      Shadow Pass (Pass 0)                            │
│                                                                     │
│  ShadowMap::CalculateCascades(lightDir, view, proj, near, far)       │
│    ├── 3 级联 PSSM 分割                                             │
│    ├── 3× LightSpaceMatrix → ShadowUBO                              │
│    └── Output: u_LightSpaceMatrices[3] + u_CascadeSplits[3]         │
│                                                                     │
│  ShadowRenderer::RenderShadowPass(shadowMap, casters)               │
│    ├── For each cascade:                                            │
│    │   ├── glViewport(cascade 区域)                                 │
│    │   ├── Bind shadow FBO (GL_DEPTH_ATTACHMENT)                    │
│    │   └── For each caster: glDrawElements() → 深度写入             │
│    └── Output: 2048×2048 depth texture array                        │
└─────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      Scene Pass (Pass 1)                             │
│                                                                     │
│  SceneFBO::Bind() → glClear(COLOR | DEPTH)  [RGBA16F HDR]           │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐ │
│  │ Skybox (glDepthFunc=LEQUAL, 无位移 View)                       │ │
│  │   ├── IBL::BindEnvironmentMap() → 天空盒使用环境贴图            │ │
│  │   └── glDrawElements(Cube)                                     │ │
│  └───────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐ │
│  │ Scene::OnRender(camera)                                       │ │
│  │                                                               │ │
│  │ 1. CollectLights()                                            │ │
│  │    ├── DirectionalLight ×1                                    │ │
│  │    ├── PointLight ×N → LightUBO (binding=1)                   │ │
│  │    └── SpotLight ×N                                           │ │
│  │                                                               │ │
│  │ 2. SortEntities()                                             │ │
│  │    └── 按 Material 排序 (减少状态切换)                         │ │
│  │                                                               │ │
│  │ 3. Frustum Culling                                            │ │
│  │    └── 6 平面 × AABB/Sphere 测试 → 剔除不可见实体              │ │
│  │                                                               │ │
│  │ 4. For Each Visible Entity:                                   │ │
│  │    ├── LOD::Select(cameraDist) → 选择 VAO 级别                │ │
│  │    │                                                          │ │
│  │    ├── Material::Bind()                                       │ │
│  │    │   ├── Shader::Bind()                                     │ │
│  │    │   ├── SetMat4("u_Model", worldMatrix)                    │ │
│  │    │   ├── SetMat4("u_ModelInvTrans", normalMatrix)           │ │
│  │    │   ├── BindTextures(AlbedoMap[N], slot0..5)               │ │
│  │    │   └── SetMaterialProps(albedo, metallic, roughness...)   │ │
│  │    │                                                          │ │
│  │    ├── ShadowMap::Bind() (binding=2)                          │ │
│  │    │   └── u_LightSpaceMatrices + u_CascadeSplits             │ │
│  │    │                                                          │ │
│  │    ├── IBL::Bind()                                            │ │
│  │    │   ├── u_IrradianceMap → slot 7                           │ │
│  │    │   ├── u_PrefilteredMap → slot 8                          │ │
│  │    │   └── u_BRDF_LUT → slot 9                                │ │
│  │    │                                                          │ │
│  │    ├── VertexArray::Bind()                                    │ │
│  │    └── Renderer::Submit() → glDrawElements()                  │ │
│  └───────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  SceneFBO::Unbind()                                                 │
│  Output: RGBA16F HDR 场景纹理                                       │
└─────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────┐
│                   PostProcess Pipeline (Pass 2)                      │
│                                                                     │
│  PostProcessPipeline::Execute(sceneFBO)                              │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ Bloom Pass                                                   │   │
│  │                                                              │   │
│  │ SceneFBO (HDR)                                               │   │
│  │   │                                                          │   │
│  │   ├── [BrightnessPass]  threshold × brightness             │   │
│  │   │   → Downsampled BrightFBO (1/2 size)                    │   │
│  │   │                                                          │   │
│  │   ├── [GaussianBlurH]  水平模糊                              │   │
│  │   │   → BlurHFBO                                             │   │
│  │   │                                                          │   │
│  │   ├── [GaussianBlurV]  垂直模糊                              │   │
│  │   │   → BlurVFBO                                             │   │
│  │   │                                                          │   │
│  │   └── [BloomCombine]   原图 + Bloom × intensity             │   │
│  │       → BloomedFBO                                           │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│  ┌───────────────────────────▼─────────────────────────────────┐   │
│  │ ToneMapping Pass                                             │   │
│  │                                                              │   │
│  │ BloomedFBO (HDR)                                             │   │
│  │   │                                                          │   │
│  │   └── [ACES Film Tone Mapping]                               │   │
│  │       + Gamma Correction (1/2.2)                             │   │
│  │       → FinalFBO (LDR, RGBA8)                                │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  每个 Pass 由 GLStateSaver RAII 保护:                               │
│    Save → Restore (DepthTest, Blend, CullFace, VAO/FBO Binding)     │
└─────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────┐
│                           Blit Pass (Pass 3)                         │
│                                                                     │
│  glBindFramebuffer(GL_READ, FinalFBO)                                │
│  glBindFramebuffer(GL_DRAW, ViewportFBO)                             │
│  glBlitFramebuffer(0,0,1600,900, 0,0,1600,900, COLOR, NEAREST)      │
│                                                                     │
│  Output: ViewportFBO → EditorLayer::DrawViewport() → ImGui::Image   │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 五、GPU 端 Shader 执行流程图

```
                         ┌────────────────┐
                         │   pbr.vert     │
                         │                │
                         │ layout(0) aPos │
                         │ layout(1) aNorm│
                         │ layout(2) aTex │
                         │                │
                         │ u_Model        │← model mat
                         │ u_ViewProj     │← CameraUBO
                         │                │
                         │ → gl_Position  │
                         │ → vsNormal     │
                         │ → vsTexCoord   │
                         │ → vsWorldPos   │
                         │ → vsFragPosLS  │← LightSpace x3
                         └───────┬────────┘
                                 │
                                 ▼
              ┌──────────────────────────────────────┐
              │            pbr.frag                  │
              │                                      │
              │ // 1. 纹理采样                       │
              │ albedo   = texture(albedoMap, UV)    │
              │ normal   = texture(normalMap, UV)    │
              │ metallic = texture(metalMap, UV).r   │
              │ rough    = texture(roughMap, UV).r   │
              │ ao       = texture(aoMap, UV).r     │
              │                                      │
              │ // 2. Direct Light (Cook-Torrance)   │
              │ for each light:                      │
              │   L = normalize(lightPos - WorldPos) │
              │   V = normalize(camPos - WorldPos)   │
              │   H = normalize(L + V)               │
              │                                      │
              │   D = GGX(normal, H, rough)          │
              │   G = Smith(normal, V, L, rough)     │
              │   F = FresnelSchlick(H, V, metallic) │
              │                                      │
              │   spec  = D*G*F / (4*dot(N,L)*dot(N,V)) │
              │   kd    = (1-F) * (1-metallic)       │
              │   Lo   += (kd*albedo/PI + spec) * L*atten │
              │                                      │
              │ // 3. CSM Shadow                     │
              │ cascade = selectCascade(depth)       │
              │ shadow  = PoissonPCF(32 samples)     │
              │ Lo     *= shadow                     │
              │                                      │
              │ // 4. IBL                            │
              │ F        = FresnelSchlickRoughness() │
              │ kd       = (1-F) * (1-metallic)      │
              │ diffuse  = texture(irradiance, N)*albedo │
              │ specular = textureLod(prefiltered, R, rough*4) │
              │ specular *= texture(BRDF_LUT, NdotV, rough).rg │
              │ ambient  = (kd*diffuse + specular) * ao │
              │                                      │
              │ // 5. Emission                       │
              │ emission = texture(emissiveMap, UV)  │
              │                                      │
              │ FragColor = vec4(Lo + ambient + emission, 1) │
              │ ↑ 线性 HDR, 不做 Tone Mapping       │
              └──────────────────────────────────────┘
                                 │
                                 ▼
              ┌──────────────────────────────────────┐
              │       后处理管线 (GPU 端)              │
              │                                      │
              │ 1. brightness.frag                   │
              │    → dot(color, lumWeights) > threshold │
              │    → brightFBO                       │
              │                                      │
              │ 2. gaussian_blur.frag × 2            │
              │    → 水平+垂直 可分离模糊             │
              │    → blurFBO                          │
              │                                      │
              │ 3. bloom_combine.frag                │
              │    → original + blur*bloomIntensity   │
              │    → bloomedFBO                       │
              │                                      │
              │ 4. tonemapping.frag                  │
              │    → ACESFilm(color)                 │
              │    → pow(color, 1.0/2.2)             │
              │    → FragColor (LDR, sRGB)           │
              └──────────────────────────────────────┘
```

---

## 六、物理系统碰撞检测流程

```
PhysicsWorld::Step(dt)
  │
  ├── 1. 积分 (所有动态刚体)
  │   ├── velocity += gravity * GravityScale * dt
  │   ├── velocity += (force / mass) * dt
  │   └── position += velocity * dt
  │
  ├── 2. 碰撞检测 (O(n²) 遍历)
  │   ├── IsStatic==true → 跳过
  │   │
  │   ├── AABB-AABB:
  │   │   for each axis (x,y,z):
  │   │     if (a.max[axis] < b.min[axis]) → no overlap
  │   │     if (a.min[axis] > b.max[axis]) → no overlap
  │   │   → collision normal from minimum penetration axis
  │   │
  │   ├── Sphere-Sphere:
  │   │   dist = distance(a.center, b.center)
  │   │   if (dist < a.radius + b.radius) → collision
  │   │   → normal = (a.center - b.center) / dist
  │   │
  │   └── AABB-Sphere:
  │       closest = clamp(sphere.center, aabb.min, aabb.max)
  │       dist = distance(closest, sphere.center)
  │       if (dist < sphere.radius) → collision
  │
  └── 3. 碰撞响应
      ├── 沿碰撞法线分离叠穿实体
      ├── 相对速度 = (vA - vB) · normal
      ├── 冲量 = -(1 + restitution) * 相对速度 / (1/mA + 1/mB)
      ├── vA += 冲量 * normal / mA
      ├── vB -= 冲量 * normal / mB
      ├── 摩擦: 切向速度 × friction
      │
      ├── OnCollisionEnter(collisionInfo)  ← 首次碰撞
      ├── OnCollisionStay(collisionInfo)   ← 持续碰撞
      └── OnCollisionExit(entityA, entityB)← 碰撞结束
```

---

## 七、构建系统链路

```
CMakeLists.txt
 │
 ├── project(OpenGlEngine VERSION 1.0.0 LANGUAGES C CXX)
 ├── C++17 Standard
 │
 ├── add_library(OpenGlEngine STATIC
 │   ├── engine/core/*.cpp      (Application, Window, Layer, Event...)
 │   ├── engine/renderer/*.cpp  (Renderer, Shader, Texture, Camera...)
 │   ├── engine/resource/*.cpp  (ShaderLibrary, Model, MeshLibrary...)
 │   ├── engine/scene/*.cpp     (Scene, Prefab, SceneSerializer...)
 │   ├── engine/postprocess/*.cpp  (Bloom, ToneMapping...)
 │   ├── engine/editor/*.cpp    (EditorLayer)
 │   ├── engine/physics/*.cpp   (PhysicsWorld)
 │   ├── engine/particle/*.cpp  (ParticleEmitter)
 │   ├── engine/audio/*.cpp     (AudioSystem)
 │   ├── engine/ui/*.cpp        (UIElements)
 │   ├── external/imgui-master/*.cpp  (ImGui 5 files)
 │   └── src/glad/glad.c               (GLAD)
 │   )
 │
 ├── target_link_libraries(OpenGlEngine
 │   ├── libglfw3.a / glfw     (窗口)
 │   ├── libassimp.dll.a / assimp (模型)
 │   └── opengl32 gdi32 / GL dl pthread  (平台)
 │   )
 │
 ├── add_executable(Sandbox
 │   └── app/SandboxApp.cpp
 │   ) → link OpenGlEngine
 │   └── POST_BUILD: copy resources/ shader/ assimp.dll
 │
 └── add_executable(Tests
     └── tests/*.cpp
     ) → link OpenGlEngine
     └── POST_BUILD: copy resources/ shader/ assimp.dll
```

---

## 八、CI/CD 流水线

```
GitHub PR → dev|main
 │
 ├── Job: build-linux
 │   ├── runs-on: ubuntu-22.04
 │   ├── 1. checkout
 │   ├── 2. 安装: libgl1-mesa-dev, libglfw3-dev, libassimp-dev, xorg-dev
 │   ├── 3. cmake -B build -DCMAKE_BUILD_TYPE=Release
 │   ├── 4. cmake --build build -j $(nproc)
 │   └── 5. ./Tests  (全部通过 ✓)
 │
 ├── Job: build-windows
 │   ├── runs-on: windows-2022
 │   ├── 1. checkout
 │   ├── 2. cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
 │   ├── 3. cmake --build build -j 4
 │   └── 4. ./Tests.exe  (全部通过 ✓)
 │
 └── Job: format-check
     ├── runs-on: ubuntu-22.04
     ├── 1. checkout
     ├── 2. 安装 clang-format
     └── 3. 遍历 .cpp/.h → diff vs clang-format (仅报告)
```

---

## 九、快速参考表

### 文件职责速查

| 你想改什么 | 去哪个文件 |
|-----------|-----------|
| 窗口大小/标题 | `engine/core/Window.h` |
| 键盘快捷键 | `app/SandboxApp.cpp → HandleInput()` |
| 渲染管线顺序 | `app/SandboxApp.cpp → OnUpdate()` |
| 着色器效果 | `shader/pbr.frag` |
| 后处理效果 | `shader/*.frag` + `engine/postprocess/PostProcess.cpp` |
| 编辑器面板 | `engine/editor/EditorLayer.cpp` |
| 物理参数 | `engine/physics/PhysicsWorld.h` |
| 粒子效果 | `engine/particle/ParticleEmitter.h` |
| 场景保存格式 | `engine/scene/SceneSerializer.cpp` |
| 添加测试 | `tests/` + 注册到 TestMain.cpp |

### 第三方库位置

| 库 | 类型 | 位置 |
|----|------|------|
| GLFW | 预编译 | `lib/libglfw3.a` / 系统包 |
| GLAD | 源文件 | `src/glad/glad.c` + `include/glad/` |
| GLM | header-only | `external/glm-master/` |
| Assimp | 预编译 | `lib/libassimpd.dll.a` + `libassimp-6d.dll` / 系统包 |
| stb_image | header-only | `include/stb_image.h` |
| ImGui | 源文件 | `external/imgui-master/` |
| SoLoud | 未集成 | (音频 Stub) |

### 关键枚举值

```
LightType:  Directional, Point, Spot
ColliderType: AABB, Sphere
LODMode:    Distance, ScreenSize
CameraMovement: FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN, UP_WORLD, DOWN_WORLD
MouseCode:  Button0(left), Button1(right), Button2(middle)
```

---

## 十、文档索引

| 文档 | 内容 | 适合 |
|------|------|------|
| **README.md** | 项目简介、快速开始、CI 信息 | 首次接触项目 |
| **ONBOARDING.md** | ★ 新员工 3 天上手指南 | 新人入职 |
| **CODE_STRUCTURE_DIAGRAM.md** | ★ 本文档 — 视觉化架构图 | 理解全局结构 |
| **ARCHITECTURE.md** | 完整架构文档、子系统详解 | 深入理解各模块 |
| **CODE_REVIEW_SPEC.md** | Code Review 检查清单 | PR Reviewer |
| **FIX_REPORT.md** | 代码质量修复历史 | 了解技术债背景 |
| **todo.md** | 开发路线图 | 规划下一步 |
| **CMakeLists.txt** | 构建系统 | 添加依赖/目标 |

---

> 本文档中所有 ASCII 图表均可在大纲视图与终端环境中正常显示。  
> 随着项目演进，请同步更新此文档。

> — 总架构师, 2026-06-12
