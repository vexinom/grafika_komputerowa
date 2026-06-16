#pragma once

#include <glm/glm.hpp>
#include "shader.h"

class Tube
{
    public:
        void Init();
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos);

    private:
        unsigned int VAO, VBO, EBO;
        int indexCount;
};
