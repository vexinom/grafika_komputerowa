#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform float uTime;
uniform float uPhase;
uniform float uFlapSpeed;
uniform float uFlapAmp;

void main()
{
    float wingT = smoothstep(0.10, 0.5, abs(aPos.x));
    float flap  = sin(uTime * uFlapSpeed + uPhase);

    vec3 p = aPos;
    p.y += wingT * uFlapAmp * flap;

    vec4 worldPos = model * vec4(p, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = aUV;
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    gl_Position = projection * view * worldPos;
}
