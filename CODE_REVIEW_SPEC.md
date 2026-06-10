# Code Review 规范文档

> **项目**: OpenGl-GraphicsEngine  
> **版本**: v1.0  
> **生效日期**: 2026-06-10  
> **适用范围**: 所有 C++/OpenGL 引擎代码提交

---

## 一、Code Review 流程

1. **提交前自检**：开发者在提交 PR 前须按本规范逐项自查
2. **Review 分配**：至少 1 名资深开发者审核，高优先级模块需 2 人
3. **评审标准**：所有 **P0** 项必须通过，**P1** 项不超过 3 个可合并，**P2** 项记录为技术债
4. **反馈时效**：Review 意见需在 24 小时内回复

---

## 二、资源管理检查清单 (P0 - 必须通过)

### 2.1 OpenGL 资源生命周期

| 检查项 | 规则 | 违反后果 |
|--------|------|----------|
| GL ID 初始化 | 所有 OpenGL 对象 ID（纹理、缓冲区、着色器、FBO 等）**必须在声明时初始化为 0** | 未初始化 ID 导致 `glDelete*` 操作无效句柄 → 未定义行为 |
| GL 资源配对 | `glGen*` 与 `glDelete*` 必须成对出现，且在同一对象的生命周期内 | 资源泄漏或 double-free |
| 加载失败保护 | 资源加载失败后，不得在析构函数中调用 `glDelete*` 操作非零但无效的 ID | GPU 驱动崩溃 |

### 2.2 RAII 与所有权

| 检查项 | 规则 |
|--------|------|
| 拷贝控制 | 持有 OpenGL 资源的类**必须删除拷贝构造和拷贝赋值**，可选择性实现移动语义 |
| 智能指针 | 使用 `Ref<T>` (shared_ptr) / `Scope<T>` (unique_ptr) 管理堆对象生命周期 |
| 裸指针所有权 | **禁止**使用裸指针表达所有权。如果 LayerStack 拥有 Layer，使用 `unique_ptr` 或 `shared_ptr` |

### 2.3 错误传播

| 检查项 | 规则 |
|--------|------|
| 编译/链接状态 | Shader 编译、程序链接**必须**返回实际的 bool 状态，不得硬编码 `return true` |
| 加载失败 | 资源加载失败后，对象应处于安全的"空"状态（ID=0，标记为无效），不得静默忽略 |
| 异常捕获 | 禁止空 `catch(...)` 块。至少记录 `CORE_ERROR` 日志 |

---

## 三、OpenGL 最佳实践 (P0)

### 3.1 帧缓冲

| 检查项 | 规则 |
|--------|------|
| HDR 支持 | 场景 FBO 的颜色附件**必须使用浮点格式** (`GL_RGBA16F`)，仅最终输出 FBO 使用 `GL_RGBA8` |
| FBO 完整性 | 创建后必须检查 `glCheckFramebufferStatus`，失败时记录 `CORE_ERROR` |

### 3.2 状态管理

| 检查项 | 规则 |
|--------|------|
| 状态保存/恢复 | 进入后处理 Pass 时**必须**保存/恢复所有受影响的 OpenGL 状态（Depth、Blend、CullFace、绑定对象） |
| 状态泄露 | 任何渲染操作后，不得残留未预期的 OpenGL 状态变更 |

### 3.3 Shader

| 检查项 | 规则 |
|--------|------|
| GLSL 版本 | 当前目标 GLSL 3.30 Core，**禁止使用** GLSL 4.10+ 才支持的特性（如 uniform 默认值） |
| 类型安全 | `vec3` 不得隐式截断赋值给 `vec2`，须显式取 `.xy` |
| Tone Mapping | PBR Shader 中**不得**执行 Tone Mapping + Gamma 校正，统一由后处理管线负责 |

---

## 四、代码风格与质量 (P1)

### 4.1 命名规范

