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
    int nChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned short *rawData = stbi_load_16("assets/worldmap.png", &width, &height, &nChannels, 1);
    int channels = 1;

    if (!rawData) {
        fprintf(stderr, "Failed to load terrain texture heightmap map!\n");
        return;
    }

    std::vector<float> dataArray(width * height);
    for (int i = 0; i < width * height; i++) {
        dataArray[i] = static_cast<float>(rawData[i]) / 65535.0f;
    }
    stbi_image_free(rawData);

    float* data = dataArray.data();

    std::vector<float> smoothedData(width * height);
    int blurRadius = 1;

    for(int z = 0; z < height; z++)
    {
        for(int x = 0; x < width; x++)
        {
            float sum = 0.0f;
            int count = 0;

            for(int bz = -blurRadius; bz <= blurRadius; bz ++)
            {
                for(int bx = -blurRadius; bx <= blurRadius; bx++)
                {
                    int nx = std::max(0, std::min(x + bx, width -1 ));
                    int nz = std::max(0, std::min(z + bz, height - 1));

                    sum += data[nx + nz * width];
                    count++;
                }
            }
            smoothedData[x + z * width] = sum / count;
        }
    }

    for(int i = 0; i < width * height; i++)
    {
        data[i] = smoothedData[i];
    }

    NUM_STRIPS = height - 1;
    NUM_VERTS_PER_STRIP = width * 2;

    const int CHUNK_SIZE = 64;

    float yScale = 146.0f, yShift = 16.0f;

    glGenBuffers(3, globalEBO);
    int lodStrides[3] = {1, 2, 4};

    unsigned int restartIndex = 0xFFFFFFFF;
    int vertexWidth = CHUNK_SIZE + 1;
    int vertexHeigth = CHUNK_SIZE + 1;

    int baseVertexCount = vertexWidth * vertexHeigth;
    int skirtOffset = baseVertexCount;
    int topSkirtIndex = skirtOffset;
    skirtOffset += vertexWidth;

    int bottomSkirtIndex = skirtOffset;
    skirtOffset += vertexWidth;

    int leftSkirtIndex = skirtOffset;
    skirtOffset += vertexHeigth;

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    chunks.clear();

    for(int ch_z = 0; ch_z < height - 1; ch_z += CHUNK_SIZE)
    {
        for(int ch_x = 0; ch_x < width - 1; ch_x += CHUNK_SIZE)
        {
            Chunk chunk;
            chunk.x = ch_x / CHUNK_SIZE;
            chunk.x = ch_z / CHUNK_SIZE;

            std::vector <float> chunk_vertices;

            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();

            float skirtDepth = 25.0f;

            for(int z = 0; z <= CHUNK_SIZE; z++)
            {
                for(int x = 0; x <= CHUNK_SIZE; x++)
                {
                    int globalX = ch_x + x;
                    int globalZ = ch_z + z;

                    int clampedX = std::min(globalX, width - 1);
                    int clampedZ = std::min(globalZ, width - 1);

                    int index = (clampedX + width * clampedZ) * channels;
                    float y = data[index];

                    float posX = (float)globalX;
                    float posY = (y * yScale - yShift) - skirtDepth;
                    float posZ = (float)globalZ;

                    minY = std::min(minY, posY);
                    maxY = std::max(maxY, posY);

                    glm::vec3 normal = GetVertexNormal(clampedX, clampedZ, data, width, height, channels, yScale);

                    chunk_vertices.push_back(posX);
                    chunk_vertices.push_back(posY);
                    chunk_vertices.push_back(posZ);

                    chunk_vertices.push_back(normal.x);
                    chunk_vertices.push_back(normal.y);
                    chunk_vertices.push_back(normal.z);

                }
            }

            std::function<void(int, int)> pushSkirtVertex = [&](int x, int z)
            {
                int globalX = ch_x + x;
                int globalZ = ch_z + z;

                int clampedX = std::min(globalX, width - 1);
                int clampedZ = std::min(globalZ, width - 1);

                int index = (clampedX + width * clampedZ) * channels;
                float y = data[index];

                float posX = (float)globalX;
                float posY = (y * yScale - yShift) - skirtDepth;
                float posZ = (float)globalZ;

                minY = std::min(minY, posY);
                maxY = std::max(maxY, posY);

                glm::vec3 normal = GetVertexNormal(clampedX, clampedZ, data, width, height, channels, yScale);

                chunk_vertices.push_back(posX);
                chunk_vertices.push_back(posY);
                chunk_vertices.push_back(posZ);

                chunk_vertices.push_back(normal.x);
                chunk_vertices.push_back(normal.y);
                chunk_vertices.push_back(normal.z);
            };

            for(int x = 0; x <= CHUNK_SIZE; x++)
            {
                pushSkirtVertex(x, 0);
            }

            for(int x = 0; x <= CHUNK_SIZE; x++)
            {
                pushSkirtVertex(x, CHUNK_SIZE);
            }

            for(int z = 0; z <= CHUNK_SIZE; z++)
            {
                pushSkirtVertex(z, 0);
            }

            for(int z = 0; z <= CHUNK_SIZE; z++)
            {
                pushSkirtVertex(z, CHUNK_SIZE);
            }

            chunk.minBoundBox = glm::vec3(ch_x, minY - skirtDepth, ch_z);
            chunk.maxBoundBox = glm::vec3(ch_x + CHUNK_SIZE, maxY, ch_z + CHUNK_SIZE);

            glGenVertexArrays(1, &chunk.VAO);
            glBindVertexArray(chunk.VAO);

            glGenBuffers(1, &chunk.VBO);
            glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
            glBufferData(GL_ARRAY_BUFFER, chunk_vertices.size() * sizeof(float), chunk_vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            chunks.push_back(chunk);
        }
    }       
    
}

void WorldMesh::Draw(glm::mat4 & viewProjection, glm::vec3 & cameraPosition)
{
    std::vector<Plane> frustrumPlanes = GetFrustumPlanes(viewProjection);
    int chunksDrawn = 0;

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

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

        glBindVertexArray(chunks[i].VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globalEBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, globalindexCount[lod], GL_UNSIGNED_INT, (void*)0 );
        chunksDrawn++;
    }

    glDisable(GL_PRIMITIVE_RESTART);
    glBindVertexArray(0);
}

WorldMesh::~WorldMesh()
{
    glDeleteBuffers(3, globalEBO);
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