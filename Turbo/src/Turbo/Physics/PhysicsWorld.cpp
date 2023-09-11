#include "tbopch.h"
#include "PhysicsWorld.h"

#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Scene/Entity.h"

#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

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

    namespace Utils {

        static void PopulateRigidbodySettings(JPH::BodyCreationSettings& bodySettings, const TransformComponent& transform, const RigidbodyComponent& rb, const IDComponent& ic)
        {
            glm::vec3 translation = transform.Translation;
            glm::vec3 rotation = transform.Rotation;

            // Basic info
            bodySettings.mPosition = JPH::RVec3(translation.x, translation.y, translation.z);
            bodySettings.mRotation = JPH::Quat::sEulerAngles(JPH::Vec3(rotation.x, rotation.y, rotation.z));

            // Lock movement
            JPH::EAllowedDOFs allowedMovement = JPH::EAllowedDOFs::All;
            if (rb.LockTranslationX) allowedMovement &= ~JPH::EAllowedDOFs::TranslationX;
            if (rb.LockTranslationY) allowedMovement &= ~JPH::EAllowedDOFs::TranslationY;
            if (rb.LockTranslationZ) allowedMovement &= ~JPH::EAllowedDOFs::TranslationZ;
            if (rb.LockRotationX)    allowedMovement &= ~JPH::EAllowedDOFs::RotationX;
            if (rb.LockRotationY)    allowedMovement &= ~JPH::EAllowedDOFs::RotationY;
            if (rb.LockRotationZ)    allowedMovement &= ~JPH::EAllowedDOFs::RotationZ;

            bodySettings.mAllowedDOFs = allowedMovement;

            // NOTE: Layer and activation might go to the rigidbody component
            bodySettings.mMotionType = static_cast<JPH::EMotionType>(rb.Type);
            bodySettings.mObjectLayer = rb.Type == RigidbodyType::Static ? Layers::STATIC : Layers::DYNAMIC;

            // Override mass calculation because the default mass is too big
            bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            bodySettings.mMassPropertiesOverride.mMass = rb.Mass;

            bodySettings.mGravityFactor = rb.GravityScale;
            bodySettings.mIsSensor = rb.IsTrigger;
            bodySettings.mFriction = rb.Friction;
            bodySettings.mRestitution = rb.Restitution;
            bodySettings.mLinearDamping = rb.LinearDamping;
            bodySettings.mAngularDamping = rb.AngularDamping;
            bodySettings.mMotionQuality = static_cast<JPH::EMotionQuality>(rb.CollisionDetection);
            bodySettings.mUserData = ic.ID;
        }

    }

    template <class CollectorType>
    class FurthestHitCollisionCollector : public CollectorType
    {
    public:
        /// Redeclare ResultType
        using ResultType = typename CollectorType::ResultType;

        virtual void Reset() override
        {
            CollectorType::Reset();
            m_HadHit = false;
        }

        virtual void AddHit(const ResultType& result) override
        {
            float earlyOut = result.GetEarlyOutFraction();
            if (!m_HadHit || earlyOut > m_Hit.GetEarlyOutFraction())
            {
                // Update early out fraction
                CollectorType::UpdateEarlyOutFraction(earlyOut);

                // Store hit
                m_Hit = result;
                m_HadHit = true;
            }
        }

        /// Check if this collector has had a hit
        inline bool	HadHit() const { return m_HadHit; }

        ResultType m_Hit;
    private:
        bool m_HadHit = false;
    };


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
        TBO_PROFILE_FUNC();

        // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        m_PhysicsSystem.Init(TBO_MAX_BODIES, TBO_MAX_MUTEXES, TBO_MAX_BODYPAIRS, TBO_MAX_CONTACT_CONSTAINSTS, m_BroadPhaseLayer, m_ObjectVsBroadPhaseLayerFilter, m_ObjectLayerPairFilter);
        m_PhysicsSystem.SetContactListener(&m_ContactListener);
        m_PhysicsSystem.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
#if 0
        // A body activation listener gets notified when bodies activate and go to sleep
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        MyBodyActivationListener body_activation_listener;
        physics_system.SetBodyActivationListener(&body_activation_listener);

