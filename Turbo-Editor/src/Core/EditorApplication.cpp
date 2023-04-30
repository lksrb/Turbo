#include <Turbo/Core/EntryPoint.h>

#include "Editor.h"

Turbo::Application* Turbo::CreateApplication()
{
    Turbo::Application::Config config;
    config.Width = 1600;
    config.Height = 900;
    config.Title = "TurboEditor";
    config.VSync = true; // TODO(later): Swapchain enable vsync
    config.Resizable = true;
    config.StartMaximized = true;
    config.EnableUI = true;

    return new Turbo::Ed::Editor(config);
}

