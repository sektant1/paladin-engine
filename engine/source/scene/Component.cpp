#include "scene/Component.h"

namespace ENG
{

usize Component::nextId = 1;

GameObject *Component::GetOwner()
{
    return m_owner;
}

}  // namespace ENG
