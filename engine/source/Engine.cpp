#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

#include "Engine.h"

#include "Application.h"
#include "Common.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Log.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/opengl/OpenGLRendererBackend.h"
#include "imgui.h"
#include "physics/PhysicsManager.h"
#include "render/RenderQueue.h"
#include "scene/components/CameraComponent.h"

namespace mnd
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

    m_rendererBackend = CreateOpenGLRendererBackend(m_graphicsAPI);
    if (!m_rendererBackend->Init())
    {
        LOG_ERROR("Renderer backend init failed");
        glfwTerminate();
        return false;
    }
    m_physicsManager.Init();
    if (!m_audioManager.Init())
    {
        LOG_ERROR("AudioManager Init failed");
    }

    m_sceneTarget.Create(m_renderSettings.internalW, m_renderSettings.internalH);
    if (!m_postProcess.Init())
    {
        LOG_ERROR("PostProcess Init failed — outline pass disabled");
        m_renderSettings.useOutline = false;
    }
    if (!m_editor.Init(m_window))
    {
        LOG_ERROR("Editor Init failed");
    }
    if (!m_spriteRenderer.Init())
    {
        LOG_ERROR("SpriteRenderer Init failed");
    }
    m_particleSystem.Init();

    DrawLoadingScreen("Loading...", 0.0F);

    bool appOk = m_application->Init();

    UpdateLoadingProgress(1.0F, "Ready");
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

        float scaledDt = m_paused ? 0.0F : deltaTime * m_timeScale;

        m_physicsManager.Update(scaledDt);
        m_application->Update(scaledDt);
        m_particleSystem.Update(scaledDt);

        // Bind scene target or default framebuffer for the scene pass.
        // Path B keeps the scene target at framebuffer size and performs
        // pixelation in the post-pass by snapping shader sample UVs.
        int winW = 0, winH = 0;
        glfwGetFramebufferSize(m_window, &winW, &winH);
        if (winW <= 0) winW = kDefaultInternalWidth;
        if (winH <= 0) winH = kDefaultInternalHeight;

        if (m_renderSettings.pixelSize < 1)
        {
            m_renderSettings.pixelSize = 1;
        }
        else if (m_renderSettings.pixelSize > 32)
        {
            m_renderSettings.pixelSize = 32;
        }

        if (m_renderSettings.useInternalRes)
        {
            m_renderSettings.internalW = winW;
            m_renderSettings.internalH = winH;
            m_sceneTarget.Resize(m_renderSettings.internalW, m_renderSettings.internalH);
            if (m_sceneTarget.IsValid())
            {
                m_sceneTarget.Bind();
            }
            else
            {
                RenderTarget::BindDefault(winW, winH);
            }
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

        f32 aspect = static_cast<f32>(winW) / static_cast<f32>(winH);

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
                    cameraData.nearPlane        = cameraComponent->GetNearPlane();
                    cameraData.farPlane         = cameraComponent->GetFarPlane();
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
        m_particleSystem.Render(cameraData);

        // Run the pixelation/outline post-pass for the production color view.
        // Debug-view selector overrides it and shows raw MRT attachments.
        if (m_renderSettings.useInternalRes && m_sceneTarget.IsValid())
        {
            const bool runPostProcess = m_renderSettings.debugView == DebugView::Color
                                        && m_postProcess.IsValid();
            if (runPostProcess)
            {
                const float oldDepthStrength  = m_postProcess.depthEdgeStrength;
                const float oldNormalStrength = m_postProcess.normalEdgeStrength;
                if (!m_renderSettings.useOutline)
                {
                    m_postProcess.depthEdgeStrength  = 0.0F;
                    m_postProcess.normalEdgeStrength = 0.0F;
                }

                m_postProcess.RunOutline(m_sceneTarget, cameraData);

                m_postProcess.depthEdgeStrength  = oldDepthStrength;
                m_postProcess.normalEdgeStrength = oldNormalStrength;
            }

            RenderTarget::BindDefault(winW, winH);
            GLuint   tex  = m_sceneTarget.ColorTex();
            BlitMode mode = BlitMode::Color;
            switch (m_renderSettings.debugView)
            {
                case DebugView::Normal:
                    tex  = m_sceneTarget.NormalTex();
                    mode = BlitMode::DecodeNrm;
                    break;
                case DebugView::Depth:
                    tex  = m_sceneTarget.DepthTex();
                    mode = BlitMode::SplatRed;
                    break;
                case DebugView::Color:
                default:
                    if (runPostProcess && m_postProcess.OutputTex() != 0)
                    {
                        tex = m_postProcess.OutputTex();
                    }
                    break;
            }
            BlitNearest(tex, winW, winH, mode);
        }

        if (m_renderSettings.showFps)
        {
            const float instantFps = deltaTime > 0.0F ? 1.0F / deltaTime : 0.0F;
            const float emaAlpha   = 0.1F;
            m_fpsSmoothed = (m_fpsSmoothed <= 0.0F)
                                ? instantFps
                                : m_fpsSmoothed + (instantFps - m_fpsSmoothed) * emaAlpha;
            m_fpsRefreshTimer += deltaTime;
            if (m_fpsRefreshTimer >= 0.2F)
            {
                m_fpsDisplayed    = m_fpsSmoothed;
                m_fpsRefreshTimer = 0.0F;
            }

            char fpsBuf[64];
            std::snprintf(fpsBuf, sizeof(fpsBuf), "FPS %.0f  %.2fms", m_fpsDisplayed,
                          m_fpsDisplayed > 0.0F ? 1000.0F / m_fpsDisplayed : 0.0F);

            const float padding   = 12.0F;
            const float textSize  = 22.0F;
            const float boxWidth  = textSize * 0.55F * static_cast<float>(std::strlen(fpsBuf)) + padding * 2.0F;
            const float boxHeight = textSize + padding;
            m_spriteRenderer.DrawRect(vec2(padding, padding),
                                      vec2(boxWidth, boxHeight),
                                      vec4(0.04F, 0.05F, 0.07F, 0.65F));
            m_spriteRenderer.DrawText(fpsBuf,
                                      vec2(padding + padding * 0.5F + 1.0F,
                                           padding + padding * 0.5F * 0.5F + 1.0F),
                                      textSize,
                                      vec4(0.0F, 0.0F, 0.0F, 0.85F));
            m_spriteRenderer.DrawText(fpsBuf,
                                      vec2(padding + padding * 0.5F,
                                           padding + padding * 0.5F * 0.5F),
                                      textSize,
                                      vec4(0.85F, 1.0F, 0.85F, 1.0F));
        }

        m_spriteRenderer.Flush(winW, winH);

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
        m_particleSystem.Shutdown();
        m_spriteRenderer.Shutdown();
        m_postProcess.Destroy();
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

namespace
{
void RenderLoadingFrameImpl(GLFWwindow *window, const char *message, float progress)
{
    glfwPollEvents();

    int winW = 0, winH = 0;
    glfwGetFramebufferSize(window, &winW, &winH);

    RenderTarget::BindDefault(winW, winH);
    glClearColor(0.05F, 0.05F, 0.07F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5F, vp->WorkPos.y + vp->WorkSize.y * 0.5F),
                            ImGuiCond_Always,
                            ImVec2(0.5F, 0.5F));
    ImGui::SetNextWindowSize(ImVec2(420.0F, 0.0F), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0F, 24.0F));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10F, 0.10F, 0.13F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.35F, 0.65F, 1.0F, 1.0F));
    ImGui::Begin("##loading",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav
                     | ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::SetWindowFontScale(1.6F);
    ImGui::TextUnformatted(message);
    ImGui::SetWindowFontScale(1.0F);
    ImGui::Spacing();

    float barFraction = (progress < 0.0F) ? -1.0F * static_cast<float>(ImGui::GetTime()) : progress;
    char  overlay[16] = "";
    if (progress >= 0.0F)
    {
        std::snprintf(overlay, sizeof(overlay), "%d%%", static_cast<int>(progress * 100.0F + 0.5F));
    }
    ImGui::ProgressBar(barFraction, ImVec2(-1.0F, 18.0F), progress >= 0.0F ? overlay : "");

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}
}  // namespace

void Engine::DrawLoadingScreen(const char *message, float progress)
{
    if (m_window == nullptr)
    {
        return;
    }
    // Swap twice: on some platforms/compositors the very first swap after
    // window creation isn't presented until the next frame is queued.
    RenderLoadingFrameImpl(m_window, message, progress);
    RenderLoadingFrameImpl(m_window, message, progress);
}

void Engine::UpdateLoadingProgress(float progress, const char *message)
{
    if (m_window == nullptr)
    {
        return;
    }
    RenderLoadingFrameImpl(m_window, message, progress);
}

}  // namespace mnd
