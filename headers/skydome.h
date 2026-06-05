#pragma once

class Sun
{   public:
        glm::vec3 direction = glm::normalize(glm::vec3(0.5f, 0.4f, -1.0f));
        glm::vec3 color = glm::vec3(1.0f, 0.95f, 0.8f);

};

class Skydome
{
private:
    float yOffset;
    unsigned VAO, VBO, EBO;
    unsigned indexCount;
    unsigned int textureID;
    unsigned int shaderProgram;

public:
    Sun sun;
    void Init();
    void Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int skydomeShaderID, float time);
};

