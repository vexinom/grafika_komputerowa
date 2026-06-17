#version 330 core
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in float aIsSkirt;

uniform mat4 lightSpaceMatrix;
uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
uniform vec3 terrainParams;

// must match worldmesh_vertex.glsl terrainHeight()
const float DEEP   = -700.0;
const float SHELF  =   30.0;
const float ISLAND =  320.0;

float terrainHeight(vec2 g)
{
    vec2 uv = g / textureSize;
    float s = texture(heightmap, uv).r;
    float rn = length(uv - vec2(0.5)) * 2.0;
    float slope = smoothstep(0.40, 0.72, rn);
    float base  = mix(DEEP, SHELF, slope);
    float isl   = smoothstep(0.90, 1.30, rn);
    base = mix(base, ISLAND, isl);
    float deepAmt = 1.0 - slope;
    float amt = 18.0 + 160.0 * deepAmt + 190.0 * isl;
    float h = base + (s - 0.5) * 2.0 * amt;
    h -= isl * (1.0 - s) * 240.0;
    return h;
}

void main()
{
    vec2 globalXZ = chunkOffset + aLocalPos;
    float worldY = terrainHeight(globalXZ);
    if (aIsSkirt > 0.5) worldY -= max(terrainParams.z, 30.0);
    gl_Position = lightSpaceMatrix * vec4(globalXZ.x, worldY, globalXZ.y, 1.0);
}
