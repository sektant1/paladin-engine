#include <memory>

#include "TestObject.h"

#include "GLFW/glfw3.h"
#include "render/Material.h"
#include "scene/components/MeshComponent.h"

TestObject::TestObject()
{
    LOG_INFO("TestObject constructing");

    // load shaders
    ENG::FileReader vertShader("assets/shaders/lab.vert");
    ENG::FileReader fragShader("assets/shaders/lab.frag");
    std::string     vertexShaderSource   = vertShader.ReadToString();
    std::string     fragmentShaderSource = fragShader.ReadToString();

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        LOG_ERROR("TestObject shader source is empty (vert=%zu frag=%zu bytes)",
                  vertexShaderSource.size(),
                  fragmentShaderSource.size());
    }

    auto &graphicsAPI   = ENG::Engine::GetInstance().GetGraphicsAPI();
    auto  shaderProgram = graphicsAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (!shaderProgram) {
        LOG_ERROR("TestObject failed to create shader program");
    }

    auto material = std::make_shared<ENG::Material>();
    material->SetShaderProgram(shaderProgram);

    auto mesh_data = ENG::Builder::CreateRectangle(1.0F, 1.0F);
    auto mesh      = mesh_data.buildMesh();

    AddComponent(new ENG::MeshComponent(material, mesh));
    LOG_INFO("TestObject constructed");
}

void TestObject::Update(ENG::f32 deltaTime)
{
    ENG::GameObject::Update(deltaTime);

    auto position = GetPosition();

    auto &input = ENG::Engine::GetInstance().GetInputManager();
    if (input.IsKeyPressed(GLFW_KEY_W)) {
        position.y += 1.0F * deltaTime;
    }
    if (input.IsKeyPressed(GLFW_KEY_A)) {
        position.x -= 1.0F * deltaTime;
    }
    if (input.IsKeyPressed(GLFW_KEY_S)) {
        position.y -= 1.0F * deltaTime;
    }
    if (input.IsKeyPressed(GLFW_KEY_D)) {
        position.x += 1.0F * deltaTime;
    }

    SetPosition(position);
}
