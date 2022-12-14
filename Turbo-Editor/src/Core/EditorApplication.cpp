#include <Turbo/Core/EntryPoint.h>

#include "Editor.h"

Turbo::Application* Turbo::CreateApplication()
{
    Turbo::Application::Config config;
    config.Width = 1600;
    config.Height = 900;
    config.Title = "TurboEditor";
    config.VSync = true;
    config.Resizable = true;
    config.StartMaximized = false;
    //specification.EnableUI = false;

    return new Turbo::Ed::Editor(config);
}

