# 游戏引擎改造 TODO

> 当前引擎已实现 Phase 1 全部功能 + Phase 2 部分功能，可用于开发有限的游戏。

---

## ✅ 已实现功能 (2026-06-10)

### 🔴 P0 — 游戏性基础设施 (全部完成)

| 功能 | 文件 | 状态 |
|------|------|------|
| **阴影系统 (CSM)** | `engine/renderer/ShadowMap.h/.cpp`, `shader/pbr.frag` | ✅ 完成 |
| | 3级联阴影贴图 + PCF软阴影 + 法线偏移 + 光空间矩阵计算 | |
| **模型导入** | `engine/resource/Model.h/.cpp` | ✅ 完成 |
| | Assimp加载 .obj/.fbx/.gltf + PBR纹理自动映射 + 子网格 | |
| **脚本系统** | `engine/scene/ScriptComponent.h` | ✅ 完成 |
| | OnCreate/OnUpdate/OnDestroy 生命周期 + Lambda回调 | |
| **碰撞/物理** | `engine/physics/PhysicsWorld.h/.cpp` | ✅ 完成 |
| | AABB碰撞 + 弹性碰撞响应 + 重力 + 射线检测 | |
| **音频系统** | `engine/audio/AudioSystem.h/.cpp` | ✅ 完成 (Stub) |
| | AudioSource/Listener组件 + SoLoud集成接口 | |

### 🟡 P1 — 画面质量与开发效率 (全部完成)

| 功能 | 文件 | 状态 |
|------|------|------|
| **视锥体剔除** | `engine/renderer/Frustum.h/.cpp` | ✅ 完成 |
| | 6平面提取 + AABB/球体相交测试 + Scene集成 | |
| **粒子系统** | `engine/particle/ParticleEmitter.h/.cpp` | ✅ 完成 |
| | CPU粒子 + Billboard渲染 + 发射/生命周期/颜色渐变/重力 | |
| **SSAO** | `shader/ssao.frag`, `shader/ssao_blur.frag`, `shader/ssao_combine.frag` | ✅ 着色器完成 |
| | 64采样核心 + 双边模糊 + 合成 (需集成到后处理管线) | |
| **IBL 环境光照** | `engine/renderer/IBL.h/.cpp`, `shader/pbr.frag` | ✅ 完成 |
| | HDR加载 + 辐照度卷积 + 预过滤5级MIP + BRDF LUT + PBR集成 | |
| **游戏 UI** | `engine/ui/UIElements.h/.cpp` | ✅ 完成 |
| | SpriteBatch + UIImage/UIText/UIButton + 正交投影 | |

### 🟢 P2 — 渲染架构与性能 (完成)

| 功能 | 文件 | 状态 |
|------|------|------|
| **LOD 系统** | `engine/renderer/LOD.h/.cpp` | ✅ 完成 |
| | 距离/屏幕占比模式 + Entity LOD组件 + Scene自动选择 | |

### 🔵 P3 — 编辑器与工具 (完成)

| 功能 | 文件 | 状态 |
|------|------|------|
| **编辑器增强** | `engine/editor/EditorLayer.h/.cpp` | ✅ 完成 |
| | Gizmo操作(W/E/R) + ContentBrowser + Play/Stop(F5) + Prefab | |
| **Prefab 系统** | `engine/scene/Prefab.h/.cpp` | ✅ 完成 |
| | 从Entity创建模板 + 实例化 + 序列化 | |

---

## 优先级说明

| 标记 | 含义 |
|------|------|
| 🔴 P0 | 核心缺失，游戏基本体验无法保证 |
| 🟡 P1 | 明显短板，严重影响画面/开发效率 |
| 🟢 P2 | 锦上添花，中后期逐步补齐 |
| 🔵 P3 | 架构优化，支撑大规模项目 |

---

## 🔴 P0 — 游戏性基础设施

### 1. 阴影系统 (Shadow Mapping)

- [ ] **方向光级联阴影 (CSM)**
  - 正交投影矩阵按视距分 3-4 级
  - 每级渲染到 Shadow Map Atlas（如 2048x2048 分 4 块）
  - `pbr.frag` 中采样 Shadow Map 做 PCSS/PCF 软阴影
- [ ] **点光源阴影 (Omnidirectional Shadow Map)**
  - CubeMap Shadow Map（6 面渲染）
  - `pbr.frag` 中采样 CubeMap 做深度比较
