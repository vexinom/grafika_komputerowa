#pragma once

#include "shader.h"

class Skydome
{
private:
    float yOffset;
    unsigned VAO, VBO, EBO;
    unsigned indexCount;
    unsigned int textureID;
    unsigned int shaderProgram;

public:
    void Init();
    void Draw(Shader& shader, const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const glm::vec3& sunDirection, float time);
};

