#version 330 core

// ============================================================
//  psx.frag — PSX / N64 style fragment shader
//  Adapted for coagula-engine:
//    - Sampler named `baseColorTexture` to match .mat loader
//    - No int uniforms (Material::SetParam has no int overload)
//    - Safe defaults on every uniform (GL inits to 0)
// ============================================================

noperspective in vec2 vTexCoord;
in vec4  vColor;
in float vFog;
in float vLight;
in vec3  vViewNormal;

// Sampler name matches engine convention (Material JSON loader)
uniform sampler2D baseColorTexture;

// Tuning uniforms (all optional)
uniform vec4  uTintColor;        // if alpha == 0 -> treated as white
uniform vec3  uFogColor;         // default black when unset
uniform float uColorDepth;       // levels per channel; <= 1 disables quantize. PS1 ~= 32
uniform float uDitherStrength;   // 0 = off; typical 1.0
uniform float uAlphaCutoff;      // 0 -> default 0.01

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

float bayer4x4(vec2 frag)
{
    int x = int(mod(frag.x, 4.0));
    int y = int(mod(frag.y, 4.0));
    int idx = x + y * 4;
    float m[16] = float[16](
         0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
         3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
    );
    return m[idx];
}

void main()
{
    vec4 tex   = texture(baseColorTexture, vTexCoord);
    vec4 tint  = (uTintColor.a > 0.0) ? uTintColor : vec4(1.0);
    vec4 color = tex * vColor * tint;

    float cutoff = (uAlphaCutoff > 0.0) ? uAlphaCutoff : 0.01;
    if (color.a < cutoff) discard;

    // Gouraud from VS
    color.rgb *= vLight;

    // Quantize (+ optional dither)
    if (uColorDepth > 1.0)
    {
        float levels = uColorDepth;
        if (uDitherStrength > 0.0)
        {
            float d = (bayer4x4(gl_FragCoord.xy) - 0.5) * uDitherStrength / levels;
            color.rgb += d;
        }
        color.rgb = floor(clamp(color.rgb, 0.0, 1.0) * (levels - 1.0) + 0.5) / (levels - 1.0);
    }

    // Fog (vFog = 1 when uFogEnd <= 0 -> no-op)
    color.rgb = mix(uFogColor, color.rgb, vFog);

    FragColor = vec4(color.rgb, color.a);
    FragNormal = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
}