#endif
        auto& bodyInterface = GetBodyInterfaceUnsafe();

        {
            auto group = m_Scene->GroupAllEntitiesWith<RigidbodyComponent, IDComponent>();
            for (auto e : group)
            {
                const auto& [rb, ic] = group.get<RigidbodyComponent, IDComponent>(e);
                Entity entity = { e, m_Scene.Get() };

                auto transform = m_Scene->GetWorldSpaceTransform(entity);

                // NOTE: For now we do not allow a geometry to have multiple colliders
                JPH::BodyCreationSettings bodySettings;
                Utils::PopulateRigidbodySettings(bodySettings, transform, rb, ic);

                // Create shape
                {
                    JPH::ShapeRefC shapeRef = nullptr;
                    if (entity.HasComponent<BoxColliderComponent>())
                    {
                        glm::vec3 scale = glm::abs(transform.Scale);

                        auto& bc = entity.GetComponent<BoxColliderComponent>();

                        JPH::Vec3 boxColliderSize = JPH::Vec3(
                            bc.Offset.x + scale.x * bc.Size.x,
                            bc.Offset.y + scale.y * bc.Size.y,
                            bc.Offset.z + scale.z * bc.Size.z);

                        // Settings for the shape
                        JPH::BoxShapeSettings boxShapeSettings(boxColliderSize);
                        shapeRef = boxShapeSettings.Create().Get();
                    }
                    else if (entity.HasComponent<SphereColliderComponent>())
                    {
                        auto& sc = entity.GetComponent<SphereColliderComponent>();

                        // TODO: Figure out how to set offset in local space
                        JPH::SphereShapeSettings sphereShapeSettings(transform.Scale.x * sc.Radius);
                        shapeRef = sphereShapeSettings.Create().Get();
                    }
                    else if (entity.HasComponent<CapsuleColliderComponent>())
                    {
                        glm::vec3 scale = glm::abs(transform.Scale);
                        auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                        JPH::CapsuleShapeSettings capsuleShapeSettings(cc.Height * 0.5f, scale.x * cc.Radius);
                        shapeRef = capsuleShapeSettings.Create().Get();
                    }
                    else
                    {
                        TBO_ENGINE_WARN("Entity '{}' does not have a collider attached to it! Skipping...");
                        continue;
                    }

                    bodySettings.SetShape(shapeRef);
                }

                // Create body and add it the world
                rb.RuntimeBodyHandle = bodyInterface.CreateAndAddBody(bodySettings, rb.Type == RigidbodyType::Dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate).GetIndexAndSequenceNumber();
            }
        }

        // Optimize first physics update 
        m_OptimizeBoardPhase = true;
    }

    void PhysicsWorld::OnRuntimeStop()
    {
    }

    void PhysicsWorld::CreateRigidbody(Entity entity)
    {
        auto [rb, ic] = entity.GetComponent<RigidbodyComponent, IDComponent>();
        auto transform = m_Scene->GetWorldSpaceTransform(entity);

        auto& bodyInterface = GetBodyInterfaceUnsafe();

        // NOTE: For now we do not allow a geometry to have multiple colliders
        JPH::BodyCreationSettings bodySettings;
        Utils::PopulateRigidbodySettings(bodySettings, transform, rb, ic);

        // Create shape
        {
            JPH::ShapeRefC shapeRef = nullptr;
            if (entity.HasComponent<BoxColliderComponent>())
            {
                glm::vec3 scale = glm::abs(transform.Scale);

                auto& bc = entity.GetComponent<BoxColliderComponent>();

                JPH::Vec3 boxColliderSize = JPH::Vec3(
                    bc.Offset.x + scale.x * bc.Size.x,
                    bc.Offset.y + scale.y * bc.Size.y,
                    bc.Offset.z + scale.z * bc.Size.z);

                // Settings for the shape
                JPH::BoxShapeSettings boxShapeSettings(boxColliderSize);
                shapeRef = boxShapeSettings.Create().Get();
            }
            else if (entity.HasComponent<SphereColliderComponent>())
            {
                auto& sc = entity.GetComponent<SphereColliderComponent>();

                // TODO: Figure out how to set offset in local space
                JPH::SphereShapeSettings sphereShapeSettings(transform.Scale.x * sc.Radius);
                shapeRef = sphereShapeSettings.Create().Get();
            }
            else if (entity.HasComponent<CapsuleColliderComponent>())
            {
                glm::vec3 scale = glm::abs(transform.Scale);
                auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                JPH::CapsuleShapeSettings capsuleShapeSettings(cc.Height * 0.5f, scale.x * cc.Radius);
                shapeRef = capsuleShapeSettings.Create().Get();
            }
            else
            {
                TBO_ENGINE_ERROR("Rigidbody does not have any colliders! Skipping...");
                return;
            }

            bodySettings.SetShape(shapeRef);
        }

        // Create body and add it the world
        rb.RuntimeBodyHandle = bodyInterface.CreateAndAddBody(bodySettings, rb.Type == RigidbodyType::Dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate).GetIndexAndSequenceNumber();

        // This will ensure that PhysicsSystem::OptimizeBoardPhase() is called
        // This may a performance killer so beware
        m_OptimizeBoardPhase = true;
    }

    void PhysicsWorld::DestroyRigidbody(Entity entity)
    {
        auto& bodyInterface = GetBodyInterfaceUnsafe();

        auto& rb = entity.GetComponent<RigidbodyComponent>();
        auto bodyId = JPH::BodyID(rb.RuntimeBodyHandle);
        bodyInterface.RemoveBody(bodyId);
        bodyInterface.DestroyBody(bodyId);
    }

    CastRayResult PhysicsWorld::CastRay(const Ray& ray, RayTarget target)
    {
        JPH::RayCastSettings settings;
        settings.mBackFaceMode = JPH::EBackFaceMode::IgnoreBackFaces;
        settings.mTreatConvexAsSolid = true;

        JPH::RRayCast rayCast;
        rayCast.mOrigin = { ray.Start.x, ray.Start.y, ray.Start.z };
        rayCast.mDirection = { ray.Direction.x, ray.Direction.y, ray.Direction.z };

        // Cast ray and return the closest entity
        if (target == RayTarget::Nearest)
        {
            JPH::ClosestHitCollisionCollector<JPH::CastRayCollector> collector;
            // TODO: We should take advantage of Jolt's layer system
            //m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::STATIC), JPH::SpecifiedObjectLayerFilter(Layers::STATIC));
            m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector);

            if (collector.HadHit())
            {
                JPH::RayCastResult hit = collector.mHit;
                auto& bodyInterface = GetBodyInterfaceUnsafe();

                JPH::Vec3 hitPosition = rayCast.GetPointOnRay(hit.mFraction);
                return { glm::vec3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()), bodyInterface.GetUserData(hit.mBodyID) };
            }
        }
        else if (target == RayTarget::Furthest)
        {
            FurthestHitCollisionCollector<JPH::CastRayCollector> collector;
            // TODO: We should take advantage of Jolt's layer system
            //m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::STATIC), JPH::SpecifiedObjectLayerFilter(Layers::STATIC));
            m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector);

            if (collector.HadHit())
            {
                JPH::RayCastResult hit = collector.m_Hit;
                auto& bodyInterface = GetBodyInterfaceUnsafe();

                JPH::Vec3 hitPosition = rayCast.GetPointOnRay(hit.mFraction);
                return { glm::vec3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()), bodyInterface.GetUserData(hit.mBodyID) };
            }

