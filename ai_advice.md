检查下我的项目，我要做个图形引擎该怎么改造，列出具体改造方案
项目探索完毕。现在我对你的项目有了全面了解，来制定详细的改造方案。

项目现状分析
你的项目目前是一个 LearnOpenGL 学习项目，包含：

OpenGL 3.3 + GLFW + GLM + Assimp + stb_image
Shader、Camera、Mesh、Model 四个核心工具类
当前 main.cpp 是一个 Stencil 描边 Demo
旧代码有 Phong 多光源光照、Assimp 模型加载 Demo
缺乏引擎架构：渲染逻辑、资源管理、场景管理全部混在 main.cpp 里
改造方案：从 Demo 到轻量图形引擎
我帮你规划了一个 5 层架构 的轻量图形引擎（Lightweight Render Engine，简称 LRE）：

┌─────────────────────────────────────────────────────────────────┐
│  Application Layer (应用层)                                      │
│  ├── SandboxApp / EditorApp                                      │
│  └── 入口 main.cpp                                               │
├─────────────────────────────────────────────────────────────────┤
│  Engine Layer (引擎层) - 引擎入口，协调各子系统                    │
│  ├── Engine / Application 基类                                   │
│  ├── Layer Stack (层叠系统，支持多层叠加渲染)                      │
│  └── EntryPoint (跨平台入口)                                     │
├─────────────────────────────────────────────────────────────────┤
│  Subsystem Layer (子系统层)                                      │
│  ├── Renderer (渲染器)           ├── Scene (场景图)               │
│  ├── ResourceManager (资源管理)  ├── Input (输入系统)              │
│  ├── Window (窗口抽象)           ├── Event (事件系统)              │
│  └── ImGui (调试UI)                                              │
├─────────────────────────────────────────────────────────────────┤
│  Core Layer (核心层)                                              │
│  ├── Buffer (VBO/IBO)            ├── VertexArray (VAO)            │
│  ├── Shader (着色器)             ├── Texture (纹理)               │
│  ├── Framebuffer (帧缓冲)        ├── Material (材质)              │
│  ├── Camera (相机)               ├── Mesh (网格)                  │
│  └── Model (模型)                                                │
├─────────────────────────────────────────────────────────────────┤
│  Platform/Math Layer (基础层)                                    │
│  ├── GLFW/GLAD (平台抽象)        ├── GLM (数学库)                 │
│  ├── Assimp (模型导入)           ├── stb_image (图片加载)         │
│  └── Log/CoreMacros (日志/断言/宏)                                │
└─────────────────────────────────────────────────────────────────┘
目录结构改造
LearnOpenGl/
├── CMakeLists.txt                    # 增强：多目标构建
├── engine/                           # ★ 引擎代码（独立库）
│   ├── CMakeLists.txt               # 引擎子项目
│   ├── Core/                        # 核心层
│   │   ├── Core.h                   # 预编译头/公共头
│   │   ├── Log.h / Log.cpp          # 日志系统 (spdlog 封装)
│   │   ├── Assert.h                 # 断言宏
│   │   ├── Timestep.h               # 时间步
│   │   └── Base.h                   # 基础类型定义 (uint32_t 等)
│   ├── Renderer/                    # 渲染子系统
│   │   ├── Renderer.h / .cpp        # 渲染器主类（提交/绘制）
│   │   ├── RenderCommand.h / .cpp   # 渲染命令抽象 (glClear等)
│   │   ├── RendererAPI.h / .cpp     # API 选择 (OpenGL)
│   │   ├── Shader.h / .cpp          # ★ 从 src/include 重构
│   │   ├── Texture.h / .cpp         # ★ 新建：纹理抽象
│   │   ├── Framebuffer.h / .cpp     # ★ 新建：帧缓冲
│   │   ├── Material.h / .cpp        # ★ 新建：材质系统
│   │   ├── Buffer.h / .cpp          # ★ 新建：VBO/IBO 抽象
│   │   ├── VertexArray.h / .cpp     # ★ 新建：VAO 抽象
│   │   ├── Mesh.h / .cpp            # ★ 从 src/include 重构
│   │   ├── Model.h / .cpp           # ★ 从 src/include 重构
│   │   ├── Camera.h                 # ★ 从 src/include 重构为 Editor/Runtime 双模式
│   │   └── UniformBuffer.h          # ★ 新建：UBO 支持
│   ├── Platform/                    # 平台相关层
│   │   └── OpenGL/
│   │       ├── OpenGLShader.cpp     # OpenGL 着色器实现
│   │       ├── OpenGLTexture.cpp
│   │       ├── OpenGLFramebuffer.cpp
│   │       ├── OpenGLBuffer.cpp
│   │       ├── OpenGLVertexArray.cpp
│   │       └── OpenGLRendererAPI.cpp
│   ├── Resource/                    # 资源管理
│   │   ├── ResourceManager.h / .cpp # 资源池（Shader/Texture/Model缓存）
│   │   └── AssetImporter.h / .cpp   # 资产导入（封装 Assimp/stb）
│   ├── Scene/                       # 场景子系统
│   │   ├── Scene.h / .cpp           # 场景容器
│   │   ├── Entity.h                 # 实体（ECS Entity + Component）
│   │   ├── Components.h             # 组件定义 (Transform, MeshRenderer, Light, Camera)
│   │   ├── SceneRenderer.h / .cpp   # 场景渲染器
│   │   └── SceneSerializer.h / .cpp # 场景序列化
│   ├── Core/Engine/                 # 引擎核心
│   │   ├── Application.h / .cpp     # 应用基类
│   │   ├── Engine.h / .cpp          # 引擎初始化/关闭
│   │   ├── Window.h / .cpp          # 窗口抽象（封装 GLFW）
│   │   ├── Input.h / .cpp           # 输入系统
│   │   ├── Event.h / .cpp           # 事件系统
│   │   ├── Layer.h                  # Layer 基类
│   │   ├── LayerStack.h / .cpp      # Layer 栈
│   │   └── EntryPoint.h             # 入口点宏
│   └── Debug/                       # 调试工具
│       ├── ImGuiLayer.h / .cpp      # ImGui 集成层
│       └── Profiler.h               # 性能分析器
├── sandbox/                         # ★ 沙盒应用（使用引擎的示例）
│   ├── CMakeLists.txt
│   └── SandboxApp.cpp
├── editor/                          # ★ 编辑器应用
│   ├── CMakeLists.txt
│   ├── EditorApp.cpp
│   └── Panels/                      # 编辑器面板
│       ├── SceneHierarchyPanel.h
│       ├── InspectorPanel.h
│       └── ViewportPanel.h
├── external/                        # 第三方库（不动）
│   └── glm-master/
├── include/                         # 第三方头文件（不动，或移到 external）
├── lib/                             # 预编译库（不动）
├── shader/                          # ★ 引擎内置 shader
│   ├── PBR/
│   │   ├── pbr.vs / pbr.fs
│   └── Debug/
│       ├── outline.vs / outline.fs
│       └── grid.vs / grid.fs
└── resources/                       # 资源（不动，可增强）
    ├── textures/
    ├── models/
    └── fonts/
