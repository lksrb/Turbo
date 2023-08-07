#pragma once

#include "Turbo/Renderer/Camera.h"

namespace Turbo
{
    class SceneCamera : public Camera
    {
    public:
        SceneCamera();
        ~SceneCamera();

        void SetOrthographic(f32 size, f32 nearClip, f32 farClip);
        void SetPerspective(f32 v_fov, f32 nearClip, f32 farClip);

        void SetViewportSize(u32 width, u32 height);

        // Orthographic Camera
        void SetOrthographicSize(f32 size) { m_OrthographicSize = size; RecalculateProjection(); }
        void SetOrthographicNearClip(f32 nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
        void SetOrthographicFarClip(f32 farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

        f32 GetOrthographicSize() const { return m_OrthographicSize; }
        f32 GetOrthographicNearClip() const { return m_OrthographicNear; }
        f32 GetOrthographicFarClip() const { return m_OrthographicFar; }

        // Perspective Camera
        void SetPerspectiveVerticalFOV(f32 v_fov) { m_PerspectiveFOV = v_fov; RecalculateProjection(); }
        void SetPerspectiveNearClip(f32 nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
        void SetPerspectiveFarClip(f32 farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

        f32 GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
        f32 GetPerspectiveNearClip() const { return m_PerspectiveNear; }
        f32 GetPerspectiveFarClip() const { return m_PerspectiveFar; }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type;  RecalculateProjection(); }
    private:
        void RecalculateProjection();
    private:
        ProjectionType m_ProjectionType = ProjectionType::Perspective;

        f32 m_OrthographicSize = 10.0;
        f32 m_OrthographicNear = -1.0, m_OrthographicFar = 1.0;

        f32 m_PerspectiveFOV = glm::radians(45.0f);
        f32 m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

        f32 m_AspectRatio = 0.0f;
    };
}

