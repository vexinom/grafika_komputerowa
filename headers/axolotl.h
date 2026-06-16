#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

class AxolotlInstance
{
    public:
        std::vector<glm::vec3> pathPos;
        std::vector<glm::vec3> pathTan;
        std::vector<float> cumLen;
        float totalLen;
        float dist;
        float scale;
        glm::vec3 currentPos;
        glm::vec3 forward;
        glm::mat4 model;
};

class Axolotl
{
    public:
        void Init();
        void Update(float dt);
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap);
        void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix);
        glm::vec3 NearestTo(const glm::vec3& point) const;
        glm::vec3 HeadlightPosition() const;

    private:
        unsigned int VAO, VBO, EBO;
        unsigned int baseColorTex, opacityTex;
        int indexCount;

        glm::vec3 bboxCenter;
        float baseScale;
        float bodyMinY;
        float bodyLenY;
        float animTime;

        std::vector<AxolotlInstance> instances;
};
