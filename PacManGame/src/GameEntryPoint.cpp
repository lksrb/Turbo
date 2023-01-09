#include <Turbo/Core/EntryPoint.h>

#include "Game.h"

Turbo::Application* Turbo::CreateApplication()
{
    Turbo::Application::Config config;
    config.Width = 1600;
    config.Height = 900;
    config.Title = "PacMan";
    config.VSync = true;
    config.Resizable = true;
    config.StartMaximized = false;
    config.EnableUI = false;

    return new PacMan::Game(config);
}

