#pragma once

#include <memory>

#include "graphics/RendererBackend.h"

namespace mnd
{

class GraphicsAPI;

class OpenGLRendererBackend final : public RendererBackend
{
public:
    explicit OpenGLRendererBackend(GraphicsAPI &graphicsAPI);

    bool Init() override;

private:
    GraphicsAPI &m_graphicsAPI;
};

std::unique_ptr<RendererBackend> CreateOpenGLRendererBackend(GraphicsAPI &graphicsAPI);

}  // namespace mnd
