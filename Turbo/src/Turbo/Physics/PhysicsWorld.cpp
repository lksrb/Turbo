#include "tbopch.h"
#include "PhysicsWorld.h"

// Job system config
#define TBO_MAX_JOBS 2048
#define TBO_MAX_BARRIERS 8

// Maximum ammount of bodies in the simulation (65536 is ideal)
#define TBO_MAX_BODIES 1024

// How many mutexes to allocate to protect rigid bodies from concurrent access
#define TBO_MAX_MUTEXES 0 // 0 - default

// Maximum amount of bodypairs (idk what that is, 65536 is ideal)
#define TBO_MAX_BODYPAIRS 1024

// Maximum amount of the contact buffer (10240 is ideal)
#define TBO_MAX_CONTACT_CONSTAINSTS 2024

namespace Turbo {

    PhysicsWorld::PhysicsWorld(WeakRef<Scene> scene)
        : m_Scene(scene), m_PhysicsUpdateAllocator(10 * 1024 * 1024), m_JobSystem(TBO_MAX_JOBS, TBO_MAX_BARRIERS, -1)
    {
        // Trace bodies
        //JPH::Trace = TraceImpl
    }

    PhysicsWorld::~PhysicsWorld()
    {
    }

    void PhysicsWorld::OnRuntimeStart()
    {
        using namespace JPH;

        // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        m_PhysicsSystem = CreateOwned<PhysicsSystem>();
        m_PhysicsSystem->Init(TBO_MAX_BODIES, TBO_MAX_MUTEXES, TBO_MAX_BODYPAIRS, TBO_MAX_CONTACT_CONSTAINSTS, m_BroadPhaseLayer, m_ObjectVsBroadPhaseLayerFilter, m_ObjectLayerPairFilter);
        m_PhysicsSystem->SetContactListener(&m_ContactListener);
        m_PhysicsSystem->SetGravity(Vec3(0.0f, -9.81f, 0.0f));
#if 0
        // A body activation listener gets notified when bodies activate and go to sleep
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        MyBodyActivationListener body_activation_listener;
        physics_system.SetBodyActivationListener(&body_activation_listener);

#endif
        BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();

        // Box Colliders
        {
            auto group = m_Scene->GroupAllEntitiesWith<BoxColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [bc, rb, transform] = group.get<BoxColliderComponent, RigidbodyComponent, TransformComponent>(e);
                glm::vec3 translation = transform.Translation;
                glm::vec3 rotation = transform.Rotation;
                glm::vec3 scale = glm::abs(transform.Scale);

                Vec3 boxColliderSize = JPH::Vec3(
                    bc.Offset.x + scale.x * bc.Size.x,
                    bc.Offset.y + scale.y * bc.Size.y,
                    bc.Offset.z + scale.z * bc.Size.z);

                // NOTE: Layer and activation might go to the rigidbody component
                u8 layer = rb.Type == RigidbodyComponent::BodyType_Static ? 0 : 1;
                EMotionType motionType = rb.Type == RigidbodyComponent::BodyType_Static ? EMotionType::Static : EMotionType::Dynamic;
                EActivation activate = rb.Type == RigidbodyComponent::BodyType_Static ? EActivation::DontActivate : EActivation::Activate;

                // Settings for the shape
                JPH::BoxShapeSettings boxShapeSettings(boxColliderSize);

                // Creates and validates the shape
                ShapeRefC boxShape = boxShapeSettings.Create().Get();

                BodyCreationSettings boxSettings(boxShape,
                    RVec3(translation.x, translation.y, translation.z), Quat::sEulerAngles(Vec3(rotation.x, rotation.y, rotation.z)), motionType, layer);

                // Create body and add it the world
                rb.BodyHandle = bodyInterface.CreateAndAddBody(boxSettings, activate).GetIndexAndSequenceNumber();
            }
        }

        // Sphere Colliders
        {
            auto group = m_Scene->GroupAllEntitiesWith<SphereColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [sc, rb, transform] = group.get<SphereColliderComponent, RigidbodyComponent, TransformComponent>(e);
                glm::vec3 translation = transform.Translation + sc.Offset;
                glm::vec3 rotation = transform.Rotation;
                // NOTE: Scale does affect sphere but only the X axis

                // NOTE: Layer and activation might go to the rigidbody component
                u8 layer = rb.Type == RigidbodyComponent::BodyType_Static ? 0 : 1;
                EMotionType motionType = rb.Type == RigidbodyComponent::BodyType_Static ? EMotionType::Static : EMotionType::Dynamic;
                EActivation activate = rb.Type == RigidbodyComponent::BodyType_Static ? EActivation::DontActivate : EActivation::Activate;

                // Settings for the shape
                // TODO: Figure out how to set offset in local space
                JPH::SphereShapeSettings circleShapeSettings(transform.Scale.x * sc.Radius);

                // Creates and validates the shape
                ShapeRefC circleShape = circleShapeSettings.Create().Get();

                BodyCreationSettings circleSettings(circleShape,
                    RVec3(translation.x, translation.y, translation.z), 
                    Quat::sEulerAngles(Vec3(rotation.x, rotation.y, rotation.z)), motionType, layer);

                // Create body and add it the world
                rb.BodyHandle = bodyInterface.CreateAndAddBody(circleSettings, activate).GetIndexAndSequenceNumber();
            }
        }

        // Optimize first physics update 
        m_PhysicsSystem->OptimizeBroadPhase();
    }

    void PhysicsWorld::OnRuntimeStop()
    {
        m_PhysicsSystem.reset();
    }

    void PhysicsWorld::Simulate(FTime ts)
    {
        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
        // Do 1 collision step per 1 / 60th of a second (round up).
        constexpr i32 collisionSteps = 1;

        // TODO: Maybe use fixed time step to ensure that physics calculation is stable
        m_PhysicsSystem->Update(ts, collisionSteps, &m_PhysicsUpdateAllocator, &m_JobSystem);

        // Retrieve transforms
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();

        // Ensuring that BodyID is present by grouping box colliders
        // if BodyID is not present, bodyInterface will return zero vector which is better than crashing but also not ideal
        // To avoid unnecessary branching, we group components together, EnTT will ensure that only components with colliders are updated
        // This whole mess is because Jolt doesnt allow us to have separate rigid body and the collider

        // Box Colliders Update
        {
            auto group = m_Scene->GroupAllEntitiesWith<BoxColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);
                auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.BodyHandle));
                auto rotation = bodyInterface.GetRotation(JPH::BodyID(rb.BodyHandle)).GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

        // Sphere Colliders Update
        {
            auto group = m_Scene->GroupAllEntitiesWith<SphereColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);
                auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.BodyHandle));
                auto rotation = bodyInterface.GetRotation(JPH::BodyID(rb.BodyHandle)).GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

    }

}
