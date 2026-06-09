#pragma once

#include <string>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Event.h"

struct WindowProps
{
    std::string Title;
    unsigned int Width;
    unsigned int Height;

    WindowProps(const std::string& title = "OpenGL Engine",
                unsigned int width = 1600,
                unsigned int height = 900)
        : Title(title), Width(width), Height(height) {}
};

class Window
{
public:
    using EventCallbackFn = std::function<void(Event&)>;

    Window(const WindowProps& props = WindowProps());
    ~Window();

    void OnUpdate();

    unsigned int GetWidth() const { return m_Data.Width; }
    unsigned int GetHeight() const { return m_Data.Height; }
    float GetAspectRatio() const { return (float)m_Data.Width / (float)m_Data.Height; }

    void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled);
    bool IsVSync() const;

    GLFWwindow* GetNativeWindow() const { return m_Window; }

    static Window* Create(const WindowProps& props = WindowProps());

private:
    void Init(const WindowProps& props);
    void Shutdown();

    GLFWwindow* m_Window;

    struct WindowData
    {
        std::string Title;
        unsigned int Width, Height;
        bool VSync;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};
