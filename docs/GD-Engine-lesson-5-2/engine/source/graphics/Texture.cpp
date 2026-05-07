#include "graphics/Texture.h"
#include "Engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace eng
{
    Texture::Texture(int width, int height, int numChannels, unsigned char* data)
        : m_width(width), m_height(height), m_numChannels(numChannels)
    {
        Init(width, height, numChannels, data);
    }

    Texture::~Texture()
    {
        if (m_textureID > 0)
        {
            glDeleteTextures(1, &m_textureID);
        }
    }

    GLuint Texture::GetID() const
    {
        return m_textureID;
    }

    void Texture::Init(int width, int height, int numChannels, unsigned char* data)
    {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;

        if (numChannels == 4)
        {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    std::shared_ptr<Texture> Texture::Load(const std::string& path)
    {
        int width, height, numChannels;

        auto& fs = Engine::GetInstance().GetFileSystem();
        auto fullPath = fs.GetAssetsFolder() / path;

        if (!std::filesystem::exists(fullPath))
        {
            return nullptr;
        }

        std::shared_ptr<Texture> result;

        unsigned char* data = stbi_load(fullPath.string().c_str(), &width, &height, &numChannels, 0);

        if (data)
        {
            result = std::make_shared<Texture>(width, height, numChannels, data);
            stbi_image_free(data);
        }

        return result;
    }

    std::shared_ptr<Texture> TextureManager::GetOrLoadTexture(const std::string& path)
    {
        auto it = m_textures.find(path);
        if (it != m_textures.end())
        {
            return it->second;
        }

        auto texture = Texture::Load(path);
        m_textures[path] = texture;
        return texture;
    }
}