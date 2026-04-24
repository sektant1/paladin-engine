#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Types.h"

struct GLFWwindow;

namespace COA
{

class GameObject;

/// ImGui-based in-game editor overlay. Owned by the Engine singleton and driven
/// from Engine::Run(). Provides scene hierarchy, inspector, asset browser, render
/// settings (including internal-resolution pixelation) and stats panels.
class Editor
{
public:
    using PanelFn = std::function<void()>;

    bool Init(GLFWwindow *window);
    void Shutdown();

    void BeginFrame();  ///< Call after glfwPollEvents.
    void Draw();        ///< Builds all panels (no GL state changes yet).
    void EndFrame();    ///< Renders the ImGui draw data to the current framebuffer.

    bool IsVisible() const { return m_visible; }
    void ToggleVisible() { m_visible = !m_visible; }

    bool WantsCaptureMouse() const;
    bool WantsCaptureKeyboard() const;

    void NotifyDrawCount(int n) { m_lastDrawCount = n; }

    void RegisterPanel(std::string name, PanelFn fn)
    {
        m_customPanels.emplace_back(std::move(name), std::move(fn));
    }

private:
    void DrawMenuBar();
    void DrawHierarchy();
    void DrawInspector();
    void DrawAssets();
    void DrawRender();
    void DrawStats();
    void DrawObjectNode(GameObject *obj);

    bool        m_initialized     = false;
    bool        m_visible         = true;
    bool        m_showHierarchy   = true;
    bool        m_showInspector   = true;
    bool        m_showAssets      = true;
    bool        m_showRender      = true;
    bool        m_showStats       = true;
    bool        m_showDemo        = false;
    GameObject *m_selected        = nullptr;
    int         m_lastDrawCount   = 0;
    f32         m_fpsSmoothed     = 0.0F;

    std::vector<std::pair<std::string, PanelFn>> m_customPanels;
};

}  // namespace COA
