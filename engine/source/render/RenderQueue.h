#pragma once

#include <vector>

#include "Types.h"

namespace ENG
{
class Mesh;
class Material;
class GraphicsAPI;

struct RenderCommand
{
    Mesh     *mesh     = nullptr;
    Material *material = nullptr;
    mat4      modelMatrix;
};

struct CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

class RenderQueue
{
public:
    void Submit(const RenderCommand &command);
    void Draw(GraphicsAPI &graphicsAPI, const CameraData &cameraData);

private:
    std::vector<RenderCommand> m_commands;
};
}  // namespace ENG
