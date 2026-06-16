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

uniform float time;
uniform float bodyMinY;
uniform float bodyLenY;

void main()
{
    float t = clamp((aPos.y - bodyMinY) / bodyLenY, 0.0, 1.0);
    float amplitude = bodyLenY * (0.02 + 0.10 * t);
    float wave = sin(t * 4.5 - time * 3.0);

    vec3 local = aPos;
    local.x += wave * amplitude;

    vec4 world = model * vec4(local, 1.0);
    FragPos = world.xyz;
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    UV = aUV;
    FragPosLightSpace = lightSpaceMatrix * world;
    gl_Position = projection * view * world;
}