分阶段实施方案
第一阶段：基础设施搭建（1-2天）
模块	任务	说明
日志系统	新建 Core/Log.h	封装 spdlog，提供 CORE_TRACE/INFO/WARN/ERROR 宏
断言系统	新建 Core/Assert.h	CORE_ASSERT(expr, msg) 在 Debug 下断言
基础类型	新建 Core/Base.h	类型别名、智能指针别名 Ref<T> / Scope<T>
CMake 重构	重写 CMakeLists.txt	改为 engine 静态库 + sandbox 可执行文件
窗口抽象	新建 Engine/Window.h/.cpp	将 GLFW 初始化/窗口创建从 main.cpp 剥离
应用基类	新建 Engine/Application.h/.cpp	Application::Run() 主循环，生命周期管理
入口点	新建 Engine/EntryPoint.h	CreateApplication() 外部定义，引擎内部 main()
目标： SandboxApp.cpp 只需继承 Application 即可运行一个空窗口。

第二阶段：核心图形抽象（2-3天）
模块	任务	说明
VertexArray	新建 VertexArray.h + OpenGLVertexArray.cpp	封装 VAO，支持动态/静态顶点布局
Buffer	新建 Buffer.h + OpenGLBuffer.cpp	封装 VBO (VertexBuffer) / IBO (IndexBuffer)
Shader 重构	从 src/include/shader.h 迁移到 Renderer/Shader.h	增加 ShaderLibrary (按名缓存)，支持二进制缓存
Texture	新建 Texture.h + OpenGLTexture.cpp	支持 2D/CubeMap，自动 mipmap，参数配置
Framebuffer	新建 Framebuffer.h + OpenGLFramebuffer.cpp	支持多附件 (color+depth/stencil)，支持 resize
RenderCommand	新建 RenderCommand.h/.cpp	抽象 glClear、glDrawIndexed、glViewport 等
RendererAPI	新建 RendererAPI.h + OpenGLRendererAPI.cpp	单例，管理当前 API 状态
Renderer	新建 Renderer.h/.cpp	渲染入口：BeginScene() / Submit() / EndScene() / Flush()
Camera 重构	从 src/include/camera.h 迁移	拆分为 SceneCamera (编辑器) 和 EditorCamera (自由视角)
Material	新建 Material.h/.cpp	材质 = Shader + Uniform 集合，支持属性编辑器
目标： 可以通过 Renderer::BeginScene(camera) → Renderer::Submit(mesh, material, transform) → Renderer::EndScene() 绘制。

