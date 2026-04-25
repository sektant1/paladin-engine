#include "graphics/RenderTarget.h"

#include "Constants.h"
#include "Log.h"

namespace COA
{

bool RenderTarget::Create(int w, int h)
{
    if (w <= 0 || h <= 0)
    {
        return false;
    }
    Destroy();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_color);
    glBindTexture(GL_TEXTURE_2D, m_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color, 0);

    glGenRenderbuffers(1, &m_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("RenderTarget FBO incomplete (0x%x) at %dx%d", status, w, h);
        Destroy();
        return false;
    }

    m_w = w;
    m_h = h;
    return true;
}

void RenderTarget::Resize(int w, int h)
{
    if (w == m_w && h == m_h && m_fbo != 0)
    {
        return;
    }
    Create(w, h);
}

void RenderTarget::Destroy()
{
    if (m_color != 0) glDeleteTextures(1, &m_color);
    if (m_depth != 0) glDeleteRenderbuffers(1, &m_depth);
    if (m_fbo   != 0) glDeleteFramebuffers(1, &m_fbo);
    m_color = m_depth = m_fbo = 0;
    m_w = m_h = 0;
}

void RenderTarget::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_w, m_h);
}

void RenderTarget::BindDefault(int winW, int winH)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winW, winH);
}

// ---- Nearest blit (vertex-less fullscreen triangle) -------------------

namespace
{
GLuint gBlitProg = 0;
GLuint gBlitVao  = 0;

const char *kBlitVS = R"(#version 330 core
out vec2 vUV;
void main()
{
    vec2 p = vec2((gl_VertexID == 1) ? 3.0 : -1.0,
                  (gl_VertexID == 2) ? 3.0 : -1.0);
    vUV = (p + 1.0) * 0.5;
    gl_Position = vec4(p, 0.0, 1.0);
}
)";

const char *kBlitFS = R"(#version 330 core
in  vec2 vUV;
out vec4 oColor;
uniform sampler2D uTex;
void main()
{
    oColor = texture(uTex, vUV);
}
)";

GLuint CompileStage(GLenum stage, const char *src)
{
    GLuint s = glCreateShader(stage);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (ok == 0)
    {
        char log[kShaderInfoLogSize];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR("Blit shader compile failed: %s", log);
    }
    return s;
}

void EnsureBlitResources()
{
    if (gBlitProg != 0)
    {
        return;
    }
    GLuint vs = CompileStage(GL_VERTEX_SHADER, kBlitVS);
    GLuint fs = CompileStage(GL_FRAGMENT_SHADER, kBlitFS);
    gBlitProg = glCreateProgram();
    glAttachShader(gBlitProg, vs);
    glAttachShader(gBlitProg, fs);
    glLinkProgram(gBlitProg);
    GLint ok = 0;
    glGetProgramiv(gBlitProg, GL_LINK_STATUS, &ok);
    if (ok == 0)
    {
        char log[kShaderInfoLogSize];
        glGetProgramInfoLog(gBlitProg, sizeof(log), nullptr, log);
        LOG_ERROR("Blit program link failed: %s", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &gBlitVao);
}
}  // namespace

void BlitNearest(GLuint srcTexture, int dstW, int dstH)
{
    EnsureBlitResources();
    if (gBlitProg == 0)
    {
        return;
    }

    glViewport(0, 0, dstW, dstH);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glUseProgram(gBlitProg);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    GLint loc = glGetUniformLocation(gBlitProg, "uTex");
    if (loc >= 0)
    {
        glUniform1i(loc, 0);
    }

    glBindVertexArray(gBlitVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

}  // namespace COA
