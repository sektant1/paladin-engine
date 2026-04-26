/**
 * @file Editor.h
 * @ingroup mnd_editor
 * @brief In-game ImGui overlay: hierarchy, inspector, console, stats.
 *
 * The Engine creates one Editor and drives its three-phase frame from
 * mnd::Engine::Run(): `BeginFrame()` after polling input, `Draw()` to
 * lay out every panel, and `EndFrame()` after the scene render so the
 * GUI composites on top.
 *
 * Toggle visibility at runtime, or register your own debug panels:
 * @code
 *   engine.GetEditor().RegisterPanel("My Tool", [] {
 *       ImGui::Text("hello from a custom panel");
 *   });
 * @endcode
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Types.h"

struct GLFWwindow;

namespace mnd
{

class GameObject;

/**
 * @ingroup mnd_editor
 * @brief ImGui-based in-game editor overlay.
 *
 * Owned by the Engine singleton and driven from Engine::Run(). Provides
 * scene hierarchy, inspector, asset browser, render settings (including
 * internal-resolution pixelation) and stats panels.
 */
class Editor
{
public:
    /// Custom-panel callback — render any ImGui widgets you like inside.
    using PanelFn = std::function<void()>;

    /// Initialise ImGui against the given GLFW window. Call once at startup.
    bool Init(GLFWwindow *window);
    /// Tear down ImGui state. Call before destroying the GL context.
    void Shutdown();

    void BeginFrame();  ///< Call after glfwPollEvents.
    void Draw();        ///< Builds all panels (no GL state changes yet).
    void EndFrame();    ///< Renders the ImGui draw data to the current framebuffer.

    bool IsVisible() const { return m_visible; }       ///< Editor overlay currently shown?
    void ToggleVisible() { m_visible = !m_visible; }   ///< Flip overlay visibility.

    /// Tell game code whether ImGui is consuming the mouse this frame
    /// (e.g. when hovering a panel) — skip your own click handling if so.
    bool WantsCaptureMouse() const;
    /// Same as WantsCaptureMouse() but for the keyboard.
    bool WantsCaptureKeyboard() const;

    /// Stats panel hook: report per-frame draw call count.
    void NotifyDrawCount(int n) { m_lastDrawCount = n; }

    /// Register a custom ImGui panel rendered alongside the built-ins.
    void RegisterPanel(std::string name, PanelFn fn)
    {
        m_customPanels.emplace_back(std::move(name), std::move(fn));
    }

private:
    void DrawMenuBar();
    void DrawHierarchy();
    void DrawInspector();
    void DrawConsole();
    void DrawStats();
    void DrawSettings();      ///< Tabbed left-column window holding the four bodies below.
    void DrawRenderBody();    ///< Render tab content (no Begin/End).
    void DrawEngineBody();    ///< Engine tab content.
    void DrawPhysicsBody();   ///< Physics tab content.
    void DrawPlayerBody();    ///< Player tab content.
    void DrawObjectNode(GameObject *obj);

    bool        m_initialized   = false;
    bool        m_visible       = false;  ///< Hidden at startup; F1 opens.
    bool        m_showHierarchy = true;
    bool        m_showInspector = true;
    bool        m_showConsole   = true;
    bool        m_showRender    = true;
    bool        m_showStats     = true;
    bool        m_showEngine    = true;
    bool        m_showPhysics   = false;
    bool        m_showPlayer    = false;
    bool        m_showDemo      = false;
    GameObject *m_selected      = nullptr;
    int         m_lastDrawCount = 0;
    f32         m_fpsSmoothed   = 0.0F;

    bool m_vsyncEnabled  = true;
    bool m_wireframe     = false;
    bool m_cursorLocked  = true;

    std::vector<std::pair<std::string, PanelFn>> m_customPanels;
};

}  // namespace mnd
