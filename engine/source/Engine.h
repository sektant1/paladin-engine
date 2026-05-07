/**
 * @file Engine.h
 * @ingroup mnd_core
 * @brief Central singleton that owns and coordinates all engine subsystems.
 *
 * ## Startup sequence (see main.cpp)
 * @code
 *   Engine &engine = Engine::GetInstance();
 *   engine.SetApplication(new Game());   // hand over ownership
 *   engine.Init(1280, 720);              // GLFW → window → GLEW → app->Init()
 *   engine.Run();                        // main loop until close requested
 *   engine.Destroy();                    // app->Destroy() → GLFW cleanup
 * @endcode
 *
 * ## Main loop (Engine::Run)
 * Each iteration:
 * 1. `glfwPollEvents()` → GLFW callbacks update InputManager.
 * 2. Compute `deltaTime` with a steady_clock timestamp.
 * 3. `app->Update(dt)` — game logic runs here.
 * 4. Collect CameraData and LightData from the active Scene.
 * 5. `RenderQueue::Draw()` drains all submitted RenderCommands.
 * 6. `glfwSwapBuffers()` — present frame.
 *
 * @note Engine is a non-copyable, non-movable singleton (Meyer's singleton pattern).
 *       Access it anywhere via `Engine::GetInstance()`.
 *
 * @see Application, RenderQueue, Scene
 */

#pragma once
#include <chrono>
#include <memory>

#include "audio/AudioManager.h"
#include "editor/Editor.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/PostProcess.h"
#include "graphics/RenderSettings.h"
#include "graphics/RenderTarget.h"
#include "graphics/RendererBackend.h"
#include "graphics/Texture.h"
#include "input/InputManager.h"
#include "io/FileSystem.h"
#include "physics/PhysicsManager.h"
#include "render/RenderQueue.h"
#include "render/ParticleSystem.h"
#include "render/SpriteRenderer.h"
#include "scene/Scene.h"

struct GLFWwindow;

namespace mnd
{
class Application;

/**
 * @brief Singleton façade that drives the full engine lifecycle.
 *
 * Owns one instance of every major subsystem (InputManager, GraphicsAPI,
 * RenderQueue, FileSystem, TextureManager, Scene) and the user Application.
 * All subsystems are reachable via the Get*() accessors from anywhere in
 * the codebase without passing references through layers.
 */
class Engine
{
public:
    /// Returns the single Engine instance (created on first call).
    static Engine &GetInstance();

    Engine()                          = default;
    Engine(const Engine &)            = delete;
    Engine(Engine &&)                 = delete;
    Engine &operator=(const Engine &) = delete;
    Engine &operator=(Engine &&)      = delete;

    /**
     * @brief Initialise the window, OpenGL context, and subsystems.
     *
     * Sequence: GLFW init → window create → GLFW callbacks → GLEW init →
     * GraphicsAPI::Init() → Application::Init().
     *
     * @param width  Framebuffer width in pixels.
     * @param height Framebuffer height in pixels.
     * @return true on success; false if any step fails (logged to stderr).
     */
    bool Init(int width, int height);

    /**
     * @brief Enter the blocking main loop.
     *
     * Returns when the window close button is pressed or the application
     * calls SetNeedsToBeClosed(true). See the class-level documentation for
     * the per-frame execution order.
     */
    void Run();

    /**
     * @brief Shut down the application and release all GL resources.
     *
     * Calls Application::Destroy(), then terminates GLFW. Must be called
     * after Run() returns to ensure clean resource cleanup.
     */
    void Destroy();

    /**
     * @brief Transfer ownership of the application to the engine.
     * @param app Heap-allocated Application subclass. Engine takes ownership.
     */
    void SetApplication(Application *app);

    /// Returns a raw pointer to the active application (owned by the engine).
    Application *GetApplication();

    /// Returns the InputManager that tracks keyboard and mouse state.
    InputManager &GetInputManager();

    /// Returns the low-level OpenGL wrapper used for all draw calls.
    GraphicsAPI &GetGraphicsAPI();

    /// Returns the per-frame command queue that batches and issues draw calls.
    RenderQueue &GetRenderQueue();