第三阶段：资源管理与场景系统（2-3天）
模块	任务	说明
ResourceManager	新建 ResourceManager.h/.cpp	Get<T>(path) 按路径加载并缓存资源
Mesh 重构	从 src/include/mesh.h 迁移到 Renderer/	适配新的 VertexArray/Buffer 体系
Model 重构	从 src/include/model.h 迁移到 Renderer/	适配 Assimp + ResourceManager
AssetImporter	新建 AssetImporter.h/.cpp	统一入口导入 Texture/Mesh/Model
ECS 实体	新建 Scene/Entity.h + Components.h	entt 库实现 ECS，定义 Transform/MeshRenderer/Light 组件
Scene	新建 Scene.h/.cpp	场景容器，管理实体集合
SceneRenderer	新建 SceneRenderer.h/.cpp	遍历场景实体，分类提交到 Renderer
SceneSerializer	新建 SceneSerializer.h/.cpp	JSON/YAML 序列化场景
目标： 可以创建场景、添加实体、保存/加载场景文件。

第四阶段：输入、事件与调试UI（1-2天）
模块	任务	说明
事件系统	新建 Event.h/.cpp	事件基类 + KeyEvent/MouseEvent/WindowEvent 派生类
输入系统	新建 Input.h/.cpp	Input::IsKeyPressed() 轮询输入，与事件系统解耦
Layer 系统	新建 Layer.h + LayerStack.h/.cpp	Layer 栈：每层可 OnUpdate/OnEvent/OnImGuiRender
ImGui 集成	新建 Debug/ImGuiLayer.h/.cpp	将 ImGui 作为特殊 Layer 注入
Profiler	新建 Debug/Profiler.h	简单的帧时间统计
目标： 具有完整的事件分发、可叠层渲染、ImGui 调试面板。

第五阶段：高级渲染功能（按需扩展）
功能	说明
PBR 着色	基于 Cook-Torrance 的 PBR 材质管线
Shadow Mapping	方向光/点光源阴影贴图
Skybox / IBL	天空盒 + 基于图像的照明
后处理	Bloom、HDR、Tone Mapping、SSAO
延迟渲染	Deferred Shading G-Buffer
骨骼动画	你的 Mesh 已有 Bone 数据，可接入 Assimp 动画
粒子系统	GPU 粒子 Compute Shader
批量渲染	DrawIndirect + 合批
关键改造对比：现在的代码 → 引擎架构
main.cpp 重构前 vs 重构后
现在（~150行全在 main.cpp）：

cpp
复制
int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(...);
    // Shader、Camera、VAO、纹理全部手写
    // 渲染循环混在一起
}
改造后：

cpp
复制
// SandboxApp.cpp
class SandboxApp : public Application {
    void OnStart() override { /* 加载资源 */ }
    void OnUpdate(Timestep ts) override { /* 更新逻辑 */ }
};

// EntryPoint.h
Application* CreateApplication() { return new SandboxApp(); }
依赖建议
库	用途	集成方式
spdlog	日志	header-only，拷贝到 external/
entt	ECS	header-only，拷贝到 external/
ImGui	调试UI	源码集成到 external/
yaml-cpp	场景序列化	编译为静态库
GLFW	窗口	✓ 已集成
GLAD	OpenGL加载	✓ 已集成
GLM	数学	✓ 已集成
Assimp	模型导入	✓ 已集成
stb_image	图片加载	✓ 已集成
建议的执行优先级
┌──────────────────────────────────────────────────────────────────┐
│  立即开始 →                                                     │
│  Phase 1: CMake重构 + 日志 + 窗口抽象 + Application基类          │
│  Phase 2: VertexArray/Buffer/Shader/Texture/Framebuffer/Renderer │
│  Phase 3: Material + Camera拆分 + Mesh/Model重构                 │
│  Phase 4: ResourceManager + Scene + ECS                          │
│  Phase 5: Event + Input + Layer + ImGui                          │
│  Phase 6+: PBR/Shadow/Skybox/后处理（按需）                       │
└──────────────────────────────────────────────────────────────────┘
要我直接开始执行第一阶段吗？我会从 CMake 重构、搭建 engine/ 目录结构、实现 Application/Window/Log 等基础设施开始。