#include "LabObject.h"

#include <GLFW/glfw3.h>

#include "Engine.h"
#include "GameConstants.h"
#include "graphics/GraphicsAPI.h"
#include "render/Builder.h"

LabObject::LabObject()
{
    LOG_INFO("LabObject constructing");

    // load shaders
    COA::FileReader vertShader(kCoolVertShaderPath);
    COA::FileReader fragShader(kGreenFragShaderPath);
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
    if (input.IsKeyPressed(COA::Key::Num1)) {
        m_timeScale = kLabTimeScale1;
    }
    if (input.IsKeyPressed(COA::Key::Num2)) {
        m_timeScale = kLabTimeScale2;
    }
    if (input.IsKeyPressed(COA::Key::Num3)) {
        m_timeScale = kLabTimeScale3;
    }
    if (input.IsKeyPressed(COA::Key::Num4)) {
        m_timeScale = kLabTimeScale4;
    }
    if (input.IsKeyPressed(COA::Key::Num5)) {
        m_timeScale = kLabTimeScale5;
    }
    // if (input.IsKeyPressed(Key::A)) {
    //     m_offsetX -= 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(Key::D)) {
    //     m_offsetX += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(Key::W)) {
    //     m_offsetY += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(Key::S)) {
    //     m_offsetY -= 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(Key::Q)) {
    //     m_angle += 1.0F * deltaTime;
    // }
    // if (input.IsKeyPressed(Key::E)) {
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

    m_material->SetParam(kUniformResolution, (float)w, (float)h);
    m_material->SetParam(kUniformTime, m_time);
    m_material->SetParam(kUniformMouse, (float)mx, (float)my, 0.0F, 0.0F);

    SetPosition(position);
}
