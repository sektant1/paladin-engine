/**
 * @file Engine.h
 * @ingroup coa_core
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

#include "graphics/GraphicsAPI.h"
#include "graphics/Texture.h"
#include "input/InputManager.h"
#include "io/FileSystem.h"
#include "render/RenderQueue.h"
#include "scene/Scene.h"

struct GLFWwindow;

namespace COA
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

    /// Returns the file system helper for asset-relative path resolution.
    FileSystem &GetFileSystem();

    /// Returns the TextureManager that caches loaded textures by path.
    TextureManager &GetTextureManager();

    /**
     * @brief Replace the active scene (takes ownership).
     * @param scene Heap-allocated Scene. Previous scene is destroyed.
     */
    void SetScene(Scene *scene);

    /// Returns the currently active scene, or nullptr if none is set.
    Scene *GetScene();

private:
    std::unique_ptr<Application>          m_application;    ///< The user game/lab instance.
    std::chrono::steady_clock::time_point m_lastTimePoint;  ///< Timestamp of the previous frame.
    GLFWwindow                           *m_window = nullptr; ///< GLFW window handle.
    InputManager                          m_inputManager;   ///< Keyboard + mouse state.
    GraphicsAPI                           m_graphicsAPI;    ///< GL wrapper (shaders, buffers, draw).
    RenderQueue                           m_renderQueue;    ///< Per-frame render command list.
    FileSystem                            m_fileSystem;     ///< Asset path resolver.
    TextureManager                        m_textureManager; ///< Texture cache.
    std::unique_ptr<Scene>                m_currentScene;   ///< Active scene graph.
};

}  // namespace COA
