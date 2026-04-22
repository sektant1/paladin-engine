#pragma once

#include <memory>
#include <unordered_map>

#include "graphics/GraphicsAPI.h"

namespace ENG
{
class Texture
{
public:
    Texture(int width, int height, int numChannels, unsigned char *data);
    ~Texture();

    [[nodiscard]] GLuint GetID() const;

    void Init(int width, int height, int numChannels, unsigned char *data);

    static std::shared_ptr<Texture> Load(const std::string &path);

private:
    int    m_width       = 0;
    int    m_height      = 0;
    int    m_numChannels = 0;
    GLuint m_textureID   = 0;
};

class TextureManager
{
public:
    std::shared_ptr<Texture> GetOrLoadTexture(const std::string &path);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
}  // namespace ENG
