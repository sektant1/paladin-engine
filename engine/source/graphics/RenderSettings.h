/**
 * @file RenderSettings.h
 * @ingroup mnd_graphics
 * @brief Per-frame renderer tuning knobs (PSX-style pixelation, fog, ambient).
 *
 * One instance lives on mnd::Engine and is forwarded to the
 * mnd::RenderQueue and PostFX shader as uniforms. Tweak it from the
 * Editor's "Render" panel or imperatively at runtime to switch between
 * crisp full-resolution and a coarse internal-resolution pixelated look.
 */

#pragma once

#include "Constants.h"
#include "Types.h"

namespace mnd
{

/**
 * @ingroup mnd_graphics
 * @brief Renderer tuning struct passed to the post-process shader each frame.
 */
/// Which scene-target attachment the final blit shows. Used to verify
/// the MRT G-buffer is producing sane normals/depth before the outline
/// post-pass is wired in. Color = production view.
enum class DebugView : int
{
    Color  = 0,
    Normal = 1,
    Depth  = 2,
};

struct RenderSettings
{
    /// When true the scene renders to an internal-resolution FBO and is
    /// then nearest-blit upscaled to the window — gives a chunky, pixely look.
    bool useInternalRes = true;

    /// Which scene-target attachment to blit. Defaults to Color.
    DebugView debugView = DebugView::Color;

    /// When true, run the outline post-pass between the scene draw and the final blit.
    /// Ignored when useInternalRes is false (the post-pass needs the MRT scene target).
    bool useOutline = true;

    /// Window-pixels per scene-pixel. 1 = native resolution; higher values
    /// downsample more aggressively for a chunkier look. Drives the internal
    /// target size each frame as window/pixelSize.
    int pixelSize = 3;

    /// Resolved each frame from window size / pixelSize. Editor reads them
    /// for display only; setting them directly has no effect.
    int internalW = kDefaultInternalWidth;
    int internalH = kDefaultInternalHeight;

    /// RGBA value used by `glClear` at the start of each frame.
    vec4 clearColor = vec4(0.0F, 0.0F, 0.0F, 1.0F);

    f32  snapX          = 320.0F;          ///< Vertex snapping grid X (PS1 wobble).
    f32  snapY          = 240.0F;          ///< Vertex snapping grid Y.
    f32  fogStart       = 0.0F;            ///< Fog near distance (0 disables fog).
    f32  fogEnd         = 0.0F;            ///< Fog far distance.
    f32  ambient        = kDefaultAmbient; ///< Global ambient term added to lit pixels.
    vec3 lightDir       = vec3(0.0F, -1.0F, 0.0F); ///< Sun/directional light vector.
    f32  colorDepth     = 0.0F;  ///< levels per channel; <= 1 disables quantize
    f32  ditherStrength = 0.0F;  ///< Bayer dither amount applied after quantization.

    bool showFps = true;  ///< Draw FPS counter overlay in top-left corner.
};

}  // namespace mnd
