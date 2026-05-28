#include "watermesh.h"

void WaterMesh::Init(int worldWidth, int worldHeight, float waterLevel, float maxWaveHeight)
{
    this->waterLevel = waterLevel;
    this->maxWaveHeight = maxWaveHeight;
    waterChunks.clear();

    //Creation of chunks

    const int CHUNK_SIZE = 64;

    for(int ch_z = 0; ch_z < worldHeight - 1; ch_z += CHUNK_SIZE)
    {
        for(int ch_x = 0; ch_x < worldWidth - 1; ch_x += CHUNK_SIZE)
        {
            WaterChunk chunk;
            chunk.x = ch_x;
            chunk.z = ch_z;

            chunk.minBoundBox = glm::vec3(ch_x, waterLevel - maxWaveHeight - 2.0f, ch_z);
            chunk.maxBoundBox = glm::vec3(ch_x + CHUNK_SIZE, waterLevel + maxWaveHeight, ch_z + CHUNK_SIZE);

            waterChunks.push_back(chunk);
        }
    }
    

}

void WaterMesh::Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int waterShaderID, const WorldMesh & worldMesh, float time) 
{
    std::vector<Plane> frustrumPlanes = GetFrustumPlanes(viewProjection);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glBindVertexArray(worldMesh.globalVAO);

    glUniform1f(glGetUniformLocation(waterShaderID, "waterLevel"), waterLevel);
    glUniform3f(glGetUniformLocation(waterShaderID, "cameraPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glUniform1f(glGetUniformLocation(waterShaderID, "time"), time);

    for(size_t i = 0; i < waterChunks.size(); i++)
    {
        if(IsBoxInFrustrum(waterChunks[i].minBoundBox, waterChunks[i].maxBoundBox, frustrumPlanes) == false)
        {
            continue;
        }

        glm::vec3 chunkCenter = (waterChunks[i].minBoundBox + waterChunks[i].maxBoundBox) * 0.5f;
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

        glUniform2f(glGetUniformLocation(waterShaderID, "chunkOffset"), (float)waterChunks[i].x, (float)waterChunks[i].z);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, worldMesh.globalEBO[lod]);
        glDrawElements(GL_TRIANGLE_STRIP, worldMesh.globalindexCount[lod], GL_UNSIGNED_INT, (void*)0 );
    }

    glDisable(GL_PRIMITIVE_RESTART);
    glBindVertexArray(0);    
}