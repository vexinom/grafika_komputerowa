#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

class IslandPalms
{
public:
    void Init();

    void Draw(Shader& shader,
              const glm::mat4& view,
              const glm::mat4& projection,
              const glm::vec3& sunDirection,
              const glm::vec3& cameraPos);

    void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix);

private:
    struct Mesh
    {
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        int indexCount = 0;
        unsigned int albedoTex = 0;
    };

    struct Instance
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 pos = glm::vec3(0.0f);
    };

    std::vector<Mesh> meshes;
    std::vector<Instance> instances;
};