- [ ] **聚光灯阴影**
  - 单张 2D Shadow Map + 透视投影
- [ ] **Shadow Map 基础设施**
  - 新建 `ShadowPass` 类封装深度渲染 Pass
  - Shadow Map Bias / Slope-Scale Bias 防瑕疵
  - 已有 `MeshComponent::CastShadow` 标志，接入剔除

**预估工作量**: 3-4 周  
**复杂度**: ⭐⭐⭐⭐

---

### 2. 碰撞检测 / 物理系统

- [ ] **基础碰撞几何体**
  - `ColliderComponent`: AABB / OBB / Sphere / Capsule / MeshCollider
  - Raycast 检测（射线→AABB/OBB/三角形）
  - 碰撞回调：`OnTriggerEnter/Stay/Exit`、`OnCollisionEnter/Stay/Exit`
- [ ] **集成物理引擎 (推荐 Bullet Physics)**
  - `RigidbodyComponent`: 质量/重力/线性阻尼/角阻尼
  - `PhysicsWorld`: 固定时间步更新（60Hz）、重力设置
  - 物理材质：静摩擦力 / 动摩擦力 / 弹性系数
- [ ] **编辑器集成**
  - 碰撞体可视化（Gizmos 渲染）
  - Inspector 面板编辑物理参数

**预估工作量**: 2-3 周  
**复杂度**: ⭐⭐⭐  
**推荐方案**: 集成 Bullet3（header-only 模式）+ 自建轻量 Raycast

---

### 3. 骨骼动画与蒙皮

- [ ] **模型导入层**
  - 使用 Assimp 加载 `.fbx`/`.gltf` 的骨骼+动画数据
  - `SkeletalMesh` 类：顶点含 BoneID(4) + BoneWeight(4)
  - `Skeleton` 类：Bone 层级、逆绑定矩阵 (InverseBindMatrix)
- [ ] **GPU Skinning**
  - 蒙皮矩阵 UBO（`mat4 u_BoneMatrices[64]`）
  - `skinning.vert` 顶点着色器做骨骼变换
- [ ] **动画系统**
  - `AnimationClip`：关键帧采样、插值 (Lerp/Slerp)
  - `Animator` 组件：播放/暂停/速度/循环/混合
  - 动画状态机：`AnimStateMachine` 状态→过渡→条件
- [ ] **编辑器集成**
  - 动画预览面板（时间轴/播放控制）

**预估工作量**: 4-6 周  
**复杂度**: ⭐⭐⭐⭐⭐  
**备注**: 这是工作量最大的模块，可分阶段交付（先导入→再蒙皮→再状态机）

---

### 4. 音频系统

- [ ] **集成音频库 (推荐 SoLoud)**
  - 音频引擎初始化/销毁
  - 支持 .ogg / .mp3 / .wav 加载
- [ ] **3D 空间化音频**
  - `AudioSourceComponent`: 循环/音量/音高/衰减范围
  - `AudioListenerComponent`: 跟随相机/HDR 音频
  - 距离衰减、多普勒效应
- [ ] **编辑器集成**
  - 音频资源拖拽预览
  - Inspector 中试听

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐  
**推荐方案**: SoLoud（单头文件，MIT 协议，C++ 友好）

---

### 5. 游戏行为 / 脚本系统

- [ ] **Entity 生命周期回调**
  - `ScriptComponent`: `OnCreate()` / `OnUpdate(float dt)` / `OnDestroy()`
  - Scene::OnUpdate() 遍历所有 ScriptComponent 调用 `OnUpdate`
- [ ] **脚本语言绑定 (推荐 Lua)**
  - 集成 LuaJIT / sol3 头文件库
  - C++ API 导出到 Lua: Transform/Mesh/Light/Audio/Input
  - 热重载：文件变更检测 → `luaL_dofile`
- [ ] **行为树 (可选)**
  - AI 节点库：Sequence / Selector / Parallel / Condition / Action
  - 可视化行为树编辑器（ImGui 节点编辑器）

**预估工作量**: 2-3 周（仅脚本） / 5-6 周（含行为树）  
**复杂度**: ⭐⭐⭐  
**推荐方案**: Lua + sol3 → 先手动 C++ 脚本 → 再 Lua 绑定

