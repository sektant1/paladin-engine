/**
 * @file Application.h
 * @brief Abstract base class for the user-defined game or lab application.
 *
 * ## How to use
 * 1. Subclass Application and implement Init(), Update(), and Destroy().
 * 2. Pass an instance to Engine::SetApplication() before calling Engine::Init().
 * 3. Engine::Run() calls Update() once per frame and stops when
 *    NeedsToBeClosed() returns true or the OS window is closed.
 *
 * @code
 *   class Game : public ENG::Application {
 *   public:
 *       bool Init()   override { ... return true; }
 *       void Update(float dt) override { ... }
 *       void Destroy() override { ... }
 *   };
 * @endcode
 *
 * @see Engine
 */

#pragma once

namespace ENG
{

/**
 * @brief Interface that every runnable game or lab must implement.
 *
 * The Engine singleton owns the Application instance (via std::unique_ptr)
 * and drives its lifecycle through the three virtual methods below.
 */
class Application
{
public:
    /**
     * @brief Called once by Engine::Init() after the GL context is ready.
     * @return true if initialisation succeeded; false aborts startup.
     */
    virtual bool Init() = 0;

    /**
     * @brief Called every frame by Engine::Run() with the elapsed time.
     * @param deltaTime Seconds since the previous frame. Use this to make
     *                  movement and animations frame-rate independent.
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief Called by Engine::Destroy() before the GL context is torn down.
     *        Release GPU resources (meshes, textures, shaders) here.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Request the engine to stop after the current frame.
     * @param value true to signal shutdown, false to keep running.
     */
    void SetNeedsToBeClosed(bool value);

    /**
     * @brief Returns true when the application has requested shutdown.
     *        Engine::Run() checks this each frame to know when to exit.
     */
    [[nodiscard]] bool NeedsToBeClosed() const;

private:
    bool m_needsToBeClosed = false;  ///< Shutdown flag polled by the main loop.
};

}  // namespace ENG
