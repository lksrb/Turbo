#pragma once

#include "Turbo/Scene/Components.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>

namespace Turbo
{
    namespace Utils
    {
        inline b2BodyType Rigidbody2DTypeToBox2DBody(RigidbodyType bodyType)
        {
            switch (bodyType)
            {
                case RigidbodyType::Static:    return b2_staticBody;
                case RigidbodyType::Dynamic:   return b2_dynamicBody;
                case RigidbodyType::Kinematic: return b2_kinematicBody;
            }

            TBO_ENGINE_ASSERT(false, "Unknown body type");
            return b2_staticBody;
        }

        inline RigidbodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
        {
            switch (bodyType)
            {
                case b2_staticBody:    return RigidbodyType::Static;
                case b2_dynamicBody:   return RigidbodyType::Dynamic;
                case b2_kinematicBody: return RigidbodyType::Kinematic;
            }

            TBO_ENGINE_ASSERT(false, "Unknown body type");
            return RigidbodyType::Static;
        }
    }
}
