#include "input/InputManager.h"

#include "Log.h"

namespace ENG
{

void InputManager::SetKeyPressed(int key, bool pressed)
{
    if (key < 0 | key >= static_cast<int>(m_keys.size())) {
        LOG_WARN("SetKeyPressed out-of-range key=%d (size=%zu)", key, m_keys.size());
        return;
    }
    m_keys[key] = pressed;
}

bool InputManager::IsKeyPressed(int key)
{
    if (key < 0 | key >= static_cast<int>(m_keys.size())) {
        LOG_WARN("IsKeyPressed out-of-range key=%d (size=%zu)", key, m_keys.size());
        return false;
    }

    return m_keys[key];
}

}  // namespace ENG
