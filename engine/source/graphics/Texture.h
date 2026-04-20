#pragma once

#include "graphics/GraphicsAPI.h"

namespace ENG
{
class Texture
{
public:
    Texture(int width, int height, int numChannels, unsigned char *data);
    ~Texture();

    [[nodiscard]] GLuint GetID() const;

private:
    int    m_width       = 0;
    int    m_height      = 0;
    int    m_numChannels = 0;
    GLuint m_textureID   = 0;
};
}  // namespace ENG
