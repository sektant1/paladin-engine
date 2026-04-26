#include "JumpPlatform.h"

void JumpPlatform::Init()
{
    auto physics = GetComponent<mnd::PhysicsComponent>();
    if (physics)
    {
        auto rigidBody = physics->GetRigidBody();
        if (rigidBody)
        {
            rigidBody->AddContactListener(this);
        }
    }
}

void JumpPlatform::OnContact(mnd::CollisionObject *obj, const mnd::vec3 &pos, const mnd::vec3 &norm)
{
    if (obj->GetCollisionObjectType() == mnd::CollisionObjectType::KinematicCharacterController)
    {
        auto controller = static_cast<mnd::KinematicCharacterController *>(obj);
        if (controller)
        {
            controller->Jump(glm::vec3(0.0f, 20.0f, 0.0f));
        }
    }
}

