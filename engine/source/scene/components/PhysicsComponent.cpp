#include "scene/components/PhysicsComponent.h"

#include "Engine.h"
#include "scene/GameObject.h"

namespace mnd
{
PhysicsComponent::PhysicsComponent(const std::shared_ptr<RigidBody> &body)
    : m_rigidBody(body)
{
}

void PhysicsComponent::LoadProperties(const nlohmann::json &json)
{
    std::shared_ptr<Collider> collider;

    // Collider
    if (json.contains("collider"))
    {
        const auto &col  = json["collider"];
        std::string type = col.value("type", "");

        if (type == "box")
        {
            glm::vec3 extents(col.value("x", 1.0f), col.value("y", 1.0f), col.value("z", 1.0f));
            collider = std::make_shared<BoxCollider>(extents);

        } else if (type == "sphere")
        {
            float radius = col.value("r", 1.0f);
            collider     = std::make_shared<SphereCollider>(radius);

        } else if (type == "capsule")
        {
            float radius = col.value("r", 1.0f);
            float height = col.value("h", 1.0f);
            collider     = std::make_shared<CapsuleCollider>(radius, height);
        }
    }

    if (!collider)
    {
        return;
    }

    // RigidBody
    std::shared_ptr<RigidBody> rigidbody;
    if (json.contains("body"))
    {
        const auto &body = json["body"];

        float       mass     = body.value("mass", 0.0f);
        float       friction = body.value("friction", 0.5f);
        std::string typeStr  = body.value("type", "static");

        BodyType type = BodyType::Static;
        if (typeStr == "dynamic")
        {
            type = BodyType::Dynamic;
        } else if (typeStr == "kinematic")
        {
            type = BodyType::Kinematic;
        }

        rigidbody = std::make_shared<RigidBody>(type, collider, mass, friction);
    }

    if (rigidbody)
    {
        SetRigidBody(rigidbody);
    }
}

void PhysicsComponent::Init()
{
    if (!m_rigidBody)
    {
        return;
    }

    const auto pos = m_owner->GetWorldPosition();
    const auto rot = m_owner->GetWorldRotation();

    m_rigidBody->SetPosition(pos);
    m_rigidBody->SetRotation(rot);

    Engine::GetInstance().GetPhysicsManager().AddRigidBody(m_rigidBody.get());
}

void PhysicsComponent::Update(float deltaTime)
{
    if (!m_rigidBody)
    {
        return;
    }

    if (m_rigidBody->GetType() == BodyType::Dynamic)
    {
        m_owner->SetWorldPosition(m_rigidBody->GetPosition());
        m_owner->SetWorldRotation(m_rigidBody->GetRotation());
    }
}

void PhysicsComponent::SetRigidBody(const std::shared_ptr<RigidBody> &body)
{
    m_rigidBody = body;
}

const std::shared_ptr<RigidBody> &PhysicsComponent::GetRigidBody()
{
    return m_rigidBody;
}

}  // namespace mnd
