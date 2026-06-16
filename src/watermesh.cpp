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

    chunks.clear();

    int worldRadius = 4;

    for (int z = -worldRadius; z <= worldRadius; z++)
    {
        for (int x = -worldRadius; x <= worldRadius; x++)
        {
            WaterChunk chunk;
            chunk.offsetX = x * chunkWorldSize;
            chunk.offsetZ = z * chunkWorldSize;

            chunk.minBoundBox = glm::vec3(
                chunk.offsetX - offset, 
                waterLevel - maxWaveHeight, 
                chunk.offsetZ - offset
            );
            
            chunk.maxBoundBox = glm::vec3(
                chunk.offsetX + offset, 
                waterLevel + maxWaveHeight, 
                chunk.offsetZ + offset
            );

            chunks.push_back(chunk);
        }
    }


}

void WaterMesh::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, 
                    const glm::vec3& sunDirection, unsigned int heightmapTexture, float time) 
{
    shader.Use();
    glm::mat4 viewProjection = projection * view;
    std::vector<Plane> frustrumPlanes = GetFrustumPlanes(viewProjection);

    shader.SetMat4("model", glm::mat4(1.0f));
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("viewPos", cameraPosition);
    shader.SetFloat("time", time);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    shader.SetInt("heightmap", 3);
    shader.SetVec2("textureSize", glm::vec2(2048.0f, 2048.0f));
    shader.SetVec3("terrainParams", glm::vec3(146.0f, 0.0f, 5.0f));

    glBindVertexArray(VAO);

    int chunkSize = 64;
    float spacing = 4.0f;
    float chunkWorldSize = chunkSize * spacing;
    float offset = chunkWorldSize / 2.0f;

    int cameraChunkX = (int)std::floor(cameraPosition.x / chunkWorldSize);
    int cameraChunkZ = (int)std::floor(cameraPosition.z / chunkWorldSize);

    int viewDistance = 7;

    for (int z = -viewDistance; z <= viewDistance; z++)
    {
        for (int x = -viewDistance; x <= viewDistance; x++)
        {
            float currentChunkOffsetX = (cameraChunkX + x) * chunkWorldSize;
            float currentChunkOffsetZ = (cameraChunkZ + z) * chunkWorldSize;

            glm::vec3 minBoundBox = glm::vec3(
                currentChunkOffsetX - offset, 
                this->waterLevel - this->maxWaveHeight, 
                currentChunkOffsetZ - offset
            );
            
            glm::vec3 maxBoundBox = glm::vec3(
                currentChunkOffsetX + offset, 
                this->waterLevel + this->maxWaveHeight, 
                currentChunkOffsetZ + offset
            );

            if (IsBoxInFrustrum(minBoundBox, maxBoundBox, frustrumPlanes) == false)
            {
                continue; 
            }

            shader.SetVec2("chunkOffset", glm::vec2(currentChunkOffsetX, currentChunkOffsetZ));
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
        }
    }
    glBindVertexArray(0);
}