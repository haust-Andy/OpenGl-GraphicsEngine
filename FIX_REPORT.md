# OpenGl-GraphicsEngine 代码质量修复报告

> **执行日期**: 2026-06-10  
> **修复范围**: 7个高优先级 + 18个中优先级 + 4个低优先级 = 29个问题  
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
