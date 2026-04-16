#include "Game.h"

#include <GLFW/glfw3.h>

#include "Log.h"
#include "utils/FileReader.h"

bool Game::Init()
{
    ENG::FileReader vertShader("assets/shaders/lab.vert");
    ENG::FileReader fragShader("assets/shaders/lab.frag");
    std::string     vertexShaderSource   = vertShader.readToString();
    std::string     fragmentShaderSource = fragShader.readToString();

    auto &graphicsAPI   = ENG::Engine::GetInstance().GetGraphicsAPI();
    auto  shaderProgram = graphicsAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    m_material.SetShaderProgram(shaderProgram);

    std::vector<float> vertices = {0.5F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, -0.5F, 0.5F,  0.0F, 0.0F, 1.0F, 0.0F,
                                   -0.5f, -0.5f, 0.0F, 0.0F, 0.0F, 1.0F, 0.5F,  -0.5f, 0.0f, 1.0f, 1.0f, 0.0f};

    std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

    ENG::VertexLayout vertexLayout;

    // Postion
    vertexLayout.elements.push_back({0, 3, GL_FLOAT, 0});
    // Color
    vertexLayout.elements.push_back({1, 3, GL_FLOAT, sizeof(float) * 3});
    vertexLayout.stride = sizeof(float) * 6;

    m_mesh = std::make_unique<ENG::Mesh>(vertexLayout, vertices, indices);

    return true;
}

void Game::Update(float deltaTime)
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

    if (input.IsKeyPressed(GLFW_KEY_Q)) {
        m_angle += 1.0F * deltaTime;
    }

    if (input.IsKeyPressed(GLFW_KEY_E)) {
        m_angle -= 1.0F * deltaTime;
    }

    m_material.SetParam("uOffset", m_offsetX, m_offsetY);
    m_material.SetParam("uAngle", m_angle);

    ENG::RenderCommand command;
    command.material = &m_material;
    command.mesh     = m_mesh.get();

    auto &renderQueue = ENG::Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}

void Game::Destroy() {}
