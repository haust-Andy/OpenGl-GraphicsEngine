#include "Test.h"
#include "engine/core/Event.h"
#include <string>

// =====================================
//  Test: Event System
// =====================================

// ----- Window Events -----

TEST(Event, WindowCloseEvent_TypeAndName)
{
    WindowCloseEvent e;
    CHECK_TRUE(e.GetEventType() == EventType::WindowClose);
    CHECK_STR_EQ(e.GetName(), "WindowClose");
    CHECK_TRUE(e.IsInCategory(EventCategoryApplication));
    CHECK_FALSE(e.IsInCategory(EventCategoryInput));
}

TEST(Event, WindowResizeEvent)
{
    WindowResizeEvent e(1920, 1080);
    CHECK_TRUE(e.GetEventType() == EventType::WindowResize);
    CHECK_EQ(e.GetWidth(),  1920);
    CHECK_EQ(e.GetHeight(), 1080);

    std::string s = e.ToString();
    CHECK_TRUE(s.find("1920") != std::string::npos);
    CHECK_TRUE(s.find("1080") != std::string::npos);
}

// ----- Key Events -----

TEST(Event, KeyPressedEvent)
{
    KeyPressedEvent e(Key::A, 0);
    CHECK_TRUE(e.GetEventType() == EventType::KeyPressed);
    CHECK_EQ(e.GetKeyCode(), Key::A);
    CHECK_EQ(e.GetRepeatCount(), 0);
    CHECK_TRUE(e.IsInCategory(EventCategoryKeyboard));
    CHECK_TRUE(e.IsInCategory(EventCategoryInput));
}

TEST(Event, KeyReleasedEvent)
{
    KeyReleasedEvent e(Key::Escape);
    CHECK_TRUE(e.GetEventType() == EventType::KeyReleased);
    CHECK_EQ(e.GetKeyCode(), Key::Escape);
}

TEST(Event, KeyRepeatCount)
{
    KeyPressedEvent e(Key::W, 5);
    CHECK_EQ(e.GetRepeatCount(), 5);
    std::string s = e.ToString();
    CHECK_TRUE(s.find("repeat=5") != std::string::npos);
}

// ----- Mouse Events -----

TEST(Event, MouseButtonPressedEvent)
{
    MouseButtonPressedEvent e(Mouse::ButtonLeft);
    CHECK_TRUE(e.GetEventType() == EventType::MouseButtonPressed);
    CHECK_EQ(e.GetMouseButton(), Mouse::ButtonLeft);
    CHECK_TRUE(e.IsInCategory(EventCategoryMouse));
    CHECK_TRUE(e.IsInCategory(EventCategoryMouseButton));
    CHECK_TRUE(e.IsInCategory(EventCategoryInput));
}

TEST(Event, MouseButtonReleasedEvent)
{
    MouseButtonReleasedEvent e(Mouse::ButtonRight);
    CHECK_TRUE(e.GetEventType() == EventType::MouseButtonReleased);
    CHECK_EQ(e.GetMouseButton(), Mouse::ButtonRight);
}

TEST(Event, MouseMovedEvent)
{
    MouseMovedEvent e(320.5f, 240.25f);
    CHECK_TRUE(e.GetEventType() == EventType::MouseMoved);
    CHECK_FLOAT_EQ(e.GetX(), 320.5f,  0.001f);
    CHECK_FLOAT_EQ(e.GetY(), 240.25f, 0.001f);
    CHECK_FALSE(e.IsInCategory(EventCategoryMouseButton));
    CHECK_TRUE(e.IsInCategory(EventCategoryMouse));
}

TEST(Event, MouseScrolledEvent)
{
    MouseScrolledEvent e(-0.5f, 2.0f);
    CHECK_TRUE(e.GetEventType() == EventType::MouseScrolled);
    CHECK_FLOAT_EQ(e.GetXOffset(), -0.5f, 0.001f);
    CHECK_FLOAT_EQ(e.GetYOffset(),  2.0f, 0.001f);
}

// ----- Event Dispatcher -----

TEST(Event, Dispatcher_DispatchToCorrectType)
{
    KeyPressedEvent e(Key::Space, 0);
    EventDispatcher dispatcher(e);

    bool called = false;
    bool dispatched = dispatcher.Dispatch<KeyPressedEvent>(
        [&called](KeyPressedEvent& ev) -> bool {
            called = true;
            CHECK_EQ(ev.GetKeyCode(), Key::Space);
            return true;  // handled
        });

    CHECK_TRUE(dispatched);
    CHECK_TRUE(called);
    CHECK_TRUE(e.Handled);  // handler returned true
}

TEST(Event, Dispatcher_NoDispatchToWrongType)
{
    WindowCloseEvent e;
    EventDispatcher dispatcher(e);

    bool called = false;
    bool dispatched = dispatcher.Dispatch<KeyPressedEvent>(
        [&called](KeyPressedEvent&) { called = true; return true; });

    CHECK_FALSE(dispatched);
    CHECK_FALSE(called);
}

TEST(Event, Dispatcher_HandlerReturnsFalse)
{
    KeyPressedEvent e(Key::A, 0);
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<KeyPressedEvent>(
        [](KeyPressedEvent&) { return false; });

    // handler 返回 false 表示未处理
    CHECK_FALSE(e.Handled);
}

TEST(Event, Dispatcher_MultipleCalls)
{
    KeyPressedEvent e(Key::A, 0);
    EventDispatcher d1(e);
    EventDispatcher d2(e);

    bool first = false, second = false;
    d1.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&) { first = true;  return true; });
    d2.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&) { second = true; return true; });

    CHECK_TRUE(first);
    CHECK_TRUE(second);
}

// ----- IsInCategory -----

TEST(Event, IsInCategory_MultipleFlags)
{
    MouseButtonPressedEvent e(Mouse::ButtonLeft);

    // 单类别
    CHECK_TRUE(e.IsInCategory(EventCategoryMouse));
    CHECK_TRUE(e.IsInCategory(EventCategoryMouseButton));
    CHECK_TRUE(e.IsInCategory(EventCategoryInput));

    // 不在其他类别
    CHECK_FALSE(e.IsInCategory(EventCategoryKeyboard));
    CHECK_FALSE(e.IsInCategory(EventCategoryApplication));
}

TEST(Event, WindowEvent_OnlyApplicationCategory)
{
    WindowCloseEvent e;
    CHECK_TRUE(e.IsInCategory(EventCategoryApplication));
    CHECK_FALSE(e.IsInCategory(EventCategoryInput));
    CHECK_FALSE(e.IsInCategory(EventCategoryKeyboard));
}
