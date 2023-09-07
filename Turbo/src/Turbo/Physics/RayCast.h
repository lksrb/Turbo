#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include <glm/glm.hpp>

namespace Turbo {

    enum class RayTarget : u32
    {
        Nearest = 0,
        Furthest,
        Any
    };

    struct Ray
    {
        explicit Ray(const glm::vec3& start, const glm::vec3& direction) : Start(start), Direction(direction) {}

        glm::vec3 Start;
        glm::vec3 Direction;
    };

    // Matches C# side
    struct CastRayResult
    {
        glm::vec3 HitPosition{ 0.0f };
        u64 HitEntity = 0;
    };

}
