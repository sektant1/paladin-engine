#pragma once

#include "GL/glew.h"

namespace COA
{

/// Offscreen color+depth framebuffer used for low-res "internal resolution" rendering.
/// Call Bind() to redirect subsequent draws; then BlitNearest() to upscale into the
/// default framebuffer with nearest-neighbor sampling for a pixelated look.
class RenderTarget
{
public:
    bool Create(int w, int h);
    void Resize(int w, int h);
    void Destroy();

    void        Bind();
    static void BindDefault(int winW, int winH);

    GLuint ColorTex() const { return m_color; }
    int    Width() const { return m_w; }
    int    Height() const { return m_h; }
    bool   IsValid() const { return m_fbo != 0; }

private:
    GLuint m_fbo   = 0;
    GLuint m_color = 0;
    GLuint m_depth = 0;
    int    m_w     = 0;
    int    m_h     = 0;
};

/// Fullscreen nearest-neighbor blit of a 2D texture into the currently bound framebuffer.
/// Lazily compiles its own vertex-less shader on first call. Safe to call every frame.
void BlitNearest(GLuint srcTexture, int dstW, int dstH);

}  // namespace COA