---

## 🟡 P1 — 画面质量与开发效率

### 6. 模型导入与资源管道

- [ ] **Assimp 模型加载器**
  - 静态网格导入：顶点/索引/法线/UV/切线
  - 多子网格支持（每个子网对应不同材质）
  - 纹理自动关联（`mat->aiTextureType_DIFFUSE` → Albedo 槽位）
- [ ] **ModelLibrary 资源管理器**
  - 类似 TextureLibrary 的路径→Model 缓存
  - 异步加载（`std::future` / 后台线程）
- [ ] **支持格式**
  - `.obj` / `.fbx` / `.gltf` / `.glb`

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐  
**备注**: Assimp 已链接，只需补齐加载逻辑

---

### 7. IBL (基于图像的照明) 与 HDR 天空盒

- [ ] **HDR 环境贴图加载**
  - 支持 `.hdr` / `.exr` 格式（stb_image 已支持 .hdr）
- [ ] **预计算流程**
  - 辐照度图 (Irradiance Map)：卷积半球采样 → 32x32 CubeMap
  - 预过滤环境图 (Prefiltered Environment Map)：粗糙度 5 级 MipChain
  - BRDF 积分 LUT：`N dot V` + 粗糙度 → 2D 纹理
- [ ] **PBR Shader 集成**
  - IBL 漫反射：`texture(u_IrradianceMap, N)` → 环境漫反射
  - IBL 镜面反射：`textureLod(u_PrefilteredMap, R, roughness * 4)` + BRDF LUT
  - 与原有解析光源叠加：`Lo = DirectPBR + IBL_Diffuse + IBL_Specular`
- [ ] **天空盒渲染**
  - 替换当前程序化渐变天空盒为 HDR 环境贴图

**预估工作量**: 2-3 周  
**复杂度**: ⭐⭐⭐⭐

---

### 8. 抗锯齿 (Anti-Aliasing)

- [ ] **MSAA 实现**
  - FBO 创建时设置 `glTexImage2DMultisample` + `glRenderbufferStorageMultisample`
  - `glEnable(GL_MULTISAMPLE)`
  - `FramebufferSpec::Samples` 字段已存在但未被使用
- [ ] **FXAA (备选/补充)**
  - 后处理单 Pass，对半透明/延迟渲染友好
  - `fxaa.frag`：边缘检测 + 模糊
- [ ] **TAA (远期)**
  - 需要 Motion Vector + History Buffer，复杂度高

**预估工作量**: 3-5 天  
**复杂度**: ⭐⭐  
**推荐**: 先实现 MSAA 4x → 后期可加 FXAA

---

### 9. SSAO (屏幕空间环境光遮蔽)

- [ ] **SSAO Pass**
  - 输入：G-Buffer (ViewSpace Normal + Depth)
  - 半球采样核（16-64 samples）+ 随机旋转噪声
  - Range Check 防远距离伪影
- [ ] **SSAO 模糊**
  - 4x4 Bilateral Blur 消除噪点
- [ ] **与 PBR 合成**
  - `FinalColor *= (1.0 - AO)` 或直接乘环境光项

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐⭐

---

### 10. 游戏内 UI 系统

- [ ] **2D 渲染管线**
  - `SpriteBatch` 批量渲染（DrawCall 合并）
  - `Sprite` 组件：纹理/UV/颜色/锚点
  - 正交投影 UI 相机（独立于 3D 相机）
- [ ] **文本渲染**
  - 集成 FreeType 生成字符纹理图集 (Texture Atlas)
  - `TextRenderer`: 多行排版、对齐、颜色、字号
- [ ] **UI 控件体系**
  - `Canvas` 画布（自动缩放适配分辨率）
  - `Button` / `Slider` / `Toggle` / `InputField` / `Image`
  - 事件系统：`OnClick` / `OnValueChanged`
- [ ] **UI 布局**
  - `UIStack` 横/竖排列
  - `UIGrid` 网格布局
  - 锚点系统（TopLeft / Center / BottomRight / Stretch）

**预估工作量**: 3-4 周  
**复杂度**: ⭐⭐⭐⭐

---

### 11. 粒子系统

- [ ] **CPU 粒子系统**
  - `ParticleEmitter`: 发射速率 / 生命周期 / 初始速度 / 重力 / 颜色渐变 / 大小渐变
  - Billboard 渲染（始终面向相机）
  - 粒子池预分配（避免运行时 new/delete）
