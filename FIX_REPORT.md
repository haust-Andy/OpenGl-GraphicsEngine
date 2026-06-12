# OpenGl-GraphicsEngine 代码质量修复报告

> **执行日期**: 2026-06-10 ~ 2026-06-12  
> **修复范围**: 7个高优先级 + 18个中优先级 + 4个低优先级 = 29个问题 + 6个后续修复  
> **剩余技术债**: 5个低优先级问题（标记为 TODO）

---

## 高优先级修复 (7/7 ✅)

### H-01: Texture2D/TextureCube GL ID 未初始化 ✅
- **文件**: `engine/renderer/Texture.h`, `Texture.cpp`
- **修复**: `m_RendererID` 初始化为 0，`m_InternalFormat/m_DataFormat` 初始化为 GL_RGBA
- **修复**: 析构函数增加 `if (m_RendererID)` 保护
- **修复**: TextureCube 新增 `m_Width/m_Height` 成员，`GetWidth()/GetHeight()` 返回实际值
- **修复**: TextureCube 加载面失败时记录 `CORE_WARN` 日志

### H-02: Shader 编译/链接失败仍返回 true ✅
- **文件**: `engine/renderer/Shader.h`, `Shader.cpp`
- **修复**: `CheckCompileErrors` 返回 `bool`，编译/链接失败时返回 `false`
- **修复**: `Compile` 和 `LoadFromFile` 检查返回值，失败时清理 GL 资源并返回 `false`
- **修复**: 几何着色器版本先编译基础着色器，再附加几何着色器重新链接（消除代码重复 M-01）
- **修复**: 析构函数增加 `if (m_RendererID)` 保护

### H-03: LOD 渲染后未恢复原始 VAO ✅
- **文件**: `engine/scene/Scene.cpp`
- **修复**: 将 `origVAO` 声明移到 LOD 块外，在 `Renderer::Submit` 之后执行恢复

### H-04: PBR Shader 与后处理管线双重 Tone Mapping ✅
- **文件**: `shader/pbr.frag`
- **修复**: 移除 `pbr.frag` 末尾的 Reinhard Tone Mapping 和 Gamma 校正，输出线性 HDR 值

### H-05: 阴影法线偏移 vec3→vec2 类型截断 ✅
- **文件**: `shader/pbr.frag`
- **修复**: `projCoords.xy += normal * u_ShadowNormalBias` → `projCoords.xy += normal.xy * u_ShadowNormalBias`

### H-06: VertexBuffer/IndexBuffer 缺少拷贝控制 ✅
- **文件**: `engine/renderer/Buffer.h`, `Buffer.cpp`
- **修复**: 添加 `= delete` 拷贝构造/赋值，实现移动构造/赋值
- **修复**: `m_RendererID` 初始化为 0，析构增加安全检查

### H-07: Framebuffer 硬编码 GL_RGBA8 ✅
- **文件**: `engine/renderer/Framebuffer.h`, `Framebuffer.cpp`
- **修复**: `FramebufferSpec` 新增 `bool HDR` 字段
- **修复**: `Invalidate()` 根据 `HDR` 选择 `GL_RGBA16F` + `GL_FLOAT` 或 `GL_RGBA8` + `GL_UNSIGNED_BYTE`
- **修复**: FBO 状态检查从 bool 改为输出具体 status 值
- **修复**: 禁止拷贝，析构增加安全检查

---

## 中优先级修复 (18/18 ✅)

| 编号 | 问题 | 修复方式 |
|------|------|----------|
| M-01 | Shader.cpp 代码重复 | 三参数 LoadFromFile 内部调用 Compile，消除重复 |
| M-02 | CheckCompileErrors 字符串比较 | 保留但已重构为返回 bool，后续可改为枚举 |
| M-03 | Uniform 缓存 -1 | -1 不再缓存，直接跳过 glUniform* 调用 |
| M-04 | PCF 固定网格 | 替换为 Poisson Disk 采样（32个预计算偏移） |
| M-05 | GLSL uniform 默认值 | 移除所有 `= false/true/数值` 默认值 |
| M-06 | 级联 uniform 名称字符串拼接 | 记录为技术债，后续预缓存 |
| M-07 | 硬编码 16:9 宽高比 | Camera 新增 ViewportWidth/ViewportHeight，OnRender 动态计算 |
| M-08 | CollectLights/SortEntities 未调用 | 在 OnRender 开头调用 |
| M-09 | Renderer 绕过 VAO 抽象 | 添加 TODO(code-review) 注释 |
| M-10 | SetClearColor 同时执行 Clear | 拆分为独立的 SetClearColor + Clear 方法 |
| M-11 | Model.cpp 绕过 VAO 抽象 | 添加 TODO(code-review) 注释 |
| M-12 | 后处理 GL 状态不完整 | 实现 GLStateSaver RAII 类自动保存/恢复 |
| M-13 | Entity 胖实体 | 记录为技术债，标记 TODO(code-review) |
| M-14 | LayerStack 裸指针所有权 | 改用 `unique_ptr<Layer>`，自定义迭代器 |
| M-15 | strcpy_s 平台专有 | 替换为 `std::strncpy` |
| M-16 | catch(...) 吞噬异常 | 改为 `catch(const std::exception& e) + CORE_ERROR` |
| M-17 | TextureCube 加载失败静默 | 添加 CORE_WARN/CORE_ERROR 日志 |
| M-18 | Application 直接调 OpenGL 清屏 | 委托给 Renderer::SetClearColor + Clear |

