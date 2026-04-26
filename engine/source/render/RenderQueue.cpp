#include "render/RenderQueue.h"

#include "Common.h"
#include "Engine.h"
#include "Log.h"
#include "editor/Editor.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/RenderSettings.h"
#include "graphics/ShaderProgram.h"
#include "render/Material.h"
#include "render/Mesh.h"

namespace mnd
{

void RenderQueue::Submit(const RenderCommand &command)
{
    if (!command.material || !command.mesh)
    {
        LOG_WARN(
            "RenderQueue::Submit received incomplete command (material=%p mesh=%p)",
            (void *)command.material,
            (void *)command.mesh
        );
    }
    m_commands.push_back(command);
}

void RenderQueue::Draw(GraphicsAPI &graphicsAPI, const CameraData &cameraData, const std::vector<LightData> &lights)
{
    const RenderSettings &rs    = Engine::GetInstance().GetRenderSettings();
    int                   drawn = 0;

    for (auto &command : m_commands)
    {
        if (!command.material)
        {
            LOG_ERROR("RenderQueue::Draw skipping command with null material");
            continue;
        }
        graphicsAPI.BindMaterial(command.material);
        auto *program = command.material->GetShaderProgram();
        if (!program)
        {
            LOG_ERROR("RenderQueue::Draw material has no shader program");
            continue;
        }
        program->SetUniform("uModel", command.modelMatrix);
        program->SetUniform("uView", cameraData.viewMatrix);
        program->SetUniform("uProjection", cameraData.projectionMatrix);
        program->SetUniform("uCameraPos", cameraData.position);
        if (!lights.empty())
        {
            auto &light = lights[0];
            program->SetUniform("uLight.color", light.color);
            program->SetUniform("uLight.direction", normalize(-light.position));
        }

        // Editor-controlled PSX shader uniforms. ShaderProgram::SetUniform is a no-op
        // for shaders that don't declare the uniform, so this is safe across materials.
        // program->SetUniform("uSnapResolutionX", rs.snapX);
        // program->SetUniform("uSnapResolutionY", rs.snapY);
        // program->SetUniform("uFogStart", rs.fogStart);
        // program->SetUniform("uFogEnd", rs.fogEnd);
        // program->SetUniform("uAmbient", rs.ambient);
        // program->SetUniform("uLightDir", rs.lightDir);
        // program->SetUniform("uColorDepth", rs.colorDepth);
        // program->SetUniform("uDitherStrength", rs.ditherStrength);

        graphicsAPI.BindMesh(command.mesh);
        graphicsAPI.DrawMesh(command.mesh);
        graphicsAPI.UnbindMesh(command.mesh);
        ++drawn;
    }

    m_commands.clear();
    Engine::GetInstance().GetEditor().NotifyDrawCount(drawn);
}

}  // namespace mnd
