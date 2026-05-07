#version 330 core

// Pixelation + outline pass inspired by the Three.js pixelation example.
// Reference: https://threejs.org/examples/webgl_postprocessing_pixel.html
//
// Scene is rendered at full resolution; pixelation happens HERE by
// snapping every sample UV to a `uPixelSize`-pixel grid. Edge taps
// step by uPixelSize as well, so edges land on chunky-pixel boundaries
// without changing the rest of the engine.

in  vec2 vUV;
out vec4 oColor;

uniform sampler2D uColor;
uniform sampler2D uNormal;   // RGB = N*0.5+0.5
uniform sampler2D uDepth;
uniform vec2      uTexel;    // 1 / scene-target resolution (full window)

uniform float uDepthEdgeStrength;
uniform float uNormalEdgeStrength;
uniform float uPixelSize;    // window-pixels per scene-pixel; >= 1

vec2 SnappedUV()
{
    float pixelSize = max(uPixelSize, 1.0);
    vec2 snappedPx = floor(gl_FragCoord.xy / pixelSize) * pixelSize + vec2(0.5 * pixelSize);
    return clamp(snappedPx * uTexel, uTexel * 0.5, 1.0 - uTexel * 0.5);
}

float SampleDepth(vec2 baseUV, int dx, int dy)
{
    vec2 step = uTexel * max(uPixelSize, 1.0);
    return texture(uDepth, baseUV + vec2(float(dx), float(dy)) * step).r;
}

vec3 SampleNormal(vec2 baseUV, int dx, int dy)
{
    vec2 step = uTexel * max(uPixelSize, 1.0);
    return texture(uNormal, baseUV + vec2(float(dx), float(dy)) * step).rgb * 2.0 - 1.0;
}

float DepthEdgeIndicator(vec2 baseUV, float depth)
{
    float diff = 0.0;
    diff += clamp(SampleDepth(baseUV,  1,  0) - depth, 0.0, 1.0);
    diff += clamp(SampleDepth(baseUV, -1,  0) - depth, 0.0, 1.0);
    diff += clamp(SampleDepth(baseUV,  0,  1) - depth, 0.0, 1.0);
    diff += clamp(SampleDepth(baseUV,  0, -1) - depth, 0.0, 1.0);
    return floor(smoothstep(0.01, 0.02, diff) * 2.0) / 2.0;
}

float NeighborNormalEdgeIndicator(vec2 baseUV, int dx, int dy, float depth, vec3 normal)
{
    float depthDiff    = SampleDepth(baseUV, dx, dy) - depth;
    vec3  neighborNorm = SampleNormal(baseUV, dx, dy);

    vec3  normalEdgeBias = vec3(1.0, 1.0, 1.0);
    float normalDiff     = dot(normal - neighborNorm, normalEdgeBias);
    float normalIndicator = clamp(smoothstep(-0.01, 0.01, normalDiff), 0.0, 1.0);
    float depthIndicator  = clamp(sign(depthDiff * 0.25 + 0.0025), 0.0, 1.0);

    return (1.0 - dot(normal, neighborNorm)) * depthIndicator * normalIndicator;
}

float NormalEdgeIndicator(vec2 baseUV, float depth, vec3 normal)
{
    float indicator = 0.0;
    indicator += NeighborNormalEdgeIndicator(baseUV,  0, -1, depth, normal);
    indicator += NeighborNormalEdgeIndicator(baseUV,  0,  1, depth, normal);
    indicator += NeighborNormalEdgeIndicator(baseUV, -1,  0, depth, normal);
    indicator += NeighborNormalEdgeIndicator(baseUV,  1,  0, depth, normal);
    return step(0.1, indicator);
}

void main()
{
    vec2 uv    = SnappedUV();
    vec4 texel = texture(uColor, uv);

    float depth  = 0.0;
    vec3  normal = vec3(0.0);
    if (uDepthEdgeStrength > 0.0 || uNormalEdgeStrength > 0.0)
    {
        depth  = SampleDepth(uv, 0, 0);
        normal = SampleNormal(uv, 0, 0);
    }

    float dei = (uDepthEdgeStrength  > 0.0) ? DepthEdgeIndicator(uv, depth)         : 0.0;
    float nei = (uNormalEdgeStrength > 0.0) ? NormalEdgeIndicator(uv, depth, normal): 0.0;

    float strength = (dei > 0.0)
        ? (1.0 - uDepthEdgeStrength  * dei)
        : (1.0 + uNormalEdgeStrength * nei);

    oColor = vec4(texel.rgb * strength, 1.0);
}
