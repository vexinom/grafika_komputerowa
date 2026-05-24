#include "worldmesh.h"
#include <cmath>
#include <glad/glad.h>
#include <algorithm>

//must be here for #include "stb_image.h" to work, without it linker throw errors
#define STB_IMAGE_IMPLEMENTATION 

#include "stb_image.h"

void WorldMesh::Init()
{
    int nChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load("assets/worldmap.png", &width, &height, &nChannels, 0);

    if (!data) {
        fprintf(stderr, "Failed to load terrain texture heightmap map!\n");
        return;
    }

    NUM_STRIPS = height - 1;
    NUM_VERTS_PER_STRIP = width * 2;

    const int CHUNK_SIZE = 64;

    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    unsigned bytePerPixel = nChannels;

    chunks.clear();

    for(int ch_z = 0; ch_z < height - 1; ch_z += CHUNK_SIZE)
    {
        for(int ch_x = 0; ch_x < width - 1; ch_x += CHUNK_SIZE)
        {

            Chunk chunk;
            chunk.x = ch_x / CHUNK_SIZE;
            chunk.z = ch_z / CHUNK_SIZE;

            std::vector<float> chunk_vertices;
            std::vector<unsigned int> chunk_indices;

            int curr_chunk_width = std::min(CHUNK_SIZE, width - 1 - ch_x);
            int curr_chunk_height = std::min(CHUNK_SIZE, height - 1 - ch_z);


            for(int z = 0; z <= curr_chunk_height; z++)
            {
                for(int x = 0; x <= curr_chunk_width; x++)
                {
                    int globalX = ch_x + x;
                    int globalZ = ch_z + z;

                    unsigned char* pixelOffset = data + (globalX + width * globalZ) * bytePerPixel;
                    unsigned char y = pixelOffset[0];

                    float posX = (float)globalX;
                    float posY = (float)y * yScale - yShift;
                    float posZ = (float)globalZ;

                    chunk_vertices.push_back(posX);
                    chunk_vertices.push_back(posY);
                    chunk_vertices.push_back(posZ);

                }
            }

            chunk.minBoundBox = glm::vec3(ch_x, -yShift, ch_z);
            chunk.maxBoundBox = glm::vec3(ch_x + curr_chunk_width, yShift * 4, ch_z + curr_chunk_height);

            glGenVertexArrays(1, &chunk.VAO);
            glBindVertexArray(chunk.VAO);

            glGenBuffers(1, &chunk.VBO);
            glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
            glBufferData(GL_ARRAY_BUFFER, chunk_vertices.size() * sizeof(float), chunk_vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glGenBuffers(3, chunk.EBO);

            int lodStrides[3] = {1, 2, 4};
            unsigned int restartIndex = 0xFFFFFFFF;
            int vertexWidth = curr_chunk_width + 1;

            for(int lod = 0; lod < 3; lod++)
            {
                std::vector<unsigned int> lod_indices;
                int stride = lodStrides[lod];
                
                for(int z = 0; z < curr_chunk_height; z += stride)       
                {
                    for(int x = 0; x <= curr_chunk_width; x += stride)      
                    {
                        lod_indices.push_back(x + vertexWidth * z);
                        lod_indices.push_back(x + vertexWidth * std::min(z + stride, curr_chunk_height)); 
                    }
                    lod_indices.push_back(restartIndex);
                }
                chunk.indexCount[lod] = static_cast<int>(lod_indices.size()); 

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.EBO[lod]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, lod_indices.size() * sizeof(unsigned int), lod_indices.data(), GL_STATIC_DRAW);
            }


            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0); 
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
            chunks.push_back(chunk);
        }
    }
    


    stbi_image_free(data);


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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunks[i].EBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, chunks[i].indexCount[lod], GL_UNSIGNED_INT, (void*)0 );
        chunksDrawn++;
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