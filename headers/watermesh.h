#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "worldmesh.h"


class WaterChunk
{
    public:
    int x, z;

    glm::vec3 minBoundBox;
    glm::vec3 maxBoundBox;

};

class WaterMesh
{
    public:
        void Init(int worldWidth, int worldHeight, float waterLevel, float maxWaveHeight = 5.0f);
        void Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int waterShaderID, const WorldMesh & worldMesh, float time);

    private:
        std::vector <WaterChunk> waterChunks;
        float waterLevel;
        float maxWaveHeight;

};