#include "render/RenderQueue.h"
#include "render/Mesh.h"
#include "render/Material.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"

namespace eng
{
    void RenderQueue::Submit(const RenderCommand& command)
    {
        m_commands.push_back(command);
    }

    void RenderQueue::Draw(GraphicsAPI& graphicsAPI, const CameraData& cameraData)
    {
        for (auto& command : m_commands)
        {
            graphicsAPI.BindMaterial(command.material);
            auto shaderProgram = command.material->GetShaderProgram();
            shaderProgram->SetUniform("uModel", command.modelMatrix);
            shaderProgram->SetUniform("uView", cameraData.viewMatrix);
            shaderProgram->SetUniform("uProjection", cameraData.projectionMatrix);
            graphicsAPI.BindMesh(command.mesh);
            graphicsAPI.DrawMesh(command.mesh);
        }

        m_commands.clear();
    }
}