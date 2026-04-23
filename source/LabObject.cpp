#include "LabObject.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "graphics/GraphicsAPI.h"
#include "render/Builder.h"

LabObject::LabObject()
{
    LOG_INFO("LabObject constructing");

    // load shaders
    COA::FileReader vertShader("assets/shaders/cool.vert");
    COA::FileReader fragShader("assets/shaders/green.frag");
    auto            vs = vertShader.ReadToString();
    auto            fs = fragShader.ReadToString();

    if (vs.empty() || fs.empty()) {
        LOG_ERROR("LabObject shader source is empty (vert=%zu frag=%zu bytes)", vs.size(), fs.size());
    }

    auto &gfx  = COA::Engine::GetInstance().GetGraphicsAPI();
    auto  prog = gfx.CreateShaderProgram(vs, fs);
    if (!prog) {
        LOG_ERROR("LabObject failed to create shader program");
    }

    m_material = std::make_shared<COA::Material>();
    m_material->SetShaderProgram(prog);

    auto mesh = COA::Builder::CreateFullscreenQuad().buildMesh();
    AddComponent(new COA::MeshComponent(m_material, mesh));
    LOG_INFO("LabObject constructed");
}

void LabObject::Update(COA::f32 deltaTime)
{
    COA::GameObject::Update(deltaTime);

    auto  position = GetPosition();
    auto &input    = COA::Engine::GetInstance().GetInputManager();
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
    if (input.IsKeyPressed(GLFW_KEY_5)) {
        m_timeScale = -1.5F;
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

    GLFWwindow *window = glfwGetCurrentContext();
    int         w = 1, h = 1;
    double      mx = 0.0, my = 0.0;
    if (window) {
        glfwGetFramebufferSize(window, &w, &h);
        glfwGetCursorPos(window, &mx, &my);
        my = (double)h - my;
    }

    m_material->SetParam("iResolution", (float)w, (float)h);
    m_material->SetParam("iTime", m_time);
    m_material->SetParam("iMouse", (float)mx, (float)my, 0.0F, 0.0F);

    SetPosition(position);
}
