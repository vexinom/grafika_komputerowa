#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;

out float Height;
out vec3 Position;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    
    Height = aPos.y;

    vec3 localPos = aPos; 
    vec4 viewSpacePos = view * model * vec4(localPos, 1.0);
    Position = viewSpacePos.xyz;
    
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    Normal = normalize(normalMatrix * aNormal);
    
    gl_Position = projection * viewSpacePos;
}