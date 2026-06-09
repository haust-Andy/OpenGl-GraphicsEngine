# OpenGL Graphics Engine

基于 OpenGL 3.3+ Core Profile 的轻量级现代图形渲染引擎，支持 PBR 渲染、多光源系统、后处理管线及 ImGui 编辑器。

---

## 目录

- [项目架构](#项目架构)
- [功能特性](#功能特性)
- [快速开始](#快速开始)
- [操控方式](#操控方式)
- [依赖](#依赖)
- [扩展路线图](#扩展路线图)

---

## 项目架构

```
┌──────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                          │
│                    app/SandboxApp.cpp                         │
│              游戏逻辑 / 场景初始化 / 输入处理                   │
├──────────────────────────────────────────────────────────────┤
│                     EDITOR LAYER                              │
│               engine/editor/EditorLayer                       │
│        ImGui 面板：Hierarchy / Inspector / Stats / Light      │
├──────────────────────────────────────────────────────────────┤
│                     ENGINE CORE                               │
│  ┌────────────┬────────────┬────────────┬──────────────────┐ │
│  │Application │   Window   │   Input    │   Event System   │ │
│  │ 主循环      │ GLFW 窗口   │  键盘/鼠标  │   事件分发        │ │
│  ├────────────┼────────────┼────────────┼──────────────────┤ │
│  │Layer Stack │  Timestep  │    Log     │     Assert       │ │
│  │ 分层架构     │  帧时间步   │  分级日志   │   断言系统        │ │
│  └────────────┴────────────┴────────────┴──────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│                      RENDERER                                │
│  ┌────────────┬────────────┬────────────┬──────────────────┐ │
│  │ Renderer   │RendererAPI │  Shader    │  Framebuffer     │ │
│  │ Begin/End  │ OpenGL 抽象 │  着色器编译  │  多附件 FBO      │ │
│  ├────────────┼────────────┼────────────┼──────────────────┤ │
│  │ Texture    │VertexArray │  Buffer    │    Material      │ │
│  │ 2D/CubeMap │ VAO 封装    │ VBO / IBO  │  PBR 材质系统     │ │
│  ├────────────┼────────────┼────────────┼──────────────────┤ │
│  │  Light     │  Camera    │ UniformBuf │  MeshLibrary     │ │
│  │ 多光源系统   │  自由相机   │  UBO 管理  │  内置几何体库     │ │
│  └────────────┴────────────┴────────────┴──────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│                    POSTPROCESS                                │
│         BloomPass → ToneMappingPass → Pipeline                │
├──────────────────────────────────────────────────────────────┤
│                     RESOURCE                                  │
│      ShaderLibrary / TextureLibrary — 缓存池 + 热重载          │
├──────────────────────────────────────────────────────────────┤
│                      SCENE                                    │
│         Scene → Entity → [Transform | Mesh | Light]           │
│                SceneSerializer (.scene)                       │
└──────────────────────────────────────────────────────────────┘
```

### 目录结构

```
OpenGl-GraphicsEngine/
├── app/                        # 应用层
│   └── SandboxApp.cpp          # 示例程序 (PBR 场景 + 编辑器)
├── engine/                     # 引擎核心（静态库）
│   ├── core/                   # 基础设施 (Application/Window/Input/Event/Layer/Log/...)
│   ├── renderer/               # 渲染系统 (Shader/Texture/Framebuffer/Camera/Material/Light)
│   ├── resource/               # 资源管理 (ShaderLibrary/TextureLibrary/MeshLibrary)
│   ├── scene/                  # 场景系统 (Entity-Component + 序列化)
│   ├── postprocess/            # 后处理管线 (Bloom + ToneMapping)
│   └── editor/                 # ImGui 编辑器面板
├── shader/                     # GLSL 着色器
│   ├── pbr.vert / pbr.frag     # PBR 渲染 (Cook-Torrance BRDF)
│   ├── screen.vert / .frag     # 全屏后处理
│   └── skybox.vert / .frag     # 天空盒
├── tests/                      # 单元测试 (7 个模块)
├── external/                   # 第三方库 (header-only)
├── include/                    # GLAD / GLFW / stb 头文件
├── lib/                        # 预编译库 (glfw3, assimp)
└── CMakeLists.txt              # 构建配置
```

---

## 功能特性

### 渲染系统
| 功能 | 说明 |
|------|------|
| **PBR 渲染管线** | Cook-Torrance BRDF，支持 Albedo / Normal / Metallic / Roughness / AO / Emission 6 通道纹理 |
| **多光源** | 方向光 + 点光源 + 聚光灯，通过 UBO 高效传递 |
| **Uniform Buffer** | CameraUBO + LightUBO，减少 uniform 调用 |
| **线框模式** | F3 一键切换 Wireframe / Solid |

### 后处理
| 功能 | 说明 |
|------|------|
| **Bloom** | 亮度提取 → 高斯模糊 → 合成，阈值/强度/迭代次数可调 |
| **ToneMapping** | ACES 色调映射 |

### 场景管理
| 功能 | 说明 |
|------|------|
| **Entity-Component** | Transform / Mesh / Light 组件，轻量 ECS |
| **层级变换** | 父子节点，世界矩阵自动计算 |
| **场景序列化** | `.scene` 文本格式保存 / 加载 |

### ImGui 编辑器
| 面板 | 功能 |
|------|------|
| **Viewport** | 3D 场景实时渲染视图，支持焦点检测 |
| **Scene Hierarchy** | 实体树形列表，点击选中 |
| **Inspector** | Tag / Transform / Mesh 属性实时编辑 |
| **Rendering Stats** | FPS / DrawCalls / Triangles / Vertices / Entity Count |
| **Light Editor** | 方向光参数调节 |

### 资源管理
| 功能 | 说明 |
|------|------|
| **ShaderLibrary** | 着色器按名缓存，支持热重载 |
| **TextureLibrary** | 纹理按路径幂等加载 |
| **MeshLibrary** | 内置 Cube / Sphere / Plane / ScreenQuad |
| **3D 模型导入** | 通过 Assimp 加载 .obj / .fbx / .gltf |

---

## 快速开始

```bash
# 配置
mkdir build && cd build
cmake ..

# 编译
cmake --build . --config Release

# 运行
./Sandbox        # Linux / macOS
Sandbox.exe      # Windows

# 运行单元测试
./Tests
```

### 构建产物

| 目标 | 类型 | 输出 |
|------|------|------|
| `OpenGlEngine` | 静态库 | `libOpenGlEngine.a` |
| `Sandbox` | 可执行文件 | `Sandbox(.exe)` |
| `Tests` | 可执行文件 | `Tests(.exe)` |

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
| **ESC** | 退出 |

---

## 依赖

| 库 | 版本 | 用途 | 集成方式 |
|----|------|------|----------|
| GLFW | 3.4 | 窗口 / 输入 / OpenGL 上下文 | 预编译 |
| GLAD | 4.6 | OpenGL 函数加载 | 源文件编译 |
| GLM | 0.9.9 | 数学库 (vec3 / mat4 / quat) | header-only |
| Assimp | 6.x | 3D 模型导入 | 预编译 |
| stb_image | — | 图片加载 | header-only |
| ImGui | master | 调试 UI | 源文件编译 |

---

## 扩展路线图

### 已完成 (v1.0)
- [x] 引擎核心基础设施 (Application / Window / Input / Event / Layer / Log / Assert)
- [x] PBR 渲染管线 (Cook-Torrance + 多纹理通道)
- [x] 多光源系统 (Directional / Point / Spot + UBO)
- [x] 后处理管线 (Bloom + ToneMapping)
- [x] 场景系统 (Entity-Component + 序列化)
- [x] 资源管理 (Shader / Texture / Mesh 缓存池)
- [x] ImGui 编辑器 (Hierarchy / Inspector / Stats / Light Editor / Viewport)
- [x] CMake 多目标构建 (引擎静态库 + Sandbox + Tests)

### 计划中 (v1.1+)
| 优先级 | 功能 |
|--------|------|
| P0 | Shadow Mapping (CSM) |
| P0 | Skybox / IBL |
| P1 | 骨骼动画 (GPU Skinning) |
| P1 | SSAO |
| P2 | 延迟渲染管线 |
| P2 | 粒子系统 |
| P3 | spdlog / entt / yaml-cpp 集成 |

---

## 代码规范

| 类别 | 规则 |
|------|------|
| 文件 / 类 / 方法 | PascalCase |
| 成员变量 | `m_` 前缀 |
| 静态变量 | `s_` 前缀 |
| 指针别名 | `Ref<T>` = shared_ptr, `Scope<T>` = unique_ptr |
| 日志 | `CORE_TRACE` / `CORE_INFO` / `CORE_WARN` / `CORE_ERROR` |
| 断言 | `CORE_ASSERT` 检查前置条件 |
| 时间 | 使用 `Timestep` 类型 |

---

*基于 LearnOpenGL 教学项目改造，C++17，OpenGL 3.3 Core Profile。*
