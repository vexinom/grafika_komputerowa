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
uniform vec4 clipPlane;

#if __VERSION__ >= 130
out float gl_ClipDistance[1];
#endif

float terrainHeight(vec2 g) {
    return (texture(heightmap, g / textureSize).r * terrainParams.x) + terrainParams.y;
}

void main() {
    vec2 globalXZ = chunkOffset + aLocalPos;
    TexCoord = globalXZ / textureTileSize;
    float worldY = terrainHeight(globalXZ);
    if (aIsSkirt > 0.5) worldY -= max(terrainParams.z, 30.0);

    Height = worldY;
    WorldPos = vec3(globalXZ.x, worldY, globalXZ.y);

    float eps = 2.0;
    float hL = terrainHeight(globalXZ + vec2(-eps, 0.0));
    float hR = terrainHeight(globalXZ + vec2( eps, 0.0));
    float hD = terrainHeight(globalXZ + vec2(0.0, -eps));
    float hU = terrainHeight(globalXZ + vec2(0.0,  eps));
    vec3 N = normalize(vec3(hL - hR, 2.0 * eps, hD - hU));
    vec3 T = normalize(vec3(1.0, 0.0, 0.0) - N * N.x);
    TBN = mat3(T, cross(N, T), N);

    gl_ClipDistance[0] = dot(vec4(WorldPos, 1.0), clipPlane);

    FragPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);
    gl_Position = projection * view * model * vec4(WorldPos, 1.0);
}