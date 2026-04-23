#include "scene/Component.h"

namespace COA
{

usize Component::nextId = 1;

GameObject *Component::GetOwner()
{
    return m_owner;
}

}  // namespace COA
