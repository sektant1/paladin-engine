#include <chrono>
#include <vector>

#include "Engine.h"

#include "Application.h"
#include "Common.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Log.h"
#include "graphics/GraphicsAPI.h"
#include "render/RenderQueue.h"
#include "scene/components/CameraComponent.h"

namespace ENG
{
void KeyCallback(GLFWwindow *window, int key, int, int action, int)
{
    auto &inputManager = ENG::Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetKeyPressed(key, true);
    } else if (action == GLFW_RELEASE)
    {
        inputManager.SetKeyPressed(key, false);
    }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    auto &inputManager = ENG::Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetMouseButtonPressed(button, true);
    } else if (action == GLFW_RELEASE)
    {
        inputManager.SetMouseButtonPressed(button, false);
    }
}

void CursorPositionCallback(GLFWwindow *window, f64 xpos, f64 ypos)
{
    auto &inputManager = ENG::Engine::GetInstance().GetInputManager();

    inputManager.SetMousePositionOld(inputManager.GetMousePositionCurrent());

    vec2 currentPos(static_cast<f32>(xpos), static_cast<f32>(ypos));
    inputManager.SetMousePositionCurrent(currentPos);
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

    if (glfwInit() == 0)
    {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    LOG_INFO("GLFW initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, "GL Game Engine", nullptr, nullptr);

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

    glfwMakeContextCurrent(m_window);

    if (!glewInit() != GLEW_OK)
    {
        LOG_ERROR("Failed to initialize GLEW");
        glfwTerminate();
        return false;
    }
    LOG_INFO("GLEW initialized (GL %s)", reinterpret_cast<const char *>(glGetString(GL_VERSION)));

    m_graphicsAPI.Init();
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

        auto  now       = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
        m_lastTimePoint = now;

        m_application->Update(deltaTime);

        m_graphicsAPI.SetClearColor(1.0F, 1.0F, 1.0F, 1.0F);
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
                }
            }

            lights = m_currentScene->CollectLight();
        }

        m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);

        glfwSwapBuffers(m_window);

        m_inputManager.SetMousePositionOld(m_inputManager.GetMousePositionCurrent());
    }
}

void Engine::Destroy()
{
    LOG_INFO("Engine::Destroy requested");
    if (m_application)
    {
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

Scene *Engine::GetScene()
{
    return m_currentScene.get();
}

FileSystem &Engine::GetFileSystem()
{
    return m_fileSystem;
}

}  // namespace ENG
