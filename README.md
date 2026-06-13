# OpenGL Graphics Engine

基于 OpenGL 3.3+ Core Profile 的轻量级游戏/渲染引擎，支持 PBR + IBL 渲染、级联阴影、HDR 后处理管线、物理碰撞、脚本系统、粒子特效、游戏 UI 及全功能 ImGui 编辑器。

---

## 目录

- [项目架构](#项目架构)
- [功能特性](#功能特性)
- [Sandbox.exe 说明](#sandboxexe-说明)
- [快速开始](#快速开始)
- [操控方式](#操控方式)
- [依赖](#依赖)
- [代码质量](#代码质量)
- [扩展路线图](#扩展路线图)

---

## 项目架构

```
┌──────────────────────────────────────────────────────────────────────┐
│                       APPLICATION LAYER                              │
│                       app/SandboxApp.cpp                             │
│            游戏逻辑 / 场景初始化 / 输入处理 / 物理演示                    │
├──────────────────────────────────────────────────────────────────────┤
│                        EDITOR LAYER                                  │
│                  engine/editor/EditorLayer                            │
│  ImGui 面板：Hierarchy / Inspector / Stats / ContentBrowser / Prefab │
│  Gizmo (W/E/R) / Play-Stop (F5) / Save-Load Scene                   │
├──────────────────────────────────────────────────────────────────────┤
│                        ENGINE CORE                                   │
│  ┌────────────┬────────────┬────────────┬──────────────────────────┐│
│  │Application │   Window   │   Input    │     Event System         ││
│  │ 主循环      │ GLFW 窗口   │  键盘/鼠标  │     事件分发              ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │Layer Stack │  Timestep  │    Log     │       Assert             ││
│  │ 分层架构     │  帧时间步   │  分级日志   │     断言系统              ││
│  │ unique_ptr │            │            │                          ││
│  └────────────┴────────────┴────────────┴──────────────────────────┘│
├──────────────────────────────────────────────────────────────────────┤
│                         RENDERER                                     │
│  ┌────────────┬────────────┬────────────┬──────────────────────────┐│
│  │ Renderer   │RendererAPI │  Shader    │    Framebuffer           ││
│  │ Begin/End  │ OpenGL 抽象 │  着色器编译  │   HDR FBO + RGBA16F     ││
│  │ SetClearColor│           │  错误传播    │    多附件 FBO            ││
│  │ Clear      │            │            │                          ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │ Texture    │VertexArray │  Buffer    │      Material            ││
│  │ 2D/CubeMap │ VAO 封装    │VBO/IBO    │    PBR 材质系统           ││
│  │ 安全ID初始化 │            │ 拷贝控制    │                          ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │  Light     │  Camera    │ UniformBuf │    MeshLibrary           ││
│  │ 多光源系统   │ 自由相机    │  UBO 管理  │    内置几何体库           ││
│  │            │ 视口宽高    │            │                          ││
│  ├────────────┼────────────┼────────────┼──────────────────────────┤│
│  │ ShadowMap  │   IBL      │  Frustum   │       LOD               ││
│  │ CSM 级联阴影│ IBL 环境光照│ 视锥体剔除  │  距离/屏幕占比LOD        ││
│  │ Poisson PCF│            │            │     VAO 安全恢复          ││
│  └────────────┴────────────┴────────────┴──────────────────────────┘│
├──────────────────────────────────────────────────────────────────────┤
│                      POSTPROCESS                                     │
│    BloomPass → ToneMappingPass → Pipeline                             │
│    HDR FBO + GLStateSaver RAII 状态管理                                │
│    (SSAO 着色器已就绪，待集成到管线)                                     │
├──────────────────────────────────────────────────────────────────────┤
│                        RESOURCE                                      │
│  ShaderLibrary / TextureLibrary / Model (.obj/.fbx/.gltf 导入)       │
├──────────────────────────────────────────────────────────────────────┤
│                         SCENE                                        │
│  Scene → Entity → [Transform | Mesh | Light | Script | Physics       │
│                | Audio | Particle | LOD | Collider | Rigidbody]      │
│  Prefab 系统 · SceneSerializer (.scene) · CollectLights · SortEntities│
├──────────────────────────────────────────────────────────────────────┤
│                     SUBSYSTEMS                                        │
│  ┌──────────────┬──────────────┬──────────────┬─────────────────────┐│
│  │   Physics    │    Audio     │   Particle   │      Game UI        ││
│  │ AABB碰撞     │ SoLoud接口   │ CPU粒子系统   │ SpriteBatch         ││
│  │ 弹性响应      │ 3D空间化音频  │ Billboard渲染 │ UIImage/Text/Button ││
│  │ 射线检测      │ 音频组件     │ 颜色渐变/重力  │ Canvas + 锚点       ││
│  └──────────────┴──────────────┴──────────────┴─────────────────────┘│
└──────────────────────────────────────────────────────────────────────┘
```

### 目录结构

```
OpenGl-GraphicsEngine/
├── app/                        # 应用层
│   └── SandboxApp.cpp          # 演示程序 (PBR 场景 + 物理 + 粒子 + 脚本)
├── engine/                     # 引擎核心（静态库）
│   ├── core/                   # 基础设施 (Application/Window/Input/Event/Layer/Log/...)
│   ├── renderer/               # 渲染系统
│   │   ├── Shader/Texture/Framebuffer/Camera/Material/Light
│   │   ├── ShadowMap.h/.cpp    # CSM 级联阴影
│   │   ├── IBL.h/.cpp          # IBL 环境光照
│   │   ├── Frustum.h/.cpp      # 视锥体剔除
│   │   ├── LOD.h/.cpp          # LOD 系统
│   │   ├── GLStateSaver.h      # OpenGL 状态 RAII 保护
│   │   └── UniformBuffer.h     # UBO 管理
│   ├── resource/               # 资源管理
│   │   ├── ShaderLibrary/TextureLibrary/MeshLibrary
│   │   └── Model.h/.cpp        # Assimp 模型导入
│   ├── scene/                  # 场景系统
│   │   ├── Scene/Entity/SceneSerializer
│   │   ├── ScriptComponent.h   # 脚本生命周期回调
│   │   └── Prefab.h/.cpp       # 预制体系统
│   ├── physics/                # 物理系统 (AABB碰撞/弹性响应/射线检测)
│   ├── particle/               # 粒子系统 (CPU Billboard)
│   ├── audio/                  # 音频系统 (SoLoud 接口 Stub)
│   ├── ui/                     # 游戏 UI (SpriteBatch/UIImage/Text/Button/Canvas)
│   ├── postprocess/            # 后处理管线 (Bloom + ToneMapping + HDR)
│   └── editor/                 # ImGui 编辑器 (7+ 面板)
├── shader/                     # GLSL 着色器
│   ├── pbr.vert / pbr.frag     # PBR + CSM阴影 + IBL (线性 HDR 输出)
│   ├── screen.vert / .frag     # 全屏后处理
│   ├── skybox.vert / .frag     # 天空盒
│   ├── ssao.frag               # SSAO 采样
│   ├── ssao_blur.frag          # SSAO 双边模糊
│   └── ssao_combine.frag       # SSAO 合成
├── external/                   # 第三方库 (header-only)
├── include/                    # GLAD / GLFW / stb 头文件
├── lib/                        # 预编译库 (glfw3, assimp)
├── CMakeLists.txt              # 构建配置
├── CODE_REVIEW_SPEC.md         # Code Review 规范文档
└── FIX_REPORT.md               # 代码质量修复报告
```

---

## 功能特性

### 渲染系统
| 功能 | 说明 |
|------|------|
| **PBR 渲染管线** | Cook-Torrance BRDF，支持 Albedo / Normal / Metallic / Roughness / AO / Emission 6 通道纹理，输出线性 HDR |
| **IBL 环境光照** | HDR 环境贴图加载，辐照度卷积，预过滤 5 级 MIP，BRDF LUT 积分 |
| **级联阴影 (CSM)** | 3 级联阴影贴图 + PSSM 分割 + Poisson Disk PCF 软阴影 + 法线偏移 |
| **HDR 后处理管线** | RGBA16F Framebuffer + Bloom 亮度提取 + ACES 色调映射 + Gamma 校正 |
| **多光源** | 方向光 + 点光源 + 聚光灯，通过 UBO 高效传递 |
| **Uniform Buffer** | CameraUBO + LightUBO，减少 uniform 调用 |
| **视锥体剔除** | 6 平面提取，AABB / 球体相交测试，Scene 自动剔除 |
| **LOD 系统** | 距离模式 / 屏幕占比模式，支持 4 级 LOD + Bias 控制，VAO 安全恢复 |
| **线框模式** | F3 一键切换 Wireframe / Solid |

### 后处理
| 功能 | 说明 |
|------|------|
| **Bloom** | 亮度提取 → 高斯模糊 → 合成，阈值/强度/迭代次数可调，HDR FBO |
| **ToneMapping** | ACES 色调映射 + Gamma 校正（统一由后处理管线执行） |
| **GL 状态保护** | GLStateSaver RAII 类自动保存/恢复 OpenGL 状态，防止状态污染 |
| **SSAO** | 64 采样核心 + 双边模糊 + 合成（着色器已就绪） |

### 场景管理
| 功能 | 说明 |
|------|------|
| **Entity-Component** | Transform / Mesh / Light / Script / Physics / Audio / Particle / LOD 组件 |
| **脚本系统** | ScriptComponent：OnCreate / OnUpdate / OnDestroy 生命周期，Lambda 回调 |
| **Prefab 预制体** | Entity 模板创建 / 实例化 / 序列化 |
| **层级变换** | 父子节点，世界矩阵自动计算 |
| **场景序列化** | `.scene` 文本格式保存 / 加载 |
| **光照收集** | CollectLights / SortEntities 自动优化渲染顺序 |

### 物理系统
| 功能 | 说明 |
|------|------|
| **碰撞检测** | AABB / Sphere Collider，碰撞回调 |
| **弹性碰撞** | 碰撞响应 + 半隐式欧拉积分 |
| **射线检测** | Raycast AABB / Sphere 相交测试 |
| **重力模拟** | 可配置重力加速度 |

### 粒子与音频
| 功能 | 说明 |
|------|------|
| **CPU 粒子系统** | 发射速率 / 生命周期 / 速度 / 重力 / 颜色渐变 / Billboard 渲染 |
| **音频系统** | AudioSource / AudioListener 组件，SoLoud 集成接口（Stub） |

### 游戏 UI
| 功能 | 说明 |
|------|------|
| **SpriteBatch** | 2D 批量渲染，正交投影 + Alpha 混合 |
| **UI 控件** | UIImage / UIText / UIButton，OnClick / OnHover 事件 |
| **UICanvas** | 设计分辨率 + 9 种锚点 + 缩放适配 |

### ImGui 编辑器
| 面板 | 功能 |
|------|------|
| **Viewport** | 3D 场景实时渲染视图，支持焦点检测，动态宽高比 |
| **Scene Hierarchy** | 实体树形列表，添加/删除实体，右键保存为 Prefab |
| **Inspector** | Transform / Material / Light / Physics / Script / LOD 属性编辑，添加组件 |
| **Content Browser** | 文件系统浏览器，目录导航，文件类型图标，异常日志 |
| **Prefab Panel** | Prefab 列表，点击实例化到场景 |
| **Rendering Stats** | FPS / DrawCalls / Triangles / Vertices / Entity Count |
| **Menu Bar** | File (Save/Load Scene) · View (面板切换) · Play (F5) · Gizmo (W/E/R) |

### 资源管理
| 功能 | 说明 |
|------|------|
| **ShaderLibrary** | 着色器按名缓存，支持热重载，编译错误正确传播 |
| **TextureLibrary** | 纹理按路径幂等加载，单/双通道格式支持 |
| **MeshLibrary** | 内置 Cube / Sphere / Plane / ScreenQuad |
| **Model 导入** | Assimp 加载 .obj / .fbx / .gltf，PBR 纹理自动映射，子网格支持 |

---

## Sandbox.exe 说明

`Sandbox.exe` 是引擎的**演示应用程序**，展示引擎的全部核心功能。运行后你会看到一个 3D PBR 场景，包含：

| 演示内容 | 说明 |
|----------|------|
| **PBR 材质球** | 金属 / 非金属球体，展示 Cook-Torrance BRDF 渲染 |
| **级联阴影** | 方向光 CSM 阴影，Poisson Disk PCF 软阴影 |
| **IBL 环境光照** | HDR 环境贴图驱动的间接光照（辐照度 + 预过滤 + BRDF LUT） |
| **HDR 后处理** | Bloom 辉光 + ACES 色调映射，线性 HDR 管线 |
| **物理碰撞** | 金属球受重力影响落地，与方块弹性碰撞 |
| **脚本系统** | 旋转方块（OnUpdate 中旋转 Transform），F5 给金属球施加冲量 |
| **粒子特效** | 火焰粒子发射器，展示 CPU 粒子系统 + Billboard 渲染 |
| **ImGui 编辑器** | 左侧 Hierarchy + 右侧 Inspector + Content Browser + Stats 面板 |

**简单来说：Sandbox.exe 就是"引擎能做什么"的活文档。** 它不是游戏，而是一个可交互的技术展示场景，同时也可作为你自己游戏项目的起点——修改 `app/SandboxApp.cpp` 即可替换场景内容。

---

## 快速开始

```bash
# 配置 (需要 MinGW 或 MSVC + CMake)
mkdir build && cd build

# Windows MinGW
cmake .. -G "MinGW Makefiles"
# Windows MSVC
cmake ..
# Linux (需先安装: sudo apt install libglfw3-dev libassimp-dev xorg-dev)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release

# 运行
./Sandbox        # Linux / macOS
Sandbox.exe      # Windows

# 运行测试
./Tests          # Linux / macOS
Tests.exe        # Windows
```

### 构建产物

| 目标 | 类型 | 输出 |
|------|------|------|
| `OpenGlEngine` | 静态库 | `libOpenGlEngine.a` |
| `Sandbox` | 可执行文件 | `Sandbox(.exe)` |

---

## 操控方式

| 操作 | 效果 |
|------|------|
| **右键拖拽** | 旋转视角（Viewport 内） |
| **中键拖拽** | 平移视角（Viewport 内） |
| **滚轮滚动** | 缩放 |
| **左键点击 Viewport** | 聚焦视口，激活键盘飞行 |
| **W / A / S / D** | 前后左右飞行（需先聚焦 Viewport） |
| **Ctrl** | 下降 |
| **Space** | 上升 |
| **F3** | 切换线框模式 |
| **F5** | 给金属球施加冲量（物理演示） / Play 模式 |
| **ESC** | 退出 |

---

## 依赖

| 库 | 版本 | 用途 | 集成方式 |
|----|------|------|----------|
| GLFW | 3.4 | 窗口 / 输入 / OpenGL 上下文 | 预编译 (Win) / 系统包 (Linux) |
| GLAD | 4.6 | OpenGL 函数加载 | 源文件编译 |
| GLM | 0.9.9 | 数学库 (vec3 / mat4 / quat) | header-only |
| Assimp | 6.x | 3D 模型导入 (.obj/.fbx/.gltf) | 预编译 (Win) / 系统包 (Linux) |
| stb_image | — | 图片加载 (LDR + HDR) | header-only |
| ImGui | master | 调试 / 编辑器 UI | 源文件编译 |

---

## CI/CD

项目已配置 GitHub Actions 自动化检查 (`.github/workflows/pr-checks.yml`)：

| Job | 平台 | 内容 |
|-----|------|------|
| `build-linux` | Ubuntu 22.04 | CMake 配置 + 编译 + 运行测试 |
| `build-windows` | Windows 2022 | CMake MinGW 配置 + 编译 + 运行测试 |
| `format-check` | Ubuntu 22.04 | clang-format 格式检查 (仅报告) |

触发条件：向 `dev` 或 `main` 分支提交 PR 时自动运行。

---

## 跨平台

| 平台 | 状态 |
|------|------|
| Windows (MinGW/MSVC) | ✅ 主开发平台 |
| Linux (GCC) | ✅ CI 验证 |
| macOS | ⚠️ 未测试 |

---

## 文档

| 文档 | 内容 |
|------|------|
| **[ONBOARDING.md](ONBOARDING.md)** | ★ 合一开发手册：架构 / 结构图 / 代码规范 / Code Review / 路线图 / 调试 / Cheat Sheet |

---

*基于 LearnOpenGL 教学项目改造，C++17，OpenGL 3.3 Core Profile。*
*v2.1 — 35 项代码质量问题已修复，跨平台 CI (Windows)。*
