#version 330 core
layout (location = 0) in vec2 aLocalPos; 
layout (location = 1) in float aIsSkirt;

out float Height;
out vec3 Position;
out vec3 Normal;
out vec2 TexCoord; 
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
uniform vec3 terrainParams; 

uniform float textureTileSize;

void main()
{
    float yScale     = terrainParams.x;
    float yShift     = terrainParams.y;
    float skirtDepth = terrainParams.z;

    vec2 globalXZ = chunkOffset + aLocalPos;
    vec2 texCoordHeight = globalXZ / textureSize;

    TexCoord = globalXZ / textureTileSize; 

    float rawY = texture(heightmap, texCoordHeight).r;
    float worldY = (rawY * yScale) - yShift;

    if (aIsSkirt > 0.5) {
        worldY -= skirtDepth;
    }

    Height = worldY;
    vec3 worldPos = vec3(globalXZ.x, worldY, globalXZ.y);

    vec2 texel = 1.0 / textureSize;
    
    float hL = texture(heightmap, texCoordHeight + vec2(-texel.x, 0.0)).r * yScale;
    float hR = texture(heightmap, texCoordHeight + vec2(texel.x, 0.0)).r * yScale;
    float hD = texture(heightmap, texCoordHeight + vec2(0.0, -texel.y)).r * yScale;
    float hU = texture(heightmap, texCoordHeight + vec2(0.0, texel.y)).r * yScale;

    float dX = (hL - hR) * 0.5;
    float dZ = (hD - hU) * 0.5;
    vec3 calculatedNormal = normalize(vec3(dX, 1.0, dZ));

    vec4 viewSpacePos = view * model * vec4(worldPos, 1.0);
    Position = viewSpacePos.xyz;
    
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    Normal = normalize(normalMatrix * calculatedNormal);
    
    gl_Position = projection * viewSpacePos;
}