#pragma once

#include "Camera.h"

#include "Turbo/Core/Time.h"
#include "Turbo/Event/Event.h"
#include "Turbo/Event/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Turbo
{
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(f32 fov, f32 aspectRatio, f32 nearClip, f32 farClip);

        virtual ~EditorCamera() = default;

        void OnUpdate(FTime ts);
        void OnEvent(Event& e);

        inline f32 GetDistance() const { return m_Distance; }
        inline void SetDistance(f32 distance) { m_Distance = distance; }

        inline void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = (f32)width; m_ViewportHeight = (f32)height; UpdateProjection(); }

        inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

        inline glm::vec3 GetUpDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
        inline glm::vec3 GetRightDirection() const { return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }
        inline glm::vec3 GetForwardDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

        inline const glm::vec3& GetPosition() const { return m_Position; }
        inline glm::quat GetOrientation() const { return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f)); }

        inline f32 GetPitch() const { return m_Pitch; }
        inline f32 GetYaw() const { return m_Yaw; }

        bool IsControlling() const { return m_IsControlling; }

        void ResetRotation();
    private:
        void UpdateProjection();
        void UpdateView();

        bool OnMouseScroll(MouseScrolledEvent& e);

        void MousePan(const glm::vec2& delta);
        void MouseRotate(const glm::vec2& delta);
        void MouseZoom(f32 delta);

        inline glm::vec3 CalculatePosition() const { return m_FocalPoint - GetForwardDirection() * m_Distance; }

        std::pair<f32, f32> PanSpeed() const;
        f32 RotationSpeed() const;
        f32 ZoomSpeed() const;
    private:
        bool m_IsControlling = false;

        f32 m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

        glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

        f32 m_Distance = 10.0f;
        f32 m_Pitch = 0.0f, m_Yaw = 0.0f;

        f32 m_ViewportWidth = 1280, m_ViewportHeight = 720;
    };
}
