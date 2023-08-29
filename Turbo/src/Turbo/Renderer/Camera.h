#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Turbo
{
    class Camera
    {
    public:
        enum class ProjectionType : u32 { Perspective = 0, Orthographic = 1 };

        Camera() = default;
        virtual ~Camera() = default;

        inline glm::mat4 GetViewProjection() const noexcept { return m_Projection * m_ViewMatrix; }
        inline glm::mat4 GetProjection() const noexcept { return m_Projection; }
        inline void SetViewMatrix(const glm::mat4& viewMatrix) noexcept { m_ViewMatrix = viewMatrix; }
        inline glm::mat4 GetViewMatrix() const noexcept { return m_ViewMatrix; }
    protected:
        glm::mat4 m_Projection{ glm::mat4(1.0f) };
        glm::mat4 m_ViewMatrix{ glm::mat4(1.0f) };
    };
}
