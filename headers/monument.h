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

        // World-space center and height of the lighthouse, used to anchor the
        // shadow-map frustum on the lighthouse instead of on the camera.
        glm::vec3 Center() const { return worldCenter; }
        float Height() const { return worldHeight; }

    private:
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        int vertexCount = 0;
        unsigned int albedoTex = 0;
        bool useAlbedoTex = false;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec3 worldCenter = glm::vec3(0.0f);
        float worldHeight = 178.0f;
};
