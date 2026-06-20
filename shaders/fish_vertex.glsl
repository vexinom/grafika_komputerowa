#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in mat4 aInstanceModel;   

uniform mat4 view, projection;
uniform float time;

out vec3 vNormal; out vec2 vUV;

void main()
{
    float phase = aInstanceModel[3].x * 0.05 + aInstanceModel[3].z * 0.05;
    float wave  = sin(time * 3.0 + aPos.z * 2.5 + phase);
    vec3 p = aPos;
    p.x += wave * 0.12;                       
    vec4 world = aInstanceModel * vec4(p, 1.0);
    gl_Position = projection * view * world;
    vNormal = mat3(aInstanceModel) * aNormal;
    vUV = aUV;
}
