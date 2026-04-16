#include <chrono>

#include "Engine.h"
#include "Log.h"

#include "Application.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "graphics/GraphicsAPI.h"
#include "render/RenderQueue.h"

namespace ENG
{
void KeyCallback(GLFWwindow *window, int key, int, int action, int)
{
    auto &inputManager = ENG::Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS) {
        inputManager.SetKeyPressed(key, true);
    } else if (action == GLFW_RELEASE) {
        inputManager.SetKeyPressed(key, false);
    }
}

Engine &Engine::GetInstance()
{
    static Engine instance;
    return instance;
}

bool Engine::Init(int width, int height)
{
    if (!m_application) {
        LOG_ERROR("No application set");
        return false;
    }

    if (glfwInit() == 0) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, "GL Game Engine", nullptr, nullptr);

    if (m_window == nullptr) {
        LOG_ERROR("Error creating window");
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(m_window, KeyCallback);

    glfwMakeContextCurrent(m_window);

    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        glfwTerminate();
        return false;
    }

    return m_application->Init();
}

void Engine::Run()
{
    if (!m_application) {
        return;
    }

    m_lastTimePoint = std::chrono::steady_clock::now();

    while ((glfwWindowShouldClose(m_window) == 0) && !m_application->NeedsToBeClosed()) {
        glfwPollEvents();

        auto  now       = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
        m_lastTimePoint = now;

        m_application->Update(deltaTime);

        m_graphicsAPI.SetClearColor(1.0F, 1.0F, 1.0F, 1.0F);
        m_graphicsAPI.ClearBuffers();

        m_renderQueue.Draw(m_graphicsAPI);

        glfwSwapBuffers(m_window);
    }
}

void Engine::Destroy()
{
    if (m_application) {
        m_application->Destroy();
        m_application.reset();
        glfwTerminate();
        m_window = nullptr;
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

}  // namespace ENG
