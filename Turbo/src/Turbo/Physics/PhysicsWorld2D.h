#pragma once

#include "Turbo/Core/Time.h"
#include "Turbo/Scene/Entity.h"

#include <box2d/b2_world.h>

namespace Turbo {

    class Scene;

    class PhysicsWorld2D : public RefCounted
    {
    public:
        PhysicsWorld2D(WeakRef<Scene> scene);
        ~PhysicsWorld2D();

        void OnRuntimeStart();
        void OnRuntimeStop();

        // [Runtime]
        void CreateRigidbody(Entity entity);
        void ConstructBoxCollider(Entity entity);
        void ConstructCircleCollider(Entity entity);
        void DestroyRigidbody(Entity entity);
        void DestroyBoxCollider(Entity entity);
        void DestroyCircleCollilder(Entity entity);

        Entity RayCast(glm::vec2 a, glm::vec2 b);

        void Simulate(FTime ts);

        glm::vec2 RetrieveLinearVelocity(Entity entity);
    private:
        b2World m_Box2DWorld;

        WeakRef<Scene> m_Scene;
    };
}
