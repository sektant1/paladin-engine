#include "render/RenderQueue.h"

#include "Log.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"

namespace ENG
{

void RenderQueue::Submit(const RenderCommand &command)
{
    if (!command.material || !command.mesh) {
        LOG_WARN("RenderQueue::Submit received incomplete command (material=%p mesh=%p)",
                 (void *)command.material,
                 (void *)command.mesh);
    }
    m_commands.push_back(command);
}

void RenderQueue::Draw(GraphicsAPI &graphicsAPI, const CameraData &cameraData)
{
    for (auto &command : m_commands) {
        if (!command.material) {
            LOG_ERROR("RenderQueue::Draw skipping command with null material");
            continue;
        }
        graphicsAPI.BindMaterial(command.material);
        auto *program = command.material->GetShaderProgram();
        if (!program) {
            LOG_ERROR("RenderQueue::Draw material has no shader program");
            continue;
        }
        program->SetUniform("uModel", command.modelMatrix);
        program->SetUniform("uView", cameraData.viewMatrix);
        program->SetUniform("uProjection", cameraData.projectionMatrix);
        graphicsAPI.BindMesh(command.mesh);
        graphicsAPI.DrawMesh(command.mesh);
    }

    m_commands.clear();
}

}  // namespace ENG
