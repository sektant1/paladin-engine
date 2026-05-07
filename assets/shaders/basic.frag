#version 330 core

in vec4 vColor;
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

void main() {
    FragColor = vColor;
    FragNormal = vec4(0.5, 0.5, 1.0, 1.0);
}
