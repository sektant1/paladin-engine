#pragma once

#include "physics/CollisionObject.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class btPairCachingGhostObject;
class btKinematicCharacterController;

namespace eng
{
    class KinematicCharacterController : public CollisionObject
    {
    public:
        KinematicCharacterController(float raduis, float height, const glm::vec3& position);
        ~KinematicCharacterController();

        glm::vec3 GetPosition() const;
        glm::quat GetRotation() const;

        void Walk(const glm::vec3& direction);
        void Jump(const glm::vec3& direction);
        bool OnGround() const;

    private:
        float m_height = 1.2f;
        float m_radius = 0.4f;

        std::unique_ptr<btPairCachingGhostObject> m_ghost;
        std::unique_ptr<btKinematicCharacterController> m_controller;
    };
}