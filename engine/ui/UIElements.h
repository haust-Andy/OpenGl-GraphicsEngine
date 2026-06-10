#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/VertexArray.h"

// ===== 2D 渲染器 (用于游戏内 UI) =====

// UI 画布 - 管理所有 UI 元素的根节点
class UICanvas
{
public:
    enum class ScalingMode
    {
        ConstantPixelSize,  // 固定像素大小
        ScaleWithScreen     // 随屏幕缩放
    };

    glm::vec2 Resolution = glm::vec2(1600.0f, 900.0f);  // 设计分辨率
    ScalingMode Scaling = ScalingMode::ConstantPixelSize;

    // 渲染所有 UI 元素
    void Render();
};

// UI 元素基类
class UIElement
{
public:
    virtual ~UIElement() = default;

    std::string Name = "UIElement";
    glm::vec2 Position = glm::vec2(0.0f);
    glm::vec2 Size = glm::vec2(100.0f, 50.0f);
    glm::vec2 Pivot = glm::vec2(0.5f);  // 锚点 (0-1)
    float Rotation = 0.0f;
    glm::vec4 Color = glm::vec4(1.0f);
    bool Visible = true;
    bool Interactable = true;

    // 锚点
    enum class Anchor
    {
        TopLeft, TopCenter, TopRight,
        MiddleLeft, MiddleCenter, MiddleRight,
        BottomLeft, BottomCenter, BottomRight
    };
    Anchor AnchorPoint = Anchor::TopLeft;

    // 事件
    std::function<void()> OnClick;
    std::function<void()> OnHover;

    virtual void Render(const glm::mat4& projection) = 0;
    virtual bool HitTest(const glm::vec2& point) const;

    // 计算世界空间位置
    glm::vec2 GetWorldPosition(const glm::vec2& screenResolution) const;
};

// 图像 UI 元素
class UIImage : public UIElement
{
public:
    std::shared_ptr<Texture2D> ImageTexture;
    glm::vec4 Tint = glm::vec4(1.0f);

    void Render(const glm::mat4& projection) override;
};

// 文本 UI 元素 (简易版 - 使用程序化字符贴图)
class UIText : public UIElement
{
public:
    std::string Text = "Label";
    float FontSize = 16.0f;
    glm::vec4 TextColor = glm::vec4(1.0f);
    enum class Alignment { Left, Center, Right };
    Alignment TextAlign = Alignment::Left;

    void Render(const glm::mat4& projection) override;
};

// 按钮 UI 元素
class UIButton : public UIElement
{
public:
    glm::vec4 NormalColor   = glm::vec4(0.2f, 0.3f, 0.5f, 0.9f);
    glm::vec4 HoverColor    = glm::vec4(0.3f, 0.4f, 0.6f, 0.9f);
    glm::vec4 PressedColor  = glm::vec4(0.15f, 0.2f, 0.4f, 0.9f);
    std::string Label = "Button";
    float FontSize = 14.0f;

    void Render(const glm::mat4& projection) override;
    bool IsHovered = false;
    bool IsPressed = false;
};

// 2D Sprite 批量渲染器
class SpriteBatch
{
public:
    static void Init();
    static void Shutdown();

    // 开始/结束批量渲染
    static void Begin(const glm::mat4& projection);
    static void End();

    // 绘制 API
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color, float rotation = 0.0f);
    static void DrawTexture(const std::shared_ptr<Texture2D>& texture,
                             const glm::vec2& position, const glm::vec2& size,
                             const glm::vec4& tint = glm::vec4(1.0f), float rotation = 0.0f);
    static void DrawText(const std::string& text, const glm::vec2& position,
                          float fontSize, const glm::vec4& color);

    // 渲染 UI 元素
    static void RenderUIElement(UIElement* element, const glm::mat4& projection);

private:
    static void Flush();

    static std::shared_ptr<Shader> s_SpriteShader;
    static std::shared_ptr<VertexArray> s_QuadVAO;
    static bool s_Initialized;
    static bool s_InBatch;
};
