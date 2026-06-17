#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aInfo;   // heightFrac, phase, swayX, swayZ

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 current;

out float vH;

void main()
{
    float t = aInfo.x;
    float phase = aInfo.y;
    vec2 sway = aInfo.zw;

    vec3 p = aPos;
    float amp = 6.0 + length(current) * 0.5;
    float bend = sin(time * 1.5 + phase + t * 2.0) * t * t * amp;
    p.x += sway.x * bend + current.x * 0.05 * t * t;
    p.z += sway.y * bend + current.z * 0.05 * t * t;

    vH = t;
    gl_Position = projection * view * vec4(p, 1.0);
}
