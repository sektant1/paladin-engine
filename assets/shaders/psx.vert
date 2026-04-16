#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uSnapResolution; // vertex snapping grid (e.g. 160.0)
uniform float uFogStart;       // fog start distance
uniform float uFogEnd;         // fog end distance

out vec2 vTexCoord;
out vec4 vColor;
out float vFog;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vec4 viewPos  = uView * worldPos;
    vec4 clipPos  = uProjection * viewPos;

    // VERTEX SNAPPING: simulate PS1 fixed-point GTE precision
    if (uSnapResolution > 0.0) {
        clipPos.xyz = clipPos.xyz / clipPos.w;
        clipPos.xy = floor(clipPos.xy * uSnapResolution + 0.5) / uSnapResolution;
        clipPos.xyz *= clipPos.w;
    }

    gl_Position = clipPos;

    // Affine texture mapping (noperspective qualifier handles this)
    vTexCoord = aTexCoord;

    // Per-vertex color
    vColor = aColor;

    // Per-vertex fog (linear distance fog)
    float dist = length(viewPos.xyz);
    vFog = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
}
