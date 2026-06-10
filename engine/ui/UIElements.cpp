#include "UIElements.h"
#include "renderer/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// ===== SpriteBatch 静态成员 =====
std::shared_ptr<Shader> SpriteBatch::s_SpriteShader;
std::shared_ptr<VertexArray> SpriteBatch::s_QuadVAO;
bool SpriteBatch::s_Initialized = false;
bool SpriteBatch::s_InBatch = false;

void SpriteBatch::Init()
{
    if (s_Initialized) return;

    const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_Model;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_Projection * u_Model * vec4(a_Position, 0.0, 1.0);
}
)";
    const char* fragSrc = R"(
#version 330 core
in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform bool u_HasTexture;
uniform sampler2D u_Texture;

out vec4 FragColor;

void main()
{
    if (u_HasTexture)
        FragColor = texture(u_Texture, v_TexCoord) * u_Color;
    else
        FragColor = u_Color;
}
)";

    s_SpriteShader = std::make_shared<Shader>();
    s_SpriteShader->Compile(vertSrc, fragSrc);
    s_SpriteShader->SetName("Sprite");

    float quadVerts[] = {
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f,
    };
    uint32_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

    auto vbo = std::make_shared<VertexBuffer>(quadVerts, sizeof(quadVerts));
    auto ibo = std::make_shared<IndexBuffer>(quadIndices, 6);
    s_QuadVAO = std::make_shared<VertexArray>();
    s_QuadVAO->AddVertexBuffer(vbo);
    s_QuadVAO->SetIndexBuffer(ibo);

    glBindVertexArray(s_QuadVAO->GetRendererID());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    s_Initialized = true;
    std::cout << "[SpriteBatch] Initialized" << std::endl;
}

void SpriteBatch::Shutdown()
{
    s_SpriteShader.reset();
    s_QuadVAO.reset();
    s_Initialized = false;
}

void SpriteBatch::Begin(const glm::mat4& projection)
{
    if (!s_Initialized) Init();
    s_InBatch = true;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    s_SpriteShader->Bind();
    s_SpriteShader->SetMat4("u_Projection", projection);
}

void SpriteBatch::End()
{
    s_InBatch = false;
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void SpriteBatch::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                             const glm::vec4& color, float rotation)
{
    if (!s_Initialized || !s_InBatch) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    if (rotation != 0.0f)
        model = glm::rotate(model, rotation, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    s_SpriteShader->SetMat4("u_Model", model);
    s_SpriteShader->SetVec4("u_Color", color);
    s_SpriteShader->SetBool("u_HasTexture", false);

    s_QuadVAO->Bind();
    auto& ib = s_QuadVAO->GetIndexBuffer();
    if (ib)
        glDrawElements(GL_TRIANGLES, (GLsizei)ib->GetCount(), GL_UNSIGNED_INT, 0);
}

void SpriteBatch::DrawTexture(const std::shared_ptr<Texture2D>& texture,
                                const glm::vec2& position, const glm::vec2& size,
                                const glm::vec4& tint, float rotation)
{
    if (!s_Initialized || !s_InBatch) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    if (rotation != 0.0f)
        model = glm::rotate(model, rotation, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    s_SpriteShader->SetMat4("u_Model", model);
    s_SpriteShader->SetVec4("u_Color", tint);
    s_SpriteShader->SetBool("u_HasTexture", true);
    s_SpriteShader->SetInt("u_Texture", 0);

    if (texture)
        texture->Bind(0);

    s_QuadVAO->Bind();
    auto& ib = s_QuadVAO->GetIndexBuffer();
    if (ib)
        glDrawElements(GL_TRIANGLES, (GLsizei)ib->GetCount(), GL_UNSIGNED_INT, 0);
}

void SpriteBatch::DrawText(const std::string& text, const glm::vec2& position,
                             float fontSize, const glm::vec4& color)
{
    // 简易文本渲染: 逐字符绘制矩形
    // 生产级实现应使用 FreeType + 字形图集
    float charWidth = fontSize * 0.6f;
    float x = position.x;
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == ' ')
        {
            x += charWidth * 0.5f;
            continue;
        }
        // 绘制字符占位矩形 (白色)
        glm::vec2 charPos(x, position.y);
        DrawQuad(charPos, glm::vec2(charWidth, fontSize), color * 0.5f);
        x += charWidth;
    }
}

void SpriteBatch::RenderUIElement(UIElement* element, const glm::mat4& projection)
{
    if (!element || !element->Visible) return;
    // 委托给元素自身渲染
    element->Render(projection);
}

void SpriteBatch::Flush()
{
    // 当前实现是立即模式, 无需 flush
}

// ===== UICanvas =====
void UICanvas::Render()
{
    glm::mat4 projection = glm::ortho(0.0f, Resolution.x, Resolution.y, 0.0f, -1.0f, 1.0f);
    SpriteBatch::Begin(projection);
    // 渲染子元素...
    SpriteBatch::End();
}

// ===== UIElement =====
bool UIElement::HitTest(const glm::vec2& point) const
{
    return point.x >= Position.x && point.x <= Position.x + Size.x &&
           point.y >= Position.y && point.y <= Position.y + Size.y;
}

glm::vec2 UIElement::GetWorldPosition(const glm::vec2& screenResolution) const
{
    glm::vec2 pos = Position;
    switch (AnchorPoint)
    {
    case Anchor::TopLeft:      break;
    case Anchor::TopCenter:    pos.x += screenResolution.x * 0.5f; break;
    case Anchor::TopRight:     pos.x += screenResolution.x; break;
    case Anchor::MiddleLeft:  pos.y += screenResolution.y * 0.5f; break;
    case Anchor::MiddleCenter: pos.x += screenResolution.x * 0.5f; pos.y += screenResolution.y * 0.5f; break;
    case Anchor::MiddleRight:  pos.x += screenResolution.x; pos.y += screenResolution.y * 0.5f; break;
    case Anchor::BottomLeft:   pos.y += screenResolution.y; break;
    case Anchor::BottomCenter: pos.x += screenResolution.x * 0.5f; pos.y += screenResolution.y; break;
    case Anchor::BottomRight:  pos.x += screenResolution.x; pos.y += screenResolution.y; break;
    }
    return pos;
}

// ===== UIImage =====
void UIImage::Render(const glm::mat4& projection)
{
    SpriteBatch::DrawTexture(ImageTexture, Position, Size, Tint, Rotation);
}

// ===== UIText =====
void UIText::Render(const glm::mat4& projection)
{
    SpriteBatch::DrawText(Text, Position, FontSize, TextColor);
}

// ===== UIButton =====
void UIButton::Render(const glm::mat4& projection)
{
    glm::vec4 color = NormalColor;
    if (IsPressed) color = PressedColor;
    else if (IsHovered) color = HoverColor;

    SpriteBatch::DrawQuad(Position, Size, color, Rotation);
    SpriteBatch::DrawText(Label, Position + glm::vec2(Size.x * 0.1f, Size.y * 0.3f),
                           FontSize, glm::vec4(1.0f));
}
