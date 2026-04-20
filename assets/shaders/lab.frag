#version 330 core
out vec4 FragColor;

in vec3 vColor;
in vec2 vUV;

uniform sampler2D brickTexture;

void main()
{
    vec4 texColor = texture(brickTexture, vUV);
    FragColor = texColor * vec4(vColor, 1.0);
}
