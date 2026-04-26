#include "KinematicCharacterController.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <btBulletDynamicsCommon.h>

#include "Engine.h"

namespace mnd
{

KinematicCharacterController::KinematicCharacterController(float radius, float height, const glm::vec3 &position)
    : m_height(height)
    , m_radius(radius)
{
    m_collisionObjType = CollisionObjectType::KinematicCharacterController;
    auto world         = Engine::GetInstance().GetPhysicsManager().GetWorld();

    // Capsule collider (standard for characters)
    auto capsule = new btCapsuleShape(m_radius, m_height);

    m_ghost = std::make_unique<btPairCachingGhostObject>();
    btTransform start;
    start.setIdentity();
    start.setOrigin(btVector3(position.x, position.y, position.z));
    m_ghost->setWorldTransform(start);
    m_ghost->setCollisionShape(capsule);
    m_ghost->setCollisionFlags(m_ghost->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
    m_ghost->setUserPointer(this);

    world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    const btScalar stepHeight = kDefaultStepHeight;
    m_controller              = std::make_unique<btKinematicCharacterController>(m_ghost.get(), capsule, stepHeight);

    m_controller->setMaxSlope(btRadians(50.0F));
    m_controller->setGravity(world->getGravity());

    world->addCollisionObject(
        m_ghost.get(),
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger
    );
    world->addAction(m_controller.get());
}

KinematicCharacterController::~KinematicCharacterController()
{
    auto world = Engine::GetInstance().GetPhysicsManager().GetWorld();
    if (m_controller)
    {
        world->removeAction(m_controller.get());
    }
    if (m_ghost)
    {
        world->removeCollisionObject(m_ghost.get());
    }
}

glm::vec3 KinematicCharacterController::GetPosition() const
{
    const auto &pos = m_ghost->getWorldTransform().getOrigin();
    // Offset upwards: camera is not at capsule center
    const glm::vec3 offset(0.0F, m_height * 0.5F + m_radius, 0.0F);
    return glm::vec3(pos.x(), pos.y(), pos.z()) + offset;
}

glm::quat KinematicCharacterController::GetRotation() const
{
    const auto &rot = m_ghost->getWorldTransform().getRotation();
    return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

void KinematicCharacterController::Walk(const vec3 &dir)
{
    m_controller->setWalkDirection(btVector3(dir.x, dir.y, dir.z));
}

void KinematicCharacterController::Jump(const glm::vec3 &dir)
{
    if (m_controller->onGround())
    {
        m_controller->jump(btVector3(dir.x, dir.y, dir.z));
    }
}

bool KinematicCharacterController::OnGround() const
{
    return m_controller->onGround();
}

}  // namespace mnd
