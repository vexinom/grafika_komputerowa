#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

// Loads the external "HDRI Underwater Ocean Stylized" OBJ meshes (B09 - OBJ model
// loading) and scatters many instances across the seabed as boulders / coral heads.
// Placement reads the same heightmap the terrain uses, so props sit on the floor.
class Reef
{
    public:
        void Init();
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& sunDirection, const glm::vec3& cameraPos,
                  const glm::mat4& lightSpaceMatrix, unsigned int shadowMap,
                  const glm::vec3& headlightPos, const glm::vec3& headlightColor);
        void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix);

    private:
        struct Mesh { unsigned int VAO = 0, VBO = 0, EBO = 0; int indexCount = 0;
                      unsigned int albedoTex = 0, normalTex = 0, ormTex = 0; };
        struct Instance { glm::mat4 model; glm::vec3 pos; glm::vec3 tint; float rough; float metal; int mesh; };

        std::vector<Mesh> meshes;
        std::vector<Instance> instances;
};
