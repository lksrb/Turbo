#pragma once

#include "RayCast.h"
#include "ContactListener.h"

#include "Turbo/Scene/Scene.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace Turbo {

    namespace Layers {
        inline constexpr JPH::ObjectLayer STATIC = 0;
        inline constexpr JPH::ObjectLayer DYNAMIC = 1;

        inline constexpr u32 COUNT = 2;
    }

    namespace BroadPhaseLayers {
        static inline constexpr JPH::BroadPhaseLayer STATIC(Layers::STATIC);
        static inline constexpr JPH::BroadPhaseLayer DYNAMIC(Layers::DYNAMIC);
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

    class PhysicsWorld : public RefCounted
    {
    public:
        PhysicsWorld(WeakRef<Scene> scene);
        ~PhysicsWorld();

        void OnRuntimeStart();
        void OnRuntimeStop();

        void CreateRigidbody(Entity entity);
        void DestroyRigidbody(Entity entity);

        JPH::BodyInterface& GetBodyInterface() { return m_PhysicsSystem.GetBodyInterface(); }
        JPH::BodyInterface& GetBodyInterfaceUnsafe() { return m_PhysicsSystem.GetBodyInterfaceNoLock(); }

        const JPH::Body* TryGetBodyUnsafe(JPH::BodyID bodyId) const { return m_PhysicsSystem.GetBodyLockInterfaceNoLock().TryGetBody(bodyId); };

        CastRayResult CastRay(const Ray& ray, RayTarget target);
        std::vector<CastRayResult> CastRay(const Ray& ray);

        void Simulate(FTime ts);
    private:
        bool m_OptimizeBoardPhase = false;

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
        ContactListener m_ContactListener;

        // Physics system
        JPH::PhysicsSystem m_PhysicsSystem;

        // This cannot be a strong reference because Scene wont get deleted 
        WeakRef<Scene> m_Scene;
    };

}
