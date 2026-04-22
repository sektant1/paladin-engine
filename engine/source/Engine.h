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

namespace ENG
{
class Application;

class Engine
{
public:
    static Engine &GetInstance();

    Engine()                          = default;
    Engine(const Engine &)            = delete;
    Engine(Engine &&)                 = delete;
    Engine &operator=(const Engine &) = delete;
    Engine &operator=(Engine &&)      = delete;

    bool Init(int width, int height);
    void Run();
    void Destroy();

    void            SetApplication(Application *app);
    Application    *GetApplication();
    InputManager   &GetInputManager();
    GraphicsAPI    &GetGraphicsAPI();
    RenderQueue    &GetRenderQueue();
    FileSystem     &GetFileSystem();
    TextureManager &GetTextureManager();

    void   SetScene(Scene *scene);
    Scene *GetScene();

private:
    std::unique_ptr<Application>          m_application;
    std::chrono::steady_clock::time_point m_lastTimePoint;
    GLFWwindow                           *m_window = nullptr;
    InputManager                          m_inputManager;
    GraphicsAPI                           m_graphicsAPI;
    RenderQueue                           m_renderQueue;
    FileSystem                            m_fileSystem;
    TextureManager                        m_textureManager;
    std::unique_ptr<Scene>                m_currentScene;
};

}  // namespace ENG
