#pragma once

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
    void Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int skydomeShaderID, float time);
};

