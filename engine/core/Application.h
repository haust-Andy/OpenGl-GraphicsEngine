#pragma once

#include "Base.h"
#include "Window.h"
#include "Input.h"
#include "Layer.h"
#include "Log.h"
#include "Timestep.h"
#include <functional>
#include <string>

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

// Application - 引擎应用基类
// 用户游戏继承此类并实现自己的逻辑
class Application
{
public:
    Application(const std::string& name = "OpenGL Engine", unsigned int width = 1600, unsigned int height = 900);
    virtual ~Application();

    void Run();
    void Close();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    Window& GetWindow() { return *m_Window; }
    static Application& Get() { return *s_Instance; }

private:
    void OnEvent(Event& e);
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    Scope<Window> m_Window;
    LayerStack m_LayerStack;
    bool m_Running = true;
    bool m_Minimized = false;
    float m_LastFrameTime = 0.0f;

    static Application* s_Instance;
};

// 由客户端定义
Application* CreateApplication();
