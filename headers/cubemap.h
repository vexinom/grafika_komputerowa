#pragma once

#include <glm/glm.hpp>
#include "shader.h"

class Cubemap
{
    public:
        void Init();
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    private:
        unsigned int VAO, VBO;
        unsigned int textureID;
};
