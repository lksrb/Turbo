#include <Turbo/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Turbo::Ed {

    class EditorApplication : public Application
    {
    public:
        EditorApplication(const Application::Config& config)
            : Application(config)
        {
        }

        void OnInit() override
        {
            PushLayer(new EditorLayer);
        }
    };

}

Turbo::Application* Turbo::CreateApplication()
{
    Turbo::Application::Config config;
    config.Width = 1600;
    config.Height = 900;
    config.Title = "Turbo Editor";
    config.VSync = true; // TODO: Swapchain enable vsync
    config.Resizable = true;
    config.StartMaximized = true;
    config.EnableUI = true;

    return new Turbo::Ed::EditorApplication(config);
}

