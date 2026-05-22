#version 330 core
layout (location = 0) in float aHeight; // Just a single float!

out float Height;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Pass your map dimensions as uniforms
uniform int terrainWidth;
uniform int terrainHeight;

void main()
{
    Height = aHeight;

    // Reconstruct grid coordinates based on the sequential vertex ID
    int xIndex = gl_VertexID % terrainWidth;
    int zIndex = gl_VertexID / terrainWidth;

    // Shift to align center just like your C++ code did
    float posX = -float(terrainHeight) / 2.0 + float(zIndex);
    float posZ = -float(terrainWidth) / 2.0 + float(xIndex);

    vec3 reconstructedPos = vec3(posX, aHeight, posZ);
    
    vec4 viewSpacePos = view * model * vec4(reconstructedPos, 1.0);
    Position = viewSpacePos.xyz;
    gl_Position = projection * viewSpacePos;
}