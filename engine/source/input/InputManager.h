#pragma once
#include <array>

namespace ENG
{

class InputManager
{
public:
    InputManager()                     = default;
    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&)      = delete;

    InputManager &operator=(const InputManager &) = delete;
    InputManager &operator=(InputManager &&)      = delete;

    void SetKeyPressed(int key, bool pressed);
    bool IsKeyPressed(int key);

private:
    std::array<bool, 256> m_keys = {false};
};

}  // namespace ENG
