#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 LocalPos;

uniform mat4 model;
uniform mat4 viewProjection;

void main()
{
    LocalPos = aPos;

    vec4 pos = viewProjection * model * vec4(aPos, 1.0);
    
    gl_Position = pos.xyww; 
}