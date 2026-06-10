#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"
#include <string>
#include <functional>
#include <sstream>

// 事件类型枚举
enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

// L-01: 使用 constexpr 替代预处理器宏
constexpr int BIT(int x) { return 1 << x; }

enum EventCategory
{
    None           = 0,
    EventCategoryApplication  = BIT(0),
    EventCategoryInput        = BIT(1),
    EventCategoryKeyboard     = BIT(2),
    EventCategoryMouse        = BIT(3),
    EventCategoryMouseButton  = BIT(4)
};

// 事件基类
class Event
{
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool IsInCategory(EventCategory category) const
    {
        return GetCategoryFlags() & category;
    }
};

// 事件分发器
class EventDispatcher
{
public:
    EventDispatcher(Event& event)
        : m_Event(event) {}

    template<typename T, typename F>
    bool Dispatch(const F& func)
    {
        if (m_Event.GetEventType() == T::GetStaticType())
        {
            m_Event.Handled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

// ============ 窗口事件 ============

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(int width, int height)
        : m_Width(width), m_Height(height) {}

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    static EventType GetStaticType() { return EventType::WindowResize; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "WindowResize"; }
    int GetCategoryFlags() const override { return EventCategoryApplication; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
        return ss.str();
    }

private:
    int m_Width, m_Height;
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() = default;

    static EventType GetStaticType() { return EventType::WindowClose; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "WindowClose"; }
    int GetCategoryFlags() const override { return EventCategoryApplication; }
};

// ============ 键盘事件 ============

class KeyPressedEvent : public Event
{
public:
    KeyPressedEvent(KeyCode keycode, int repeatCount)
        : m_KeyCode(keycode), m_RepeatCount(repeatCount) {}

    KeyCode GetKeyCode() const { return m_KeyCode; }
    int GetRepeatCount() const { return m_RepeatCount; }

    static EventType GetStaticType() { return EventType::KeyPressed; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "KeyPressed"; }
    int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (repeat=" << m_RepeatCount << ")";
        return ss.str();
    }

private:
    KeyCode m_KeyCode;
    int m_RepeatCount;
};

class KeyReleasedEvent : public Event
{
public:
    KeyReleasedEvent(KeyCode keycode)
        : m_KeyCode(keycode) {}

    KeyCode GetKeyCode() const { return m_KeyCode; }

    static EventType GetStaticType() { return EventType::KeyReleased; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "KeyReleased"; }
    int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
    }

private:
    KeyCode m_KeyCode;
};

// ============ 鼠标事件 ============

class MouseButtonPressedEvent : public Event
{
public:
    MouseButtonPressedEvent(MouseCode button)
        : m_Button(button) {}

    MouseCode GetMouseButton() const { return m_Button; }

    static EventType GetStaticType() { return EventType::MouseButtonPressed; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "MouseButtonPressed"; }
    int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryMouseButton | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << m_Button;
        return ss.str();
    }

private:
    MouseCode m_Button;
};

class MouseButtonReleasedEvent : public Event
{
public:
    MouseButtonReleasedEvent(MouseCode button)
        : m_Button(button) {}

    MouseCode GetMouseButton() const { return m_Button; }

    static EventType GetStaticType() { return EventType::MouseButtonReleased; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "MouseButtonReleased"; }
    int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryMouseButton | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << m_Button;
        return ss.str();
    }

private:
    MouseCode m_Button;
};

class MouseMovedEvent : public Event
{
public:
    MouseMovedEvent(float x, float y)
        : m_MouseX(x), m_MouseY(y) {}

    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }

    static EventType GetStaticType() { return EventType::MouseMoved; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "MouseMoved"; }
    int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
        return ss.str();
    }

private:
    float m_MouseX, m_MouseY;
};

class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(float xOffset, float yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }

    static EventType GetStaticType() { return EventType::MouseScrolled; }
    EventType GetEventType() const override { return GetStaticType(); }
    const char* GetName() const override { return "MouseScrolled"; }
    int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
        return ss.str();
    }

private:
    float m_XOffset, m_YOffset;
};
