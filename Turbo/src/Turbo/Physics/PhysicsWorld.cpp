#include "tbopch.h"
#include "PhysicsWorld.h"

#include "Turbo/Scene/Entity.h"

#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

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

    // TODO: Maybe allocator should be static since the memory gets freed when PhysicsSystem is destroyed 

    PhysicsWorld::PhysicsWorld(WeakRef<Scene> scene)
        : m_Scene(scene), m_PhysicsUpdateAllocator(10 * 1024 * 1024), m_JobSystem(TBO_MAX_JOBS, TBO_MAX_BARRIERS, -1)
    {
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
        m_PhysicsSystem.Init(TBO_MAX_BODIES, TBO_MAX_MUTEXES, TBO_MAX_BODYPAIRS, TBO_MAX_CONTACT_CONSTAINSTS, m_BroadPhaseLayer, m_ObjectVsBroadPhaseLayerFilter, m_ObjectLayerPairFilter);
        m_PhysicsSystem.SetContactListener(&m_ContactListener);
        m_PhysicsSystem.SetGravity(Vec3(0.0f, -9.81f, 0.0f));
#if 0
        // A body activation listener gets notified when bodies activate and go to sleep
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        MyBodyActivationListener body_activation_listener;
        physics_system.SetBodyActivationListener(&body_activation_listener);

#endif
        BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        // Box Colliders
        {
            auto group = m_Scene->GetAllEntitiesWith<RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [rb, transform] = group.get<RigidbodyComponent, TransformComponent>(e);
                Entity entity = { e, m_Scene.Get() };
                //glm::mat4 transform = m_Scene->GetWorldSpaceTransformMatrix({ e, m_Scene });

                glm::vec3 translation = transform.Translation;
                glm::vec3 rotation = transform.Rotation;
                glm::vec3 scale = glm::abs(transform.Scale);

                // NOTE: Layer and activation might go to the rigidbody component
                u8 layer = rb.Type == RigidbodyType::Static ? 0 : 1;
                EMotionType motionType = rb.Type == RigidbodyType::Static ? EMotionType::Static : EMotionType::Dynamic;
                EActivation activate = rb.Type == RigidbodyType::Static ? EActivation::DontActivate : EActivation::Activate;

                // NOTE: For now we do not allow a geometry to have multiple colliders
                BodyCreationSettings bodySettings;
                if (entity.HasComponent<BoxColliderComponent>())
                {
                    auto& bc = entity.GetComponent<BoxColliderComponent>();

                    Vec3 boxColliderSize = JPH::Vec3(
                        bc.Offset.x + scale.x * bc.Size.x,
                        bc.Offset.y + scale.y * bc.Size.y,
                        bc.Offset.z + scale.z * bc.Size.z);

                    // Settings for the shape
                    JPH::BoxShapeSettings boxShapeSettings(boxColliderSize);

                    // Creates and validates the shape
                    ShapeRefC boxShape = boxShapeSettings.Create().Get();

                    bodySettings = BodyCreationSettings(boxShape,
                        RVec3(translation.x, translation.y, translation.z),
                        Quat::sEulerAngles(Vec3(rotation.x, rotation.y, rotation.z)), motionType, layer);
                }
                else if (entity.HasComponent<SphereColliderComponent>())
                {
                    auto& sc = entity.GetComponent<SphereColliderComponent>();

                    // TODO: Figure out how to set offset in local space
                    JPH::SphereShapeSettings sphereShapeSettings(transform.Scale.x * sc.Radius);

                    // Creates and validates the shape
                    ShapeRefC sphereShape = sphereShapeSettings.Create().Get();

                    bodySettings = BodyCreationSettings(sphereShape,
                        RVec3(translation.x, translation.y, translation.z),
                        Quat::sEulerAngles(Vec3(rotation.x, rotation.y, rotation.z)), motionType, layer);
                }
                else if (entity.HasComponent<CapsuleColliderComponent>())
                {
                    auto& cc = entity.GetComponent<CapsuleColliderComponent>();

                    JPH::CapsuleShapeSettings capsuleShapeSettings(cc.Height * 0.5f, cc.Radius);
                    ShapeRefC capsuleShape = capsuleShapeSettings.Create().Get();

                    bodySettings = BodyCreationSettings(capsuleShape,
                       RVec3(translation.x, translation.y, translation.z),
                       Quat::sEulerAngles(Vec3(rotation.x, rotation.y, rotation.z)), motionType, layer);
                }
                else
                {
                    TBO_ENGINE_ASSERT(false);
                }

                // Locking movement
                JPH::EAllowedDOFs allowedMovement = JPH::EAllowedDOFs::All;
                if (rb.LockTranslationX) allowedMovement &= ~JPH::EAllowedDOFs::TranslationX;
                if (rb.LockTranslationY) allowedMovement &= ~JPH::EAllowedDOFs::TranslationY;
                if (rb.LockTranslationZ) allowedMovement &= ~JPH::EAllowedDOFs::TranslationZ;
                if (rb.LockRotationX)    allowedMovement &= ~JPH::EAllowedDOFs::RotationX;
                if (rb.LockRotationY)    allowedMovement &= ~JPH::EAllowedDOFs::RotationY;
                if (rb.LockRotationZ)    allowedMovement &= ~JPH::EAllowedDOFs::RotationZ;

                // Override mass calculation since the default mass is too much
                bodySettings.mAllowedDOFs = allowedMovement;
                bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                bodySettings.mMassPropertiesOverride.mMass = rb.Mass;

                // Create body and add it the world
                rb.RuntimeBodyHandle = bodyInterface.CreateAndAddBody(bodySettings, activate).GetIndexAndSequenceNumber();
            }
        }

        // Optimize first physics update 
        m_PhysicsSystem.OptimizeBroadPhase();
    }

    void PhysicsWorld::OnRuntimeStop()
    {
    }

    void PhysicsWorld::Simulate(FTime ts)
    {
        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
        // Do 1 collision step per 1 / 60th of a second (round up).
        constexpr i32 collisionSteps = 1;

        f32 fixedDeltaTime = 1.0f / 60.0f;

        // TODO: Maybe use fixed time step to ensure that physics calculation is stable
        m_PhysicsSystem.Update(fixedDeltaTime, collisionSteps, &m_PhysicsUpdateAllocator, &m_JobSystem);

        // Retrieve transforms

        // We can use the unsafe variant of body interface since all the simulation is already done
        auto& bodyInterface = GetBodyInterfaceUnsafe();

        // Ensuring that BodyID is present by grouping box colliders
        // if BodyID is not present, bodyInterface will return zero vector which is better than crashing but also not ideal
        // To avoid unnecessary branching, we group components together, EnTT will ensure that only components with colliders are updated
        // This whole mess is because Jolt doesnt allow us to have separate rigid body and the collider

        // TODO: Possible optimalization - static actors doesn't need transform synchronization

        // Box colliders retrieve transform
        {
            auto group = m_Scene->GroupAllEntitiesWith<BoxColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);
                auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.RuntimeBodyHandle));
                auto rotation = bodyInterface.GetRotation(JPH::BodyID(rb.RuntimeBodyHandle)).GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

        // Sphere colliders retrieve transform
        {
            auto group = m_Scene->GroupAllEntitiesWith<SphereColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);
                auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.RuntimeBodyHandle));
                auto rotation = bodyInterface.GetRotation(JPH::BodyID(rb.RuntimeBodyHandle)).GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

        // Capsule colliders retrieve transform
        {
            auto group = m_Scene->GroupAllEntitiesWith<CapsuleColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);
                auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.RuntimeBodyHandle));
                auto rotation = bodyInterface.GetRotation(JPH::BodyID(rb.RuntimeBodyHandle)).GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }
    }

}
