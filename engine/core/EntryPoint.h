#pragma once

// 简化程序入口
// 用户只需定义 CreateApplication() 返回 Application* 即可
// main 函数在引擎内部定义

extern Application* CreateApplication();

int main(int argc, char** argv)
{
    auto app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}
