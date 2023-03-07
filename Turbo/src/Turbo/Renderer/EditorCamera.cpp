#include "tbopch.h"
#include "EditorCamera.h"

#include "Turbo/Core/Input.h"
#include "Turbo/Core/KeyCodes.h"
#include "Turbo/Core/MouseCodes.h"

namespace Turbo
{
    EditorCamera::EditorCamera(f32 fov, f32 aspectRatio, f32 nearClip, f32 farClip)
        : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
    {
        m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);

        UpdateView();
    }

    void EditorCamera::UpdateProjection()
    {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    }

    void EditorCamera::UpdateView()
    {
        //m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
        m_Position = CalculatePosition();

        glm::quat orientation = GetOrientation();
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    std::pair<f32, f32> EditorCamera::PanSpeed() const
    {
        f32 x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
        f32 x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        f32 y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
        f32 y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { x_factor, y_factor };
    }

    f32 EditorCamera::RotationSpeed() const
    {
        return 0.8f;
    }

    f32 EditorCamera::ZoomSpeed() const
    {
        f32 distance = m_Distance * 0.2f;
        distance = std::max(distance, 0.0f);
        f32 speed = distance * distance;
        speed = std::min(speed, 100.0f); // max speed = 100
        return speed;
    }

    void EditorCamera::OnUpdate(Time_T ts)
    {
        m_IsControlling = Input::IsKeyPressed(Key::LeftAlt);

        if (m_IsControlling)
        {
            const glm::vec2& mouse = { Input::GetMouseX(), Input::GetMouseY() };
            glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse;

            if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
                MousePan(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
                MouseRotate(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
                MouseZoom(delta.y);
        }

        UpdateView();
    }

    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(TBO_BIND_FN(EditorCamera::OnMouseScroll));
    }

    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
    {
        f32 delta = e.GetOffsetY() * 0.1f;
        MouseZoom(delta);
        UpdateView();
        return false;
    }

    void EditorCamera::MousePan(const glm::vec2& delta)
    {
        auto [x_speed, y_speed] = PanSpeed();
        m_FocalPoint += -GetRightDirection() * delta.x * x_speed * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * y_speed * m_Distance;
    }

    void EditorCamera::MouseRotate(const glm::vec2& delta)
    {
        // TODO: Flipped pitch behaviour
        f32 yaw_sign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
        m_Yaw += yaw_sign * delta.x * RotationSpeed();
        m_Pitch += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(f32 delta)
    {
        m_Distance -= delta * ZoomSpeed();
        if (m_Distance < 1.0f)
        {
            m_FocalPoint += GetForwardDirection();
            m_Distance = 1.0f;
        }
    }
}