    /// Returns the 2D sprite/font renderer. Queue draw calls during Application::Update().
    SpriteRenderer &GetSpriteRenderer() { return m_spriteRenderer; }

    /// Returns the global particle pool. Emitter components push particles via Spawn().
    ParticleSystem &GetParticleSystem() { return m_particleSystem; }

    /// Returns the file system helper for asset-relative path resolution.
    FileSystem &GetFileSystem();

    /// Returns the TextureManager that caches loaded textures by path.
    TextureManager &GetTextureManager();

    PhysicsManager &GetPhysicsManager();

    AudioManager &GetAudioManager();

    /**
     * @brief Replace the active scene (takes ownership).
     * @param scene Heap-allocated Scene. Previous scene is destroyed.
     */
    void SetScene(Scene *scene);

    /// Returns the currently active scene, or nullptr if none is set.
    Scene *GetScene();

    /// ImGui-based editor overlay.
    Editor &GetEditor() { return m_editor; }

    /// Mutable render settings edited by the Editor's Render panel.
    RenderSettings &GetRenderSettings() { return m_renderSettings; }

    /// Offscreen low-res render target (only active when RenderSettings::useInternalRes is true).
    RenderTarget &GetSceneTarget() { return m_sceneTarget; }

    /// Outline / highlight post-pass; tunables exposed via the Editor render panel.
    PostProcess &GetPostProcess() { return m_postProcess; }

    GLFWwindow *GetWindow() { return m_window; }

    /// Game-time multiplier applied to deltaTime each frame (1.0 = realtime).
    float GetTimeScale() const { return m_timeScale; }
    void  SetTimeScale(float scale) { m_timeScale = scale; }

    /// When true, deltaTime is forced to 0.0 (Application::Update still runs).
    bool IsPaused() const { return m_paused; }
    void SetPaused(bool paused) { m_paused = paused; }

    /// Render a centered loading panel with optional progress bar.
    /// Pass `progress < 0` for an indeterminate (animated) bar.
    /// Performs two swaps on first call so the frame is presented before
    /// blocking work begins on platforms that delay the first present.
    void DrawLoadingScreen(const char *message, float progress = -1.0F);

    /// Update the loading panel mid-load (single swap). Cheap to call per step.
    void UpdateLoadingProgress(float progress, const char *message);

private:
    std::unique_ptr<Application>          m_application;       ///< The user game/lab instance.
    std::chrono::steady_clock::time_point m_lastTimePoint;     ///< Timestamp of the previous frame.
    GLFWwindow                           *m_window = nullptr;  ///< GLFW window handle.
    InputManager                          m_inputManager;      ///< Keyboard + mouse state.
    GraphicsAPI                           m_graphicsAPI;       ///< GL wrapper (shaders, buffers, draw).
    std::unique_ptr<RendererBackend>      m_rendererBackend;   ///< Active renderer backend facade.
    RenderQueue                           m_renderQueue;       ///< Per-frame render command list.
    SpriteRenderer                        m_spriteRenderer;    ///< 2D sprite/text overlay queue.
    ParticleSystem                        m_particleSystem;    ///< CPU billboard particle pool.
    FileSystem                            m_fileSystem;        ///< Asset path resolver.
    TextureManager                        m_textureManager;    ///< Texture cache.
    AudioManager                          m_audioManager;

    PhysicsManager         m_physicsManager;
    std::unique_ptr<Scene> m_currentScene;  ///< Active scene graph.

    float m_timeScale = 1.0F;
    bool  m_paused    = false;

    float m_fpsSmoothed     = 0.0F;  ///< EMA of 1/dt for the overlay counter.
    float m_fpsRefreshTimer = 0.0F;  ///< Throttles the displayed value's refresh.
    float m_fpsDisplayed    = 0.0F;  ///< The currently rendered FPS value.

    Editor         m_editor;          ///< ImGui overlay.
    RenderTarget   m_sceneTarget;     ///< Low-res FBO for pixelated look.
    PostProcess    m_postProcess;     ///< Outline post-pass on the scene target.
    RenderSettings m_renderSettings;  ///< Editor-tweakable render params.
};

}  // namespace mnd
