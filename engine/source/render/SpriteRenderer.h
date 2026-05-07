#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Types.h"
#include "graphics/GLForward.h"

namespace mnd
{

class Texture;

class SpriteRenderer
{
public:
    bool Init();
    void Shutdown();

    void DrawSprite(Texture *texture, const vec2 &position, const vec2 &size, const vec4 &color = vec4(1.0F));
    void DrawSprite(const std::string &assetPath, const vec2 &position, const vec2 &size, const vec4 &color = vec4(1.0F));
    void DrawRect(const vec2 &position, const vec2 &size, const vec4 &color);
    void DrawText(const std::string &text, const vec2 &position, f32 pixelHeight, const vec4 &color = vec4(1.0F));

    void Flush(int viewportWidth, int viewportHeight);

private:
    struct Vertex
    {
        vec2 position;
        vec2 uv;
        vec4 color;
    };

    struct DrawCommand
    {
        GLuint texture = 0;
        bool   redOnly = false;
        std::array<Vertex, 6> vertices {};
    };

    struct Glyph
    {
        f32 x0 = 0.0F;
        f32 y0 = 0.0F;
        f32 x1 = 0.0F;
        f32 y1 = 0.0F;
        f32 xoff = 0.0F;
        f32 yoff = 0.0F;
        f32 xadvance = 0.0F;
    };

    bool LoadDefaultFont();
    void QueueQuad(GLuint texture,
                   bool redOnly,
                   const vec2 &position,
                   const vec2 &size,
                   const vec2 &uvMin,
                   const vec2 &uvMax,
                   const vec4 &color);

    GLuint m_shader = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_fontTexture = 0;
    GLuint m_whiteTexture = 0;

    std::array<unsigned char, 512 * 512> m_fontBitmap {};
    std::array<Glyph, 96>                m_glyphs {};
    f32                                  m_fontBakeHeight = 32.0F;
    bool                                 m_fontReady = false;
    std::vector<DrawCommand>             m_commands;
};

}  // namespace mnd
