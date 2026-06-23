#include "worldmesh.h"
#include <cmath>
#include <glad/glad.h>
#include <algorithm>
#include <vector>
#include <functional>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

// Anisotropic filtering keeps tiled ground textures sharp at grazing angles
// instead of smearing into a blur in the distance.
static void SetAniso()
{
    float maxA = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxA);
    if (maxA > 1.0f)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA < 8.0f ? maxA : 8.0f);
}

static unsigned int LoadTexture(const char* path)
{
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);
    if (!data)
    {
        fprintf(stderr, "Failed to load texture %s\n", path);
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SetAniso();

    stbi_image_free(data);
    return texture;
}

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
    SetAniso();

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
    SetAniso();

    stbi_image_free(grassData);

    surfaceNormalTexture = LoadTexture("assets/sand_normal.png");
    grassNormalTexture = LoadTexture("assets/grass_normal.png");


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
    glDeleteTextures(1, &grassTexture);
    glDeleteTextures(1, &surfaceNormalTexture);
    glDeleteTextures(1, &grassNormalTexture);
}

void WorldMesh::Draw(Shader& shader, const glm::mat4 & view, glm::mat4 & projection, const glm::vec3& cameraPosition, const glm::vec3& sunDirection, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap)
{

    glm::mat4 viewProjection = projection * view;
    std::vector<Plane> frustrumPlanes = GetFrustumPlanes(viewProjection);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glBindVertexArray(globalVAO);

    glm::mat4 model = glm::mat4(1.0f);
    shader.SetMat4("model", model);
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetVec3("sunDirection", sunDirection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    shader.SetInt("heightmap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, surfaceTexture);
    shader.SetInt("sandTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    shader.SetInt("grassTexture", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, surfaceNormalTexture);
    shader.SetInt("sandNormal", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grassNormalTexture);
    shader.SetInt("grassNormal", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    shader.SetInt("shadowMap", 5);

    shader.SetFloat("textureTileSize", 8.0f);
    shader.SetVec2("textureSize", glm::vec2((float)width, (float)height));
    shader.SetVec3("terrainParams", glm::vec3(yScale, yShift, skirtDepth));
    shader.SetVec3("cameraPos", cameraPosition);
    shader.SetVec3("sunColor", glm::vec3(1.0f, 0.97f, 0.9f));
    shader.SetFloat("metallic", metallic);
    shader.SetFloat("roughness", roughness);
    shader.SetVec3("headlightPos", headlightPos);
    shader.SetVec3("headlightColor", headlightColor);
    shader.SetFloat("water_level", config::WATERLEVEL);


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

        shader.SetVec2("chunkOffset", glm::vec2((float)chunks[i].x, (float)chunks[i].z));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, globalindexCount[lod], GL_UNSIGNED_INT, (void*)0 );
    }

    glDisable(GL_PRIMITIVE_RESTART);
    glBindVertexArray(0);
}

void WorldMesh::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix, const glm::vec3& cameraPosition)
{
    std::vector<Plane> lightPlanes = GetFrustumPlanes(lightSpaceMatrix);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);
    glBindVertexArray(globalVAO);

    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetVec2("textureSize", glm::vec2((float)width, (float)height));
    shader.SetVec3("terrainParams", glm::vec3(yScale, yShift, skirtDepth));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    shader.SetInt("heightmap", 0);


    for (size_t i = 0; i < chunks.size(); i++)
    {
        if (IsBoxInFrustrum(chunks[i].minBoundBox, chunks[i].maxBoundBox, lightPlanes) == false)
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

        shader.SetVec2("chunkOffset", glm::vec2((float)chunks[i].x, (float)chunks[i].z));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, globalindexCount[lod], GL_UNSIGNED_INT, (void*)0);
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

bool IsBoxInFrustrum(const glm::vec3 & min, const glm::vec3 max, const std::vector<Plane>& planes)
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