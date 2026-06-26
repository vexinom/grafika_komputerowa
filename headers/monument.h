#pragma once

#include <glm/glm.hpp>
#include "shader.h"

class Monument
{
    public:
        void Init(float worldX, float worldZ, float baseY, float topY);
        void Draw(Shader& shader,
                  const glm::mat4& view,
                  const glm::mat4& projection,
                  const glm::vec3& sunDirection,
                  const glm::vec3& cameraPos,
                  const glm::mat4& lightSpaceMatrix,
                  unsigned int shadowMap);
        void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix);

    private:
        unsigned int VAO, VBO;
        int vertexCount;
};
