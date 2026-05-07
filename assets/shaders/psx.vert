#version 330 core

// ============================================================
//  psx.vert — PSX / N64 style vertex shader
//  Adapted for coagula-engine:
//    - Attribute layout matches engine (pos/uv/normal/color)
//    - Matrix uniforms auto-set by RenderQueue
//    - All tuning uniforms optional (safe defaults when unset)
// ============================================================

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aColor;

// Auto-set by RenderQueue
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Retro controls (optional; zero = disabled/default)
uniform float uSnapResolutionX; // virtual grid width, e.g. 320.  <= 0 = off
uniform float uSnapResolutionY; // virtual grid height, e.g. 240
uniform float uFogStart;
uniform float uFogEnd;          // <= 0 disables fog
uniform vec3  uLightDir;        // world-space direction light travels. Zero = no dir light
uniform float uAmbient;         // 0 -> defaults to 0.35

noperspective out vec2 vTexCoord; // affine UVs = PSX wobble
out vec4  vColor;
out float vFog;
out float vLight;
out vec3  vViewNormal;

void main()
{
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vec4 viewPos  = uView  * worldPos;
    vec4 clipPos  = uProjection * viewPos;

    // --- Vertex snapping (GTE fixed-point emulation) ----------------
    if (uSnapResolutionX > 0.0 && uSnapResolutionY > 0.0)
    {
        vec3 ndc = clipPos.xyz / clipPos.w;
        vec2 grid = vec2(uSnapResolutionX, uSnapResolutionY) * 0.5;
        ndc.xy = floor(ndc.xy * grid + 0.5) / grid;
        clipPos.xyz = ndc * clipPos.w;
    }
    gl_Position = clipPos;

    vTexCoord = aTexCoord;
    vColor    = aColor;

    // --- Gouraud directional light ----------------------------------
    vec3 N = normalize(mat3(uModel) * aNormal);
    vViewNormal = normalize(mat3(uView) * N);
    float ambient = (uAmbient > 0.0) ? uAmbient : 0.35;
    float ndl = 0.0;
    if (dot(uLightDir, uLightDir) > 0.0)
    {
        vec3 L = normalize(-uLightDir);
        ndl = max(dot(N, L), 0.0);
    }
    vLight = clamp(ambient + (1.0 - ambient) * ndl, 0.0, 1.0);

    // --- Linear fog (disabled when uFogEnd <= 0) --------------------
    if (uFogEnd > 0.0)
    {
        float dist = length(viewPos.xyz);
        vFog = clamp((uFogEnd - dist) / max(uFogEnd - uFogStart, 1e-4), 0.0, 1.0);
    }
    else
    {
        vFog = 1.0;
    }
}
