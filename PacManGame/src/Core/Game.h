#pragma once

#include <Turbo.h>

namespace PacMan
{
    using namespace Turbo;

    class Game : public Application
    {
    public:
        Game(const Application::Config& config);
    protected:
        void OnInitialize() override;
        void OnShutdown() override;
        void OnUpdate() override;
        void OnEvent(Event& e) override;
        void OnDraw() override;
        void OnDrawUI() override;
    private:
        bool OnWindowResize(WindowResizeEvent& e);

        u32 m_ViewportWidth = 0;
        u32 m_ViewportHeight = 0;

        SceneRenderer* m_Renderer = nullptr;
        Scene* m_CurrentScene = nullptr;
    };
}
