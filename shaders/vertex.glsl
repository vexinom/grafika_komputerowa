#version 330 core
layout (location = 0) in vec3 aPos; 

out float Height;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    
    Height = aPos.y;

    vec3 localPos = aPos; 

    vec4 viewSpacePos = view * model * vec4(localPos, 1.0);
    Position = viewSpacePos.xyz;
    gl_Position = projection * viewSpacePos;
}