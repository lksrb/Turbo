#include <Turbo/Core/EntryPoint.h>

//#include "Runtime.h"

Turbo::Application* Turbo::CreateApplication()
{
    Turbo::Application::Config config;
    config.Width = 1600;
    config.Height = 900;
    config.Title = "TurboRuntime";
    config.VSync = true;
    config.Resizable = true;
    config.StartMaximized = false;
    //specification.EnableUI = false;

    return nullptr;/*new Runtime(config);*/
}

