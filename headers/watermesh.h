#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "worldmesh.h"



class WaterChunk
{ 
public:
    glm::vec3 minBoundBox;
    glm::vec3 maxBoundBox;
    float offsetX;
    float offsetZ;

};


class WaterMesh
{
    public:
        std::vector<WaterChunk> chunks;

        void Init();
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, 
                    const glm::vec3& sunDirection, unsigned int heightmapTexture, float time,
                    GLuint reflectionTexture, GLuint refractionTexture,
                    float terrainWidth, float terrainHeight, float yScale, float yShift);



        unsigned int VAO,VBO, EBO;
        int indexCount;

        float waterLevel;
        float maxWaveHeight;

};
