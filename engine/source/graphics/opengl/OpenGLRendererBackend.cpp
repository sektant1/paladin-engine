#include "graphics/opengl/OpenGLRendererBackend.h"

#include "graphics/GraphicsAPI.h"

namespace mnd
{

OpenGLRendererBackend::OpenGLRendererBackend(GraphicsAPI &graphicsAPI)
    : m_graphicsAPI(graphicsAPI)
{
}

bool OpenGLRendererBackend::Init()
{
    return m_graphicsAPI.Init();
}

std::unique_ptr<RendererBackend> CreateOpenGLRendererBackend(GraphicsAPI &graphicsAPI)
{
    return std::make_unique<OpenGLRendererBackend>(graphicsAPI);
}

}  // namespace mnd
