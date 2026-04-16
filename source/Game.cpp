#include <iostream>

#include "Game.h"

#include <GLFW/glfw3.h>

bool Game::Init()
{
    std::string vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;

        out vec3 vColor;

        uniform vec2 uOffset;

        void main()
        {
            vColor = color;
            gl_Position = vec4(position.x + uOffset.x, position.y + uOffset.y, position.z, 1.0);
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 vColor;

        void main()
        {
            FragColor = vec4(vColor, 1.0);
        }
    )";

    auto &graphicsAPI   = ENG::Engine::GetInstance().GetGraphicsAPI();
    auto  shaderProgram = graphicsAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    m_material.SetShaderProgram(shaderProgram);

    std::vector<float> vertices = {0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, 0.0f,
                                   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, 0.0f};

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
        std::cout << "[A] button is pressed" << std::endl;
        m_offsetX -= 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_D)) {
        std::cout << "[D] button is pressed" << std::endl;
        m_offsetX += 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_W)) {
        std::cout << "[W] button is pressed" << std::endl;
        m_offsetY += 0.01F;
    }

    if (input.IsKeyPressed(GLFW_KEY_S)) {
        std::cout << "[S] button is pressed" << std::endl;
        m_offsetY -= 0.01F;
    }

    m_material.SetParam("uOffset", m_offsetX, m_offsetY);

    ENG::RenderCommand command;
    command.material = &m_material;
    command.mesh     = m_mesh.get();

    auto &renderQueue = ENG::Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}

void Game::Destroy() {}
