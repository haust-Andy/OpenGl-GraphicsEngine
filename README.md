# OpenGL Graphics Engine

基于 OpenGL 3.3+ 的现代图形渲染引擎。

## 架构

```
OpenGlEngine/
├── engine/                    # 引擎核心
│   ├── core/                  # 基础设施
│   │   ├── Application        # 应用入口 / 主循环
│   │   ├── Window             # GLFW 窗口抽象
│   │   ├── Input              # 输入轮询
│   │   ├── Event              # 事件系统
│   │   ├── Layer              # 分层架构
│   │   └── EntryPoint         # main() 入口
│   ├── renderer/              # 渲染系统
│   │   ├── Renderer           # 渲染器主类
│   │   ├── RendererAPI        # 渲染API抽象
│   │   ├── Shader             # 着色器 (含Uniform缓存)
│   │   ├── VertexArray        # VAO封装
│   │   ├── Buffer             # VBO/IBO封装
│   │   ├── Texture            # 纹理2D/CubeMap
│   │   ├── Framebuffer        # 帧缓冲
│   │   ├── Camera             # 自由相机
│   │   ├── Material           # PBR材质
│   │   └── Light              # 光源系统
│   ├── resource/              # 资源管理
│   │   ├── ShaderLibrary      # 着色器缓存池
│   │   ├── TextureLibrary     # 纹理缓存池
│   │   └── MeshLibrary        # 基础几何体库
│   ├── scene/                 # 场景系统
│   │   ├── Scene              # 场景管理器
│   │   ├── TransformComponent # 变换+层级
│   │   ├── MeshComponent      # 渲染网格
│   │   └── LightComponent     # 光源组件
│   ├── postprocess/           # 后处理
│   │   └── PostProcess        # Bloom/ToneMapping管线
│   └── editor/                # 编辑器
│       └── EditorLayer        # ImGui调试面板
├── app/                       # 应用层
│   └── SandboxApp.cpp         # 示例程序
├── shader/                    # 着色器
│   ├── pbr.vert / pbr.frag    # PBR渲染
│   ├── screen.vert / .frag    # 全屏后处理
│   └── skybox.vert / .frag    # 天空盒
├── external/                  # 第三方库
└── CMakeLists.txt
```

## 特性

- **PBR 渲染**: Cook-Torrance BRDF，金属度/粗糙度工作流
- **光源系统**: 方向光 / 点光源 / 聚光灯
- **ImGui 编辑器**: 场景层级、属性检视、渲染统计
- **后处理管线**: Bloom 泛光 + 色调映射
- **资源管理**: 着色器/纹理缓存池，支持热重载
- **分层架构**: LayerStack 模式，便于模块化扩展

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 运行

```bash
./build/Sandbox
```

## 操作

| 按键 | 功能 |
|------|------|
| W/A/S/D | 移动相机 |
| Q/E | 下降/上升 |
| 鼠标 | 环视 |
| F3 | 切换线框模式 |
| ESC | 退出 |

## 依赖

- OpenGL 3.3+
- GLFW 3.x
- GLAD
- GLM
- ImGui
- Assimp
- stb_image