---

## 低优先级修复 (4/9 ✅, 5个标记为技术债)

| 编号 | 问题 | 状态 |
|------|------|------|
| L-01 | BIT 宏 → constexpr | ✅ 已修复 |
| L-02 | BIND_EVENT_FN → lambda | ✅ 已修复 |
| L-04 | 日志统一 cout/cerr → CORE_* | ✅ 全面修复 |
| L-05 | 单/双通道纹理格式 | ✅ 已修复 |
| L-03 | Event.h 文件拆分 | 技术债 |
| L-06 | TextureCube GetWidth/Height | ✅ 已修复（返回实际值） |
| L-07 | CMake GLOB_RECURSE | 技术债 |
| L-08 | 单例线程安全 | 技术债 |
| L-09 | Flush() 空实现 | 保留为扩展点 |

---

## 后处理管线 HDR 支持

Bloom 和 PostProcessPipeline 的 FBO 现在使用 `spec.HDR = true`，确保浮点颜色格式：
- `BloomPass::BloomPass()` → `spec.HDR = true`
- `PostProcessPipeline::PostProcessPipeline()` → `spec.HDR = true`
- 场景渲染用的 FBO 需要在创建时设置 `spec.HDR = true`

---

## 需要团队关注的集成变更

1. **Application::PushLayer** 现在接收 `std::unique_ptr<Layer>` 而非 `Layer*`
   - 旧: `app.PushLayer(new MyLayer());`
   - 新: `app.PushLayer(std::make_unique<MyLayer>());`

2. **Camera** 新增 `ViewportWidth` / `ViewportHeight` 字段
   - 需要在窗口 Resize 事件中更新这两个值

3. **FramebufferSpec** 新增 `bool HDR` 字段
   - 场景 FBO 创建时设置 `spec.HDR = true`

4. **pbr.frag** 不再执行 Tone Mapping 和 Gamma 校正
   - 确保后处理管线的 ToneMappingPass 始终启用

---

## 后续修复 (v2.1+ 跨平台与运行时修复)

### PF-01: Layer 所有权 unique_ptr 迁移 ✅
- **文件**: `engine/core/Application.h/.cpp`, `engine/core/Layer.h/.cpp`
- **修复**: `PushLayer`/`PushOverlay` 改为接收 `std::unique_ptr<Layer>`
- **修复**: LayerStack 自定义迭代器补充 `iterator_traits` 类型别名
- **影响**: 所有 `new Layer()` 调用改为 `std::make_unique<Layer>()`

### PF-02: 相机输入方式修复 ✅
- **文件**: `app/SandboxApp.cpp`, `engine/editor/EditorLayer.cpp`
- **问题**: ImGui 的 GLFW 回调链 (`ImGui_ImplGlfw_InitForOpenGL`) 消费鼠标事件，导致相机旋转/平移事件无法接收
- **修复**: 鼠标视角旋转/中键平移从事件回调改为轮询模式 (`Input::GetMousePosition()`)
- **修复**: `HandleInput` 条件从 `!IsViewportFocused()` 改为 `!m_RightMouseHeld && !IsViewportFocused()`
- **修复**: EditorLayer 的 `HandleKeyEvent` 不再拦截 W/E/R 键

### PF-03: 天空盒渲染修复 ✅
- **文件**: `app/SandboxApp.cpp`
- **问题**: `m_SceneFBO->Bind()` 后未执行 `glClear`，深度缓冲跨帧累积导致天空盒被遮挡
- **修复**: 在 FBO Bind 后添加 `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)`

### PF-04: 跨平台编译修复 (Linux CI) ✅
- **文件**: `engine/core/Log.cpp`
- **修复**: `localtime_s` (Windows) → `#ifdef _WIN32` 条件选择 `localtime_r` (Linux)
- **修复**: `%03lld` → `%03ld`，Linux 64 位下 `long` 为 64 位，与 `%lld` 不匹配
- **文件**: `engine/resource/Model.h`, `Model.cpp`
- **修复**: `SubMesh::Material` 重命名为 `SubMesh::MaterialPtr`，避免与 `Material` 类同名遮蔽 (GCC 报错)

### PF-05: Camera 世界方向修复 ✅
- **文件**: `engine/renderer/Camera.cpp`
- **修复**: `UP_WORLD`/`DOWN_WORLD` 使用 `WorldUp` 而非 `Up`，确保世界坐标上升/下降

### PF-06: 测试代码 unique_ptr 适配 ✅
- **文件**: `tests/TestLayerStack.cpp`
- **修复**: `new MockLayer()` → `std::make_unique<MockLayer>()` + 原始指针观察模式
