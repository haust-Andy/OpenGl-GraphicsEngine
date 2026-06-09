#include "Application.h"
#include <cstring>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name, unsigned int width, unsigned int height)
{
    s_Instance = this;

    m_Window = Scope<Window>(Window::Create({ name, width, height }));
    m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

    Input::SetWindow(m_Window->GetNativeWindow());

    // ImGui 初始化
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;            // 只有标题栏可拖动窗口
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window->GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 默认显示鼠标光标，允许交互 ImGui
    glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    CORE_INFO("Application '", name, "' initialized (", width, "x", height, ")");
}

Application::~Application()
{
    CORE_INFO("Application shutting down.");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::Run()
{
    CORE_INFO("Entering main loop.");

    while (m_Running)
    {
        float time = (float)glfwGetTime();
        Timestep ts = Timestep::Clamp(Timestep(time - m_LastFrameTime));
        m_LastFrameTime = time;

        if (!m_Minimized)
        {
            // 先清默认帧缓冲（3D 渲染到 FBO 后，默认 FB 是上一帧的残留内容）
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(ts);

            // ImGui 帧开始
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            for (Layer* layer : m_LayerStack)
                layer->OnImGuiRender();

            // ImGui 帧结束并渲染
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        m_Window->OnUpdate();
    }
}

void Application::Close()
{
    CORE_INFO("Close requested.");
    m_Running = false;
}

void Application::PushLayer(Layer* layer)
{
    CORE_INFO("Pushing layer: ", layer->GetName());
    m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* overlay)
{
    CORE_INFO("Pushing overlay: ", overlay->GetName());
    m_LayerStack.PushOverlay(overlay);
}

void Application::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

    // 反向遍历 (Overlay 层先处理事件)
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
    {
        if (e.Handled) break;
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& /*e*/)
{
    m_Running = false;
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
    if (e.GetWidth() == 0 || e.GetHeight() == 0)
    {
        m_Minimized = true;
        return false;
    }

    m_Minimized = false;
    glViewport(0, 0, e.GetWidth(), e.GetHeight());
    return false;
}
