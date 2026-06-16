#version 330 core
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in float aIsSkirt;

uniform mat4 lightSpaceMatrix;
uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
uniform vec3 terrainParams;

void main()
{
    vec2 globalXZ = chunkOffset + aLocalPos;
    float worldY = texture(heightmap, globalXZ / textureSize).r * terrainParams.x - terrainParams.y;
    if (aIsSkirt > 0.5) worldY -= terrainParams.z;
    gl_Position = lightSpaceMatrix * vec4(globalXZ.x, worldY, globalXZ.y, 1.0);
}
