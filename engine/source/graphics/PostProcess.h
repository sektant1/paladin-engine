/**
 * @file PostProcess.h
 * @ingroup mnd_graphics
 * @brief Three.js-style pixel-art post-pass.
 *
 * Reads the scene RenderTarget's three attachments (colour, view-normals,
 * depth), computes depth-edge and normal-edge indicators a la
 * threejs.org/examples/webgl_postprocessing_pixel.html, and writes the
 * result into its own colour FBO for the final BlitNearest upscale.
 *
 * For the current full-resolution RenderTarget path, pixelation is done in
 * this pass by snapping colour/depth/normal samples to RenderSettings::pixelSize
 * screen-pixel cells before applying the optional edge term.
 */

#pragma once

#include "GL/glew.h"

namespace mnd
{

class RenderTarget;
struct CameraData;

class PostProcess
{
public:
    bool Init();
    void Destroy();

    /// Lazily resize the internal output FBO to match the scene target.
    void Resize(int w, int h);

    /// Run the outline pass, sampling `scene` and writing to OutputTex().
    void RunOutline(const RenderTarget &scene, const CameraData &cam);

    GLuint OutputTex() const { return m_color; }
    bool   IsValid() const { return m_program != 0 && m_vao != 0; }

    // Editor-tunable. Both 0 = passthrough (no edges).
    float depthEdgeStrength  = 0.4F;
    float normalEdgeStrength = 0.3F;

private:
    bool CreateFBO(int w, int h);
    void DestroyFBO();

    GLuint m_program = 0;
    GLuint m_vao     = 0;

    GLuint m_fbo   = 0;
    GLuint m_color = 0;
    int    m_w     = 0;
    int    m_h     = 0;

    int m_locColor       = -1;
    int m_locNormal      = -1;
    int m_locDepth       = -1;
    int m_locTexel       = -1;
    int m_locDepthStr    = -1;
    int m_locNormalStr   = -1;
    int m_locPixelSize   = -1;
};

}  // namespace mnd
