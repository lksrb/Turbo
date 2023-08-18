#include "tbopch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Turbo
{
    bool Math::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
    {
        using namespace glm;
        using T = f32;

        mat4 localMatrix(transform);

        // Normalize the matrix.
        if (epsilonEqual(localMatrix[3][3], static_cast<f32>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (
            epsilonNotEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
            localMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(localMatrix[3]);
        localMatrix[3] = vec4(0, 0, 0, localMatrix[3].w);

        vec3 Row[3];

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = localMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]);
        Row[0] = detail::scale(Row[0], static_cast<T>(1));
        scale.y = length(Row[1]);
        Row[1] = detail::scale(Row[1], static_cast<T>(1));
        scale.z = length(Row[2]);
        Row[2] = detail::scale(Row[2], static_cast<T>(1));

        rotation.y = asin(-Row[0][2]);
        if (cos(rotation.y) != 0)
        {
            rotation.x = atan2(Row[1][2], Row[2][2]);
            rotation.z = atan2(Row[0][1], Row[0][0]);
        }
        else
        {
            rotation.x = atan2(-Row[2][0], Row[1][1]);
            rotation.z = 0;
        }

        return true;
    }

    glm::vec3 Math::UnProject(glm::vec2 position, const glm::vec4& viewport, const glm::mat4& viewProjection)
    {
        glm::mat4 inverse = glm::inverse(viewProjection);

        // Convert to [-1, 1] space
        glm::vec4 temp = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        temp.x = ((2.0f * (position.x - viewport[0])) / viewport[2]) - 1.0f;
        temp.y = 1.0f - ((2.0f * (position.y - viewport[1])) / viewport[3]);

        // Unprojecting
        glm::vec4 result = inverse * temp;

        // Magic
        result /= result.w;

        return glm::vec3(result);
	}

    glm::vec3 Math::ScreenToRayDirection(glm::vec2 position, const glm::vec4& viewport, const glm::mat4& viewProjection)
    {
        glm::mat4 inverse = glm::inverse(viewProjection);
        glm::vec4 worldNear(0.0f);
        glm::vec4 worldFar(0.0f);
        {
            // Convert to [-1, 1] space
            glm::vec4 nearClip = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
            nearClip.x = ((2.0f * (position.x - viewport[0])) / viewport[2]) - 1.0f;
            nearClip.y = 1.0f - ((2.0f * (position.y - viewport[1])) / viewport[3]);

            // Unprojecting
            worldNear = inverse * nearClip;

            // Magic
            worldNear /= worldNear.w;
        }

        {
            // Convert to [-1, 1] space
            glm::vec4 farClip = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            farClip.x = ((2.0f * (position.x - viewport[0])) / viewport[2]) - 1.0f;
            farClip.y = 1.0f - ((2.0f * (position.y - viewport[1])) / viewport[3]);

            // Unprojecting
            worldFar = inverse * farClip;

            // Magic
            worldFar /= worldFar.w;
        }

        return glm::normalize(worldFar - worldNear);
    }

}
