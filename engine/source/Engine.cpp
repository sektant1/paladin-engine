#include <chrono>
#include <vector>

#include "Engine.h"

#include "Application.h"
#include "Common.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Log.h"
#include "graphics/GraphicsAPI.h"
#include "physics/PhysicsManager.h"
#include "render/RenderQueue.h"
#include "scene/components/CameraComponent.h"

namespace COA
{
void KeyCallback(GLFWwindow *window, int key, int, int action, int)
{
    auto &inputManager = Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetKeyPressed(static_cast<Key>(key), true);
    } else if (action == GLFW_RELEASE)
    {
        inputManager.SetKeyPressed(static_cast<Key>(key), false);
    }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    auto &inputManager = Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetMouseButtonPressed(static_cast<MouseButton>(button), true);
    } else if (action == GLFW_RELEASE)
    {
        inputManager.SetMouseButtonPressed(static_cast<MouseButton>(button), false);
    }
}

void CursorPositionCallback(GLFWwindow *window, f64 xpos, f64 ypos)
{
    auto &inputManager = Engine::GetInstance().GetInputManager();

    inputManager.SetMousePositionOld(inputManager.GetMousePositionCurrent());

    vec2 currentPos(static_cast<f32>(xpos), static_cast<f32>(ypos));
    inputManager.SetMousePositionCurrent(currentPos);
    inputManager.SetMousePositionChanged(true);
}

Engine &Engine::GetInstance()
{
    static Engine instance;
    return instance;
}