- [ ] **GPU 粒子 (远期)**
  - Compute Shader 更新粒子状态
  - Transform Feedback / Indirect Draw
- [ ] **编辑器**
  - 粒子效果预览
  - 参数可视化调节

**预估工作量**: 2-3 周  
**复杂度**: ⭐⭐⭐

---

## 🟢 P2 — 渲染架构与性能

### 12. 延迟渲染管线 (Deferred Rendering)

- [ ] **G-Buffer 布局**
  - RT0: Albedo (RGB) + Metallic (A)
  - RT1: WorldSpace Normal (RGB) + Roughness (A)
  - RT2: WorldSpace Position (RGB) + AO (A)
  - DepthStencil (共享)
- [ ] **Lighting Pass**
  - 屏幕空间 Tile-Based 光照（Tile 内收集影响光源）
  - 支持大量动态点光源
- [ ] **与 Forward 共存**
  - Forward Pass 处理半透明物体
  - Forward+ 混合方案

**预估工作量**: 3-4 周  
**复杂度**: ⭐⭐⭐⭐  
**备注**: 当前 Forward 渲染对小场景足够，大场景（>10 光源）才需要

---

### 13. 视锥体剔除 (Frustum Culling)

- [ ] **视锥体裁剪**
  - `Camera::GetFrustumPlanes()`：提取 6 个平面
  - `AABB::IsOnFrustum()`：与 6 平面求交
  - `Scene::OnRender()` 中剔除不可见 Entity
- [ ] **空间加速结构 (BVH / Octree)**
  - `Octree`：空间八叉树划分场景
  - 动态实体插入/移除
  - 范围查询（Frustum / AABB ）

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐⭐

---

### 14. LOD 系统

- [ ] **Mesh LOD**
  - `MeshComponent` 存储多级 LOD 的 VAO 引用
  - 根据相机距离自动切换
  - Screen-Size-Based 选择（投影后屏幕占比）
- [ ] **自动 LOD 生成**
  - MeshOptimizer 库简化网格
  - 预生成 LOD0~LOD3

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐

---

### 15. 渲染管线抽象 (Render Graph / Frame Graph)

- [ ] **Pass 依赖声明**
  - 每个 Pass 声明 Input/Output 资源（纹理引用）
  - 自动推断执行顺序
  - 自动插入 Resource Barrier（Vulkan/DX12 需要）
- [ ] **资源生命周期管理**
  - 临时 RT 自动分配/复用（Transient Resource）
  - 内存别名（Aliasing）

**预估工作量**: 3-4 周  
**复杂度**: ⭐⭐⭐⭐  
**备注**: OpenGL 没有 Barrier 概念，此改造更多为未来 Vulkan 迁移做准备

---

## 🔵 P3 — 引擎架构与工具链

### 16. 资源打包与虚拟文件系统

- [ ] **资源包 (.pak)**
  - 文件打包：目录 → 二进制 blob + 索引表
  - 运行时挂载：`VFS::Mount("data.pak")`
  - 透明读取：`VFS::ReadFile("textures/albedo.png")`
- [ ] **资源引用/句柄系统**
  - `AssetHandle`（UUID）替代裸路径
  - 资源数据库 (`.assetdb`)：路径 ↔ GUID ↔ 类型 映射

**预估工作量**: 2-3 周  
**复杂度**: ⭐⭐⭐

---

### 17. 多线程渲染

- [ ] **渲染线程分离**
  - 主线程：逻辑更新 + 生成 RenderCommand
  - 渲染线程：消费 RenderCommand → 执行 GL 调用
  - 双缓冲命令队列（无锁 Ring Buffer）
- [ ] **异步资源加载**
  - Texture/Model 后台线程加载
  - 完成回调 → 主线程注册 GPU 资源

**预估工作量**: 3-4 周  
**复杂度**: ⭐⭐⭐⭐  
**备注**: OpenGL 多线程限制多，此项更适合 Vulkan 迁移后实现

---

### 18. 网络系统

- [ ] **Client-Server 架构**
  - UDP 可靠层（重传/排序/拥塞控制）
  - Entity 同步：`NetTransform` 插值/预测
- [ ] **RPC 系统**
  - C++ 函数标记为 `[RPC]` → 自动序列化+远程调用