| 类型 | 规则 | 示例 |
|------|------|------|
| 类名 | PascalCase | `VertexBuffer`, `SceneRenderer` |
| 成员变量 | m_ 前缀 + PascalCase | `m_RendererID`, `m_ViewMatrix` |
| 局部变量 | camelCase | `lodVAO`, `cascadeIndex` |
| 常量/宏 | UPPER_SNAKE_CASE | `MAX_POINT_LIGHTS` |
| 函数 | PascalCase | `GetUniformLocation`, `OnRender` |
| 枚举值 | PascalCase | `Format::RGBA`, `LightType::Point` |

### 4.2 日志规范

| 规则 | 说明 |
|------|------|
| **统一使用引擎 Log 系统** | 使用 `CORE_INFO`/`CORE_WARN`/`CORE_ERROR` 等宏，**禁止** `std::cout`/`std::cerr` |
| 错误级别 | 加载失败 → `CORE_ERROR`；性能警告 → `CORE_WARN`；运行信息 → `CORE_INFO` |
| 位置信息 | 宏自动附加 `__FILENAME__` 和 `__LINE__`，无需手动添加 |

### 4.3 跨平台

| 规则 | 说明 |
|------|------|
| 禁止平台专有 API | 不得使用 `strcpy_s`、`sprintf_s` 等 MSVC 专有函数 |
| 替代方案 | `strncpy`、`snprintf`、`std::string::copy`、C++ 标准库 |

### 4.4 函数职责

| 规则 | 说明 |
|------|------|
| 单一职责 | `SetClearColor` 只设颜色，`Clear` 只清屏，不得合并 |
| DRY | 消除重复代码，三参数 `LoadFromFile` 内部应调用 `Compile` |
| 宽高比 | 投影矩阵不得硬编码 16:9，必须从 Window/Viewport 获取实际比例 |

---

## 五、性能与架构 (P2)

### 5.1 性能

| 检查项 | 规则 |
|--------|------|
| 字符串分配 | 渲染循环内不得执行 `std::string` 拼接。预缓存 uniform 名称 |
| Uniform 缓存 | `GetUniformLocation` 返回 -1 时不缓存，直接跳过后续 `glUniform*` 调用 |
| 排序优化 | `SortEntities()` 必须在 `OnRender` 中调用，按 shader 排序减少状态切换 |

### 5.2 架构

| 检查项 | 规则 |
|--------|------|
| 抽象层一致性 | 禁止绕过引擎封装直接调用 `glBindVertexArray` + `glVertexAttribPointer` |
| 事件宏 | `BIND_EVENT_FN` 改用 lambda 替代 `std::bind` |
| 位运算宏 | `BIT(x)` 改为 `constexpr` 函数 |

---

## 六、Review 模板

```markdown
## PR: [标题]

### 变更概述
- [简要描述]

### P0 检查
- [ ] GL ID 全部初始化为 0
- [ ] 拷贝控制：持有 GL 资源的类已 =delete 拷贝
- [ ] 加载失败有保护，不会调用 glDelete 操作无效 ID
- [ ] Shader 编译/链接返回真实状态
- [ ] FBO 颜色格式正确（场景用 RGBA16F，输出用 RGBA8）
- [ ] GLSL 代码无类型隐式截断
- [ ] 后处理 Pass 保存/恢复 GL 状态

### P1 检查
- [ ] 日志使用 CORE_* 宏，无 cout/cerr
- [ ] 无平台专有 API
- [ ] 函数职责单一
- [ ] 投影矩阵宽高比动态获取

### P2 检查
- [ ] 渲染循环内无字符串拼接
- [ ] Uniform 缓存处理 -1
- [ ] 未绕过引擎抽象层
```

---

## 七、技术债追踪

在代码中发现但本次未修复的问题，标记 `// TODO(code-review)` 注释：

```cpp
// TODO(code-review): Entity 胖实体模式需迁移至 ECS (M-13)
// TODO(code-review): CMake GLOB_RECURSE 需改为显式源文件列表 (L-07)
```

---

*本文档由高级开发工程师制定，团队成员须在每次提交前完成自查。*
