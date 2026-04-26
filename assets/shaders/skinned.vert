#version 330 core

// GPU skinning. Mirrors the default lit vertex shader but applies a
// per-vertex 4-bone matrix blend before the model transform.
//
// Layout slots match VertexLayout.h:
//   0 = position, 1 = color, 2 = uv, 3 = normal,
//   4 = ivec4 bone indices, 5 = vec4 bone weights.
//
// uBones[] is the final skinning palette: globalPose * offsetMatrix.
// Cap matches the renderer (see Skeleton bone count guard).

#define MAX_BONES 128

layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec3  aColor;
layout(location = 2) in vec2  aTexCoord;
layout(location = 3) in vec3  aNormal;
layout(location = 4) in ivec4 aBoneIndices;
layout(location = 5) in vec4  aBoneWeights;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uBones[MAX_BONES];

out vec2 vUV;
out vec3 vNormal;
out vec3 vFragPos;

void main()
{
    float weightSum = aBoneWeights.x + aBoneWeights.y + aBoneWeights.z + aBoneWeights.w;
    mat4 skin;
    if (weightSum < 1e-4) {
        // Vertex unbound (no skin) — fall back to identity so static meshes
        // sharing the skinned shader still render correctly.
        skin = mat4(1.0);
    } else {
        skin = uBones[aBoneIndices.x] * aBoneWeights.x
             + uBones[aBoneIndices.y] * aBoneWeights.y
             + uBones[aBoneIndices.z] * aBoneWeights.z
             + uBones[aBoneIndices.w] * aBoneWeights.w;
    }

    vec4 skinnedPos = skin * vec4(aPosition, 1.0);
    vec3 skinnedNrm = mat3(skin) * aNormal;

    vec4 worldPos = uModel * skinnedPos;
    vFragPos = worldPos.xyz;
    vNormal  = mat3(transpose(inverse(uModel))) * skinnedNrm;
    vUV      = aTexCoord;

    gl_Position = uProjection * uView * worldPos;
}