- [ ] **房间/匹配**
  - Lobby 创建/加入/离开

**预估工作量**: 6-8 周  
**复杂度**: ⭐⭐⭐⭐⭐  
**备注**: 这是独立大模块，可参考 ENet / GameNetworkingSockets

---

### 19. 性能分析工具

- [ ] **GPU Profiling**
  - `glBeginQuery(GL_TIME_ELAPSED)` 测量每个 Pass 耗时
  - 编辑器面板展示 GPU 时间线
- [ ] **CPU Profiling**
  - `InstrumentationTimer` RAII 计时器
  - 函数级统计（Tracy / Optick 集成）
- [ ] **内存追踪**
  - 自定义 Allocator 记录分配/释放
  - 编辑器 Memory 面板

**预估工作量**: 1-2 周  
**复杂度**: ⭐⭐

---

### 20. 跨平台支持

- [ ] **Qt / SDL2 窗口后端**
  - 当前仅 GLFW，需抽象 Window 接口支持其他后端
- [ ] **平台适配**
  - Windows ✅ (已支持)
  - Linux (Wayland + X11)
  - macOS (Metal via MoltenVK 远期)
  - Android / iOS (OpenGL ES 3.0)

**预估工作量**: 1-2 周（仅 Linux）/ 长期  
**复杂度**: ⭐⭐⭐

---

### 21. 编辑器增强

- [ ] **场景视图 Gizmos**
  - 平移/旋转/缩放 Gizmo（ImGuizmo 集成）
  - 网格地面线
- [ ] **资源浏览器 (Content Browser)**
  - 目录树浏览 textures/models/shaders/scenes
  - 缩略图预览
  - 拖拽到场景添加 Entity
- [ ] **Undo/Redo 系统**
  - 命令模式：`CommandHistory` 栈
  - Ctrl+Z / Ctrl+Y
- [ ] **Play/Stop 模式**
  - Play 进入游戏模式（ScriptComponent 运行）
  - Stop 恢复编辑状态（场景重置）
- [ ] **预制体 (Prefab)**
  - Entity 模板序列化/实例化
  - 实例与模板的连接更新

**预估工作量**: 4-6 周  
**复杂度**: ⭐⭐⭐⭐

---

## 📋 推荐实施路线图

### Phase 1 — 游戏可玩 (6-8 周)

```
阴影系统 (CSM) ──→ 碰撞/物理 ──→ 模型导入 ──→ 脚本系统 (Lua)
  Week 1-4         Week 2-5       Week 3-5       Week 4-8
```

### Phase 2 — 画面升级 (4-6 周)

```
IBL + HDR Skybox ──→ MSAA ──→ SSAO ──→ 骨骼动画初版
  Week 1-3          Week 3    Week 4    Week 2-6
```

### Phase 3 — 开发体验 (6-8 周)

```
音频系统 ──→ 游戏UI ──→ 粒子系统 ──→ 视锥体剔除
  Week 1-2   Week 2-4   Week 4-6    Week 6-7
```

### Phase 4 — 工业化 (长期)

```
延迟渲染 ──→ 资源打包 ──→ 编辑器增强 ──→ 性能分析 ──→ 网络
```

---

## 📊 当前引擎能力评估

| 维度 | 评分 (1-5) | 说明 |
|------|-----------|------|
| PBR 渲染质量 | ⭐⭐⭐⭐ | Cook-Torrance 完整，后处理齐全 |
| 编辑器工具 | ⭐⭐⭐ | ImGui 5面板，序列化完整 |
| 代码架构 | ⭐⭐⭐⭐ | 分层清晰，组件模式，易于扩展 |
| 阴影/光照 | ⭐⭐ | 多光源支持好，但无阴影 |
| 动画系统 | ⭐ | 仅 Assimp 链接，未实现 |
| 物理/碰撞 | ⭐ | 完全缺失 |
| 音频 | ⭐ | 完全缺失 |
| 脚本/AI | ⭐ | 完全缺失 |
| 性能优化 | ⭐⭐ | 无剔除/LOD/多线程 |
| 跨平台 | ⭐⭐ | 仅 Windows / GLFW |

**综合评分: 2.4/5** — 一个好的渲染引擎起点，距可用的游戏引擎约还需 20-30 周全职开发。
