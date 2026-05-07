#include "render/SpriteRenderer.h"

#include <cstddef>
#include <filesystem>
#include <vector>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine.h"
#include "Log.h"
#include "graphics/Texture.h"
#include "io/FileSystem.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "imstb_truetype.h"

namespace mnd
{
namespace
{
GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char info[1024];
        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
        LOG_ERROR("SpriteRenderer shader compile failed: %s", info);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint LinkProgram(const char *vertexSource, const char *fragmentSource)
{
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vs == 0 || fs == 0)
    {
        if (vs != 0) glDeleteShader(vs);
        if (fs != 0) glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char info[1024];
        glGetProgramInfoLog(program, sizeof(info), nullptr, info);
        LOG_ERROR("SpriteRenderer shader link failed: %s", info);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}
}  // namespace

bool SpriteRenderer::Init()
{
    const char *vertexSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPosition;
        layout(location = 1) in vec2 aUV;
        layout(location = 2) in vec4 aColor;

        uniform mat4 uProjection;

        out vec2 vUV;
        out vec4 vColor;

        void main()
        {
            vUV = aUV;
            vColor = aColor;
            gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
        }
    )";

    const char *fragmentSource = R"(
        #version 330 core
        uniform sampler2D uTexture;
        uniform bool uRedOnly;

        in vec2 vUV;
        in vec4 vColor;
        out vec4 FragColor;

        void main()
        {
            vec4 tex = texture(uTexture, vUV);
            if (uRedOnly) {
                tex = vec4(1.0, 1.0, 1.0, tex.r);
            }
            FragColor = tex * vColor;
        }
    )";

    m_shader = LinkProgram(vertexSource, fragmentSource);
    if (m_shader == 0)
    {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, color)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    {
        const unsigned char whitePixel[4] = {255, 255, 255, 255};
        glGenTextures(1, &m_whiteTexture);
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_fontReady = LoadDefaultFont();
    if (!m_fontReady)
    {
        LOG_WARN("SpriteRenderer: default font unavailable; DrawText calls will be ignored");
    }
    return true;
}

void SpriteRenderer::Shutdown()
{
    if (m_whiteTexture != 0) glDeleteTextures(1, &m_whiteTexture);
    m_whiteTexture = 0;
    if (m_fontTexture != 0) glDeleteTextures(1, &m_fontTexture);
    if (m_vbo != 0) glDeleteBuffers(1, &m_vbo);
    if (m_vao != 0) glDeleteVertexArrays(1, &m_vao);
    if (m_shader != 0) glDeleteProgram(m_shader);
    m_fontTexture = 0;
    m_vbo = 0;
    m_vao = 0;
    m_shader = 0;
    m_fontReady = false;
    m_commands.clear();
}

