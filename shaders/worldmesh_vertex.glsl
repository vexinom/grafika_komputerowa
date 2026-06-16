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

void main()
{
    float yScale = terrainParams.x;
    float yShift = terrainParams.y;
    float skirtDepth = terrainParams.z;

    vec2 globalXZ = chunkOffset + aLocalPos;
    vec2 texCoordHeight = globalXZ / textureSize;
    TexCoord = globalXZ / textureTileSize;

    float worldY = texture(heightmap, texCoordHeight).r * yScale - yShift;
    if (aIsSkirt > 0.5) worldY -= skirtDepth;

    Height = worldY;
    vec3 worldPos = vec3(globalXZ.x, worldY, globalXZ.y);
    WorldPos = worldPos;

    vec2 texel = 1.0 / textureSize;
    float hL = texture(heightmap, texCoordHeight + vec2(-texel.x, 0.0)).r * yScale;
    float hR = texture(heightmap, texCoordHeight + vec2( texel.x, 0.0)).r * yScale;
    float hD = texture(heightmap, texCoordHeight + vec2(0.0, -texel.y)).r * yScale;
    float hU = texture(heightmap, texCoordHeight + vec2(0.0,  texel.y)).r * yScale;

    vec3 N = normalize(mat3(transpose(inverse(model))) * normalize(vec3((hL - hR) * 0.5, 1.0, (hD - hU) * 0.5)));
    vec3 T = normalize(vec3(1.0, 0.0, 0.0) - N * N.x);
    TBN = mat3(T, cross(N, T), N);

    FragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
    gl_Position = projection * view * model * vec4(worldPos, 1.0);
}
