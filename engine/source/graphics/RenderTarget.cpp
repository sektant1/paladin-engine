#include "graphics/RenderTarget.h"

#include "Constants.h"
#include "Log.h"

namespace mnd
{

namespace
{
const char *FramebufferStatusName(GLenum status)
{
    switch (status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            return "complete";
        case GL_FRAMEBUFFER_UNDEFINED:
            return "undefined";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "incomplete attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "missing attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "incomplete draw buffer";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "incomplete read buffer";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "unsupported";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "incomplete multisample";
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            return "incomplete layer targets";
        default:
            return "unknown";
    }
}
}  // namespace

bool RenderTarget::Create(int w, int h)
{
    if (w <= 0 || h <= 0)
    {
        LOG_ERROR("RenderTarget::Create called with invalid size %dx%d", w, h);
        return false;
    }
    Destroy();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // ---- Colour attachment 0: shaded RGBA8 ------------------------------
    glGenTextures(1, &m_color);
    glBindTexture(GL_TEXTURE_2D, m_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color, 0);

    // ---- Colour attachment 1: view-space normals (N*0.5+0.5) ------------
    glGenTextures(1, &m_normal);
    glBindTexture(GL_TEXTURE_2D, m_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normal, 0);

    // ---- Depth attachment: sampleable 24-bit texture --------------------
    glGenTextures(1, &m_depth);
    glBindTexture(GL_TEXTURE_2D, m_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Sampling depth as a normal float, not a shadow comparison.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);

    const GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, bufs);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("RenderTarget FBO incomplete (%s, 0x%x) at %dx%d",
                  FramebufferStatusName(status),
                  status,
                  w,
                  h);
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
    if (m_color  != 0) glDeleteTextures(1, &m_color);
    if (m_normal != 0) glDeleteTextures(1, &m_normal);
    if (m_depth  != 0) glDeleteTextures(1, &m_depth);
    if (m_fbo    != 0) glDeleteFramebuffers(1, &m_fbo);
    m_color = m_normal = m_depth = m_fbo = 0;
    m_w = m_h = 0;
}

void RenderTarget::Bind()
{
    if (!IsValid())
    {
        LOG_ERROR("RenderTarget::Bind called before a valid FBO was created");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Tell GL we're writing to both attachments. Shaders that only declare
    // one fragment output (basic/lab/psx etc.) leave attachment 1 at the
    // clear value — that's fine, those objects just won't generate normal
    // edges in the post pass.
    const GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, bufs);
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
GLint  gBlitModeLoc = -1;
GLint  gBlitTexLoc  = -1;

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
uniform int uMode;          // 0 = rgb, 1 = splat .r (depth), 2 = packed normal
void main()
{
    vec4 s = texture(uTex, vUV);
    if (uMode == 1)
    {
        // Push depth contrast — raw depth is near-1.0 for most scenes
        // and looks flat-white. Bias/scale gives a usable preview.
        float d = s.r;
        float v = clamp((d - 0.95) * 20.0, 0.0, 1.0);
        oColor = vec4(v, v, v, 1.0);
    }
    else if (uMode == 2)
    {
        oColor = vec4(s.rgb, 1.0);
    }
    else
    {
        oColor = vec4(s.rgb, 1.0);
    }
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

    gBlitTexLoc  = glGetUniformLocation(gBlitProg, "uTex");
    gBlitModeLoc = glGetUniformLocation(gBlitProg, "uMode");

    glGenVertexArrays(1, &gBlitVao);
}
}  // namespace

void BlitNearest(GLuint srcTexture, int dstW, int dstH, BlitMode mode)
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
    if (gBlitTexLoc  >= 0) glUniform1i(gBlitTexLoc,  0);
    if (gBlitModeLoc >= 0) glUniform1i(gBlitModeLoc, static_cast<int>(mode));

    glBindVertexArray(gBlitVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

}  // namespace mnd
