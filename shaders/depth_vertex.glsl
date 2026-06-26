#version 330 core
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in float aIsSkirt;

uniform mat4 lightSpaceMatrix;
uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
uniform vec3 terrainParams;

float terrainHeight(vec2 g) {
    return (texture(heightmap, g / textureSize).r * terrainParams.x) + terrainParams.y;
}

void main()
{
    vec2 globalXZ = chunkOffset + aLocalPos;
    float worldY = terrainHeight(globalXZ);
    
    if (aIsSkirt > 0.5) worldY -= max(terrainParams.z, 30.0);
    
    gl_Position = lightSpaceMatrix * vec4(globalXZ.x, worldY, globalXZ.y, 1.0);
}