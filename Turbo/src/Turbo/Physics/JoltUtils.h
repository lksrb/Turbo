#pragma once

#include <Jolt/Jolt.h>
#include <glm/glm.hpp>

namespace Turbo::JoltUtils {

    inline JPH::Vec3 GetVec3(const glm::vec3& v) { return  JPH::Vec3(v.x, v.y, v.z); }
    inline glm::vec3 GetVec3(JPH::Vec3Arg v) { return { v.GetX(), v.GetY(), v.GetZ() }; }
    inline JPH::Quat GetQuat(const glm::quat& q) { return JPH::Quat(q.x, q.y, q.z, q.w); }
    inline glm::quat GetQuat(const JPH::Quat& q) { return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ()); }

}
