#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Scene/Components.h"

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

namespace Turbo
{
    namespace Utils
    {
        inline b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
        {
            switch (bodyType)
            {
                case Rigidbody2DComponent::BodyType_Static:    return b2_staticBody;
                case Rigidbody2DComponent::BodyType_Dynamic:   return b2_dynamicBody;
                case Rigidbody2DComponent::BodyType_Kinematic: return b2_kinematicBody;
            }

            TBO_ENGINE_ASSERT(false, "Unknown body type");
            return b2_staticBody;
        }

        inline Rigidbody2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
        {
            switch (bodyType)
            {
                case b2_staticBody:    return Rigidbody2DComponent::BodyType_Static;
                case b2_dynamicBody:   return Rigidbody2DComponent::BodyType_Dynamic;
                case b2_kinematicBody: return Rigidbody2DComponent::BodyType_Kinematic;
            }

            TBO_ENGINE_ASSERT(false, "Unknown body type");
            return Rigidbody2DComponent::BodyType_Static;
        }
    }
}
