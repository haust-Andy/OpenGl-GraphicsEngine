#include "Input.h"

GLFWwindow* Input::s_Window = nullptr;

bool Input::IsKeyPressed(KeyCode keycode)
{
    auto state = glfwGetKey(s_Window, (int)keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
    auto state = glfwGetMouseButton(s_Window, (int)button);
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(s_Window, &x, &y);
    return { (float)x, (float)y };
}

float Input::GetMouseX()
{
    auto [x, y] = GetMousePosition();
    return x;
}

float Input::GetMouseY()
{
    auto [x, y] = GetMousePosition();
    return y;
}
