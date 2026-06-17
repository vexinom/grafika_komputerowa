#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;
layout (location = 2) in float aType;

uniform mat4 view;
uniform mat4 projection;
uniform float sizeScale;

out float vType;

void main()
{
    vec4 clip = projection * view * vec4(aPos, 1.0);
    gl_Position = clip;
    float ps = aSize * sizeScale / max(clip.w, 0.001);
    gl_PointSize = clamp(ps, 1.0, 48.0);
    vType = aType;
}
