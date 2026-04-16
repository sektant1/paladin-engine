#include "Lab.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "graphics/GraphicsAPI.h"
#include "render/Builder.h"

bool Lab::Init()
{
    // load shaders
    ENG::FileReader vertShader("assets/shaders/cool.vert");
    ENG::FileReader fragShader("assets/shaders/green.frag");
    std::string     vertexShaderSource   = vertShader.ReadToString();
    std::string     fragmentShaderSource = fragShader.ReadToString();

    auto &graphicsAPI   = ENG::Engine::GetInstance().GetGraphicsAPI();
    auto  shaderProgram = graphicsAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    m_material.SetShaderProgram(shaderProgram);

    auto fullscreen_data = ENG::Builder::CreateFullscreenQuad();
    m_mesh               = fullscreen_data.buildMesh();

    return true;
}

void Lab::Update(ENG::f32 deltaTime)
{
    auto &input = ENG::Engine::GetInstance().GetInputManager();
    if (input.IsKeyPressed(GLFW_KEY_1)) {
        m_timeScale = 0.25F;
    }
    if (input.IsKeyPressed(GLFW_KEY_2)) {
        m_timeScale = 0.5F;
    }
    if (input.IsKeyPressed(GLFW_KEY_3)) {
        m_timeScale = 1.0F;
    }
    if (input.IsKeyPressed(GLFW_KEY_4)) {
        m_timeScale = 1.5F;
    }

    // if (input.IsKeyPressed(GLFW_KEY_A)) {
    //     m_offsetX -= 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(GLFW_KEY_D)) {
    //     m_offsetX += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(GLFW_KEY_W)) {
    //     m_offsetY += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(GLFW_KEY_S)) {
    //     m_offsetY -= 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(GLFW_KEY_Q)) {
    //     m_angle += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(GLFW_KEY_E)) {
    //     m_angle -= 1.0F * deltaTime;
    // }

    m_time += deltaTime * m_timeScale;
    // grab window size from the current GLFW context
    GLFWwindow *window = glfwGetCurrentContext();
    int         w = 1, h = 1;
    if (window) {
        glfwGetFramebufferSize(window, &w, &h);
    }

    // mouse position (in window coords, Y flipped to match OpenGL)
    double mx = 0.0, my = 0.0;
    if (window) {
        glfwGetCursorPos(window, &mx, &my);
        my = (double)h - my;  // flip Y so origin is bottom-left like Shadertoy
    }

    m_material.SetParam("iTime", m_time);
    m_material.SetParam("iResolution", (ENG::f32)w, (ENG::f32)h);
    m_material.SetParam("iMouse", (ENG::f32)mx, (ENG::f32)my, 0.0F, 0.0F);

    ENG::RenderCommand command;
    command.material  = &m_material;
    command.mesh      = m_mesh.get();
    auto &renderQueue = ENG::Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}

void Lab::Destroy() {}
