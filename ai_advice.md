# 项目改造建议 (归档文档)

> **状态**: 已归档  
> **创建日期**: 项目初始阶段  
> **归档日期**: 2026-06-12  
> **说明**: 本文档为项目最初改造时的规划建议。所有 5 个阶段的改造建议已在 v1.0~v2.1 中全部实施完毕，文档仅作历史参考。

---

## 原始改造建议摘要

### 改造前状态
- LearnOpenGL 学习项目，~8 个源文件
- 4 个核心工具类：Shader、Camera、Mesh、Model
- 渲染逻辑、资源管理、场景管理全部混在 main.cpp
- 无引擎架构

### 建议的 5 层架构

```
┌─────────────────────────────────────────────────────────────────┐
│  Application Layer (应用层)                                      │
│  ├── SandboxApp / EditorApp                                      │
│  └── 入口 main.cpp                                               │
├─────────────────────────────────────────────────────────────────┤
│  Engine Layer (引擎层)                                           │
│  ├── Engine / Application 基类                                   │
│  ├── Layer Stack                                                 │
│  └── EntryPoint                                                  │
├─────────────────────────────────────────────────────────────────┤
│  Subsystem Layer (子系统层)                                      │
│  ├── Renderer / Scene / ResourceManager / Input / Window / Event│
│  └── ImGui (调试UI)                                              │
├─────────────────────────────────────────────────────────────────┤
│  Core Layer (核心层)                                              │
│  ├── Buffer / VertexArray / Shader / Texture / Framebuffer      │
│  ├── Material / Camera / Mesh / Model                           │
│  └── UniformBuffer                                               │
├─────────────────────────────────────────────────────────────────┤
│  Platform/Math Layer (基础层)                                    │
│  ├── GLFW/GLAD / GLM / Assimp / stb_image / Log/Assert         │
└─────────────────────────────────────────────────────────────────┘
```

### 分阶段实施结果

| 阶段 | 建议 | 实施状态 |
|------|------|----------|
| Phase 1 | CMake重构 + 日志 + 窗口抽象 + Application基类 | ✅ v1.0 完成 |
| Phase 2 | VertexArray/Buffer/Shader/Texture/Framebuffer/Renderer | ✅ v1.0 完成 |
| Phase 3 | Material + Camera拆分 + Mesh/Model重构 + Scene/ECS | ✅ v1.0 完成 |
| Phase 4 | Event + Input + Layer + ImGui | ✅ v1.0 完成 |
| Phase 5+ | PBR/Shadow/IBL/后处理/物理/粒子/UI/脚本 | ✅ v2.0 完成 |

### 当前实际架构

项目已从 ~8 个文件的单体项目发展为 **87 个引擎文件** 的分层引擎，详见 [ARCHITECTURE.md](ARCHITECTURE.md)。

---

*本文档为历史归档，反映项目改造初期的规划。当前项目状态请参阅最新文档。*
