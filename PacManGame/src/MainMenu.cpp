#include "MainMenu.h"

namespace PacMan
{
    MainMenu::MainMenu()
    {
        m_Scene = new Scene({ "MainMenu" });

        // Create camera entity
        m_Camera = m_Scene->CreateEntity();
        auto& camera = m_Camera.AddComponent<CameraComponent>();
        camera.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
        m_Camera.Transform().Translation.z = 20.0f;

        m_MainMenuTexture = Texture2D::Create({ "Assets/Textures/MainMenu.png" });
        m_MainMenuArrowHead = Texture2D::Create({ "Assets/Textures/ArrowHead.png" });

        auto& mainMenuEntity = m_Scene->CreateEntity();
        mainMenuEntity.AddComponent<SpriteRendererComponent>().Texture = m_MainMenuTexture;

        mainMenuEntity.Transform().Scale.x *= 8.0f;
        mainMenuEntity.Transform().Scale.y *= 8.0f;

        m_ArrowHead = m_Scene->CreateEntity();
        m_ArrowHead.AddComponent<SpriteRendererComponent>().Texture = m_MainMenuArrowHead;
        m_ArrowHead.Transform().Translation.x = -6.0f;
        m_ArrowHead.Transform().Translation.y = -0.7f;
        m_ArrowHead.Transform().Scale.x *= 2.0f;
        m_ArrowHead.Transform().Scale.y *= 2.0f;
    }

    MainMenu::~MainMenu()
    {
        delete m_Scene;
    }

    void MainMenu::SetViewportSize(u32 width, u32 height)
    {
        m_Scene->SetViewportSize(width, height);
    }

    void MainMenu::OnUpdate(Time_T ts)
    {
    }

    bool MainMenu::OnPressedKeyEvent(KeyPressedEvent& e)
    {
        switch (e.GetKeyCode())
        {
            case Key::Up:
            {
                if (u32(m_SelectedOption) > u32(Option::Play))
                {
                    m_ArrowHead.Transform().Translation.y += 2.0f;
                    m_SelectedOption = (Option)(u32(m_SelectedOption) - 1);
                }
                return true;
            }
            case Key::Down:
            {
                if (u32(m_SelectedOption) < u32(Option::Exit))
                {
                    m_ArrowHead.Transform().Translation.y -= 2.0f;
                    m_SelectedOption = (Option)(u32(m_SelectedOption) + 1);
                }
                return true;
            }
            case Key::Enter:
            {
                OnOptionConfirmed();
                return true;
            }
        }

        return false;
    }

    void MainMenu::OnOptionConfirmed()
    {
        switch (m_SelectedOption)
        {
            case Option::Play:
                m_Callback(GameEvent::MenuNewGame, nullptr);
                break;
            case Option::Exit:
                m_Callback(GameEvent::MenuExit, nullptr);
                break;
        }
    }

}
