#version 330 core
layout (location = 0) in vec2 inLocalPos; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 chunkOffset;
uniform float waterLevel;
uniform float time;

out vec3 FramePosition;
out vec3 FrameNormal;

void main()
{
    //wave formula y = sin(x + t)
    vec2 worldXZ = inLocalPos + chunkOffset;

    float amplitude = 0.3;
    float frequency = 2.0;
    float speed = 1.5;

    float sin_wave = sin(worldXZ.x * frequency + time * speed) +
                     sin(worldXZ.y * frequency + time * speed);

    sin_wave *= amplitude;
    
    vec3 position = vec3(worldXZ.x, waterLevel + sin_wave, worldXZ.y);

    FramePosition = position;
    FrameNormal = vec3(0.0, 1.0, 0.0);

    gl_Position = projection * view * model * vec4(position, 1.0);

}