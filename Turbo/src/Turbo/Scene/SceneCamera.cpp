#include "tbopch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Turbo
{
    SceneCamera::SceneCamera()
    {
        RecalculateProjection();
    }

    SceneCamera::~SceneCamera()
    {
    }

    void SceneCamera::SetOrthographic(f32 size, f32 nearClip, f32 farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;

        RecalculateProjection();
    }

    void SceneCamera::SetPerspective(f32 v_fov, f32 nearClip, f32 farClip)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_PerspectiveFOV = v_fov;
        m_PerspectiveNear = nearClip;
        m_PerspectiveFar = farClip;

        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(u32 width, u32 height)
    {
        m_AspectRatio = (f32)width / (f32)height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_Projection = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
        }
        else
        {
            f32 orthoLeft = -0.5f * m_AspectRatio * m_OrthographicSize;
            f32 orthoRight = 0.5f * m_AspectRatio * m_OrthographicSize;
            f32 orthoBottom = -0.5f * m_OrthographicSize;
            f32 orthoTop = 0.5f * m_OrthographicSize;

            m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
        }

        // Flip y 
        m_Projection[1][1] *= -1;
    }

}
