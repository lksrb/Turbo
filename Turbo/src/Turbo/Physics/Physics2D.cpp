#include "tbopch.h"
#include "Physics2D.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Scene.h"

#include "Turbo/Script/Script.h"

namespace Turbo
{
    PhysicsScene2D::PhysicsScene2D(Scene* context)
        : m_Context(context)
    {
        Initialize();
    }

    PhysicsScene2D::~PhysicsScene2D()
    {
        delete m_PhysicsWorld;
    }

    void PhysicsScene2D::Initialize()
    {
        m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
        m_PhysicsWorld->SetContactListener(this);

        // View all physics actors
        auto& view = m_Context->GetAllEntitiesWith<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, m_Context };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            // Copy position from transform component
            b2BodyDef bodyDef;
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            // Create body
            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            body->SetEnabled(rb2d.Enabled);
            body->SetGravityScale(rb2d.GravityScale);
            rb2d.RuntimeBody = body;
            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);
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
                //fixtureDef.filter.categoryBits = bc2d.CategoryBits; // <- Is that category
                //fixtureDef.filter.maskBits = bc2d.MaskBits;		// <- Collides with other categories
                body->CreateFixture(&fixtureDef);
            }
            else if (entity.HasComponent<CircleCollider2DComponent>())
            {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

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
        }
    }

    void PhysicsScene2D::OnUpdate(FTime ts)
    {
        constexpr i32 velocityIterations = 6;
        constexpr i32 positionIterations = 2;
        m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

        // Retrieve transform from Box2D
        auto rigidbodiesView = m_Context->GetAllEntitiesWith<Rigidbody2DComponent>();
        for (auto e : rigidbodiesView)
        {
            Entity entity = { e, m_Context };
            auto& transform = entity.Transform();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Translation.x = position.x;
            transform.Translation.y = position.y;

            if (rb2d.FixedRotation == false)
                transform.Rotation.z = body->GetAngle();
        }
    }

    void PhysicsScene2D::BeginContact(b2Contact* contact)
    {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();
        
        Entity entityA = m_Context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
        Entity entityB = m_Context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

        if (entityA && entityA.HasComponent<ScriptComponent>())
            Script::InvokeEntityOnBeginCollision2D(entityA, entityB, fixtureA->IsSensor());
        if (entityB && entityB.HasComponent<ScriptComponent>())
            Script::InvokeEntityOnBeginCollision2D(entityB, entityA, fixtureB->IsSensor());
    }

    void PhysicsScene2D::EndContact(b2Contact* contact)
    {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();

        Entity entityA = m_Context->FindEntityByUUID((u64)fixtureA->GetUserData().pointer);
        Entity entityB = m_Context->FindEntityByUUID((u64)fixtureB->GetUserData().pointer);

        if (entityA && entityA.HasComponent<ScriptComponent>())
            Script::InvokeEntityOnEndCollision2D(entityA, entityB, fixtureA->IsSensor());
        if (entityB && entityB.HasComponent<ScriptComponent>())
            Script::InvokeEntityOnEndCollision2D(entityB, entityA, fixtureB->IsSensor());
    }

    void PhysicsScene2D::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
    {
    }

    void PhysicsScene2D::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
    {
    }
}
