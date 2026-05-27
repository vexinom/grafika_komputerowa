#version 330 core
out vec4 FragColor;

in vec3 Position;
uniform sampler2D heightMap;


void main()
{

    FragColor = vec4(0.0, 0.4, 0.8, 0.8); 
}