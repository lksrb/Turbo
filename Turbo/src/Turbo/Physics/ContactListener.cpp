#include "tbopch.h"
#include "ContactListener.h"

#include "Turbo/Script/Script.h"

#include <Jolt/Physics/Body/Body.h>
#include "PhysicsWorld.h"

namespace Turbo {

    ContactListener::ContactListener()
        : m_ProcessContactsQueue(5_MB) // NOTE: This is maybe too much
    {
    }

    void ContactListener::ProcessContacts()
    {
        m_ProcessContactsQueue.Execute();
    }

    JPH::ValidateResult ContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
    {
        //cout << "Contact validate callback" << endl;

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void ContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        std::scoped_lock<std::mutex> lock(m_ContactCallbackMutex);

        Ref<ScriptInstance> entityScript1 = Script::FindEntityInstance(inBody1.GetUserData());
        Ref<ScriptInstance> entityScript2 = Script::FindEntityInstance(inBody2.GetUserData());

        // First entity
        if (entityScript1)
        {
            bool isTrigger = inBody1.IsSensor();

            if (isTrigger)
            {
                m_ProcessContactsQueue.Submit([entityScript1, &inBody2]() mutable
                {
                    entityScript1->InvokeOnTriggerBegin(inBody2.GetUserData());
                });
            }
            else
            {
                m_ProcessContactsQueue.Submit([entityScript1, &inBody2]() mutable
                {
                    entityScript1->InvokeOnCollisionBegin(inBody2.GetUserData());
                });
            }
        }

        // Second entity
        if (entityScript2)
        {
            bool isTrigger = inBody2.IsSensor();

            if (isTrigger)
            {
                m_ProcessContactsQueue.Submit([entityScript2, &inBody1]() mutable
                {
                    entityScript2->InvokeOnTriggerBegin(inBody1.GetUserData());
                });
            }
            else
            {
                m_ProcessContactsQueue.Submit([entityScript2, &inBody1]() mutable
                {
                    entityScript2->InvokeOnCollisionBegin(inBody1.GetUserData());
                });
            }
        }
    }

    void ContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        //std::scoped_lock<std::mutex> lock(m_ContactCallbackMutex);
    }

    void ContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
    {
        std::scoped_lock<std::mutex> lock(m_ContactCallbackMutex);

        // This scope should be locked and since there will be a lot of contacts,
        // we don't want to submit everything

        Ref<PhysicsWorld> world = Script::GetCurrentScene()->GetPhysicsWorld();

        auto inBody1 = world->TryGetBodyUnsafe(inSubShapePair.GetBody1ID());
        auto inBody2 = world->TryGetBodyUnsafe(inSubShapePair.GetBody2ID());

        // This shouldn't happen
        TBO_ENGINE_ASSERT(inBody1 && inBody2, "[ContactListener::OnContactRemoved] One or both bodies do not exist anymore!");

        Ref<ScriptInstance> entityScript1 = Script::FindEntityInstance(inBody1->GetUserData());
        Ref<ScriptInstance> entityScript2 = Script::FindEntityInstance(inBody2->GetUserData());

        // First entity
        if (entityScript1)
        {
            bool isTrigger = inBody1->IsSensor();

            if (isTrigger)
            {
                m_ProcessContactsQueue.Submit([entityScript1, inBody2]() mutable
                {
                    entityScript1->InvokeOnTriggerEnd(inBody2->GetUserData());
                });
            }
            else
            {
                m_ProcessContactsQueue.Submit([entityScript1, inBody2]() mutable
                {
                    entityScript1->InvokeOnCollisionEnd(inBody2->GetUserData());
                });
            }
        }

        // Second entity
        if (entityScript2)
        {
            bool isTrigger = inBody2->IsSensor();

            if (isTrigger)
            {
                m_ProcessContactsQueue.Submit([entityScript2, inBody1]() mutable
                {
                    entityScript2->InvokeOnTriggerEnd(inBody1->GetUserData());
                });
            }
            else
            {
                m_ProcessContactsQueue.Submit([entityScript2, inBody1]() mutable
                {
                    entityScript2->InvokeOnCollisionEnd(inBody1->GetUserData());
                });
            }
        }
    }

}
