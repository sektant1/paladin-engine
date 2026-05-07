#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

uniform mat4 uModel;
uniform mat4 uProjection;
uniform mat4 uView;

out vec3 vViewNormal;

void main()
{
    vec3 worldNormal = normalize(mat3(uModel) * normal);
    vViewNormal = normalize(mat3(uView) * worldNormal);
    gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
}
