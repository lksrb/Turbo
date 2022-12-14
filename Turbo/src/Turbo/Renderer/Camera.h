#pragma once

#include <glm/glm.hpp>

namespace Turbo
{
    class Camera
    {
    public:
        Camera() = default;
        virtual ~Camera() = default;

        inline glm::mat4 GetViewProjection() const noexcept { return m_Projection * m_ViewMatrix; }
        inline const glm::mat4& GetProjection() const noexcept { return m_Projection; }
    protected:
        glm::mat4 m_Projection{ glm::mat4(1.0f) };
        glm::mat4 m_ViewMatrix{ glm::mat4(1.0f) };
    };
}
