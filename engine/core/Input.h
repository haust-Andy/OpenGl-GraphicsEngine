#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"
#include <GLFW/glfw3.h>
#include <utility>

// 输入系统 - 基于 GLFW 轮询
class Input
{
public:
    static bool IsKeyPressed(KeyCode keycode);
    static bool IsMouseButtonPressed(MouseCode button);
    static std::pair<float, float> GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();

    // 获取当前关联的窗口 (由 Application 设置)
    static void SetWindow(GLFWwindow* window) { s_Window = window; }

private:
    static GLFWwindow* s_Window;
};
