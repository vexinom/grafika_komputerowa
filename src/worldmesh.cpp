#include "worldmesh.h"
#include <cmath>
#include <glad/glad.h>
#include <algorithm>
#include <vector>
#include <functional>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

void WorldMesh::Init()
{

    // HEIGHT MAP
    int nChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned short *rawData = stbi_load_16("assets/worldmap.png", &width, &height, &nChannels, 1);
    int channels = 1;

    if (!rawData) {
        fprintf(stderr, "Failed to load terrain texture heightmap map!\n");
        return;
    }

    glGenTextures(1, &heightmapTexture);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, rawData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(rawData);


    // SAND TEXTURE

    int sWidth, sHeight, sChannels;
    unsigned char* surfaceData = stbi_load("assets/sand.jpg", &sWidth, &sHeight, &sChannels, 0);

    if (!surfaceData) {
        fprintf(stderr, "Failed to load terrain texture heightmap map!\n");
        return;
    }

    glGenTextures(1, &surfaceTexture);
    glBindTexture(GL_TEXTURE_2D, surfaceTexture);

    GLenum format = (sChannels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, sWidth, sHeight, 0, format, GL_UNSIGNED_BYTE, surfaceData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(surfaceData);

    //GRASS TEXTURE

    int gWidth, gHeight, gChannels;
    unsigned char* grassData = stbi_load("assets/grass.png", &gWidth, &gHeight, &gChannels, 0);

    if (!grassData) {
        fprintf(stderr, "Failed to load grass texture!\n");
        return;
    }

    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    GLenum gFormat = (gChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, gFormat, gWidth, gHeight, 0, gFormat, GL_UNSIGNED_BYTE, grassData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(grassData);


    const int CHUNK_SIZE = 64;
    int vertexWidth = CHUNK_SIZE + 1;
    int vertexHeight = CHUNK_SIZE + 1;
    
    std::vector<LocalVertex> localVertices;

    for(int z = 0; z <= CHUNK_SIZE; z++) 
    {
        for(int x = 0; x <= CHUNK_SIZE; x++) 
        {
            localVertices.push_back({ (float)x, (float)z, 0.0f });
        }
    }

    for(int x = 0; x <= CHUNK_SIZE; x++) 
    {
        localVertices.push_back({ (float)x, 0.0f, 1.0f });
    }
    for(int x = 0; x <= CHUNK_SIZE; x++) 
    {
        localVertices.push_back({ (float)x, (float)CHUNK_SIZE, 1.0f });
    }
    for(int z = 0; z <= CHUNK_SIZE; z++) 
    {
        localVertices.push_back({ 0.0f, (float)z, 1.0f });
    }
    for(int z = 0; z <= CHUNK_SIZE; z++) 
    {
        localVertices.push_back({ (float)CHUNK_SIZE, (float)z, 1.0f });
    }

    glGenVertexArrays(1, &globalVAO);
    glGenBuffers(1, &globalVBO);

    glBindVertexArray(globalVAO);
    glBindBuffer(GL_ARRAY_BUFFER, globalVBO);
    glBufferData(GL_ARRAY_BUFFER, localVertices.size() * sizeof(LocalVertex), localVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LocalVertex), (void*)offsetof(LocalVertex, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(LocalVertex), (void*)offsetof(LocalVertex, isSkirt));
    glEnableVertexAttribArray(1);

    glGenBuffers(3, globalEBO);
    int lodStrides[3] = {1, 2, 4};
    unsigned int restartIndex = 0xFFFFFFFF;
    int baseVertexCount = vertexWidth * vertexHeight;
    int skirtOffset = baseVertexCount;
    int topSkirtIndex = skirtOffset;    skirtOffset += vertexWidth;
    int bottomSkirtIndex = skirtOffset; skirtOffset += vertexWidth;
    int leftSkirtIndex = skirtOffset;   skirtOffset += vertexHeight;
    int rightSkirtIndex = skirtOffset;
    
    for(int lod = 0; lod < 3; lod++) 
    {
        std::vector<unsigned int> lod_indices;
        int stride = lodStrides[lod];
        
        for(int z = 0; z < CHUNK_SIZE; z += stride) 
        {
            for(int x = 0; x <= CHUNK_SIZE; x += stride) 
            {
                lod_indices.push_back(x + vertexWidth * z);
                lod_indices.push_back(x + vertexWidth * std::min(z + stride, CHUNK_SIZE)); 
            }
            lod_indices.push_back(restartIndex);
        }
        for(int x = 0; x <= CHUNK_SIZE; x+= stride) 
        { 
            lod_indices.push_back(x + vertexWidth * 0); 
            lod_indices.push_back(topSkirtIndex + x); 
        }
        lod_indices.push_back(restartIndex);

        for(int x = 0; x <= CHUNK_SIZE; x+= stride) 
        { 
            lod_indices.push_back(x + vertexWidth * CHUNK_SIZE); 
            lod_indices.push_back(bottomSkirtIndex + x); 
        }
        lod_indices.push_back(restartIndex);

        for(int z = 0; z <= CHUNK_SIZE; z+= stride) 
        { 
            lod_indices.push_back(0 + vertexWidth * z); 
            lod_indices.push_back(leftSkirtIndex + z); 
        }
        lod_indices.push_back(restartIndex);

        for(int z = 0; z <= CHUNK_SIZE; z+= stride) 
        { 
            lod_indices.push_back(CHUNK_SIZE + vertexWidth * z); 
            lod_indices.push_back(rightSkirtIndex + z); 
        }
        lod_indices.push_back(restartIndex);

        globalindexCount[lod] = static_cast<int>(lod_indices.size());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO[lod]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lod_indices.size() * sizeof(unsigned int), lod_indices.data(), GL_STATIC_DRAW);
    }
    
    chunks.clear();

    for(int ch_z = 0; ch_z < height - 1; ch_z += CHUNK_SIZE) 
    {
        for(int ch_x = 0; ch_x < width - 1; ch_x += CHUNK_SIZE) 
        {
            Chunk chunk;
            chunk.x = ch_x;
            chunk.z = ch_z;

            chunk.minBoundBox = glm::vec3(ch_x, -yShift - skirtDepth, ch_z);
            chunk.maxBoundBox = glm::vec3(ch_x + CHUNK_SIZE, yScale - yShift, ch_z + CHUNK_SIZE);
            chunks.push_back(chunk);
        }
    }
    glBindVertexArray(0);
}

WorldMesh::~WorldMesh() {
    glDeleteBuffers(3, globalEBO);
    glDeleteBuffers(1, &globalVBO);
    glDeleteVertexArrays(1, &globalVAO);
    glDeleteTextures(1, &heightmapTexture);
    glDeleteTextures(1, &surfaceTexture);
}

void WorldMesh::Draw(glm::mat4 & viewProjection, glm::vec3 & cameraPosition, unsigned int shaderID)
{
    std::vector<Plane> frustrumPlanes = GetFrustumPlanes(viewProjection);
    int chunksDrawn = 0;

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glBindVertexArray(globalVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    glUniform1i(glGetUniformLocation(shaderID, "heightmap"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, surfaceTexture);
    glUniform1i(glGetUniformLocation(shaderID, "sandTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glUniform1i(glGetUniformLocation(shaderID, "grassTexture"), 2);


    glUniform1f(glGetUniformLocation(shaderID, "textureTileSize"), 8.0f);

    glUniform2f(glGetUniformLocation(shaderID, "textureSize"), (float)width, (float)height);
    glUniform3f(glGetUniformLocation(shaderID, "terrainParams"), yScale, yShift, skirtDepth);

    glUniform3f(glGetUniformLocation(shaderID, "cameraPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);

    for(size_t i = 0; i < chunks.size(); i++)
    {
        if(IsBoxInFrustrum(chunks[i].minBoundBox, chunks[i].maxBoundBox, frustrumPlanes) == false)
        {
            continue;
        }

        glm::vec3 chunkCenter = (chunks[i].minBoundBox + chunks[i].maxBoundBox) * 0.5f;
        float distance = glm::distance(cameraPosition, chunkCenter);

        int lod = 0;
        if(distance > 600.0f)
        {
            lod = 2;
        }
        else if(distance > 250.0f)
        {
            lod = 1;
        }

        glUniform2f(glGetUniformLocation(shaderID, "chunkOffset"), (float)chunks[i].x, (float)chunks[i].z);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, globalindexCount[lod], GL_UNSIGNED_INT, (void*)0 );
    }

    glDisable(GL_PRIMITIVE_RESTART);
    glBindVertexArray(0);
}

Plane::Plane()
{
    normal = glm::vec3(0.0f);
    distance = 0.0f;
}

Plane::Plane(const glm::vec4& vec)
{
    float length = glm::length(glm::vec3(vec));
    normal = glm::vec3(vec) / length;
    distance = vec.w / length;
}

bool Plane::IsInFront(const glm::vec3 & point)
{
    float signedDistance = glm::dot(normal, point) + distance;

    if(signedDistance >= 0.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<Plane> GetFrustumPlanes(const glm::mat4 & viewProj)
{
    std::vector<Plane> planes;

    glm::vec4 row1 = glm::vec4(viewProj[0][0], viewProj[1][0], viewProj[2][0], viewProj[3][0]);
    glm::vec4 row2 = glm::vec4(viewProj[0][1], viewProj[1][1], viewProj[2][1], viewProj[3][1]);
    glm::vec4 row3 = glm::vec4(viewProj[0][2], viewProj[1][2], viewProj[2][2], viewProj[3][2]);
    glm::vec4 row4 = glm::vec4(viewProj[0][3], viewProj[1][3], viewProj[2][3], viewProj[3][3]);

    planes.push_back(row4 + row1); 
    planes.push_back(row4 - row1); 
    planes.push_back(row4 + row2); 
    planes.push_back(row4 - row2); 
    planes.push_back(row4 + row3); 
    planes.push_back(row4 - row3); 

    return planes;
}

bool IsBoxInFrustrum(const glm::vec3 & min, const glm::vec3 max, const std::vector<Plane> planes)
{
    for(int i = 0; i < planes.size(); i++)
    {
        glm::vec3 positiveVertex = min;

        if (planes[i].normal.x >= 0)
        {
            positiveVertex.x = max.x;
        }

        if (planes[i].normal.y >= 0)
        {
            positiveVertex.y = max.y;
        }

        if (planes[i].normal.z >= 0)
        {
            positiveVertex.z = max.z;
        }

        if(glm::dot(planes[i].normal, positiveVertex) + planes[i].distance < 0.0f)
        {
            return false;
        }
    }
    return true;
}

glm::vec3 GetVertexNormal(int globalX, int globalZ, float * data, int width, int height, int channels, float yScale)
{
    std::function<float(int, int)> GetHeight = [&](int x, int z) -> float 
    {
        int clampedX = std::max(0, std::min(x, width - 1));
        int clampedZ = std::max(0, std::min(z, height - 1));
        int index = (clampedX + width * clampedZ) * channels;
        return data[index] * yScale;
    };

    float hL = GetHeight(globalX - 1, globalZ);
    float hR = GetHeight(globalX + 1, globalZ);
    float hD = GetHeight(globalX, globalZ - 1);
    float hU = GetHeight(globalX, globalZ + 1);

    float dX = (hL - hR) * 0.5f;
    float dZ = (hD - hU) * 0.5f;

    glm::vec3 normal = glm::normalize(glm::vec3(dX, 1.0f, dZ));

    return normal;
}