#version 330 core
out vec4 FragColor;

in vec3 FramePosition;
in vec3 FrameNormal;

uniform vec3 cameraPos;

void main()
{
    FragColor = vec4(0.0, 0.45, 0.55, 0.2);
}