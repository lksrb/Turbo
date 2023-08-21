#include "tbopch.h"
#include "PhysicsWorld2D.h"

#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

#include "Turbo/Scene/Scene.h"
#include "Turbo/Script/Script.h"
#include "Turbo/Renderer/CommandQueue.h"

namespace Turbo {

    class ContactListener2D : public b2ContactListener
    {
    public:
        ContactListener2D()
            : m_ProcessContactsQueue(1_MB)
        {
        }

        void ProcessContacts()
        {
            m_ProcessContactsQueue.Execute();
        }

        void BeginContact(b2Contact* contact) override
        {
            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            bool isTrigger1 = fixtureA->IsSensor();
            bool isTrigger2 = fixtureB->IsSensor();

            u64 uuid1 = (u64)fixtureA->GetUserData().pointer;
            u64 uuid2 = (u64)fixtureA->GetUserData().pointer;

            // NOTE: Not sure if contact pointer will still be alive after PhysicsWorld::Step
            m_ProcessContactsQueue.Submit([uuid1, uuid2, isTrigger1, isTrigger2]()
            {
                Ref<ScriptInstance> entityScript1 = Script::FindEntityInstance(uuid1);
                Ref<ScriptInstance> entityScript2 = Script::FindEntityInstance(uuid2);

                if (entityScript1)
                {
                    if (isTrigger1)
                    {
                        entityScript1->InvokeOnTriggerBegin2D(uuid2);
                    }
                    else
                    {
                        entityScript1->InvokeOnCollisionBegin2D(uuid2);
                    }
                }

                if (entityScript2)
                {
                    if (isTrigger2)
                    {
                        entityScript2->InvokeOnTriggerBegin2D(uuid1);
                    }
                    else
                    {
                        entityScript2->InvokeOnCollisionBegin2D(uuid1);
                    }
                }

            });
            // TODO: VVVVVVVVVVVV what to do with this
#if 0
            // Update physics settings
            auto& rb2dA = entityA.GetComponent<Rigidbody2DComponent>();
            auto& rb2dB = entityB.GetComponent<Rigidbody2DComponent>();
            contact->SetEnabled(rb2dA.ContactEnabled && rb2dB.ContactEnabled);
#endif
        }

        void EndContact(b2Contact* contact) override
        {
            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            bool isTrigger1 = fixtureA->IsSensor();
            bool isTrigger2 = fixtureB->IsSensor();

            u64 uuid1 = (u64)fixtureA->GetUserData().pointer;
            u64 uuid2 = (u64)fixtureA->GetUserData().pointer;

            // NOTE: Not sure if contact pointer will still be alive after PhysicsWorld::Step
            m_ProcessContactsQueue.Submit([uuid1, uuid2, isTrigger1, isTrigger2]()
            {
                Ref<ScriptInstance> entityScript1 = Script::FindEntityInstance(uuid1);
                Ref<ScriptInstance> entityScript2 = Script::FindEntityInstance(uuid2);

                if (entityScript1)
                {
                    if (isTrigger1)
                    {
                        entityScript1->InvokeOnTriggerEnd2D(uuid2);
                    }
                    else
                    {
                        entityScript1->InvokeOnCollisionEnd2D(uuid2);
                    }
                }

                if (entityScript2)
                {
                    if (isTrigger2)
                    {
                        entityScript2->InvokeOnTriggerEnd2D(uuid1);
                    }
                    else
                    {
                        entityScript2->InvokeOnCollisionEnd2D(uuid1);
                    }
                }

            });
        }

        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
        {
        }

        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
        {
        }
    private:
        CommandQueue m_ProcessContactsQueue;
    };

    class RayCastCallback : public b2RayCastCallback
    {

    public:
        /// Called for each fixture found in the query. You control how the ray cast
        /// proceeds by returning a float:
        /// return -1: ignore this fixture and continue
        /// return 0: terminate the ray cast
        /// return fraction: clip the ray to this point
        /// return 1: don't clip the ray and continue
        /// @param fixture the fixture hit by the ray
        /// @param point the point of initial intersection
        /// @param normal the normal vector at the point of intersection
        /// @param fraction the fraction along the ray at the point of intersection
        /// @return -1 to filter, 0 to terminate, fraction to clip the ray for
        /// closest hit, 1 to continue
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
        {
            auto context = Script::GetCurrentScene();

            b2FixtureUserData data = fixture->GetUserData();
            m_HitEntity = context->FindEntityByUUID((u64)data.pointer);

            return 0; // 0: Terminate the raycast
        }
    public:
        Entity m_HitEntity;
    };

    static RayCastCallback s_RayCastCallback;
    static ContactListener2D s_ContactListener;