#if 0
            JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
            std::vector<JPH::RayCastResult> result(collector.mHits.size());
            m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector);
            collector.Sort();
#endif
        }
        else if (target == RayTarget::Any)
        {
            JPH::AnyHitCollisionCollector<JPH::CastRayCollector> collector;
            // TODO: We should take advantage of Jolt's layer system
            //m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::STATIC), JPH::SpecifiedObjectLayerFilter(Layers::STATIC));
            m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector);

            if (collector.HadHit())
            {
                JPH::RayCastResult hit = collector.mHit;
                auto& bodyInterface = GetBodyInterfaceUnsafe();

                JPH::Vec3 hitPosition = rayCast.GetPointOnRay(hit.mFraction);
                return { glm::vec3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()), bodyInterface.GetUserData(hit.mBodyID) };
            }
        }

        return {};
    }

    std::vector<CastRayResult> PhysicsWorld::CastRay(const Ray& ray)
    {
        std::vector<CastRayResult> results;

        JPH::RayCastSettings settings;
        settings.mBackFaceMode = JPH::EBackFaceMode::IgnoreBackFaces;
        settings.mTreatConvexAsSolid = true;

        JPH::RRayCast rayCast;
        rayCast.mOrigin = { ray.Start.x, ray.Start.y, ray.Start.z };
        rayCast.mDirection = { ray.Direction.x, ray.Direction.y, ray.Direction.z };

        JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
        // TODO: We should take advantage of Jolt's layer system
        //m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector, JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::STATIC), JPH::SpecifiedObjectLayerFilter(Layers::STATIC));
        m_PhysicsSystem.GetNarrowPhaseQueryNoLock().CastRay(rayCast, settings, collector);

        if (collector.HadHit())
        {
            results.resize(collector.mHits.size());

            auto& bodyInterface = GetBodyInterfaceUnsafe();

            for (u64 i = 0; i < results.size(); i++)
            {
                auto& hit = collector.mHits[i];

                JPH::Vec3 hitPosition = rayCast.GetPointOnRay(hit.mFraction);
                results[i] = { glm::vec3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()), bodyInterface.GetUserData(hit.mBodyID) };
            }
        }

        return results;
    }

    void PhysicsWorld::Simulate(FTime ts)
    {
        TBO_PROFILE_FUNC();

        if (m_OptimizeBoardPhase)
        {
            m_OptimizeBoardPhase = false;

            //ScopeTimer timer("OptimizeBroadPhase");
            m_PhysicsSystem.OptimizeBroadPhase();
        }

        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
        // Do 1 collision step per 1 / 60th of a second (round up).

        constexpr f32 fixedDeltaTime = 1.0f / 60.0f;
        i32 collisionSteps = (i32)glm::max(1.0f, glm::round(ts / fixedDeltaTime));

        // TODO: Maybe use fixed time step to ensure that physics calculation is stable
        m_PhysicsSystem.Update(ts, collisionSteps, &m_PhysicsUpdateAllocator, &m_JobSystem);

        // Executes contact callbacks
        // This is here because mono doesn't like being called from another thread
        // There are couple of solution in mono but this seemed like the most straight-forward and simple
        m_ContactListener.ProcessContacts();

        // --- Retrieve transforms ---

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
                const auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);

                JPH::Vec3 pos;
                JPH::Quat quatRotation;
                bodyInterface.GetPositionAndRotation(JPH::BodyID(rb.RuntimeBodyHandle), pos, quatRotation);
                JPH::Vec3 rotation = quatRotation.GetEulerAngles();

                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

        // Sphere colliders retrieve transform
        {
            auto group = m_Scene->GroupAllEntitiesWith<SphereColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                const auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);

                JPH::Vec3 pos;
                JPH::Quat quatRotation;
                bodyInterface.GetPositionAndRotation(JPH::BodyID(rb.RuntimeBodyHandle), pos, quatRotation);

                JPH::Vec3 rotation = quatRotation.GetEulerAngles();
                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }

        // Capsule colliders retrieve transform
        {
            auto group = m_Scene->GroupAllEntitiesWith<CapsuleColliderComponent, RigidbodyComponent, TransformComponent>();
            for (auto e : group)
            {
                const auto& [transform, rb] = group.get<TransformComponent, RigidbodyComponent>(e);

                JPH::Vec3 pos;
                JPH::Quat quatRotation;
                bodyInterface.GetPositionAndRotation(JPH::BodyID(rb.RuntimeBodyHandle), pos, quatRotation);

                JPH::Vec3 rotation = quatRotation.GetEulerAngles();
                transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
                transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
            }
        }
    }
}
