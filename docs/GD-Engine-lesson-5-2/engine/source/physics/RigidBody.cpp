#include "physics/RigidBody.h"
#include "Engine.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace eng
{
	RigidBody::RigidBody(BodyType type, const std::shared_ptr<Collider>& collider, float mass, float friction)
		: m_type(type), m_collider(collider), m_mass(mass), m_friction(friction)
	{
		if (!collider)
		{
			return;
		}

		m_collisionObjectType = CollisionObjectType::RigidBody;

		btVector3 intertia(0, 0, 0);
		if (m_type == BodyType::Dynamic && mass > 0.0f && m_collider->GetShape())
		{
			m_collider->GetShape()->calculateLocalInertia(btScalar(mass), intertia);
		}

		btTransform transform;
		transform.setIdentity();
		btDefaultMotionState* motionState = new btDefaultMotionState(transform);

		btRigidBody::btRigidBodyConstructionInfo info(
			(m_type == BodyType::Dynamic) ? btScalar(mass) : btScalar(0),
			motionState, m_collider->GetShape(), intertia
		);

		m_body = std::make_unique<btRigidBody>(info);
		m_body->setFriction(friction);
		m_body->setUserPointer(this);

		if (m_type == BodyType::Kinematic)
		{
			m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			m_body->setActivationState(DISABLE_DEACTIVATION);
		}
	}

	RigidBody::~RigidBody()
	{
		if (m_addedToWorld)
		{
			Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
		}
	}

	btRigidBody* RigidBody::GetBody()
	{
		return m_body.get();
	}

	void RigidBody::SetAddedToWorld(bool added)
	{
		m_addedToWorld = added;
	}

	bool RigidBody::IsAddedToWorld() const
	{
		return m_addedToWorld;
	}

	BodyType RigidBody::GetType() const
	{
		return m_type;
	}

	void RigidBody::SetPosition(const glm::vec3& pos)
	{
		if (!m_body)
		{
			return;
		}

		auto& tr = m_body->getWorldTransform();
		tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));
		if (m_body->getMotionState())
		{
			m_body->getMotionState()->setWorldTransform(tr);
		}
		m_body->setWorldTransform(tr);
	}

	glm::vec3 RigidBody::GetPosition() const
	{
		if (!m_body)
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}
		const auto& pos = m_body->getWorldTransform().getOrigin();
		return glm::vec3(pos.x(), pos.y(), pos.z());
	}

	void RigidBody::SetRotation(const glm::quat& rot)
	{
		if (!m_body)
		{
			return;
		}

		auto& tr = m_body->getWorldTransform();
		tr.setRotation(btQuaternion(btScalar(rot.x), btScalar(rot.y), btScalar(rot.z), btScalar(rot.w)));
		if (m_body->getMotionState())
		{
			m_body->getMotionState()->setWorldTransform(tr);
		}
		m_body->setWorldTransform(tr);
	}

	glm::quat RigidBody::GetRotation() const
	{
		if (!m_body)
		{
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}
		const auto& rot = m_body->getWorldTransform().getRotation();
		return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
	}

	void RigidBody::ApplyImpulse(const glm::vec3& impulse)
	{
		if (!m_body)
		{
			return;
		}
		m_body->applyCentralImpulse(btVector3(
			btScalar(impulse.x), btScalar(impulse.y), btScalar(impulse.z)
		));
	}
}