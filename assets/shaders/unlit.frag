#version 330 core

uniform vec3 uEmissive;

in vec3 vViewNormal;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

void main()
{
    FragColor = vec4(uEmissive, 1.0);
    FragNormal = vec4(normalize(vViewNormal) * 0.5 + 0.5, 1.0);
}
