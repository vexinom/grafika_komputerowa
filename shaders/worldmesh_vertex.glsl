#version 330 core
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in float aIsSkirt;

out float Height;
out vec2 TexCoord;
out vec3 WorldPos;
out vec4 FragPosLightSpace;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
uniform vec3 terrainParams;
uniform float textureTileSize;

// ---------------------------------------------------------------------------
// Radial ocean profile (must match depth_vertex.glsl and Reef::seabed):
//   centre  -> deep, characterful ocean floor
//   ring 1  -> slow, spreading slope upward
//   ring 2  -> broad shallow sandy shelf (plycizna)
//   ring 3  -> peninsulas / islands broken out of a height-map mask
// ---------------------------------------------------------------------------
const float DEEP   = -700.0;   // much deeper ocean basin
const float SHELF  =   30.0;
const float ISLAND =  320.0;   // taller surrounding peninsulas

float terrainHeight(vec2 g)
{
    vec2 uv = g / textureSize;
    float s = texture(heightmap, uv).r;            // height-map detail [0,1]
    float rn = length(uv - vec2(0.5)) * 2.0;       // 0 centre .. 1 edge .. ~1.41 corner

    float slope = smoothstep(0.40, 0.72, rn);      // deep -> shelf
    float base  = mix(DEEP, SHELF, slope);
    float isl   = smoothstep(0.90, 1.30, rn);      // shelf -> island
    base = mix(base, ISLAND, isl);

    float deepAmt = 1.0 - slope;                    // rugged in the deep + on islands, gentle on the shelf
    float amt = 18.0 + 160.0 * deepAmt + 190.0 * isl;
    float h = base + (s - 0.5) * 2.0 * amt;

    h -= isl * (1.0 - s) * 240.0;                  // carve channels -> peninsulas instead of a solid ring
    return h;
}

void main()
{
    float skirtDepth = terrainParams.z;
    vec2 globalXZ = chunkOffset + aLocalPos;
    TexCoord = globalXZ / textureTileSize;

    float worldY = terrainHeight(globalXZ);
    if (aIsSkirt > 0.5) worldY -= max(skirtDepth, 30.0);

    Height = worldY;
    vec3 worldPos = vec3(globalXZ.x, worldY, globalXZ.y);
    WorldPos = worldPos;

    float eps = 2.0;
    float hL = terrainHeight(globalXZ + vec2(-eps, 0.0));
    float hR = terrainHeight(globalXZ + vec2( eps, 0.0));
    float hD = terrainHeight(globalXZ + vec2(0.0, -eps));
    float hU = terrainHeight(globalXZ + vec2(0.0,  eps));
    vec3 N = normalize(vec3(hL - hR, 2.0 * eps, hD - hU));
    vec3 T = normalize(vec3(1.0, 0.0, 0.0) - N * N.x);
    TBN = mat3(T, cross(N, T), N);

    FragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
    gl_Position = projection * view * model * vec4(worldPos, 1.0);
}
