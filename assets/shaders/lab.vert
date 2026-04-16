#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 vColor;

uniform vec2 uOffset;

void main() {
    vColor = color;
    gl_Position = vec4(position.x + uOffset.x, position.y + uOffset.y, position.z, 1.0);
}
