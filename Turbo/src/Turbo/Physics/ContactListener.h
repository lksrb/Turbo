#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include "Turbo/Renderer/CommandQueue.h"

namespace Turbo {

    // An example contact listener
    class ContactListener : public JPH::ContactListener
    {
    public:
        ContactListener();

        void ProcessContacts();

        // See: ContactListener
        virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

        // Collision start
        virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

        // Collision happening
        virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

        // Collision end
        virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
    private:
        CommandQueue m_ProcessContactsQueue;

        std::mutex m_ContactCallbackMutex;
    };

}
