#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Scene/Entity.h"

class b2World;

namespace Turbo
{
    class PhysicsWorld2D
    {
    public:
        PhysicsWorld2D(glm::vec2 gravity);
        ~PhysicsWorld2D();

        void ConstructBody(Entity entity);
        void ConstructBoxCollider(Entity entity);
        void ConstructCircleCollider(Entity entity);

        void DestroyPhysicsBody(Entity entity);
        void DestroyBoxCollider(Entity entity);
        void DestroyCircleCollilder(Entity entity);

        Entity RayCast(glm::vec2 a, glm::vec2 b);

        void Simulate(FTime ts);

        void RetrieveTransform(Entity entity);
        glm::vec2 RetrieveLinearVelocity(Entity entity);
    private:
        b2World* m_Box2DWorld = nullptr;
    };
}
