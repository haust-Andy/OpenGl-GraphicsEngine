#include "Window.h"
#include "KeyCodes.h"
#include "MouseCodes.h"

#include <iostream>

Window::Window(const WindowProps& props)
{
    Init(props);
}

Window::~Window()
{
    Shutdown();
}

void Window::Init(const WindowProps& props)
{
    m_Data.Title  = props.Title;
    m_Data.Width  = props.Width;
    m_Data.Height = props.Height;
    m_Data.VSync  = true;

    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "[Window] Failed to initialize GLFW!" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height,
                                 m_Data.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        std::cerr << "[Window] Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);

    // 加载 OpenGL 函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "[Window] Failed to initialize GLAD!" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(m_Window, &m_Data);
    SetVSync(true);

    // ===== GLFW 回调 -> 事件系统 =====

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width  = width;
        data.Height = height;

        WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        data.EventCallback(event);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action)
        {
        case GLFW_PRESS:
        {
            KeyPressedEvent event((KeyCode)key, 0);
            data.EventCallback(event);
            break;
        }
        case GLFW_RELEASE:
        {
            KeyReleasedEvent event((KeyCode)key);
            data.EventCallback(event);
            break;
        }
        case GLFW_REPEAT:
        {
            KeyPressedEvent event((KeyCode)key, 1);
            data.EventCallback(event);
            break;
        }
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action)
        {
        case GLFW_PRESS:
        {
            MouseButtonPressedEvent event((MouseCode)button);
            data.EventCallback(event);
            break;
        }
        case GLFW_RELEASE:
        {
            MouseButtonReleasedEvent event((MouseCode)button);
            data.EventCallback(event);
            break;
        }
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.EventCallback(event);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseMovedEvent event((float)xPos, (float)yPos);
        data.EventCallback(event);
    });
}

void Window::Shutdown()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::OnUpdate()
{
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

void Window::SetVSync(bool enabled)
{
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    m_Data.VSync = enabled;
}

bool Window::IsVSync() const
{
    return m_Data.VSync;
}

Window* Window::Create(const WindowProps& props)
{
    return new Window(props);
}
