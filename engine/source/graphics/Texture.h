/**
 * @file Texture.h
 * @ingroup coa_graphics
 * @brief 2D texture resource and a path-keyed cache (TextureManager).
 *
 * ## Loading a texture
 * Prefer TextureManager::GetOrLoadTexture() over Texture::Load() directly —
 * the manager returns the same shared_ptr for repeated requests to the same
 * path, avoiding redundant GPU uploads.
 *
 * @code
 *   auto tex = Engine::GetInstance().GetTextureManager()
 *                      .GetOrLoadTexture("assets/textures/wall.png");
 *   material->SetParam("uDiffuse", tex);
 * @endcode
 *
 * Textures are uploaded to the GPU in Texture::Init() using stb_image for
 * decoding. The GL texture object is deleted in the destructor.
 *
 * @see Material::SetParam, TextureManager, ShaderProgram::SetTexture
 */

#pragma once

#include <memory>
#include <unordered_map>

#include "graphics/GraphicsAPI.h"

namespace COA
{

/**
 * @brief 2D OpenGL texture object wrapping a GL_TEXTURE_2D handle.
 *
 * Loaded from disk via stb_image. Once uploaded to the GPU the CPU pixel
 * data is freed. The GL handle lives until the Texture is destroyed.
 */
class Texture
{
public:
    /**
     * @brief Upload pixel data to the GPU immediately.
     * @param width       Image width in pixels.
     * @param height      Image height in pixels.
     * @param numChannels Number of channels (1 = greyscale, 3 = RGB, 4 = RGBA).
     * @param data        Raw pixel bytes (stb_image output). May be freed after this call.
     */
    Texture(int width, int height, int numChannels, unsigned char *data);

    /// Calls glDeleteTextures() to release the GPU handle.
    ~Texture();

    /// Returns the underlying OpenGL texture handle (GL_TEXTURE_2D target).
    [[nodiscard]] GLuint GetID() const;

    /**
     * @brief (Re-)initialise the texture with new pixel data.
     *        Called by the constructor; can also re-upload changed data.
     */
    void Init(int width, int height, int numChannels, unsigned char *data);

    /**
     * @brief Load a texture from a file path using stb_image.
     * @param path Filesystem path to a PNG/JPG/BMP/TGA image.
     * @return Shared pointer to the loaded Texture, or nullptr on failure.
     */
    static std::shared_ptr<Texture> Load(const std::string &path);

private:
    int    m_width       = 0;  ///< Image width in pixels.
    int    m_height      = 0;  ///< Image height in pixels.
    int    m_numChannels = 0;  ///< Channel count returned by stb_image.
    GLuint m_textureID   = 0;  ///< GL texture object handle.
};

/**
 * @brief Path-keyed cache that prevents the same image from being uploaded twice.
 *
 * Owned by the Engine singleton (Engine::GetTextureManager()).  Material setup
 * code should always go through this manager rather than Texture::Load()
 * directly, so the same GL texture is reused across multiple materials.
 */
class TextureManager
{
public:
    /**
     * @brief Return a cached texture, loading it from disk on first request.
     * @param path Filesystem path used as the cache key.
     * @return Shared pointer to the texture (never nullptr if the file exists).
     */
    std::shared_ptr<Texture> GetOrLoadTexture(const std::string &path);

private:
    /// In-memory cache: path → shared Texture instance.
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

}  // namespace COA
