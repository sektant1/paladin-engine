#pragma once
#include <vector>
#include <glm/mat4x4.hpp>

namespace eng
{
    class Mesh;
    class Material;
    class GraphicsAPI;

    struct RenderCommand
    {
        Mesh* mesh = nullptr;
        Material* material = nullptr;
        glm::mat4 modelMatrix;
    };

    struct CameraData
    {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    class RenderQueue
    {
    public:
        void Submit(const RenderCommand& command);
        void Draw(GraphicsAPI& graphicsAPI, const CameraData& cameraData);

    private:
        std::vector<RenderCommand> m_commands;
    };
}