    PhysicsWorld2D::PhysicsWorld2D(WeakRef<Scene> scene)
        : m_Scene(scene), m_Box2DWorld(b2Vec2(0.0f, -9.8f))
    {
        m_Box2DWorld.SetContactListener(&s_ContactListener);
    }

    PhysicsWorld2D::~PhysicsWorld2D()
    {
    }

    void PhysicsWorld2D::OnRuntimeStart()
    {
        auto rigidbodies = m_Scene->GroupAllEntitiesWith<Rigidbody2DComponent, TransformComponent>();
        for (auto e : rigidbodies)
        {
            const auto& [rb2d, transform] = rigidbodies.get<Rigidbody2DComponent, TransformComponent>(e);

            // Copy position from transform component
            b2BodyDef bodyDef;
            bodyDef.type = static_cast<b2BodyType>(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            // Create body
            b2Body* body = m_Box2DWorld.CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            body->SetEnabled(rb2d.Enabled);
            body->SetGravityScale(rb2d.GravityScale);
            body->SetBullet(rb2d.IsBullet);

            rb2d.RuntimeBody = body;
        }

        // Maybe we should warn the user is his entity does not have a rigidbody2d component?
        auto boxColliders = m_Scene->GroupAllEntitiesWith<BoxCollider2DComponent, Rigidbody2DComponent, IDComponent, TransformComponent>();
        for (auto e : boxColliders)
        {
            const auto& [bc2d, rb2d, ic, transform] = boxColliders.get<BoxCollider2DComponent, Rigidbody2DComponent, IDComponent, TransformComponent>(e);

            b2Body* body = (b2Body*)rb2d.RuntimeBody;

            b2PolygonShape boxShape;
            boxShape.SetAsBox(bc2d.Size.x * glm::abs(transform.Scale.x), bc2d.Size.y * glm::abs(transform.Scale.y), b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
            fixtureDef.isSensor = bc2d.IsTrigger;
            b2FixtureUserData data;
            data.pointer = (u64)ic.ID;
            fixtureDef.userData = data;

            fixtureDef.filter.categoryBits = bc2d.CollisionCategory;
            fixtureDef.filter.maskBits = bc2d.CollisionMask;
            fixtureDef.filter.groupIndex = 0; // TODO: Think if this has any usage

            body->CreateFixture(&fixtureDef);
        }

        // Maybe we should warn the user is his entity does not have a rigidbody2d component?
        auto circlecolliders = m_Scene->GroupAllEntitiesWith<CircleCollider2DComponent, Rigidbody2DComponent, IDComponent, TransformComponent>();
        for (auto e : circlecolliders)
        {
            const auto& [cc2d, rb2d, ic, transform] = circlecolliders.get<CircleCollider2DComponent, Rigidbody2DComponent, IDComponent, TransformComponent>(e);

            b2Body* body = (b2Body*)rb2d.RuntimeBody;

            b2CircleShape circleShape;
            circleShape.m_radius = transform.Scale.x * cc2d.Radius;
            circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = cc2d.Density;
            fixtureDef.friction = cc2d.Friction;
            fixtureDef.restitution = cc2d.Restitution;
            fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
            fixtureDef.isSensor = cc2d.IsTrigger;
            b2FixtureUserData data;
            data.pointer = (u64)ic.ID;
            fixtureDef.userData = data;

            fixtureDef.filter.categoryBits = cc2d.CollisionCategory;
            fixtureDef.filter.maskBits = cc2d.CollisionMask;
            fixtureDef.filter.groupIndex = 0; // TODO: Think if this has any usage
            body->CreateFixture(&fixtureDef);
        }
    }

    void PhysicsWorld2D::OnRuntimeStop()
    {

    }

    void PhysicsWorld2D::ConstructBody(Entity entity)
    {
        auto& transform = entity.Transform();

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

        // Copy position from transform component
        b2BodyDef bodyDef;
        bodyDef.type = static_cast<b2BodyType>(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;

        // Create body
        b2Body* body = m_Box2DWorld.CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        body->SetEnabled(rb2d.Enabled);
        body->SetGravityScale(rb2d.GravityScale);
        body->SetBullet(rb2d.IsBullet);

        rb2d.RuntimeBody = body;
    }

    void PhysicsWorld2D::ConstructBoxCollider(Entity entity)
    {
        auto& transform = entity.Transform();
        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

        if (!entity.HasComponent<Rigidbody2DComponent>())
        {
            TBO_ENGINE_ERROR("Entity ({}) does not have a Rigidbody2D component!", entity.GetName());
            return;
        }

        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        b2PolygonShape boxShape;
        boxShape.SetAsBox(bc2d.Size.x * glm::abs(transform.Scale.x), bc2d.Size.y * glm::abs(transform.Scale.y), b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = bc2d.Density;
        fixtureDef.friction = bc2d.Friction;
        fixtureDef.restitution = bc2d.Restitution;
        fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
        fixtureDef.isSensor = bc2d.IsTrigger;
        b2FixtureUserData data;
        data.pointer = (u64)entity.GetUUID();
        fixtureDef.userData = data;

        fixtureDef.filter.categoryBits = bc2d.CollisionCategory;
        fixtureDef.filter.maskBits = bc2d.CollisionMask;
        fixtureDef.filter.groupIndex = 0; // TODO: Think if this has any usage

        body->CreateFixture(&fixtureDef);
    }

    void PhysicsWorld2D::ConstructCircleCollider(Entity entity)
    {
        auto& transform = entity.Transform();
        auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
        if (!entity.HasComponent<Rigidbody2DComponent>())
        {
            TBO_ENGINE_ERROR("Entity ({}) does not have a Rigidbody2D component!", entity.GetName());
            return;
        }
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        b2CircleShape circleShape;
        circleShape.m_radius = transform.Scale.x * cc2d.Radius;
        circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &circleShape;
        fixtureDef.density = cc2d.Density;
        fixtureDef.friction = cc2d.Friction;
        fixtureDef.restitution = cc2d.Restitution;
        fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
        fixtureDef.isSensor = cc2d.IsTrigger;
        b2FixtureUserData data;
        data.pointer = (u64)entity.GetUUID();
        fixtureDef.userData = data;
        //fixtureDef.filter.categoryBits = cc2d.CategoryBits; // <- Is that category
        //fixtureDef.filter.maskBits = cc2d.MaskBits;		// <- Collides with other categories
        body->CreateFixture(&fixtureDef);
    }

    void PhysicsWorld2D::DestroyPhysicsBody(Entity entity)
    {
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;
        m_Box2DWorld.DestroyBody(body);
    }

    void PhysicsWorld2D::DestroyBoxCollider(Entity entity)
    {
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        // Iterate through the fixture list and destroy everything that is polygon shaped
        b2Fixture* fixture = body->GetFixtureList();
        while (fixture != nullptr)
        {
            b2Fixture* nextFixture = fixture->GetNext();

            b2Shape* shape = fixture->GetShape();
            
            if (shape->GetType() == b2Shape::e_polygon)
                body->DestroyFixture(fixture);

            fixture = nextFixture;
        }
    }

    void PhysicsWorld2D::DestroyCircleCollilder(Entity entity)
    {
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        // Iterate through the fixture list and destroy everything that is circle shaped
        b2Fixture* fixture = body->GetFixtureList();
        while (fixture != nullptr)
        {
            b2Fixture* nextFixture = fixture->GetNext();

            b2Shape* shape = fixture->GetShape();

            if (shape->GetType() == b2Shape::e_circle)
                body->DestroyFixture(fixture);

            fixture = nextFixture;
        }
    }

    Entity PhysicsWorld2D::RayCast(glm::vec2 a, glm::vec2 b)
    {
        s_RayCastCallback.m_HitEntity = {};
        m_Box2DWorld.RayCast(&s_RayCastCallback, b2Vec2(a.x, a.y), b2Vec2(b.x, b.y));
        return s_RayCastCallback.m_HitEntity;
    }

    void PhysicsWorld2D::Simulate(FTime ts)
    {
        constexpr i32 velocityIterations = 6;
        constexpr i32 positionIterations = 2;

        // TODO: Fixed timestep
        f32 fixedDeltaTime = 1.0f / 144.0f;
        m_Box2DWorld.Step(ts, velocityIterations, positionIterations);

        // Process all contacts here to avoid tampering with rigidbodies while simulating
        s_ContactListener.ProcessContacts();

        // Retrieve transforms
        auto rigidbodies = m_Scene->GroupAllEntitiesWith<Rigidbody2DComponent, TransformComponent>();
        for (auto e : rigidbodies)
        {
            const auto& [rb2d, transform] = rigidbodies.get<Rigidbody2DComponent, TransformComponent>(e);
            b2Body* body = (b2Body*)rb2d.RuntimeBody;

            b2Vec2 position = body->GetPosition();
            transform.Translation.x = position.x;
            transform.Translation.y = position.y;

            if (!rb2d.FixedRotation) 
                transform.Rotation.z = body->GetAngle();
        }
    }

    glm::vec2 PhysicsWorld2D::RetrieveLinearVelocity(Entity entity)
    {
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        b2Vec2 b2Vel = body->GetLinearVelocity();

        return { b2Vel.x, b2Vel.y };
    }

}
