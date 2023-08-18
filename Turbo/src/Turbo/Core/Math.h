#pragma once

#include <glm/glm.hpp>

namespace Turbo::Math
{
    bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

    /**
     * Space: [-1, 1]
     * Inversed y because of the viewport (might delete later)
     */ 
    glm::vec3 UnProject(glm::vec2 position, const glm::vec4& viewport, const glm::mat4& viewProjection);

    /**
     * @brief Converts screen coordinates to world position ray
     * @return Normalized ray direction
     */
    glm::vec3 ScreenToRayDirection(glm::vec2 position, const glm::vec4& viewport, const glm::mat4& viewProjection);
}
