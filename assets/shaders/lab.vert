#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 vColor;

uniform vec2 uOffset;
uniform float uAngle;

void main() {
    vColor = color;

    float c = cos(uAngle);
    float s = sin(uAngle);

    float x = c * position.x - s * position.y + uOffset.x;
    float y = s * position.x + c * position.y + uOffset.y;

    gl_Position = vec4(x, y, position.z, 1.0);
}
