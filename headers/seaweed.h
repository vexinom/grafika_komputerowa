#pragma once

#include <glm/glm.hpp>
#include "shader.h"

// Procedural kelp / seaweed: tall blades baked into one mesh and animated in the
// vertex shader (B05 - vegetation waving). Blades grow on the shallow sandy shelf.
class Seaweed
{
    public:
        void Init();
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& sunDirection, const glm::vec3& cameraPos,
                  float time, const glm::vec3& current);

    private:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        int indexCount = 0;
};
