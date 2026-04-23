#include "input/InputManager.h"

#include "Log.h"

namespace COA
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

void InputManager::SetMouseButtonPressed(int button, bool pressed)
{
    if (button < 0 | button >= static_cast<int>(m_keys.size())) {
        return;
    }
    m_mouseKeys[button] = pressed;
}

bool InputManager::IsMouseButtonPressed(int button)
{
    if (button < 0 | button >= static_cast<int>(m_keys.size())) {
        return false;
    }

    return m_mouseKeys[button];
}

void InputManager::SetMousePositionOld(const vec2 &pos)
{
    m_mousePositionOld = pos;
}

const vec2 InputManager::GetMousePositionOld() const
{
    return m_mousePositionOld;
}

void InputManager::SetMousePositionCurrent(const vec2 &pos)
{
    m_mousePositionCurrent = pos;
}

const vec2 InputManager::GetMousePositionCurrent() const
{
    return m_mousePositionCurrent;
}

}  // namespace COA
