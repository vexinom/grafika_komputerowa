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

        void Init(float waterLevel, float maxWaveHeight);
        void Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int waterShaderID, float time);

    private:
        unsigned int VAO,VBO, EBO;
        int indexCount;

        float waterLevel;
        float maxWaveHeight;

};
