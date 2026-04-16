#include "Lab.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "graphics/GraphicsAPI.h"
#include "render/Builder.h"

bool Lab::Init()
{
    // load shaders
    ENG::FileReader vertShader("assets/shaders/lab.vert");
    ENG::FileReader fragShader("assets/shaders/lab.frag");
    std::string     vertexShaderSource   = vertShader.readToString();
    std::string     fragmentShaderSource = fragShader.readToString();

    // get graphicsAPI instance
    auto &graphicsAPI = ENG::Engine::GetInstance().GetGraphicsAPI();

    // create shader program
    auto shaderProgram = graphicsAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    m_material.SetShaderProgram(shaderProgram);

    auto rec_data      = ENG::Builder::CreateRectangle(2.0F, 2.0F);
    auto triangle_data = ENG::Builder::CreateTriangle(1.0F);

    m_mesh = triangle_data.buildMesh();

    return true;
}

void Lab::Update(ENG::f32 deltaTime)
{
    auto &input = ENG::Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(GLFW_KEY_A)) {
        LOG_INFO("[A] button is pressed");
        m_offsetX -= 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_D)) {
        LOG_INFO("[D] button is pressed");
        m_offsetX += 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_W)) {
        LOG_INFO("[W] button is pressed");
        m_offsetY += 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_S)) {
        LOG_INFO("[S] button is pressed");
        m_offsetY -= 0.01F;
    }

    m_material.SetParam("uOffset", m_offsetX, m_offsetY);

    ENG::RenderCommand command;
    command.material = &m_material;
    command.mesh     = m_mesh.get();

    auto &renderQueue = ENG::Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}

void Lab::Destroy() {}
