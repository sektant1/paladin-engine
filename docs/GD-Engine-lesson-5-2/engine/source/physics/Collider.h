#pragma once

#include <glm/vec3.hpp>

class btCollisionShape;

namespace eng
{
    class Collider
    {
    public:
        virtual ~Collider();
        btCollisionShape* GetShape();

    protected:
        btCollisionShape* m_shape = nullptr;
    };

    class BoxCollider : public Collider
    {
    public:
        BoxCollider(const glm::vec3& extents);
    };

    class SphereCollider : public Collider
    {
    public:
        SphereCollider(float radius);
    };

    class CapsuleCollider : public Collider
    {
    public:
        CapsuleCollider(float radius, float height);
    };
}