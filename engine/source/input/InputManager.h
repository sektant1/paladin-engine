/**
 * @file InputManager.h
 * @ingroup coa_input
 * @brief Keyboard and mouse state cache updated each frame by the engine.
 *
 * Engine::Run() feeds GLFW callbacks into InputManager; game code reads state
 * via IsKeyPressed() / IsMouseButtonPressed() and mouse-position deltas.
 *
 * Keys are indexed by GLFW key codes (GLFW_KEY_*). Mouse buttons are indexed
 * by GLFW mouse-button codes (GLFW_MOUSE_BUTTON_*). The manager is not
 * instantiated directly — access it through Engine::GetInputManager().
 *
 * @see Engine::GetInputManager
 */

#pragma once
#include <array>

#include "Constants.h"
#include "Types.h"
#include "input/Key.h"

namespace COA
{

/**
 * @brief Stores per-frame keyboard and mouse input state.
 *
 * Owned by the Engine singleton. Game code reads state; only the Engine writes
 * state (it is a friend). Arrays are sized for 512 keys and 16 mouse buttons,
 * matching GLFW's key-code range.
 */
class InputManager
{
public:
    InputManager()                     = default;
    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&)      = delete;

    InputManager &operator=(const InputManager &) = delete;
    InputManager &operator=(InputManager &&)      = delete;

    /**
     * @brief Record a key press or release event.
     * @param key     Key code (out-of-range values are ignored with a warning).
     * @param pressed True when pressed, false when released.
     */
    void SetKeyPressed(Key key, bool pressed);

    /**
     * @brief Query whether a key is currently held down.
     * @param key Key code (e.g. Key::W).
     * @return True if the key is pressed.
     */
    bool IsKeyPressed(Key key) const;

    /**
     * @brief Record a mouse-button press or release event.
     * @param button  Mouse-button code.
     * @param pressed True when pressed, false when released.
     */
    void SetMouseButtonPressed(MouseButton button, bool pressed);

    /**
     * @brief Query whether a mouse button is currently held down.
     * @param button Mouse-button code (e.g. MouseButton::Left).
     * @return True if the button is pressed.
     */
    bool IsMouseButtonPressed(MouseButton button) const;

    /**
     * @brief Store the cursor position from the previous frame.
     * @param pos Screen-space cursor coordinates.
     */
    void SetMousePositionOld(const vec2 &pos);

    /**
     * @brief Retrieve the cursor position from the previous frame.
     * @return Screen-space cursor coordinates.
     */
    [[nodiscard]] const vec2 GetMousePositionOld() const;

    /**
     * @brief Store the cursor position for the current frame.
     * @param pos Screen-space cursor coordinates.
     */
    void SetMousePositionCurrent(const vec2 &pos);

    void SetMousePositionChanged(bool changed);
    bool IsMousePositionChanged() const;

    /**
     * @brief Retrieve the cursor position for the current frame.
     *
     * Subtract GetMousePositionOld() to get a per-frame delta suitable for
     * camera rotation.
     *
     * @return Screen-space cursor coordinates.
     */
    [[nodiscard]] const vec2 GetMousePositionCurrent() const;

private:
    std::array<bool, kMaxKeys>         m_keys      = {false};  ///< Pressed state for each GLFW key code.
    std::array<bool, kMaxMouseButtons> m_mouseKeys = {false};  ///< Pressed state for each GLFW mouse button.
    vec2                  m_mousePositionOld     = vec2(0.0F);  ///< Cursor position at end of previous frame.
    vec2                  m_mousePositionCurrent = vec2(0.0F);  ///< Cursor position at end of current frame.

    bool m_mousePositionChanged = false;

    friend class Engine;
};

}  // namespace COA
