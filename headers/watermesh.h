#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "worldmesh.h"


class WaterMesh
{
    public:
        void Init(float waterLevel, float maxWaveHeight);
        void Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int waterShaderID, float time);

    private:
        unsigned int VAO,VBO, EBO;
        int indexCount;

        float waterLevel;
        float maxWaveHeight;

};