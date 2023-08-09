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

    struct JPHDefaultResources
    {
        JPHDefaultResources()
        {
            JPH::RegisterDefaultAllocator();
            FactoryInstance = JPH::Factory::sInstance = new JPH::Factory;

            JPH::RegisterTypes();
        }

        ~JPHDefaultResources()
        {
            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            FactoryInstance = JPH::Factory::sInstance = nullptr;
        }

        JPH::Factory* FactoryInstance;

    };

    static JPHDefaultResources s_Instance;

    PhysicsWorld::PhysicsWorld(Scene* scene)
        : m_Scene(scene), m_PhysicsUpdateAllocator(10 * 1024 * 1024), m_JobSystem(TBO_MAX_JOBS, TBO_MAX_BARRIERS, -1)
    {
        // Trace bodies
        //JPH::Trace = TraceImpl
    }

    PhysicsWorld::~PhysicsWorld()
    {
        m_Scene = nullptr;
    }

    void PhysicsWorld::OnRuntimeStart()
    {
        using namespace JPH;

        // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
        // Note that this is called from a job so whatever you do here needs to be thread safe.
        // Registering one is entirely optional.
        m_PhysicsSystem = new PhysicsSystem;
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

        auto view = m_Scene->GroupAllEntitiesWith<TransformComponent, RigidbodyComponent>();
        for (auto e : view)
        {
            auto& [transform, rb] = view.get<TransformComponent, RigidbodyComponent>(e);

            u8 layer = rb.Type == RigidbodyComponent::BodyType_Static ? 0 : 1;
            EMotionType motionType = rb.Type == RigidbodyComponent::BodyType_Static ? EMotionType::Static : EMotionType::Dynamic;
            EActivation activate = rb.Type == RigidbodyComponent::BodyType_Static ? EActivation::DontActivate : EActivation::Activate;

            // Settings for the shape
            JPH::BoxShapeSettings boxShapeSettings(JPH::Vec3(0.5f, 0.5f, 0.5f));

            auto shapeResult = boxShapeSettings.Create();
            ShapeRefC boxShape = shapeResult.Get();

            BodyCreationSettings boxSettings(boxShape, RVec3(transform.Translation.x, transform.Translation.y, transform.Translation.z), Quat::sIdentity(), motionType, layer);

            // Create body and add it the world
            // NOTE: This returns a handle, should we store it somewhere?
            rb.BodyHandle = bodyInterface.CreateAndAddBody(boxSettings, activate).GetIndexAndSequenceNumber();
        }

        // Optimize first physics update 
        m_PhysicsSystem->OptimizeBroadPhase();
    }

    void PhysicsWorld::OnRuntimeStop()
    {
        delete m_PhysicsSystem;
        m_PhysicsSystem = nullptr;
    }

    void PhysicsWorld::Simulate(FTime ts)
    {
        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
        // Do 1 collision step per 1 / 60th of a second (round up).
        constexpr i32 collisionSteps = 1;

        m_PhysicsSystem->Update(ts, collisionSteps, &m_PhysicsUpdateAllocator, &m_JobSystem);

        // Retrieve transforms
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();

        auto view = m_Scene->GroupAllEntitiesWith<TransformComponent, RigidbodyComponent>();
        for (auto e : view)
        {
            auto& [transform, rb] = view.get<TransformComponent, RigidbodyComponent>(e);
            auto pos = bodyInterface.GetPosition(JPH::BodyID(rb.BodyHandle));

            transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
        }

    }

}
