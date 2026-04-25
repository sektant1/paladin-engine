#include "input/InputManager.h"

#include "Engine.h"
#include "Log.h"
#include "editor/Editor.h"

namespace COA
{

static bool EditorSwallowsInput()
{
    return Engine::GetInstance().GetEditor().IsVisible();
}

void InputManager::SetKeyPressed(Key key, bool pressed)
{
    const int index = static_cast<int>(key);
    if (index < 0 || index >= static_cast<int>(m_keys.size()))
    {
        LOG_WARN("SetKeyPressed out-of-range key=%d (size=%zu)", index, m_keys.size());
        return;
    }
    m_keys[index] = pressed;
}

bool InputManager::IsKeyPressed(Key key) const
{
    const int index = static_cast<int>(key);
    if (index < 0 || index >= static_cast<int>(m_keys.size()))
    {
        LOG_WARN("IsKeyPressed out-of-range key=%d (size=%zu)", index, m_keys.size());
        return false;
    }
    if (EditorSwallowsInput())
    {
        return false;
    }

    return m_keys[index];
}

void InputManager::SetMouseButtonPressed(MouseButton button, bool pressed)
{
    const int index = static_cast<int>(button);
    if (index < 0 || index >= static_cast<int>(m_mouseKeys.size()))
    {
        return;
    }
    m_mouseKeys[index] = pressed;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const
{
    const int index = static_cast<int>(button);
    if (index < 0 || index >= static_cast<int>(m_mouseKeys.size()))
    {
        return false;
    }
    if (EditorSwallowsInput())
    {
        return false;
    }

    return m_mouseKeys[index];
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

void InputManager::SetMousePositionChanged(bool changed)
{
    m_mousePositionChanged = changed;
}

bool InputManager::IsMousePositionChanged() const
{
    if (EditorSwallowsInput())
    {
        return false;
    }
    return m_mousePositionChanged;
}

}  // namespace COA
