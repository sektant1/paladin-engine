#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

in vec3 vColor;
in vec2 vUV;

uniform sampler2D brickTexture;

void main()
{
    vec4 texColor = texture(brickTexture, vUV);
    FragColor = texColor * vec4(vColor, 1.0);
    FragNormal = vec4(0.5, 0.5, 1.0, 1.0);
}
