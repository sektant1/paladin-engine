#pragma once

#include "Types.h"

namespace ENG
{

class GameObject;

class Component
{
public:
    virtual ~Component()                = default;
    virtual void  Update(f32 deltaTime) = 0;
    virtual usize GetTypeId() const     = 0;

    GameObject *GetOwner();

    template<typename T>
    static usize StaticTypeId()
    {
        static usize typeId = nextId++;
        return typeId;
    }

protected:
    GameObject *m_owner = nullptr;

    friend class GameObject;

private:
    static usize nextId;
};

// clang-format off
#define COMPONENT(ComponentClass) \
public: \
static usize TypeId() { return Component::StaticTypeId<ComponentClass>(); } \
usize GetTypeId() const override {return TypeId();} \


}  // namespace ENG
