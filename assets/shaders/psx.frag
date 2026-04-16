#version 330 core

in vec2 vTexCoord;
in vec4 vColor;
in float vFog;

uniform sampler2D uTexture;
uniform int  uUseTexture;
uniform vec4 uTintColor;
uniform vec3 uFogColor;
uniform int  uDitheringEnabled;
uniform float uColorDepth;       // color levels per channel (31.0 = PS1, 255.0 = full)

out vec4 FragColor;

// Ordered 4x4 Bayer dither matrix
float dither4x4(vec2 fragCoord) {
    int x = int(mod(fragCoord.x, 4.0));
    int y = int(mod(fragCoord.y, 4.0));
    int index = x + y * 4;

    float dither[16] = float[16](
         0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
         3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
    );

    return dither[index];
}

void main() {
    // Sample texture or use white
    vec4 texColor;
    if (uUseTexture != 0) {
        texColor = texture(uTexture, vTexCoord);
    } else {
        texColor = vec4(1.0);
    }

    vec4 color = texColor * vColor * uTintColor;

    // Discard fully transparent fragments
    if (color.a < 0.01) discard;

    // COLOR DEPTH REDUCTION: quantize to uColorDepth levels per channel
    float depth = max(uColorDepth, 1.0);
    color.rgb = floor(color.rgb * depth + 0.5) / depth;

    // DITHERING
    if (uDitheringEnabled != 0) {
        float d = dither4x4(gl_FragCoord.xy) / depth;
        color.rgb += d - (0.5 / depth);
        color.rgb = floor(color.rgb * depth + 0.5) / depth;
    }

    // FOG
    color.rgb = mix(uFogColor, color.rgb, vFog);

    FragColor = vec4(color.rgb, color.a);
}
