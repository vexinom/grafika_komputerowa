#include "watermesh.h"
#include <iostream>

void WaterMesh::Init(float waterLevel, float maxWaveHeight)
{
    this->waterLevel = waterLevel;
    this->maxWaveHeight = maxWaveHeight;

    int chunkSize = 64; 
    int vertexCount = chunkSize + 1;
    float spacing = 4.0f;

    std::vector<glm::vec2> vertices;
    std::vector<unsigned int> indices;

    float chunkWorldSize = chunkSize * spacing; 
    float offset = chunkWorldSize / 2.0f;


    for (int z = 0; z < vertexCount; z++)
    {
        for (int x = 0; x < vertexCount; x++)
        {
            vertices.push_back(glm::vec2(x * spacing - offset, z * spacing - offset));
        }
    }

    for(int z = 0; z < chunkSize; z++)
    {
        for(int x = 0; x < chunkSize; x++)
        {
            int topLeft = z * vertexCount + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * vertexCount + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
            
        }
    }

    indexCount = (int)indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}

void WaterMesh::Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int waterShaderID, float time) 
{
    glBindVertexArray(VAO);

    int chunkSize = 64;
    float spacing = 4.0f;
    float chunkWorldSize = chunkSize * spacing;

    int cameraChunkX = (int)std::floor(cameraPosition.x / chunkWorldSize);
    int cameraChunkZ = (int)std::floor(cameraPosition.z / chunkWorldSize);

    int viewDistance = 6;

    glUniform1f(glGetUniformLocation(waterShaderID, "time"), time);

    for (int z = -viewDistance; z <= viewDistance; z++)
    {
        for (int x = -viewDistance; x <= viewDistance; x++)
        {
            float currentChunkOffsetX = (cameraChunkX + x) * chunkWorldSize;
            float currentChunkOffsetZ = (cameraChunkZ + z) * chunkWorldSize;

            glUniform2f(glGetUniformLocation(waterShaderID, "chunkOffset"), currentChunkOffsetX, currentChunkOffsetZ);

            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
        }
    }
    glBindVertexArray(0);
}