bool SpriteRenderer::LoadDefaultFont()
{
    const auto cwd = std::filesystem::current_path();
    const auto exe = Engine::GetInstance().GetFileSystem().GetExecutableFolder();
    const std::filesystem::path relative = "engine/thirdparty/imgui/misc/fonts/ProggyClean.ttf";

    std::vector<std::filesystem::path> candidates = {
        cwd / relative,
        exe / relative,
        exe / ".." / ".." / relative,
    };

    std::vector<char> fontData;
    for (const auto &candidate : candidates)
    {
        if (std::filesystem::exists(candidate))
        {
            fontData = Engine::GetInstance().GetFileSystem().LoadFile(candidate);
            break;
        }
    }
    if (fontData.empty())
    {
        return false;
    }

    std::array<stbtt_bakedchar, 96> baked {};
    m_fontBitmap.fill(0);
    const int bottom = stbtt_BakeFontBitmap(reinterpret_cast<const unsigned char *>(fontData.data()),
                                            0,
                                            m_fontBakeHeight,
                                            m_fontBitmap.data(),
                                            512,
                                            512,
                                            32,
                                            static_cast<int>(baked.size()),
                                            baked.data());
    if (bottom <= 0)
    {
        LOG_ERROR("SpriteRenderer: failed to bake default font atlas");
        return false;
    }

    for (usize i = 0; i < baked.size(); ++i)
    {
        m_glyphs[i] = {
            static_cast<f32>(baked[i].x0),
            static_cast<f32>(baked[i].y0),
            static_cast<f32>(baked[i].x1),
            static_cast<f32>(baked[i].y1),
            baked[i].xoff,
            baked[i].yoff,
            baked[i].xadvance,
        };
    }

    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, m_fontBitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void SpriteRenderer::DrawSprite(Texture *texture, const vec2 &position, const vec2 &size, const vec4 &color)
{
    if (texture == nullptr || texture->GetID() == 0)
    {
        return;
    }
    QueueQuad(texture->GetID(), false, position, size, vec2(0.0F), vec2(1.0F), color);
}

void SpriteRenderer::DrawRect(const vec2 &position, const vec2 &size, const vec4 &color)
{
    if (m_whiteTexture == 0)
    {
        return;
    }
    QueueQuad(m_whiteTexture, false, position, size, vec2(0.0F), vec2(1.0F), color);
}

void SpriteRenderer::DrawSprite(const std::string &assetPath, const vec2 &position, const vec2 &size, const vec4 &color)
{
    auto texture = Engine::GetInstance().GetTextureManager().GetOrLoadTexture(assetPath);
    DrawSprite(texture.get(), position, size, color);
}

void SpriteRenderer::DrawText(const std::string &text, const vec2 &position, f32 pixelHeight, const vec4 &color)
{
    if (!m_fontReady || text.empty())
    {
        return;
    }

    const f32 scale = pixelHeight / m_fontBakeHeight;
    f32 x = position.x;
    f32 y = position.y;

    for (char c : text)
    {
        if (c == '\n')
        {
            x = position.x;
            y += pixelHeight;
            continue;
        }
        if (c < 32 || c > 127)
        {
            c = '?';
        }

        const auto &g = m_glyphs[static_cast<usize>(c - 32)];
        const vec2 glyphPos(x + g.xoff * scale, y + g.yoff * scale);
        const vec2 glyphSize((g.x1 - g.x0) * scale, (g.y1 - g.y0) * scale);
        const vec2 uvMin(g.x0 / 512.0F, g.y0 / 512.0F);
        const vec2 uvMax(g.x1 / 512.0F, g.y1 / 512.0F);
        QueueQuad(m_fontTexture, true, glyphPos, glyphSize, uvMin, uvMax, color);
        x += g.xadvance * scale;
    }
}

void SpriteRenderer::QueueQuad(GLuint texture,
                               bool redOnly,
                               const vec2 &position,
                               const vec2 &size,
                               const vec2 &uvMin,
                               const vec2 &uvMax,
                               const vec4 &color)
{
    if (texture == 0 || size.x == 0.0F || size.y == 0.0F)
    {
        return;
    }

    const f32 x0 = position.x;
    const f32 y0 = position.y;
    const f32 x1 = position.x + size.x;
    const f32 y1 = position.y + size.y;

    DrawCommand cmd;
    cmd.texture = texture;
    cmd.redOnly = redOnly;
    cmd.vertices = {
        Vertex{{x0, y0}, {uvMin.x, uvMin.y}, color},
        Vertex{{x1, y0}, {uvMax.x, uvMin.y}, color},
        Vertex{{x1, y1}, {uvMax.x, uvMax.y}, color},
        Vertex{{x0, y0}, {uvMin.x, uvMin.y}, color},
        Vertex{{x1, y1}, {uvMax.x, uvMax.y}, color},
        Vertex{{x0, y1}, {uvMin.x, uvMax.y}, color},
    };
    m_commands.push_back(cmd);
}

void SpriteRenderer::Flush(int viewportWidth, int viewportHeight)
{
    if (m_commands.empty() || m_shader == 0 || viewportWidth <= 0 || viewportHeight <= 0)
    {
        m_commands.clear();
        return;
    }

    GLboolean oldDepth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean oldBlend = glIsEnabled(GL_BLEND);
    GLint oldProgram = 0;
    GLint oldTexture = 0;
    GLint oldVao = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_shader);
    const mat4 projection = glm::ortho(0.0F, static_cast<f32>(viewportWidth), static_cast<f32>(viewportHeight), 0.0F);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjection"), 1, GL_FALSE, value_ptr(projection));
    glUniform1i(glGetUniformLocation(m_shader, "uTexture"), 0);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glActiveTexture(GL_TEXTURE0);

    for (const auto &cmd : m_commands)
    {
        glBindTexture(GL_TEXTURE_2D, cmd.texture);
        glUniform1i(glGetUniformLocation(m_shader, "uRedOnly"), cmd.redOnly ? 1 : 0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * cmd.vertices.size(), cmd.vertices.data());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cmd.vertices.size()));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(static_cast<GLuint>(oldVao));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture));
    glUseProgram(static_cast<GLuint>(oldProgram));
    if (oldDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (oldBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);

    m_commands.clear();
}

}  // namespace mnd
