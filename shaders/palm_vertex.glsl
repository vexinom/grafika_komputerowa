#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vUV;
out vec4 vFragPosLightSpace;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    vFragPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vUV = aUV;
    vFragPosLightSpace = lightSpaceMatrix * worldPos;

    gl_Position = projection * view * worldPos;
}
