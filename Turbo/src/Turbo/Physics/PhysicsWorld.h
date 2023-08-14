#pragma once

#include "Turbo/Scene/Scene.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <array>

namespace Turbo {

    namespace Layers {
        inline constexpr u8 STATIC = 0;
        inline constexpr u8 DYNAMIC = 1;

        inline constexpr u8 COUNT = 2;
    }

    namespace BroadPhaseLayers {
        static inline constexpr JPH::BroadPhaseLayer STATIC(0);
        static inline constexpr JPH::BroadPhaseLayer DYNAMIC(1);
    }

    // This defines a mapping between object and broadphase layers.
    class BPLayerInterface : public JPH::BroadPhaseLayerInterface
    {

    public:
        BPLayerInterface()
        {
            m_ObjectToBroadPhase[Layers::STATIC] = BroadPhaseLayers::STATIC;
            m_ObjectToBroadPhase[Layers::DYNAMIC] = BroadPhaseLayers::DYNAMIC;
        }

        JPH::uint GetNumBroadPhaseLayers() const override { return Layers::COUNT; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { TBO_ENGINE_ASSERT(inLayer < Layers::COUNT); return m_ObjectToBroadPhase[inLayer]; }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        /// Get the user readable name of a broadphase layer (debugging purposes)
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            return "BOO";
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
    private:
        std::array<JPH::BroadPhaseLayer, Layers::COUNT> m_ObjectToBroadPhase;
    };

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
                case Layers::STATIC:
                    return inObject2 == Layers::DYNAMIC; // Non moving only collides with moving
                case Layers::DYNAMIC:
                    return true; // Moving collides with everything
            }

            TBO_ENGINE_ASSERT(false);

            return false;
        }
    };

    /// Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case Layers::STATIC:
                    return inLayer2 == BroadPhaseLayers::DYNAMIC;
                case Layers::DYNAMIC:
                    return true;
            }

            TBO_ENGINE_ASSERT(false);
            return false;
        }
    };

    // An example contact listener
    class MyContactListener : public JPH::ContactListener
    {
    public:
        // See: ContactListener
        virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
        {
            //cout << "Contact validate callback" << endl;

            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
        {
        }

        virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
        {
        }

        virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
        {
        }
    };

    class PhysicsWorld : public RefCounted
    {
    public:
        PhysicsWorld(WeakRef<Scene> scene);
        ~PhysicsWorld();

        void OnRuntimeStart();
        void OnRuntimeStop();

        JPH::BodyInterface& GetBodyInterface() { return m_PhysicsSystem.GetBodyInterface(); }
        JPH::BodyInterface& GetBodyInterfaceUnsafe() { return m_PhysicsSystem.GetBodyInterfaceNoLock(); }

        void Simulate(FTime ts);
    private:
        // For physics update
        JPH::TempAllocatorImpl m_PhysicsUpdateAllocator;
        JPH::JobSystemThreadPool m_JobSystem;

        // Mapping table from object layout to broadpahse layer
        BPLayerInterface m_BroadPhaseLayer;

        // Filters object vs broadphase layers
        ObjectVsBroadPhaseLayerFilter m_ObjectVsBroadPhaseLayerFilter;

        // Filters object vs object layers
        ObjectLayerPairFilter m_ObjectLayerPairFilter;

        // Contact listener
        MyContactListener m_ContactListener;

        // Physics system
        JPH::PhysicsSystem m_PhysicsSystem;

        // This cannot be a strong reference because Scene wont get deleted 
        WeakRef<Scene> m_Scene;
    };

}