bool Engine::Init(int width, int height)
{
    LOG_INFO("Engine::Init requested (%dx%d)", width, height);

    if (!m_application)
    {
        LOG_ERROR("No application set");
        return false;
    }

#if defined(__linux__)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    Scene::RegisterTypes();
    m_application->RegisterTypes();

    if (glfwInit() == 0)
    {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    LOG_INFO("GLFW initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, kDefaultWindowTitle, nullptr, nullptr);

    if (m_window == nullptr)
    {
        LOG_ERROR("Error creating window");
        glfwTerminate();
        return false;
    }
    LOG_INFO("Window created (%dx%d)", width, height);

    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPositionCallback);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(m_window);

    glewExperimental  = GL_TRUE;
    GLenum glewStatus = glewInit();
#ifdef GLEW_ERROR_NO_GLX_DISPLAY
    if (glewStatus == GLEW_ERROR_NO_GLX_DISPLAY)
    {
        glewStatus = GLEW_OK;
    }
#endif
    if (glewStatus != GLEW_OK)
    {
        LOG_ERROR("Failed to initialize GLEW: %s", reinterpret_cast<const char *>(glewGetErrorString(glewStatus)));
        glfwTerminate();
        return false;
    }
    while (glGetError() != GL_NO_ERROR)
    {
    }
    LOG_INFO("GLEW initialized (GL %s)", reinterpret_cast<const char *>(glGetString(GL_VERSION)));

    m_graphicsAPI.Init();
    m_physicsManager.Init();
    if (!m_audioManager.Init())
    {
        LOG_ERROR("AudioManager Init failed");
    }

    m_sceneTarget.Create(m_renderSettings.internalW, m_renderSettings.internalH);
    if (!m_editor.Init(m_window))
    {
        LOG_ERROR("Editor Init failed");
    }

    bool appOk = m_application->Init();
    if (!appOk)
    {
        LOG_ERROR("Application Init failed");
    } else
    {
        LOG_INFO("Application initialized");
    }
    return appOk;
}

void Engine::Run()
{
    if (!m_application)
    {
        LOG_ERROR("Engine::Run called with no application");
        return;
    }

    LOG_INFO("Engine main loop starting");
    m_lastTimePoint = std::chrono::steady_clock::now();

    while ((glfwWindowShouldClose(m_window) == 0) && !m_application->NeedsToBeClosed())
    {
        glfwPollEvents();
        m_editor.BeginFrame();

        auto  now       = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
        m_lastTimePoint = now;

        m_physicsManager.Update(deltaTime);
        m_application->Update(deltaTime);

        // Bind scene target (offscreen low-res) or default framebuffer for the scene pass.
        int winW = 0, winH = 0;
        glfwGetFramebufferSize(m_window, &winW, &winH);

        if (m_renderSettings.useInternalRes)
        {
            m_sceneTarget.Resize(m_renderSettings.internalW, m_renderSettings.internalH);
            m_sceneTarget.Bind();
        }
        else
        {
            RenderTarget::BindDefault(winW, winH);
        }

        m_graphicsAPI.SetClearColor(m_renderSettings.clearColor.r,
                                    m_renderSettings.clearColor.g,
                                    m_renderSettings.clearColor.b,
                                    m_renderSettings.clearColor.a);
        m_graphicsAPI.ClearBuffers();

        CameraData             cameraData;
        std::vector<LightData> lights;

        int width  = 0;
        int height = 0;

        glfwGetWindowSize(m_window, &width, &height);
        f32 aspect = static_cast<f32>(width) / static_cast<f32>(height);

        if (m_currentScene)
        {
            if (auto cameraObject = m_currentScene->GetMainCamera())
            {
                // logic for matrices
                auto cameraComponent = cameraObject->GetComponent<CameraComponent>();
                if (cameraComponent)
                {
                    cameraData.viewMatrix       = cameraComponent->GetViewMatrix();
                    cameraData.projectionMatrix = cameraComponent->GetProjectionMatrix(aspect);
                    cameraData.position         = cameraObject->GetWorldPosition();
                } else
                {
                    static bool warned = false;
                    if (!warned)
                    {
                        LOG_WARN("Main camera GameObject '%s' has no CameraComponent", cameraObject->GetName().c_str());
                        warned = true;
                    }
                }
            } else
            {
                static bool warned = false;
                if (!warned)
                {
                    LOG_WARN("Scene has no main camera set — rendering with identity matrices");
                    warned = true;
                }
            }

            lights = m_currentScene->CollectLight();
        }

        m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);

        // Blit offscreen low-res target to default framebuffer with nearest-neighbor upscale.
        if (m_renderSettings.useInternalRes && m_sceneTarget.IsValid())
        {
            RenderTarget::BindDefault(winW, winH);
            BlitNearest(m_sceneTarget.ColorTex(), winW, winH);
        }

        // Editor overlays the scene on the default framebuffer.
        m_editor.Draw();
        m_editor.EndFrame();

        glfwSwapBuffers(m_window);

        m_inputManager.SetMousePositionOld(m_inputManager.GetMousePositionCurrent());

        m_inputManager.SetMousePositionChanged(false);
    }
}

void Engine::Destroy()
{
    LOG_INFO("Engine::Destroy requested");
    if (m_application)
    {
        m_editor.Shutdown();
        m_sceneTarget.Destroy();
        m_application->Destroy();
        m_application.reset();
        glfwTerminate();
        m_window = nullptr;
        LOG_INFO("Engine shut down");
    } else
    {
        LOG_WARN("Engine::Destroy called with no application");
    }
}

void Engine::SetApplication(Application *app)
{
    m_application.reset(app);
}

Application *Engine::GetApplication()
{
    return m_application.get();
}

InputManager &Engine::GetInputManager()
{
    return m_inputManager;
}

GraphicsAPI &Engine::GetGraphicsAPI()
{
    return m_graphicsAPI;
}

RenderQueue &Engine::GetRenderQueue()
{
    return m_renderQueue;
}

void Engine::SetScene(Scene *scene)
{
    m_currentScene.reset(scene);
}

TextureManager &Engine::GetTextureManager()
{
    return m_textureManager;
}

AudioManager &Engine::GetAudioManager()
{
    return m_audioManager;
}

PhysicsManager &Engine::GetPhysicsManager()
{
    return m_physicsManager;
}

Scene *Engine::GetScene()
{
    return m_currentScene.get();
}

FileSystem &Engine::GetFileSystem()
{
    return m_fileSystem;
}

}  // namespace COA
