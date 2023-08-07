#include "tbopch.h"
#include "PhysicsWorld2D.h"

#include "Physics2D.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Script/Script.h"

namespace Turbo
{
    class ContactListener2D : public b2ContactListener
    {
    private:
        void BeginContact(b2Contact* contact) override
        {
            auto context = Script::GetCurrentScene();

            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            Entity entityA = context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
            Entity entityB = context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

            if (entityA && entityB)
            {
                if (entityA.HasComponent<ScriptComponent>())
                    Script::InvokeEntityOnBeginCollision2D(entityA, entityB, fixtureA->IsSensor());

                if (entityB.HasComponent<ScriptComponent>())
                    Script::InvokeEntityOnBeginCollision2D(entityB, entityA, fixtureB->IsSensor());
            }
#if 0
            // Update physics settings
            auto& rb2dA = entityA.GetComponent<Rigidbody2DComponent>();
            auto& rb2dB = entityB.GetComponent<Rigidbody2DComponent>();
            contact->SetEnabled(rb2dA.ContactEnabled && rb2dB.ContactEnabled);
#endif
        }

        void EndContact(b2Contact* contact) override
        {
            auto context = Script::GetCurrentScene();

            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();

            Entity entityA = context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
            Entity entityB = context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

            if (entityA && entityB)
            {
                if (entityA.HasComponent<ScriptComponent>())
                    Script::InvokeEntityOnEndCollision2D(entityA, entityB, fixtureA->IsSensor());

                if (entityB.HasComponent<ScriptComponent>())
                    Script::InvokeEntityOnEndCollision2D(entityB, entityA, fixtureB->IsSensor());
            }
        }

        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
        {
        }

        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override
        {
        }
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

    PhysicsWorld2D::PhysicsWorld2D(glm::vec2 gravity)
    {
        m_Box2DWorld = new b2World(b2Vec2(gravity.x, gravity.y));
        m_Box2DWorld->SetContactListener(&s_ContactListener);
    }

    PhysicsWorld2D::~PhysicsWorld2D()
    {
        delete m_Box2DWorld;
        m_Box2DWorld = nullptr;
    }

    void PhysicsWorld2D::ConstructBody(Entity entity)
    {
        auto& transform = entity.Transform();

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

        // Copy position from transform component
        b2BodyDef bodyDef;
        bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;

        // Create body
        b2Body* body = m_Box2DWorld->CreateBody(&bodyDef);
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
        TBO_ENGINE_ASSERT(entity.HasComponent<Rigidbody2DComponent>());
        b2Body* body = (b2Body*)entity.GetComponent<Rigidbody2DComponent>().RuntimeBody;

        b2PolygonShape boxShape;
        boxShape.SetAsBox(bc2d.Size.x * glm::abs(transform.Scale.x), bc2d.Size.y * glm::abs(transform.Scale.y), b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = bc2d.Density;
        fixtureDef.friction = bc2d.Friction;
        fixtureDef.restitution = bc2d.Restitution;
        fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
        fixtureDef.isSensor = bc2d.IsSensor;
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
        TBO_ENGINE_ASSERT(entity.HasComponent<Rigidbody2DComponent>());
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
        fixtureDef.isSensor = cc2d.IsSensor;
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
        m_Box2DWorld->DestroyBody(body);
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
        m_Box2DWorld->RayCast(&s_RayCastCallback, b2Vec2(a.x, a.y), b2Vec2(b.x, b.y));
        return s_RayCastCallback.m_HitEntity;
    }

    void PhysicsWorld2D::Step(FTime ts)
    {
        constexpr i32 velocityIterations = 6;
        constexpr i32 positionIterations = 2;

        m_Box2DWorld->Step(ts, velocityIterations, positionIterations);
    }

    void PhysicsWorld2D::RetrieveTransform(Entity entity)
    {
        auto& transform = entity.Transform();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;

        const auto& position = body->GetPosition();
        transform.Translation.x = position.x;
        transform.Translation.y = position.y;

        if (rb2d.FixedRotation == false)
            transform.Rotation.z = body->GetAngle();

        // Copy settings from the component, 
        // NOTE: Settings are from last frame, not sure if its good or bad
        body->SetEnabled(rb2d.Enabled);
        body->SetType(Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type));
        body->SetGravityScale(rb2d.GravityScale);
    }

}
