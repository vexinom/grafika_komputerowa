#version 330 core
<<<<<<< HEAD
layout (location = 0) in vec2 aLocalPos;
layout (location = 1) in float aIsSkirt;

out float Height;
out vec2 TexCoord;
out vec3 WorldPos;
out vec4 FragPosLightSpace;
out mat3 TBN;
=======
layout (location = 0) in vec2 aLocalPos; 
layout (location = 1) in float aIsSkirt;

out float Height;
out vec3 Position;
out vec3 WorldNormal;
out vec2 TexCoord; 
out vec3 WorldPos;
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
<<<<<<< HEAD
uniform mat4 lightSpaceMatrix;
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

uniform sampler2D heightmap;
uniform vec2 chunkOffset;
uniform vec2 textureSize;
<<<<<<< HEAD
uniform vec3 terrainParams;
=======
uniform vec3 terrainParams; 

>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
uniform float textureTileSize;

void main()
{
<<<<<<< HEAD
    float yScale = terrainParams.x;
    float yShift = terrainParams.y;
=======
    float yScale     = terrainParams.x;
    float yShift     = terrainParams.y;
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
    float skirtDepth = terrainParams.z;

    vec2 globalXZ = chunkOffset + aLocalPos;
    vec2 texCoordHeight = globalXZ / textureSize;
<<<<<<< HEAD
    TexCoord = globalXZ / textureTileSize;

    float worldY = texture(heightmap, texCoordHeight).r * yScale - yShift;
    if (aIsSkirt > 0.5) worldY -= skirtDepth;
=======

    TexCoord = globalXZ / textureTileSize; 

    float rawY = texture(heightmap, texCoordHeight).r;
    float worldY = (rawY * yScale) - yShift;

    if (aIsSkirt > 0.5) {
        worldY -= skirtDepth;
    }
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

    Height = worldY;
    vec3 worldPos = vec3(globalXZ.x, worldY, globalXZ.y);
    WorldPos = worldPos;

    vec2 texel = 1.0 / textureSize;
<<<<<<< HEAD
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
=======
    
    float hL = texture(heightmap, texCoordHeight + vec2(-texel.x, 0.0)).r * yScale;
    float hR = texture(heightmap, texCoordHeight + vec2(texel.x, 0.0)).r * yScale;
    float hD = texture(heightmap, texCoordHeight + vec2(0.0, -texel.y)).r * yScale;
    float hU = texture(heightmap, texCoordHeight + vec2(0.0, texel.y)).r * yScale;

    float dX = (hL - hR) * 0.5;
    float dZ = (hD - hU) * 0.5;
    vec3 calculatedNormal = normalize(vec3(dX, 1.0, dZ));

    vec4 viewSpacePos = view * model * vec4(worldPos, 1.0);
    Position = viewSpacePos.xyz;
    
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    WorldNormal = normalize(normalMatrix * calculatedNormal);
    
    gl_Position = projection * viewSpacePos;
}
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
