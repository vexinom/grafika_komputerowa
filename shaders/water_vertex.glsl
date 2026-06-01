#version 330 core
layout (location = 0) in vec2 inLocalPos; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 chunkOffset;
uniform float waterLevel;
uniform float time;
uniform vec2 terrainTextureSize;

out vec3 FramePosition;
out vec3 FrameNormal;
out vec3 WorldPos;
out vec2 TerrainTexCoord; 

struct GerstnerWave
{
    vec2 direction; 
    float amplitude;
    float steepness;
    float waveLength;
    float speed;
}; 

vec3 GetGerstnerWave(GerstnerWave wave, vec2 pos, float t, inout vec3 tangent, inout vec3 binormal) {
    vec2 d = normalize(wave.direction);
    float k = 2.0 * 3.14159265 / wave.waveLength;
    float c = wave.speed;
    float f = k * (dot(d, pos) - c * t);
    float a = wave.amplitude;
    float q = wave.steepness;

    tangent += vec3(
        -d.x * d.x * q * sin(f),
        d.x * q * cos(f),
        -d.x * d.y * q * sin(f)
    );
    binormal += vec3(
        -d.x * d.y * q * sin(f),
        d.y * q * cos(f),
        -d.y * d.y * q * sin(f)
    );

    return vec3(
        d.x * (a * cos(f)),
        a * sin(f),
        d.y * (a * cos(f))
    );
}

void main()
{
    vec2 globalXZ = chunkOffset + inLocalPos;
    
    TerrainTexCoord = globalXZ / terrainTextureSize;

    GerstnerWave wave1 = GerstnerWave(vec2(1.0, 0.0), 1.0, 0.2, 40.0, 1.5);
    GerstnerWave wave2 = GerstnerWave(vec2(0.6, 0.8), 0.5, 0.1, 20.0, 2.5);

    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);
    
    vec3 displacement = vec3(0.0);
    displacement += GetGerstnerWave(wave1, globalXZ, time, tangent, binormal);
    displacement += GetGerstnerWave(wave2, globalXZ, time, tangent, binormal);

    vec3 worldPos = vec3(globalXZ.x, waterLevel, globalXZ.y) + displacement;
    
    WorldPos = worldPos;
    FramePosition = worldPos; 
    FrameNormal = normalize(cross(binormal, tangent));

    gl_Position = projection * view * model * vec4(worldPos, 1.0);
}